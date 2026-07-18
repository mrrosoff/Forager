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

/**
 * Growth-stage transition reveal -- shown once when the marmot advances to
 * newStage (Baby->Juvenile or Juvenile->Adult), with a couple of deliberate
 * full-refresh flashes (the flicker reads as an "upgrade" here rather than
 * something to avoid). Caller blocks on an ENTER press to acknowledge
 * before continuing to the normal view.
 */
void renderTransition(Stage newStage);

/**
 * Shown when the marmot has wandered off for good after prolonged neglect
 * (see creature::updateNeglect()). Caller blocks on an ENTER press, then
 * resets the whole game -- this is a terminal state, not something to
 * recover from in place.
 */
void renderRanAway();

// Settings overlay -- selected: 0 = Power Off, 1 = Reset Game, 2 = WiFi
// Networks. confirmPending shows a yes/no sub-screen for the destructive
// Reset Game.
void renderSettings(int selected, bool confirmPending);

// Brief goodbye screen shown right before a Power Off (no wake source
// armed) deep sleep.
void renderPowerOff();

/**
 * On-screen character picker -- prompt at top, what's typed so far below
 * it, and the currently-highlighted character/action large in the middle
 * (see textentry.h: LEFT/RIGHT scroll it, ENTER commits it). Shared by
 * marmot naming and WiFi SSID/password entry.
 */
void renderTextEntry(const char* prompt, const char* buffer, char currentPick);

/**
 * WiFi networks sub-screen (under Settings) -- lists saved networks by SSID
 * plus a trailing "Add Network" row; selected indexes into that list
 * (count() == "Add Network"). confirmRemove shows a yes/no sub-screen for
 * deleting the selected network.
 */
void renderWifiMenu(int selected, bool confirmRemove);

// Power the panel down before deep sleep. The image is retained with no power.
void hibernate();

// TEMPORARY review helpers (see DEV_MODE_SCREEN_CYCLE in config.h): total
// number of distinct screens/bitmaps the cycle covers, and rendering one by
// index (0..debugScreenCount()-1).
int debugScreenCount();
void renderDebugScreen(int index);

}  // namespace display
