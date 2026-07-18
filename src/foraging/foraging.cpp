#include "foraging.h"
#include "sprites.h"

namespace foraging {

// [month-1][0] primary, [1] secondary. PNW / Seattle.
static const Forageable kCalendar[12][2] = {
  // Jan
  {{"Hedgehog Mushroom", "mushroom", "Toothed underside, no true gills. Lingers after frost.", SPR_MUSHROOM_TOOTH},
   {"Miner's Lettuce",   "green",    "Round leaf cupping the stem. Shady wet banks.",         SPR_LEAFY_GREEN}},
  // Feb
  {{"Stinging Nettle",   "green",    "First tips now. Gloves on; cooking kills the sting.",   SPR_NETTLE},
   {"Oyster Mushroom",   "mushroom", "Shelving on dead alder and maple after rain.",          SPR_MUSHROOM_CAP}},
  // Mar
  {{"Morel",             "mushroom", "Honeycomb cap. Watch cottonwoods as soil warms.",       SPR_MOREL},
   {"Chickweed",         "green",    "Tender mats in disturbed ground. Mild, fresh.",         SPR_LEAFY_GREEN}},
  // Apr
  {{"Ramps",             "green",    "Wild leek. Harvest one leaf, leave the bulb.",          SPR_RAMP},
   {"Fiddlehead Fern",   "green",    "Tight coils of lady fern. Pick before they unfurl.",    SPR_FIDDLEHEAD}},
  // May
  {{"Elderflower",       "flower",   "Creamy umbels. Cordial weather. Never the leaves.",     SPR_FLOWER_CLUSTER},
   {"Wood Sorrel",       "green",    "Shamrock leaf, lemony bite. Forest floor.",             SPR_LEAFY_GREEN}},
  // Jun
  {{"Salmonberry",       "berry",    "Earliest berry. Golden to red, along streams.",         SPR_BERRY_CLUSTER},
   {"Oxalis",            "green",    "Redwood sorrel. Tart trailside snack.",                 SPR_LEAFY_GREEN}},
  // Jul
  {{"Huckleberry",       "berry",    "Subalpine slopes. Ripens as you gain elevation.",       SPR_BERRY_CLUSTER},
   {"Chanterelle",       "mushroom", "First flush after warm summer rain. Apricot scent.",    SPR_MUSHROOM_CAP}},
  // Aug
  {{"Chanterelle",       "mushroom", "Peak. False ridges, not blades. Conifer duff.",         SPR_MUSHROOM_CAP},
   {"Blackberry",        "berry",    "Himalayan brambles everywhere. Sun-warmed edges.",      SPR_BERRY_CLUSTER}},
  // Sep
  {{"Matsutake",         "mushroom", "Spicy-cinnamon pine mushroom. Sandy conifer soil.",     SPR_MATSUTAKE},
   {"Lobster Mushroom",  "mushroom", "Orange parasitized host. Firm, seafood-sweet.",         SPR_MUSHROOM_CAP}},
  // Oct
  {{"Hedgehog Mushroom", "mushroom", "Main season. Spines, not gills. Easy ID, no toxic lookalikes.", SPR_MUSHROOM_TOOTH},
   {"Porcini",           "mushroom", "King bolete. Fat white stem, no gills — pores.",        SPR_PORCINI}},
  // Nov
  {{"Yellowfoot",        "mushroom", "Winter chanterelle. Hollow stem, late and cold-hardy.", SPR_MUSHROOM_CAP},
   {"Cauliflower Fungus","mushroom", "Cream coral at conifer bases. Soak grit out.",          SPR_CORAL}},
  // Dec
  {{"Oyster Mushroom",   "mushroom", "Frost-tolerant shelves on hardwood snags.",             SPR_MUSHROOM_CAP},
   {"Velvet Shank",      "mushroom", "Enokitake's wild cousin. Orange caps in cold snaps.",   SPR_MUSHROOM_CAP}},
};

static const char* kSeasonNotes[12] = {
  "Deep winter. Slim pickings, prized finds.",   // Jan
  "Late winter. The first green returns.",       // Feb
  "Early spring. Morels and tender shoots.",     // Mar
  "Spring greens. Ferns uncurl, ramps rise.",    // Apr
  "Late spring. Flowers and sour leaves.",       // May
  "Early summer. Berries lead the way.",         // Jun
  "High summer. Berries up high, first 'shrooms.", // Jul
  "Late summer. Chanterelle peak begins.",       // Aug
  "Early fall. The great mushroom flush.",       // Sep
  "Peak fall. Baskets fill fast after rain.",    // Oct
  "Late fall. Cold-hardy fungi hold on.",        // Nov
  "Early winter. Frost mushrooms only.",         // Dec
};

static int clampMonth(int month) {
  if (month < 1) return 1;
  if (month > 12) return 12;
  return month;
}

Forageable featured(int month)  { return kCalendar[clampMonth(month) - 1][0]; }
Forageable secondary(int month) { return kCalendar[clampMonth(month) - 1][1]; }
const char* seasonNote(int month) { return kSeasonNotes[clampMonth(month) - 1]; }

}  // namespace foraging
