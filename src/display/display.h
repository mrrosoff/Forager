// display.h — all e-ink rendering: views, sprites, moon, creature animation.
#pragma once

#include "model.h"

namespace display {

// Initialize SPI (custom pins) and the e-ink panel. Call once after wake.
void begin();

// Full-refresh draw of an entire view from the current context.
void renderView(View v, const AppContext& ctx);

// Fast partial-refresh update of just the creature region. `frame` advances
// the idle animation (bob, drifting spores, blink). Only meaningful on views
// that show the creature (Main, Status).
void animateCreature(View v, const AppContext& ctx, uint8_t frame);

// Brief on-screen confirmation that a foraging trip was logged.
void flashFed(const AppContext& ctx);

// Power the panel down before deep sleep. The image is retained with no power.
void hibernate();

}  // namespace display
