// foraging.h — PNW / Seattle foraging calendar and species reference.
#pragma once

#include "model.h"

namespace foraging {

// month: 1..12. Returns the primary featured forageable for that month.
Forageable featured(int month);

// Returns the secondary forageable for that month.
Forageable secondary(int month);

// A short human-readable season note for the month (e.g. "Spring greens").
const char* seasonNote(int month);

// Master species reference list, for the browsable foraging menu.
int speciesCount();
const Forageable& speciesAt(int index);

// Rebuilds the Foraging view's browse order: relevance-ranked (in-season
// species first, a rain bonus for mushrooms) with per-wake randomization so
// it's not the same order every time. Call once per wake, before browsing.
void rebuildBrowseOrder(int month, bool postRain);

// Species at a browse-order rank (0..speciesCount()-1), after
// rebuildBrowseOrder(). Distinct from speciesAt(), which is raw array order
// (used by the random-pick ForagingFind event spawn).
const Forageable& speciesAtRank(int rank);

// True if this species is in season for the given month (1..12).
bool inSeason(const Forageable& f, int month);

// Short label for a biome, e.g. "Coast" -- for display in the browsable menu.
const char* biomeName(Biome b);

}  // namespace foraging
