// display.h — all e-ink rendering: views, sprites, creature.
#pragma once

#include "model.h"

namespace display {

// Initialize SPI (custom pins) and the e-ink panel. Call once after wake.
void begin();

// speciesIdx selects which entry the Foraging view shows.
void renderView(View v, const AppContext& ctx, int speciesIdx);

// Full-screen sleeping marmot with a few drifting Zzz's -- shown right
// before deep sleep, no text or nav bar, since the panel stays on this
// image (unpowered) until the next wake.
void renderSleep();

// Power the panel down before deep sleep. The image is retained with no power.
void hibernate();

}  // namespace display
