// display.h — all e-ink rendering: views, sprites, creature.
#pragma once

#include "minigames.h"
#include "model.h"
#include "textentry.h"

namespace display {

// Initialize SPI (custom pins) and the e-ink panel. Call once after wake.
void begin();

// View::Minigames is NOT drawn by renderView() -- it needs the game state,
// which model.h deliberately doesn't carry, so it has its own entry point
// (see renderMinigames() below and renderCurrent() in main.cpp).
// speciesIdx selects which entry the Foraging view shows. forceFullRefresh
// overrides the normal "only the first frame after wake is a full refresh,
// everything else is partial" behavior (see epd_adapter.h) -- used for a
// mid-session event trigger (see main.cpp), where the new photo art
// deserves a clean full draw instead of a partial-refresh update.
void renderView(View v, const AppContext& ctx, int speciesIdx, bool forceFullRefresh = false);

/**
 * The Minigames view, at whatever screen its state says (menu, a playback
 * frame, the player's turn, a quiz reveal, or the run-over summary) -- see
 * minigames.h. Separate from renderView() because it needs that state;
 * takes forceFullRefresh for the same reason Foraging does, since a run
 * racks up a lot of partial refreshes.
 */
void renderMinigames(const AppContext& ctx, const minigames::State& s,
                     bool forceFullRefresh = false);

/**
 * Winter settling-up for Snack Hunt's stockpile (see minigames::Stash),
 * shown once on the first December wake. Caller blocks on ENTER, applies
 * the reward or the shortfall, then clears the pile for next year.
 */
void renderWinterStash(bool made, bool grace, int points, const char* name);

/**
 * One-time "new minigame unlocked" reveal for g, shown the first time its
 * condition is met (see minigames::pendingUnlocks()). `name` is the
 * marmot's. Caller blocks on an ENTER press to acknowledge, same as the
 * growth-transition screen.
 */
void renderMinigameUnlock(minigames::Game g, const char* name);

/**
 * The badge/streak wall. Reached through Settings (not the LEFT/RIGHT view
 * cycle -- see View in model.h), so it draws its own frame like the other
 * Settings sub-screens rather than being a renderView() case. Still gated
 * on Adult: pre-Adult it shows a "come back once you're grown" message
 * instead of the badges.
 */
void renderAchievements(const AppContext& ctx);

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

// Settings menu rows, in the order they're drawn -- shared with main.cpp's
// handleSettingsInput() so the "which row did they pick" branching doesn't
// drift from what's on screen. Achievements lives here rather than in the
// LEFT/RIGHT view cycle (see View in model.h).
enum : int {
  SETTINGS_ACHIEVEMENTS = 0,
  SETTINGS_WIFI,
  SETTINGS_RESET_GAME,
  SETTINGS_POWER_OFF,
  SETTINGS_OPTION_COUNT,
};

// Settings overlay -- selected indexes the SETTINGS_* rows above.
// confirmPending shows a yes/no sub-screen for the two destructive rows
// (Reset Game and Power Off).
void renderSettings(int selected, bool confirmPending, uint8_t batteryPercent);

// Blank white screen shown right before a Power Off (no wake source armed)
// deep sleep -- the device is genuinely off until the physical switch
// power-cycles it, so a blank panel (not a lingering message) is the
// correct resting state.
void renderPowerOff();

// Shown while the clock is being set on a cold boot -- the one wake that
// still waits on the radio, so it needs to say so rather than look frozen.
void renderConnecting();

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
