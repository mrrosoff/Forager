#include "events.h"

#include <Preferences.h>

#include <algorithm>
#include <cstring>

#include "foraging.h"
#include "journal.h"

namespace events {

static const char* NVS_NS = "forager";
#include "events_data.h"


// Don't roll for a new event more than once every ~6h of wall-clock time,
// and even then only a 1-in-3 chance -- interactions should feel occasional,
// not constant.
static const uint32_t EVENT_COOLDOWN_HOURS = 6;
static const int EVENT_CHANCE_DENOM = 3;

/**
 * Engagement: a small persisted streak counter that goes up while the
 * device is being woken repeatedly in quick succession (an active session)
 * and drops back to 0 the moment a wake follows a longer idle gap -- see
 * bumpEngagement() below. checkForEvent() shortens the cooldown and shrinks
 * the roll denominator by the current streak, so a session of back-to-back
 * wakes sees noticeably more events, while a single occasional check-in (or
 * the first wake after the device has been sitting untouched) sees the
 * same occasional-feeling baseline as before.
 */
static const uint32_t ENGAGEMENT_WINDOW_MINUTES = 10;
static const uint8_t ENGAGEMENT_MAX = 3;

static uint8_t bumpEngagement(Preferences& p, time_t now) {
  uint64_t lastWake = p.getULong64("lastWake", 0);
  uint8_t engagement = p.getUChar("engage", 0);
  if (lastWake != 0 && (uint64_t)now - lastWake <= (uint64_t)ENGAGEMENT_WINDOW_MINUTES * 60ULL) {
    engagement = (uint8_t)std::min((int)engagement + 1, (int)ENGAGEMENT_MAX);
  } else {
    engagement = 0;
  }
  p.putULong64("lastWake", (uint64_t)now);
  p.putUChar("engage", engagement);
  return engagement;
}

template <typename T>
static uint8_t weightedPick(const T* items, int count) {
  int total = 0;
  for (int i = 0; i < count; i++) total += items[i].weight;
  int r = random(total);
  for (int i = 0; i < count; i++) {
    if (r < items[i].weight) return (uint8_t)i;
    r -= items[i].weight;
  }
  return 0;
}

// Rejection-samples a not-yet-discovered species index. A handful of misses
// near full completion (250/250) is fine -- the caller only calls this when
// journal::totalDiscovered() < foraging::speciesCount(), so an undiscovered
// one always exists; this just avoids a full linear scan on every roll.
static uint8_t pickUndiscoveredSpecies() {
  int count = foraging::speciesCount();
  for (int tries = 0; tries < 40; tries++) {
    int idx = random(count);
    if (!journal::isDiscovered(idx)) return (uint8_t)idx;
  }
  for (int idx = 0; idx < count; idx++) {
    if (!journal::isDiscovered(idx)) return (uint8_t)idx;
  }
  return 0;
}

// The actual type/dataId roll, shared by checkForEvent()'s probabilistic
// once-per-wake spawn and spawnNow()'s deterministic mid-session trigger --
// both just need "give me an event appropriate for this stage".
// Baby's non-Discovery fallback, used once every species is discovered
// (250/250) -- Baby excludes ForagingFind/TrailMishap/TrailTreasure (none
// fit a baby-at-home narrative), so BabyCare/AnimalSighting/WeatherEvent/
// MarmotEncounter share the whole 100%.
static void pickBabyFallback(PendingEvent& ev, int roll) {
  if (roll < 40) {
    ev.type = EventType::BabyCare;
    ev.dataId = weightedPick(kBabyCare, kBabyCareCount);
  } else if (roll < 65) {
    ev.type = EventType::AnimalSighting;
    ev.dataId = weightedPick(kAnimals, kAnimalCount);
  } else if (roll < 85) {
    ev.type = EventType::WeatherEvent;
    ev.dataId = weightedPick(kWeatherEvents, kWeatherCount);
  } else {
    ev.type = EventType::MarmotEncounter;
    ev.dataId = weightedPick(kEncounters, kEncounterCount);
  }
}

// Juvenile/Adult's non-Discovery mix -- `share` is the total percentage
// (out of 100) this mix should consume; `roll` is 0..share-1. Used as the
// whole 100% once every species is discovered, or as Adult's remaining 40%
// while species are still undiscovered (Discovery takes the other 60%).
static void pickAdultMix(PendingEvent& ev, int roll, int share) {
  if (roll < share * 20 / 100) {
    ev.type = EventType::AnimalSighting;
    ev.dataId = weightedPick(kAnimals, kAnimalCount);
  } else if (roll < share * 40 / 100) {
    ev.type = EventType::ForagingFind;
    ev.dataId = (uint8_t)random(foraging::speciesCount());
    // A minority of finds pin to one exact species instead of any
    // species of its kind -- rarer and harder, for variety.
    ev.exact = random(100) < 30;
  } else if (roll < share * 60 / 100) {
    ev.type = EventType::TrailMishap;
    ev.dataId = weightedPick(kMishaps, kMishapCount);
  } else if (roll < share * 78 / 100) {
    ev.type = EventType::WeatherEvent;
    ev.dataId = weightedPick(kWeatherEvents, kWeatherCount);
  } else if (roll < share * 90 / 100) {
    ev.type = EventType::TrailTreasure;
    ev.dataId = weightedPick(kTreasures, kTreasureCount);
  } else {
    ev.type = EventType::MarmotEncounter;
    ev.dataId = weightedPick(kEncounters, kEncounterCount);
  }
}

static PendingEvent pickEvent(Stage stage) {
  PendingEvent ev;
  bool hasUndiscovered = journal::totalDiscovered() < foraging::speciesCount();
  int roll = random(100);
  if (stage == Stage::Baby) {
    // Always Discovery while any species remains unfound -- a Baby's whole
    // job is filling in the Foraging list. Falls back to baby-care/social
    // flavor once all 250 are discovered.
    if (hasUndiscovered) {
      ev.type = EventType::Discovery;
      ev.dataId = pickUndiscoveredSpecies();
    } else {
      pickBabyFallback(ev, roll);
    }
  } else if (stage == Stage::Juvenile) {
    // Same "always Discovery first" rule as Baby -- Juvenile falls back to
    // the full Adult-style mix (ForagingFind etc. all reachable) once
    // every species is discovered, rather than the Baby-only flavor pool.
    if (hasUndiscovered) {
      ev.type = EventType::Discovery;
      ev.dataId = pickUndiscoveredSpecies();
    } else {
      pickAdultMix(ev, roll, 100);
    }
  } else {
    // Adult: Discovery drops to 60% (still the single biggest category,
    // but no longer guaranteed) so the full event mix -- ForagingFind,
    // TrailMishap, TrailTreasure, etc. -- gets real airtime once grown up.
    // Once every species is discovered, the remaining categories take the
    // full 100% in their usual proportions.
    if (hasUndiscovered && roll < 60) {
      ev.type = EventType::Discovery;
      ev.dataId = pickUndiscoveredSpecies();
    } else if (hasUndiscovered) {
      pickAdultMix(ev, roll - 60, 40);
    } else {
      pickAdultMix(ev, roll, 100);
    }
  }
  return ev;
}

PendingEvent checkForEvent(time_t now, int month, Stage stage) {
  (void)month;  // reserved for future seasonal weighting
  Preferences p;
  p.begin(NVS_NS, /*readOnly=*/false);

  PendingEvent ev;
  ev.type = (EventType)p.getUChar("evType", (uint8_t)EventType::None);
  ev.dataId = p.getUChar("evData", 0);
  ev.exact = p.getUChar("evExact", 0) != 0;

  uint8_t engagement = bumpEngagement(p, now);

  if (ev.type == EventType::None) {
    uint64_t lastAt = p.getULong64("evLastAt", 0);
    uint32_t cooldownHours =
        (uint32_t)std::max(1, (int)EVENT_COOLDOWN_HOURS - (int)engagement);
    bool cooldownOver = lastAt == 0 || (uint64_t)now - lastAt >= (uint64_t)cooldownHours * 3600ULL;
    int chanceDenom = std::max(1, EVENT_CHANCE_DENOM - (int)engagement);
    if (cooldownOver && random(chanceDenom) == 0) {
      ev = pickEvent(stage);
      p.putUChar("evType", (uint8_t)ev.type);
      p.putUChar("evData", ev.dataId);
      p.putUChar("evExact", ev.exact ? 1 : 0);
    }
  }

  p.end();
  return ev;
}

// Deterministic mid-session trigger (see main.cpp's screen-change-count
// hook) -- unlike checkForEvent(), this ignores the hourly cooldown/roll
// entirely and always produces an event; callers are expected to gate it
// with their own condition (e.g. recentlyResolved()) instead. Persists to
// the same NVS keys checkForEvent() reads, so it behaves exactly like a
// normal spawned event from here on (survives sleep, blocks a second
// spawn until resolved).
PendingEvent spawnNow(Stage stage) {
  PendingEvent ev = pickEvent(stage);
  Preferences p;
  p.begin(NVS_NS, /*readOnly=*/false);
  p.putUChar("evType", (uint8_t)ev.type);
  p.putUChar("evData", ev.dataId);
  p.putUChar("evExact", ev.exact ? 1 : 0);
  p.end();
  return ev;
}

bool recentlyResolved(time_t now, uint32_t withinSeconds) {
  Preferences p;
  p.begin(NVS_NS, /*readOnly=*/true);
  uint64_t lastAt = p.getULong64("evLastAt", 0);
  p.end();
  if (lastAt == 0) return false;
  return (uint64_t)now - lastAt < (uint64_t)withinSeconds;
}

const char* eventTitle(EventType type, bool negative) {
  switch (type) {
    case EventType::AnimalSighting:
      return negative ? "!! SIGHTING !!" : "SIGHTING";
    case EventType::ForagingFind:
      return "FORAGING FIND!";
    case EventType::TrailMishap:
      return "TRAIL MISHAP";
    case EventType::WeatherEvent:
      return negative ? "ROUGH WEATHER" : "NICE WEATHER";
    case EventType::BabyCare:
      return "BABY MARMOT";
    case EventType::TrailTreasure:
      return "TRAIL TREASURE!";
    case EventType::MarmotEncounter:
      return negative ? "TERRITORIAL SPAT" : "MARMOT ENCOUNTER";
    case EventType::Discovery:
      return "DISCOVERY!";
    default:
      return "";
  }
}

// Capitalized display form of a Forageable::kind, e.g. "mushroom" -> "Mushroom".
static const char* capitalizedKind(const char* kind) {
  static char buf[24];
  size_t i = 0;
  for (; kind[i] && i < sizeof(buf) - 1; i++) buf[i] = kind[i];
  buf[i] = '\0';
  if (buf[0] >= 'a' && buf[0] <= 'z') buf[0] -= 32;
  return buf;
}

const char* eventName(const PendingEvent& ev) {
  switch (ev.type) {
    case EventType::AnimalSighting:
      return kAnimals[ev.dataId % kAnimalCount].name;
    case EventType::ForagingFind:
      return ev.exact ? foraging::speciesAt(ev.dataId).name
                      : capitalizedKind(foraging::speciesAt(ev.dataId).kind);
    case EventType::TrailMishap:
      return kMishaps[ev.dataId % kMishapCount].name;
    case EventType::WeatherEvent:
      return kWeatherEvents[ev.dataId % kWeatherCount].name;
    case EventType::BabyCare:
      return kBabyCare[ev.dataId % kBabyCareCount].name;
    case EventType::TrailTreasure:
      return kTreasures[ev.dataId % kTreasureCount].name;
    case EventType::MarmotEncounter:
      return kEncounters[ev.dataId % kEncounterCount].name;
    case EventType::Discovery:
      return foraging::speciesAt(ev.dataId).name;
    default:
      return "";
  }
}

const char* eventNote(const PendingEvent& ev) {
  switch (ev.type) {
    case EventType::AnimalSighting:
      return kAnimals[ev.dataId % kAnimalCount].note;
    case EventType::ForagingFind:
      return ev.exact ? "Find and eat this exact species to feed the marmot!"
                      : "Head to the Foraging list and eat one to feed the marmot!";
    case EventType::TrailMishap:
      return kMishaps[ev.dataId % kMishapCount].note;
    case EventType::WeatherEvent:
      return kWeatherEvents[ev.dataId % kWeatherCount].note;
    case EventType::BabyCare:
      return kBabyCare[ev.dataId % kBabyCareCount].note;
    case EventType::TrailTreasure:
      return kTreasures[ev.dataId % kTreasureCount].note;
    case EventType::MarmotEncounter:
      return kEncounters[ev.dataId % kEncounterCount].note;
    case EventType::Discovery:
      return foraging::speciesAt(ev.dataId).note;
    default:
      return "";
  }
}

const char* eventCategory(const PendingEvent& ev) {
  if (ev.type != EventType::ForagingFind || ev.exact) return "";
  return foraging::speciesAt(ev.dataId).kind;
}

bool eventMatchesSpecies(const PendingEvent& ev, const Forageable& f) {
  if (ev.type != EventType::ForagingFind) return false;
  if (ev.exact) return strcmp(f.name, foraging::speciesAt(ev.dataId).name) == 0;
  return strcmp(f.kind, foraging::speciesAt(ev.dataId).kind) == 0;
}

uint8_t animalIndex(const PendingEvent& ev) { return ev.dataId % kAnimalCount; }

bool eventIsNegative(const PendingEvent& ev) {
  switch (ev.type) {
    case EventType::AnimalSighting:
      return kAnimals[ev.dataId % kAnimalCount].predator;
    case EventType::TrailMishap:
      return true;
    case EventType::WeatherEvent:
      return !kWeatherEvents[ev.dataId % kWeatherCount].positive;
    case EventType::MarmotEncounter:
      return kEncounters[ev.dataId % kEncounterCount].rival;
    default:
      return false;  // a foraging find/trail treasure is always good news
  }
}

const char* eventEffectPreview(const PendingEvent& ev) {
  switch (ev.type) {
    case EventType::ForagingFind:
      return "Happiness +15, Hunger -20";
    case EventType::TrailMishap:
      return "Happiness -10, Hunger +15";
    default:
      return eventIsNegative(ev) ? "Happiness -15" : "Happiness +10";
  }
}

void resolve(const PendingEvent& ev, CreatureState& creature, time_t now) {
  creature.lastPlayed = now;  // any acknowledged event counts as play, good or bad

  switch (ev.type) {
    case EventType::ForagingFind: {
      int h = (int)creature.happiness + 15;
      creature.happiness = (uint8_t)(h > 100 ? 100 : h);
      creature.hunger = creature.hunger > 20 ? creature.hunger - 20 : 0;
      creature.lastFed = now;  // the only remaining way "Foraged X days ago" updates
      break;
    }
    case EventType::TrailMishap: {
      creature.happiness = creature.happiness > 10 ? creature.happiness - 10 : 0;
      int hu = (int)creature.hunger + 15;
      creature.hunger = (uint8_t)(hu > 100 ? 100 : hu);
      break;
    }
    default: {
      int delta = eventIsNegative(ev) ? -15 : 10;
      int h = (int)creature.happiness + delta;
      creature.happiness = (uint8_t)(h < 0 ? 0 : (h > 100 ? 100 : h));
      break;
    }
  }

  Preferences p;
  p.begin(NVS_NS, /*readOnly=*/false);
  p.putUChar("evType", (uint8_t)EventType::None);
  p.putULong64("evLastAt", (uint64_t)now);
  p.end();
}

int mishapCount() { return kMishapCount; }
int weatherCount() { return kWeatherCount; }
int babyCareCount() { return kBabyCareCount; }
int treasureCount() { return kTreasureCount; }
int encounterCount() { return kEncounterCount; }

}  // namespace events
