// events_data.h — flavor-text pools for wake-time events (animal
// sightings, mishaps, weather, baby care, trail treasure, marmot
// encounters). Pure content, split out of events.cpp so that file is
// mostly logic. Only included from events.cpp.
#pragma once

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

struct BabyCareDef {
  const char* name;
  const char* note;
  uint8_t weight;
};

// Simple, wholesome, always-positive baby-stage moments -- no mishaps or
// foraging finds, since the marmot is too young for the trail and Foraging
// is still hidden.
static const BabyCareDef kBabyCare[] = {
    {"Nap Time", "The baby marmot curls up for an afternoon nap.", 20},
    {"Wobbly First Steps", "Takes a few wobbly steps before flopping over.", 15},
    {"Curious Sniffing", "Investigates every rock within reach.", 15},
    {"Peekaboo", "Pokes its head out, then ducks back down giggling.", 12},
    {"Tiny Yawn", "Lets out a surprisingly big yawn for such a small marmot.", 10},
    {"Snuggle Time", "Wants to be held and snuggled for a while.", 15},
    {"Copying Mom", "Watches and copies an older marmot's grooming.", 10},
    {"Chasing a Butterfly", "Chases a butterfly in wobbly circles.", 10},
    {"Burrow Peekaboo", "Peeks out of the burrow entrance, unsure about the big world.", 10},
    {"Milk Time", "Nurses contentedly in the warm burrow.", 15},
};
static const int kBabyCareCount = sizeof(kBabyCare) / sizeof(kBabyCare[0]);

struct TreasureDef {
  const char* name;
  const char* note;
  uint8_t weight;
};

// Always-positive non-food trail finds -- happiness only, no hunger change
// (see resolve()'s default branch), distinct from ForagingFind.
static const TreasureDef kTreasures[] = {
    {"Shiny Quartz", "A piece of quartz glints in a crack in the rock.", 20},
    {"Old Trail Marker", "A weathered wooden trail marker, half-swallowed by moss.", 14},
    {"Lost Compass", "Someone's compass, tucked in a patch of moss.", 12},
    {"Perfect Feather", "A hawk feather, unbroken and perfectly barbed.", 18},
    {"Fern Fossil", "A fern's imprint, pressed into a flat stone.", 10},
    {"Beach Agate", "A polished agate, tumbled smooth by the tide.", 16},
    {"Old Coin", "A coin, greened with age, half-buried at the trailhead.", 10},
    {"Abandoned Nest", "An empty nest, woven tighter than it has any right to be.", 14},
    {"Shed Antler", "A deer antler, shed and half-buried in the leaf litter.", 12},
    {"Cracked Geode", "A plain rock, cracked open to reveal sparkling crystal inside.", 10},
    {"Message in a Bottle", "A sealed bottle, washed up on the tideline.", 6},
    {"Petrified Wood", "A chunk of ancient wood, turned entirely to stone.", 8},
    {"Owl Pellet", "An owl pellet, packed with tiny, delicate bones.", 12},
    {"Sea Glass", "A smooth piece of glass, sanded soft by the surf.", 16},
    {"Carved Walking Stick", "Someone left a hand-carved stick leaning against a tree.", 10},
    {"Old Arrowhead", "A stone arrowhead, half-buried at the trail's edge.", 8},
    {"Empty Honeycomb", "An abandoned honeycomb, empty but beautiful.", 12},
};
static const int kTreasureCount = sizeof(kTreasures) / sizeof(kTreasures[0]);

struct EncounterDef {
  const char* name;
  const char* note;
  bool rival;  // territorial/unfriendly variant -- rarer, drives a worse mood
  uint8_t weight;
};

// Social encounters with other marmots -- mostly friendly, occasionally a
// rival shows up and drives the same worse-mood path as a predator sighting.
static const EncounterDef kEncounters[] = {
    {"Whistle Exchange", "Trades warning whistles with a marmot on the next ridge.", false, 18},
    {"Sunbathing Together", "Suns itself on the rocks alongside a neighbor.", false, 18},
    {"Grooming Session", "Two marmots groom each other in the afternoon sun.", false, 16},
    {"Wrestling Match", "Play-wrestles with a younger marmot from the colony.", false, 14},
    {"Shared Burrow", "Shelters in a neighbor's burrow through a rain shower.", false, 14},
    {"New Neighbor", "A new marmot moves into the talus slope nearby.", false, 12},
    {"Group Sunning", "Joins the whole colony basking on the warm rocks.", false, 16},
    {"Alarm Call Chain", "Passes an alarm whistle down the valley, marmot to marmot.", false, 14},
    {"Friendly Nuzzle", "A familiar marmot greets it with a nuzzle.", false, 16},
    {"Long-Distance Whistles", "Familiar whistles echo back from across the valley.", false, 12},
    {"Territorial Squabble", "A rival marmot chases it off its favorite sunning rock.", true, 8},
    {"Old Rival Returns", "The same grumpy marmot from last summer is back.", true, 6},
    {"Chase Through the Talus", "A bigger marmot chases it through the rockpile.", true, 6},
    {"Burrow Dispute", "Fusses with a neighbor over a contested burrow entrance.", true, 8},
};
static const int kEncounterCount = sizeof(kEncounters) / sizeof(kEncounters[0]);
