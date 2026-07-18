#include "events.h"

#include <Preferences.h>

#include "foraging.h"

namespace events {

static const char* NVS_NS = "forager";

struct AnimalDef {
  const char* name;
  const char* note;
  bool predator;
  uint8_t weight;  // relative spawn weight; predators are rare
};

// PNW wildlife the marmot might spot from its perch. Weighted so common,
// harmless animals show up far more often than rare predators.
static const AnimalDef kAnimals[] = {
    {"Deer", "A deer wanders through, nibbling at ferns.", false, 30},
    {"Mountain Goat", "A mountain goat picks its way across the rocky slope.", false, 15},
    {"Bald Eagle", "A bald eagle circles overhead, scanning the water.", false, 20},
    {"River Otter", "A river otter slips by, playful and quick.", false, 15},
    {"Raccoon", "A raccoon rummages through the underbrush at dusk.", false, 18},
    {"Elk", "A bull elk grazes in a nearby clearing.", false, 12},
    {"Black Bear", "A black bear ambles past, in no particular hurry.", true, 8},
    {"Orca", "A pod of orcas surfaces offshore, hunting together.", true, 4},
    {"Cougar", "A cougar's eyes catch the light from the treeline.", true, 3},
    {"Coyote", "A coyote trots along the treeline at dusk, unbothered.", false, 18},
    {"Douglas Squirrel", "A Douglas squirrel scolds loudly from a Douglas-fir branch.", false, 25},
    {"Snowshoe Hare", "A snowshoe hare freezes mid-hop, ears swiveling.", false, 20},
    {"Pileated Woodpecker", "A pileated woodpecker hammers a dead snag nearby.", false, 14},
    {"Steller's Jay", "A Steller's jay scolds from a low branch, crest raised.", false, 25},
    {"Banana Slug", "A banana slug inches across the wet trail, bright yellow.", false, 22},
    {"Pacific Tree Frog", "A Pacific tree frog chirps once from the ferns and goes quiet.", false,
     20},
    {"Beaver", "A beaver slaps its tail on the pond and disappears below.", false, 12},
    {"Great Blue Heron", "A great blue heron stands motionless in the shallows.", false, 16},
    {"Osprey", "An osprey hovers over the river, then plunges for a fish.", false, 12},
    {"Chestnut-backed Chickadee", "A chickadee flock works through the hemlock boughs.", false, 20},
    {"Harbor Seal", "A harbor seal's head bobs up in the cove, watching.", false, 14},
    {"Red Fox", "A red fox pauses on the trail, ears forward, before slipping away.", false, 10},
    {"Barred Owl", "A barred owl calls its who-cooks-for-you from the canopy at dusk.", true, 6},
    {"Gray Wolf", "A distant howl carries down the valley -- wolves, unmistakably.", true, 2},
    {"Bobcat", "A bobcat's tufted ears vanish into the salal before you're sure you saw it.", true,
     4},

};
static const int kAnimalCount = sizeof(kAnimals) / sizeof(kAnimals[0]);

struct MishapDef {
  const char* name;
  const char* note;
  uint8_t weight;
};

static const MishapDef kMishaps[] = {
    {"Blackberry Thorns", "Tangled in a blackberry thicket -- a few scratches.", 30},
    {"Lost the Trail", "Lost the trail in the fog for a while.", 20},
    {"Soaked Boots", "Stepped straight into a hidden puddle.", 25},
    {"Twisted Ankle", "Rolled an ankle on a loose root.", 15},
    {"Swarmed by Gnats", "A cloud of gnats followed the whole way back.", 10},
    {"Blister o'clock", "A hot spot on the heel turns into a full blister by the trailhead.", 20},
    {"Backpack Buckle Snapped", "A worn buckle finally gave out mid-hike.", 10},
    {"Dropped the Trowel", "The trowel slipped into a patch of ferns, never to be found.", 12},
    {"Slug on the Sandwich", "A banana slug beat everyone to lunch.", 15},
    {"Tick Check", "Found a tick riding along on a pant leg -- pulled it off in time.", 10},
    {"Devil's Club Encounter", "Brushed a stand of devil's club -- tiny spines for days.", 18},
    {"Stinging Nettle Sting", "Bare ankle met a patch of stinging nettle.", 22},
    {"Map Blew Away", "A gust snatched the paper map right out of hand.", 8},
    {"Bonked on a Low Branch", "Ducked a second too late for a low-hanging branch.", 14},
    {"Camera Battery Died", "The battery died right as the light got good.", 16},
    {"Sunburned Neck", "Forgot sunscreen on the back of the neck -- lesson learned.", 14},
    {"Mud to the Knees", "A shortcut through the bog wasn't much of a shortcut.", 18},
    {"Mosquito Cloud at the Pond", "Stopped by a still pond and paid for it in bites.", 20},
    {"Bear Bell Fell Off", "The bear bell rattled loose somewhere on the switchbacks.", 9},
    {"Overloaded Basket",
     "Packed in too heavy on the way out -- shoulders felt it on the way back.", 13},

};
static const int kMishapCount = sizeof(kMishaps) / sizeof(kMishaps[0]);

struct WeatherDef {
  const char* name;
  const char* note;
  bool positive;
  uint8_t weight;
};

static const WeatherDef kWeatherEvents[] = {
    {"Rainbow", "A rainbow arcs over the ridge after the rain.", true, 25},
    {"Golden Hour", "The evening light turns everything gold.", true, 20},
    {"Clear Skies", "Not a cloud in sight -- a perfect day out.", true, 20},
    {"Sudden Downpour", "A downpour rolled in with no warning.", false, 20},
    {"Cold Snap", "An unexpected cold snap settled in overnight.", false, 15},
    {"Morning Fog", "A thick fog softens every edge of the forest.", true, 22},
    {"First Frost", "Frost rimes every leaf and blade of grass.", true, 15},
    {"Marine Layer", "A cool marine layer rolls in off the Sound.", true, 18},
    {"Sun Break", "A rare sun break splits the clouds for an hour.", true, 22},
    {"Fresh Snowfall", "Fresh snow blankets the ridge overnight.", true, 12},
    {"King Tide", "An unusually high king tide floods the low trail.", false, 10},
    {"Windstorm", "A windstorm rattles branches loose all night.", false, 15},
    {"Heat Wave", "An unseasonable heat wave dries out the underbrush.", false, 12},
    {"Atmospheric River", "Days of steady rain -- an atmospheric river moved in.", false, 18},
    {"Hailstorm", "A brief hailstorm rattles off every leaf.", false, 12},
    {"Smoky Haze", "Wildfire smoke drifts in, hazing the whole valley.", false, 10},
    {"Double Rainbow", "A double rainbow spans the whole valley.", true, 8},
    {"Lightning Show", "A distant lightning show lights up the night sky.", false, 10},
};
static const int kWeatherCount = sizeof(kWeatherEvents) / sizeof(kWeatherEvents[0]);

// Don't roll for a new event more than once every ~6h of wall-clock time,
// and even then only a 1-in-3 chance -- interactions should feel occasional,
// not constant.
static const uint32_t EVENT_COOLDOWN_HOURS = 6;
static const int EVENT_CHANCE_DENOM = 3;

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

PendingEvent checkForEvent(time_t now, int month) {
  (void)month;  // reserved for future seasonal weighting
  Preferences p;
  p.begin(NVS_NS, /*readOnly=*/false);

  PendingEvent ev;
  ev.type = (EventType)p.getUChar("evType", (uint8_t)EventType::None);
  ev.dataId = p.getUChar("evData", 0);

  if (ev.type == EventType::None) {
    uint64_t lastAt = p.getULong64("evLastAt", 0);
    bool cooldownOver =
        lastAt == 0 || (uint64_t)now - lastAt >= (uint64_t)EVENT_COOLDOWN_HOURS * 3600ULL;
    if (cooldownOver && random(EVENT_CHANCE_DENOM) == 0) {
      // Category mix: sightings and foraging finds most common, mishaps and
      // weather events rarer.
      int roll = random(100);
      if (roll < 40) {
        ev.type = EventType::AnimalSighting;
        ev.dataId = weightedPick(kAnimals, kAnimalCount);
      } else if (roll < 70) {
        ev.type = EventType::ForagingFind;
        ev.dataId = (uint8_t)random(foraging::speciesCount());
      } else if (roll < 85) {
        ev.type = EventType::TrailMishap;
        ev.dataId = weightedPick(kMishaps, kMishapCount);
      } else {
        ev.type = EventType::WeatherEvent;
        ev.dataId = weightedPick(kWeatherEvents, kWeatherCount);
      }
      p.putUChar("evType", (uint8_t)ev.type);
      p.putUChar("evData", ev.dataId);
    }
  }

  p.end();
  return ev;
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
      return capitalizedKind(foraging::speciesAt(ev.dataId).kind);
    case EventType::TrailMishap:
      return kMishaps[ev.dataId % kMishapCount].name;
    case EventType::WeatherEvent:
      return kWeatherEvents[ev.dataId % kWeatherCount].name;
    default:
      return "";
  }
}

const char* eventNote(const PendingEvent& ev) {
  switch (ev.type) {
    case EventType::AnimalSighting:
      return kAnimals[ev.dataId % kAnimalCount].note;
    case EventType::ForagingFind:
      return "Head to the Foraging list and find one to feed the marmot!";
    case EventType::TrailMishap:
      return kMishaps[ev.dataId % kMishapCount].note;
    case EventType::WeatherEvent:
      return kWeatherEvents[ev.dataId % kWeatherCount].note;
    default:
      return "";
  }
}

const char* eventCategory(const PendingEvent& ev) {
  if (ev.type != EventType::ForagingFind) return "";
  return foraging::speciesAt(ev.dataId).kind;
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
    default:
      return false;  // a foraging find is always good news
  }
}

void resolve(const PendingEvent& ev, CreatureState& creature, time_t now) {
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

}  // namespace events
