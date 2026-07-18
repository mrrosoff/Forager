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

// True if this species is in season for the given month (1..12).
bool inSeason(const Forageable& f, int month);

// Short label for a biome, e.g. "Coast" -- for display in the browsable menu.
const char* biomeName(Biome b);

}  // namespace foraging
