// foraging.h — PNW / Seattle foraging calendar, hardcoded by month.
#pragma once

#include "model.h"

namespace foraging {

// month: 1..12. Returns the primary featured forageable for that month.
Forageable featured(int month);

// Returns the secondary forageable for that month.
Forageable secondary(int month);

// A short human-readable season note for the month (e.g. "Spring greens").
const char* seasonNote(int month);

}  // namespace foraging
