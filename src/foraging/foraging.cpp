#include "foraging.h"

#include <algorithm>
#include <cstring>

#include "journal.h"

namespace foraging {

#define MO(m) (1 << ((m) - 1))

// Master PNW / Seattle species reference -- see foraging_species.h.
#include "foraging_species.h"


static const int kSpeciesCount = sizeof(kSpecies) / sizeof(kSpecies[0]);

static const char* kSeasonNotes[12] = {
    "Deep winter. Slim pickings, prized finds.",      // Jan
    "Late winter. The first green returns.",          // Feb
    "Early spring. Morels and tender shoots.",        // Mar
    "Spring greens. Ferns uncurl, ramps rise.",       // Apr
    "Late spring. Flowers and sour leaves.",          // May
    "Early summer. Berries lead the way.",            // Jun
    "High summer. Berries up high, first 'shrooms.",  // Jul
    "Late summer. Chanterelle peak begins.",          // Aug
    "Early fall. The great mushroom flush.",          // Sep
    "Peak fall. Baskets fill fast after rain.",       // Oct
    "Late fall. Cold-hardy fungi hold on.",           // Nov
    "Early winter. Frost mushrooms only.",            // Dec
};

static int clampMonth(int month) {
  if (month < 1) return 1;
  if (month > 12) return 12;
  return month;
}

bool inSeason(const Forageable& f, int month) {
  return (f.monthMask & (1 << (clampMonth(month) - 1))) != 0;
}

int speciesCount() { return kSpeciesCount; }

const Forageable& speciesAt(int index) {
  index = std::max(index, 0);
  if (index >= kSpeciesCount) index = kSpeciesCount - 1;
  return kSpecies[index];
}

static uint8_t browseOrder[kSpeciesCount];
static int browseCount = 0;  // only the discovered subset -- see journal::isDiscovered()
static bool browseOrderBuilt = false;

void rebuildBrowseOrder(int month, bool postRain) {
  month = clampMonth(month);
  int scores[kSpeciesCount];
  browseCount = 0;
  for (int i = 0; i < kSpeciesCount; i++) {
    if (!journal::isDiscovered(i)) continue;
    int score = 0;
    if (inSeason(kSpecies[i], month)) score += 100;
    if (postRain && strcmp(kSpecies[i].kind, "mushroom") == 0) score += 30;
    score += random(40);  // shuffles ties/near-ties so it's not the same order every wake
    browseOrder[browseCount] = (uint8_t)i;
    scores[i] = score;
    browseCount++;
  }
  // Insertion sort by score descending -- kSpeciesCount is small (~250), and
  // this only runs once per wake.
  for (int i = 1; i < browseCount; i++) {
    uint8_t keyIdx = browseOrder[i];
    int keyScore = scores[keyIdx];
    int j = i - 1;
    while (j >= 0 && scores[browseOrder[j]] < keyScore) {
      browseOrder[j + 1] = browseOrder[j];
      j--;
    }
    browseOrder[j + 1] = keyIdx;
  }
  browseOrderBuilt = true;
}

// Number of discovered (browsable) species -- may be 0 early on. Distinct
// from speciesCount(), which is the full 250-species reference size.
int browsableCount() {
  if (!browseOrderBuilt) rebuildBrowseOrder(1, false);
  return browseCount;
}

const Forageable& speciesAtRank(int rank) {
  if (!browseOrderBuilt) rebuildBrowseOrder(1, false);
  rank = std::max(rank, 0);
  if (browseCount == 0) return kSpecies[0];  // caller must check browsableCount() == 0 first
  if (rank >= browseCount) rank = browseCount - 1;
  return kSpecies[browseOrder[rank]];
}

int indexAtRank(int rank) {
  if (!browseOrderBuilt) rebuildBrowseOrder(1, false);
  rank = std::max(rank, 0);
  if (browseCount == 0) return 0;
  if (rank >= browseCount) rank = browseCount - 1;
  return browseOrder[rank];
}

int countEatenOfKind(const char* kind) {
  int count = 0;
  for (int i = 0; i < kSpeciesCount; i++)
    if (journal::hasEaten(i) && strcmp(kSpecies[i].kind, kind) == 0) count++;
  return count;
}

int countEatenOfBiome(Biome b) {
  int count = 0;
  for (int i = 0; i < kSpeciesCount; i++)
    if (journal::hasEaten(i) && kSpecies[i].biome == b) count++;
  return count;
}

Forageable featured(int month) {
  month = clampMonth(month);
  for (int i = 0; i < kSpeciesCount; i++)
    if (inSeason(kSpecies[i], month)) return kSpecies[i];
  return kSpecies[0];
}

Forageable secondary(int month) {
  month = clampMonth(month);
  bool foundFirst = false;
  for (int i = 0; i < kSpeciesCount; i++) {
    if (inSeason(kSpecies[i], month)) {
      if (foundFirst) return kSpecies[i];
      foundFirst = true;
    }
  }
  return kSpecies[(kSpeciesCount > 1) ? 1 : 0];
}

const char* seasonNote(int month) { return kSeasonNotes[clampMonth(month) - 1]; }

const char* biomeName(Biome b) {
  switch (b) {
    case Biome::Mountain:
      return "Mountain";
    case Biome::Coast:
      return "Coast";
    case Biome::Forest:
    default:
      return "Forest";
  }
}

}  // namespace foraging
