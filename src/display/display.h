// display.h — all e-ink rendering: views, sprites, creature.
#pragma once

#include "model.h"

namespace display {

// Initialize SPI (custom pins) and the e-ink panel. Call once after wake.
void begin();

// Full refresh draw of an entire view from the current context. speciesIdx
// selects which entry the Foraging view shows.
void renderView(View v, const AppContext& ctx, int speciesIdx);

// Power the panel down before deep sleep. The image is retained with no power.
void hibernate();

}  // namespace display
