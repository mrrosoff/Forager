#include "creature.h"

#include <Preferences.h>

#include <algorithm>
#include <cstring>

#include "config.h"

namespace creature {

static const char* NVS_NS = "forager";

void load(CreatureState& s) {
  Preferences p;
  p.begin(NVS_NS, /*readOnly=*/true);
  s.hunger = p.getUChar("hunger", 30);
  s.happiness = p.getUChar("happy", 60);
  s.energy = p.getUChar("energy", 70);
  s.curiosity = p.getUChar("curio", 60);
  s.lastFed = (time_t)p.getULong64("lastFed", 0);
  s.lastPlayed = (time_t)p.getULong64("lastPlayed", 0);
  s.lastCurious = (time_t)p.getULong64("lastCur", 0);
  s.birthDate = (time_t)p.getULong64("birthDate", 0);
  s.feedStreakDays = p.getUShort("streak", 0);
  s.lastStreakDay = (time_t)p.getULong64("streakDay", 0);
  s.lastSeenStage = p.getUChar("lastStage", 0);
  strncpy(s.name, "Marmot", sizeof(s.name) - 1);  // default if never named yet
  s.name[sizeof(s.name) - 1] = '\0';
  p.getString("name", s.name, sizeof(s.name));
  s.named = p.getBool("named", false);
  p.end();
  s.mood = Mood::Content;  // recomputed by evaluate()
}

void save(const CreatureState& s) {
  Preferences p;
  p.begin(NVS_NS, /*readOnly=*/false);
  p.putUChar("hunger", s.hunger);
  p.putUChar("happy", s.happiness);
  p.putUChar("energy", s.energy);
  p.putUChar("curio", s.curiosity);
  p.putULong64("lastFed", (uint64_t)s.lastFed);
  p.putULong64("lastPlayed", (uint64_t)s.lastPlayed);
  p.putULong64("lastCur", (uint64_t)s.lastCurious);
  p.putULong64("birthDate", (uint64_t)s.birthDate);
  p.putUShort("streak", s.feedStreakDays);
  p.putULong64("streakDay", (uint64_t)s.lastStreakDay);
  p.putUChar("lastStage", s.lastSeenStage);
  p.putString("name", s.name);
  p.putBool("named", s.named);
  p.end();
}

/**
 * Multiplier on every decay period, by growth stage. A Baby can reach only
 * Snack Hunt and Marmot Says, and a Juvenile adds Forest Memory -- Burrow
 * Maze (energy) and the Species Quiz stay locked for a long while. Slowing
 * the clock early means the bars a young marmot has no tool for aren't
 * running down at adult speed while it waits.
 */
static double decayScale(Stage stage) {
  switch (stage) {
    case Stage::Baby:
      return 1.8;
    case Stage::Juvenile:
      return 1.35;
    default:
      return 1.0;
  }
}

// How long the 0..100 hunger ramp takes at this stage, in seconds.
static double hungerPeriodSec(double scale) {
  return (double)HUNGER_PERIOD_HOURS * 3600.0 * scale;
}

// Recompute hunger as a 0..100 ramp over HUNGER_PERIOD_HOURS since last fed.
static void agingHunger(CreatureState& s, time_t now, double scale) {
  if (s.lastFed == 0 || now <= s.lastFed) return;
  double frac = (double)(now - s.lastFed) / hungerPeriodSec(scale);
  frac = std::max(0.0, std::min(1.0, frac));
  s.hunger = (uint8_t)(frac * 100.0);

  // Happiness drifts toward neutral as hunger climbs; very hungry = unhappy.
  if (s.hunger > 70 && s.happiness > 25) s.happiness -= 1;
}

/**
 * Happiness decays toward a falling ceiling the longer it's been since the
 * marmot was last played with (fed, or had an event resolved) -- a clamp
 * rather than a recompute, so it never undoes a *recent* boost, but a
 * long-neglected marmot's happiness gets pulled down regardless of what it
 * was before. Independent of hunger: staying fed doesn't prevent boredom.
 */
static void agingBoredom(CreatureState& s, time_t now, double scale) {
  if (s.lastPlayed == 0 || now <= s.lastPlayed) return;
  double hrs = (double)(now - s.lastPlayed) / 3600.0;
  double frac = hrs / ((double)PLAY_PERIOD_HOURS * scale);
  frac = std::max(0.0, std::min(1.0, frac));
  uint8_t ceiling = (uint8_t)((1.0 - frac) * 100.0);
  if (ceiling < s.happiness) s.happiness = ceiling;
}

// Same ceiling-clamp shape as agingBoredom(), same lastPlayed trigger, but
// its own (longer) decay period -- energy and happiness both track neglect
// since the last interaction, just at different rates.
static void agingEnergy(CreatureState& s, time_t now, double scale) {
  if (s.lastPlayed == 0 || now <= s.lastPlayed) return;
  double hrs = (double)(now - s.lastPlayed) / 3600.0;
  double frac = hrs / ((double)ENERGY_PERIOD_HOURS * scale);
  frac = std::max(0.0, std::min(1.0, frac));
  uint8_t ceiling = (uint8_t)((1.0 - frac) * 100.0);
  if (ceiling < s.energy) s.energy = ceiling;
}

// Same ceiling-clamp shape as agingBoredom()/agingEnergy(), but on its own
// (slower) clock and driven by lastCurious rather than lastPlayed -- a marmot
// can be fed and played with and still have seen nothing new.
static void agingCuriosity(CreatureState& s, time_t now, double scale) {
  if (s.lastCurious == 0 || now <= s.lastCurious) return;
  double hrs = (double)(now - s.lastCurious) / 3600.0;
  double frac = hrs / ((double)CURIOSITY_PERIOD_HOURS * scale);
  frac = std::max(0.0, std::min(1.0, frac));
  uint8_t ceiling = (uint8_t)((1.0 - frac) * 100.0);
  if (ceiling < s.curiosity) s.curiosity = ceiling;
}

Mood evaluate(CreatureState& s, const struct tm& now, const WeatherData& weather, Stage stage) {
  time_t nowEpoch = mktime(const_cast<struct tm*>(&now));
  double scale = decayScale(stage);
  agingHunger(s, nowEpoch, scale);
  agingEnergy(s, nowEpoch, scale);
  agingBoredom(s, nowEpoch, scale);
  agingCuriosity(s, nowEpoch, scale);

  int month = now.tm_mon + 1;  // 1..12
  int hour = now.tm_hour;
  bool deepWinter = (month == 12 || month == 1 || month == 2);
  bool cold = weather.valid && weather.tempC <= 4.0f;
  bool night = (hour < 7) || (hour >= 21);
  bool goodForaging = weather.valid && weather.postRain;

  Mood m;
  if (deepWinter && cold && s.hunger < 70) {
    m = Mood::Dormant;  // hibernating through the cold dark
  } else if (s.hunger >= 70) {
    m = Mood::Hungry;
  } else if (s.happiness < 20) {
    m = Mood::Annoyed;  // neglected too long
  } else if (goodForaging || s.hunger < 20 || s.happiness >= 80) {
    m = Mood::Excited;  // fresh rain or just fed = let's go
  } else if (night) {
    m = Mood::Sleepy;
  } else {
    m = Mood::Content;
  }

  s.mood = m;
  return m;
}

// Whole-day index (not calendar-local, just a consistent 24h bucket) --
// good enough for a streak counter, no need for exact local-midnight math.
static int64_t dayIndex(time_t t) { return (int64_t)t / 86400; }

// Sweet/treat kinds lean toward happiness, protein/hearty kinds lean toward
// energy, and everything else (staple greens, fungi) gives a smaller amount
// of both -- every food still fills hunger the same regardless of kind (see
// feedForaged() below), this only varies the secondary boost.
FeedEffect feedEffectForKind(const char* kind) {
  static const char* const kTreatKinds[] = {"berry", "sap", "nut", "pine nut"};
  static const char* const kProteinKinds[] = {"shellfish", "crab",   "shrimp",
                                              "snail",     "chiton", "urchin"};
  for (const char* k : kTreatKinds) {
    if (strcmp(kind, k) == 0) return {20, 5};
  }
  for (const char* k : kProteinKinds) {
    if (strcmp(kind, k) == 0) return {5, 20};
  }
  return {8, 8};
}

void feedForaged(CreatureState& s, time_t now, bool inSeason, const char* kind) {
  // A real meal restarts the ramp outright -- lastFed below is what does it
  // (see shiftHunger()); this just keeps s.hunger consistent until then.
  s.hunger = 0;
  FeedEffect effect = feedEffectForKind(kind);
  int h = (int)s.happiness + (inSeason ? 15 : 10) + effect.happinessBoost;
  s.happiness = (uint8_t)(h > 100 ? 100 : h);
  int e = (int)s.energy + effect.energyBoost;
  s.energy = (uint8_t)(e > 100 ? 100 : e);
  s.lastFed = now;
  s.lastPlayed = now;  // eating counts as play -- resets the boredom/energy clocks too

  int64_t today = dayIndex(now);
  int64_t lastDay = dayIndex(s.lastStreakDay);
  if (s.lastStreakDay == 0 || today - lastDay == 1) {
    s.feedStreakDays++;
    s.lastStreakDay = now;
  } else if (today - lastDay > 1) {
    s.feedStreakDays = 1;
    s.lastStreakDay = now;
  }
  // today - lastDay == 0 (already fed today): streak unchanged, but still
  // bump lastStreakDay so later feeds today don't re-trigger the >1 branch.
  else {
    s.lastStreakDay = now;
  }
}

void shiftHunger(CreatureState& s, time_t now, int deltaPoints, Stage stage) {
  if (deltaPoints == 0) return;
  if (s.lastFed == 0) s.lastFed = now;  // ramp not running yet -- anchor it
  double scale = decayScale(stage);
  double periodSec = hungerPeriodSec(scale);
  s.lastFed -= (time_t)((double)deltaPoints / 100.0 * periodSec);  // hungrier == longer ago
  s.lastFed = std::min(s.lastFed, now);                            // no fuller than just-fed
  s.lastFed = std::max(s.lastFed, now - (time_t)periodSec);        // no hungrier than starved
  agingHunger(s, now, scale);  // keep s.hunger consistent before the next evaluate()
}

void rewardMinigame(CreatureState& s, time_t now, int score, Stat stat, Stage stage) {
  // Any finished run counts as play, which is what lifts the happiness and
  // energy ceilings -- so sitting down to a game is itself enough to hold
  // off both, whatever the score.
  s.lastPlayed = now;
  if (stat == Stat::Curiosity) s.lastCurious = now;
  if (score <= 0) return;

  // Scales with the run, capped below what a single feed gives: playing is
  // attention, not a substitute for food.
  int boost = 4 + score * 2;
  if (boost > 15) boost = 15;
  switch (stat) {
    case Stat::Hunger:
      // Gathering turns up something to eat, but less than a real meal (which
      // resets the ramp outright). Via shiftHunger(), since a write to
      // s.hunger wouldn't survive the caller's next evaluate().
      shiftHunger(s, now, -boost, stage);
      break;
    case Stat::Energy: {
      int e = (int)s.energy + boost;
      s.energy = (uint8_t)(e > 100 ? 100 : e);
      break;
    }
    case Stat::Curiosity: {
      int c = (int)s.curiosity + boost;
      s.curiosity = (uint8_t)(c > 100 ? 100 : c);
      break;
    }
    case Stat::Happiness:
    default: {
      int h = (int)s.happiness + boost;
      s.happiness = (uint8_t)(h > 100 ? 100 : h);
      break;
    }
  }
}

void rewardDiscovery(CreatureState& s, time_t now) {
  int c = (int)s.curiosity + 20;
  s.curiosity = (uint8_t)(c > 100 ? 100 : c);
  s.lastCurious = now;
}

/**
 * Growth stage from distinct species foraged (see BABY_STAGE_SPECIES /
 * ADULT_STAGE_SPECIES in config.h).
 */
Stage computeStage(int speciesEaten) {
#if DEV_MODE_SKIP_GROWTH
  return Stage::Adult;
#endif
  if (speciesEaten < BABY_STAGE_SPECIES) return Stage::Baby;
  if (speciesEaten < ADULT_STAGE_SPECIES) return Stage::Juvenile;
  return Stage::Adult;
}

DeathCause checkDeath(const CreatureState& s) {
  if (s.hunger >= DEATH_HUNGER_THRESHOLD) return DeathCause::Starved;
  if (s.happiness <= DEATH_HAPPINESS_THRESHOLD) return DeathCause::Heartbroken;
  if (s.energy <= DEATH_ENERGY_THRESHOLD) return DeathCause::Exhausted;
  return DeathCause::None;
}

const char* moodName(Mood m) {
  switch (m) {
    case Mood::Excited:
      return "excited";
    case Mood::Content:
      return "content";
    case Mood::Sleepy:
      return "sleepy";
    case Mood::Hungry:
      return "hungry";
    case Mood::Annoyed:
      return "annoyed";
    case Mood::Dormant:
      return "dormant";
    case Mood::Scared:
      return "scared";
  }
  return "content";
}

}  // namespace creature
