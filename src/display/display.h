// display.h — all e-ink rendering: views, sprites, creature.
#pragma once

#include "model.h"
#include "textentry.h"

namespace display {

// Initialize SPI (custom pins) and the e-ink panel. Call once after wake.
void begin();

// speciesIdx selects which entry the Foraging view shows. forceFullRefresh
// overrides the normal "only the first frame after wake is a full refresh,
// everything else is partial" behavior (see epd_adapter.h) -- used for a
// mid-session event trigger (see main.cpp), where the new photo art
// deserves a clean full draw instead of a partial-refresh update.
void renderView(View v, const AppContext& ctx, int speciesIdx, bool forceFullRefresh = false);

// Full-screen sleeping marmot with a few drifting Zzz's -- shown right
// before deep sleep, no text or nav bar, since the panel stays on this
// image (unpowered) until the next wake. Picks stage-appropriate art (a
// Baby/Juvenile marmot shouldn't fall asleep as an Adult).
void renderSleep(Stage stage);

// One-time birth reveal, shown only on the very first-ever boot.
void renderBirth();

/**
 * Growth-stage transition reveal -- shown once when the marmot advances to
 * newStage (Baby->Juvenile or Juvenile->Adult), with a couple of deliberate
 * full-refresh flashes (the flicker reads as an "upgrade" here rather than
 * something to avoid). Caller blocks on an ENTER press to acknowledge
 * before continuing to the normal view.
 */
void renderTransition(Stage newStage, const char* name);

/**
 * Shown when a bar has bottomed out from sustained neglect (see
 * creature::checkDeath()) -- cause picks which flavor-text pool the reason
 * line is drawn from. Caller blocks on an ENTER press, then resets the
 * whole game -- this is a terminal state, not something to recover from in
 * place.
 */
void renderDeath(DeathCause cause);

// Settings overlay -- selected: 0 = Power Off, 1 = Reset Game, 2 = WiFi
// Networks. confirmPending shows a yes/no sub-screen for the destructive
// Reset Game.
void renderSettings(int selected, bool confirmPending);

// Blank white screen shown right before a Power Off (no wake source armed)
// deep sleep -- the device is genuinely off until the physical switch
// power-cycles it, so a blank panel (not a lingering message) is the
// correct resting state.
void renderPowerOff();

/**
 * On-screen QWERTY keyboard grid -- prompt at top, what's typed so far
 * below it, then the keyboard (see textentry.h: LEFT/RIGHT scroll the
 * highlighted key, ENTER commits it, including the SHIFT/SYMBOLS page
 * toggles and BACKSPACE/DONE). Shared by marmot naming and WiFi SSID/
 * password entry.
 */
void renderTextEntry(const char* prompt, const textentry::State& s);

/**
 * WiFi networks sub-screen (under Settings) -- lists saved networks by SSID
 * plus a trailing "Add Network" row; selected indexes into that list
 * (count() == "Add Network"). confirmRemove shows a yes/no sub-screen for
 * deleting the selected network.
 */
void renderWifiMenu(int selected, bool confirmRemove);

// Power the panel down before deep sleep. The image is retained with no power.
void hibernate();

}  // namespace display
