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

// One-time birth reveal, shown only on the very first-ever boot.
void renderBirth();

// Settings overlay -- selected: 0 = Power Off, 1 = Reset Game.
// confirmPending shows a yes/no sub-screen for the destructive Reset Game.
void renderSettings(int selected, bool confirmPending);

// Brief goodbye screen shown right before a Power Off (no wake source
// armed) deep sleep.
void renderPowerOff();

// Power the panel down before deep sleep. The image is retained with no power.
void hibernate();

}  // namespace display
