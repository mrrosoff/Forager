#include <Arduino.h>
#include <Preferences.h>

#include <algorithm>
#include <string>

#include "config.h"
#include "creature.h"
#include "display.h"
#include "events.h"
#include "foraging.h"
#include "journal.h"
#include "minigames.h"
#include "model.h"
#include "net.h"
#include "textentry.h"
#include "wifistore.h"

SET_LOOP_TASK_STACK_SIZE(16 * 1024);

static AppContext ctx;
static View currentView = View::Main;
static int forageIdx = 0;  // Foraging view's browse position; session-only, and
                           // reset to DEV_MODE_SPECIES_BROWSE_START on every
                           // transition into/out of Foraging (see
                           // retreatView()/advanceView())
static uint32_t lastActivityMs = 0;

// Set by buildContext() when the growth stage just advanced since the last
// acknowledged wake -- setup() shows display::renderTransition() and blocks
// on ENTER before continuing, if set.
static bool pendingTransition = false;
static Stage transitionToStage = Stage::Baby;

// Set by buildContext() when creature::checkDeath() reports a bar has
// bottomed out -- setup() shows display::renderDeath() and resets the
// whole game once acknowledged. None means still alive.
static DeathCause marmotDeathCause = DeathCause::None;

struct Btn {
  int pin;
  bool prev;
  uint32_t lastEdge;
  // LEFT/RIGHT/ENTER are dedicated buttons wired to 3V3 through the power
  // switch (INPUT_PULLDOWN, reads HIGH when pressed). SETTINGS instead
  // rides the e-ink display module's own onboard KEY1 button (Waveshare
  // Pico-ePaper-4.2), which is wired switch-to-GND like every Waveshare
  // board button -- INPUT_PULLUP, reads LOW when pressed.
  bool activeHigh;
};
static Btn bLeft{PIN_BTN_LEFT, false, 0, true};
static Btn bRight{PIN_BTN_RIGHT, false, 0, true};
static Btn bEnter{PIN_BTN_ENTER, false, 0, true};
static Btn bSettings{PIN_BTN_SETTINGS, false, 0, false};

/**
 * Settings overlay state (see loop()) -- not part of the View cycle, since
 * it's triggered by its own dedicated button rather than LEFT/RIGHT paging.
 * selectedOption indexes display::SETTINGS_* (Achievements, WiFi Networks,
 * Reset Game, Power Off).
 */
static bool inSettings = false;
static int selectedOption = 0;
static bool confirmPending = false;

/**
 * Achievements sub-screen (under Settings). Purely a display -- LEFT/RIGHT/
 * ENTER do nothing there, SETTINGS/KEY1 backs out to the Settings menu like
 * every other sub-screen. It used to be the leftmost View (see View in
 * model.h); it moved here when the minigames took that slot, since it's a
 * read-only progress wall rather than something you interact with.
 */
static bool inAchievements = false;

/**
 * Minigames view state (see handleMinigamesInput()). The game menu and the
 * runs themselves all live inside View::Minigames rather than being an
 * overlay -- the marmot's still on the other views the whole time.
 */
static minigames::State mg;
/**
 * Non-interactive playback frames (Marmot Says' sequence, Forest Memory's
 * look-at-both-cards beat) advance on this timer.
 *
 * 900ms was too fast: the panel spends a few hundred of those milliseconds
 * refreshing, so consecutive frames read as one continuous flicker rather
 * than as distinct calls -- and a sequence you can't visually separate is
 * one you can't memorise. 2000ms leaves a clearly still frame between flips.
 */
static const uint32_t MG_FRAME_MS = 2000;
static uint32_t mgNextFrameMs = 0;
// A finished run tops the marmot up at most once per wake (see
// creature::rewardMinigame()) -- otherwise replaying a good run is a
// happiness faucet.
static bool minigameRewarded = false;

/**
 * WiFi Networks sub-screen state (under Settings -- see handleWifiMenuInput()).
 * wifiSelected indexes wifistore -- an index == wifistore::count() means the
 * trailing "Add Network" row is selected.
 */
static bool inWifiMenu = false;
static int wifiSelected = 0;
static bool wifiRemoveConfirm = false;

/**
 * Shared text-entry overlay (see textentry.h) -- drives marmot naming and
 * WiFi SSID/password entry through the same LEFT/RIGHT-scroll,
 * ENTER-commits picker. tePurpose says what happens when DONE is picked;
 * None means the overlay isn't active.
 */
enum class TextEntryPurpose { None, MarmotName, WifiSsid, WifiPassword };
static TextEntryPurpose tePurpose = TextEntryPurpose::None;
static textentry::State teState;
static std::string pendingWifiSsid;  // holds the SSID while its password is typed next

// Accelerating hold-to-scroll state for RIGHT on the Foraging view (see
// loop()): a held press repeats faster the longer it's held, down to a
// floor, rather than a fixed step size.
static bool rightHeld = false;
static uint32_t rightNextStepMs = 0;
static int rightHoldSteps = 0;
static const uint32_t RIGHT_HOLD_INITIAL_MS = 350;
static const uint32_t RIGHT_HOLD_FLOOR_MS = 20;
static const uint32_t RIGHT_HOLD_ACCEL_MS = 60;

// Same accelerating hold-to-scroll shape as Foraging's RIGHT above, factored
// out so other views can reuse it without their own copy of the state
// machine. Returns true once per step that should fire: immediately on a
// fresh press, then repeatedly with a shrinking interval (down to `floorMs`)
// while held. Takes its own curve rather than sharing Foraging's
// RIGHT_HOLD_* constants -- a redraw-bound context (like the text-entry
// keyboard, which measures every key's text bounds on every frame, unlike
// Foraging's simpler card) can saturate at "however fast the panel
// physically redraws" well before a 20ms floor is ever reached, making the
// ramp invisible; a higher floor keeps the whole curve inside a range where
// speeding up is actually perceptible against that redraw cost.
struct HoldAccel {
  bool held = false;
  uint32_t nextStepMs = 0;
  int steps = 0;
};

static bool holdAccelStep(HoldAccel& s, bool down, uint32_t initialMs, uint32_t floorMs,
                          uint32_t accelMs) {
  if (down && !s.held) {
    s.held = true;
    s.steps = 0;
    s.nextStepMs = millis() + initialMs;
    return true;
  }
  if (down && s.held && millis() >= s.nextStepMs) {
    s.steps++;
    uint32_t interval =
        initialMs > (uint32_t)s.steps * accelMs ? initialMs - s.steps * accelMs : floorMs;
    if (interval < floorMs) interval = floorMs;
    s.nextStepMs = millis() + interval;
    return true;
  }
  if (!down) s.held = false;
  return false;
}

// Text-entry's keyboard redraw is heavier than Foraging's card (measures
// every key's text bounds every frame), so it needs a slower ramp and a
// much higher floor -- see HoldAccel's doc comment above.
static const uint32_t TE_HOLD_INITIAL_MS = 400;
static const uint32_t TE_HOLD_FLOOR_MS = 150;
static const uint32_t TE_HOLD_ACCEL_MS = 25;

static void goToSleep() {
#if DEV_MODE_NO_SLEEP
  return;
#else
  // Always return to Main next wake, and show the sleeping marmot now --
  // the bistable e-ink panel keeps displaying whatever we draw here,
  // unpowered, until the next wake redraws it.
  currentView = View::Main;
  display::renderSleep((Stage)ctx.stage);
  display::hibernate();
  // ext1 (not ext0) so all three buttons wake the board, not just ENTER --
  // ext0 only supports a single GPIO. All three pins are RTC-capable
  // (LEFT=1, RIGHT=2, ENTER=4) and read HIGH when pressed (INPUT_PULLDOWN),
  // so ANY_HIGH wakes on whichever one is pressed.
  uint64_t wakeMask = (1ULL << PIN_BTN_LEFT) | (1ULL << PIN_BTN_RIGHT) | (1ULL << PIN_BTN_ENTER);
  esp_sleep_enable_ext1_wakeup(wakeMask, ESP_EXT1_WAKEUP_ANY_HIGH);
  esp_sleep_enable_timer_wakeup(FORCE_REFRESH_INTERVAL_US);
  esp_deep_sleep_start();
#endif
}

/**
 * Settings -> Power Off: deep-sleeps with NO wake source armed at all
 * (skips both ext0 and the timer backstop goToSleep() uses) -- only the
 * physical inline power switch brings the device back, which re-runs
 * setup() fresh. This is a true "off", not just a longer sleep.
 */
static void doPowerOff() {
  display::renderPowerOff();
  display::hibernate();
  esp_deep_sleep_start();
}

/**
 * Settings -> Reset Game (confirmed): wipes the single "forager" NVS
 * namespace -- creature state, event cooldown, and the journal all live
 * there (see the module-level Preferences usage in creature.cpp/events.cpp/
 * journal.cpp), so one clear() resets everything at once -- then reboots
 * straight into the first-ever-boot path, triggering the birth sequence.
 */
static void doResetGame() {
  Preferences p;
  p.begin("forager", /*readOnly=*/false);
  p.clear();
  p.end();
  esp_restart();
}

// Reads the BATT_ADC divider (see config.h) and maps it to a 0-100 percent
// across BATT_VOLTAGE_EMPTY..BATT_VOLTAGE_FULL. analogReadMilliVolts()
// applies the ESP32's factory ADC calibration, so this is a real voltage
// reading, not a raw uncalibrated ADC count.
static uint8_t readBatteryPercent() {
  float vBatt = (analogReadMilliVolts(PIN_BATT_ADC) / 1000.0f) * BATT_DIVIDER_RATIO;
  float frac = (vBatt - BATT_VOLTAGE_EMPTY) / (BATT_VOLTAGE_FULL - BATT_VOLTAGE_EMPTY);
  frac = std::max(0.0f, std::min(1.0f, frac));
  return (uint8_t)(frac * 100.0f + 0.5f);
}

// Returns true the very first time this ever runs (birthDate == 0, no prior
// save) -- setup() uses this to show the one-time birth sequence. Sets
// birthDate immediately so a later wake never re-triggers it.
static bool buildContext() {
  time_t nowUtc = time(nullptr);
  localtime_r(&nowUtc, &ctx.now);
  int month = ctx.now.tm_mon + 1;

  ctx.featured = foraging::featured(month);
  ctx.batteryPercent = readBatteryPercent();

  creature::load(ctx.creature);
  // firstBoot tracks whether the birth/naming flow still needs to run --
  // driven by `named`, not birthDate. birthDate gets persisted right away
  // below (before naming completes) so growth timing starts at the true
  // creation moment; if firstBoot were keyed off birthDate == 0, losing
  // power mid-naming would skip straight to Main with a default name on the
  // next boot instead of returning to the birth screen. See CreatureState::
  // named's doc comment.
  bool firstBoot = !ctx.creature.named;
  if (ctx.creature.birthDate == 0) ctx.creature.birthDate = nowUtc;

  creature::evaluate(ctx.creature, ctx.now, ctx.weather, (Stage)ctx.stage);
  creature::save(ctx.creature);

  journal::load();
  minigames::load();

  Stage stage = creature::computeStage(journal::totalEaten());
  ctx.stage = (uint8_t)stage;

  // A stage advance since the last acknowledged wake gets a one-time
  // transition screen (see setup()) -- skipped on first boot, since the
  // birth sequence already covers that reveal. Persist the new
  // lastSeenStage right away so losing power mid-screen can't cause it to
  // repeat forever.
  if (!firstBoot && stage != (Stage)ctx.creature.lastSeenStage) {
    pendingTransition = true;
    transitionToStage = stage;
  }
  ctx.creature.lastSeenStage = (uint8_t)stage;

  // Any bar bottoming out (fully starved, or happiness/energy hitting 0) is
  // terminal -- see setup(). Those extremes only happen after multiple days
  // of zero feeding (see checkDeath()'s doc comment), so this is a real
  // neglect consequence, not a one-bad-wake gotcha.
  marmotDeathCause = creature::checkDeath(ctx.creature);
  creature::save(ctx.creature);

  // Spawn-check runs exactly once per wake, here -- there's no live
  // background timer since the device is asleep the rest of the time.
  // Skipped entirely on firstBoot: checkForEvent() treats a never-set
  // evLastAt as "cooldown already elapsed" (there's nothing to measure
  // from yet), so without this guard a newborn marmot could roll straight
  // into a pending event before the player's even finished naming it.
  if (!firstBoot) {
    events::PendingEvent ev = events::checkForEvent(nowUtc, month, stage);
    ctx.eventType = (uint8_t)ev.type;
    ctx.eventDataId = ev.dataId;
    ctx.eventExact = ev.exact ? 1 : 0;
  }

  // Foraging browse order is relevance-ranked (season + rain) with per-wake
  // randomization, so it's not the same order every time either.
  foraging::rebuildBrowseOrder(month, ctx.weather.postRain);

  return firstBoot;
}

static bool pressed(Btn& b) {
  bool raw = digitalRead(b.pin) == HIGH;
  bool now = b.activeHigh ? raw : !raw;
  bool fired = false;
  if (now && !b.prev && (millis() - b.lastEdge) > BTN_DEBOUNCE_MS) {
    fired = true;
    b.lastEdge = millis();
  }
  b.prev = now;
  return fired;
}

/**
 * Draws whatever the current view is. View::Minigames doesn't go through
 * display::renderView() -- it needs the game state, which AppContext
 * deliberately doesn't carry (see display.h) -- so every "redraw where we
 * are" call site goes through here instead of calling renderView() directly.
 */
static void renderCurrent(bool forceFullRefresh = false) {
  if (currentView == View::Minigames) {
    display::renderMinigames(ctx, mg, forceFullRefresh);
  } else {
    display::renderView(currentView, ctx, forageIdx, forceFullRefresh);
  }
}

// Rewards an actively-browsing session with a guaranteed event instead of
// making it wait on the same background odds as an idle device: hitting
// SCREEN_CHANGE_EVENT_TRIGGER view changes within SCREEN_CHANGE_WINDOW_MS
// of each other spawns one on the spot, as long as one wasn't already
// pending or just resolved within EVENT_RECENCY_GATE_SECONDS (see
// config.h). A gap longer than the window resets the count instead of
// letting it accumulate across a slow, sparse session. Returns true if it
// spawned one, so the caller can force a full refresh for the new art.
static int screenChangeCount = 0;
static uint32_t screenChangeWindowStart = 0;

static bool bumpScreenChangeAndMaybeSpawn() {
  uint32_t now = millis();
  if (screenChangeCount == 0 || now - screenChangeWindowStart > SCREEN_CHANGE_WINDOW_MS) {
    screenChangeCount = 1;
    screenChangeWindowStart = now;
  } else {
    screenChangeCount++;
  }
  if (screenChangeCount < SCREEN_CHANGE_EVENT_TRIGGER) return false;
  screenChangeCount = 0;
  if (ctx.eventType != 0) return false;  // already have a pending event
  if (events::recentlyResolved(time(nullptr), EVENT_RECENCY_GATE_SECONDS)) return false;

  events::PendingEvent ev = events::spawnNow((Stage)ctx.stage);
  ctx.eventType = (uint8_t)ev.type;
  ctx.eventDataId = ev.dataId;
  ctx.eventExact = ev.exact ? 1 : 0;
  // The encounter screen only shows/resolves as a takeover of Main -- force
  // the player there so the new event is actually visible and ENTER can
  // acknowledge it, instead of leaving them on whatever view they were
  // navigating to with the event silently pending underneath.
  currentView = View::Main;
  return true;
}

// Foraging's browse position resets to species 1 on *any* transition into or
// out of Foraging (not just the sleep/wake reset noted in foraging.h) --
// leaving the browse position wherever it was last time reads as "lost my
// place" when Foraging is only ever a step away from Main. Both
// retreatView() (leaving Foraging for Status/Main) and advanceView()
// (entering Foraging from Main) reset it, so a round trip through Main
// always lands back on species 1.
static void retreatView() {
  if ((int)currentView <= 0) return;
  View prev = (View)((int)currentView - 1);
  if (currentView == View::Foraging) forageIdx = DEV_MODE_SPECIES_BROWSE_START;
  // Always arrive at Minigames on its menu, never mid-run -- a run can only
  // be left via BACK (which already returns to the menu), but paging in
  // shouldn't be able to resume a stale one either way.
  if (prev == View::Minigames) mg.screen = minigames::Screen::Menu;
  currentView = prev;
  bool spawned = bumpScreenChangeAndMaybeSpawn();
  renderCurrent(spawned);
}

static void advanceView() {
  int n = (int)View::COUNT;
  if ((int)currentView >= n - 1) return;
  View next = (View)((int)currentView + 1);
  if (next == View::Foraging) forageIdx = DEV_MODE_SPECIES_BROWSE_START;
  currentView = next;
  bool spawned = bumpScreenChangeAndMaybeSpawn();
  renderCurrent(spawned);
}

// Defined below (it needs the minigame render helpers) -- onEnter() calls it
// because resolving a Discovery can cross a minigame's species threshold
// mid-session, and the reveal should fire right then rather than next wake.
static void showPendingMinigameUnlocks();

// ENTER's action depends on the current view: resolve a pending sighting/
// mishap/weather event on Main (ForagingFind events are NOT resolved here --
// see below), eat the species currently on screen on Foraging (this always
// feeds the creature -- see creature::feedForaged() -- and additionally
// resolves a pending ForagingFind on top if the eaten species satisfies it,
// per events::eventMatchesSpecies()), or nothing on Status.
static void onEnter() {
  switch (currentView) {
    case View::Main: {
      events::PendingEvent ev;
      ev.type = (events::EventType)ctx.eventType;
      ev.dataId = ctx.eventDataId;
      ev.exact = ctx.eventExact != 0;
      if (ev.type != events::EventType::None && ev.type != events::EventType::ForagingFind) {
        if (ev.type == events::EventType::Discovery) {
          journal::markDiscovered(ev.dataId);
          journal::save();
          // The one curiosity source available from birth, before either
          // species minigame unlocks.
          creature::rewardDiscovery(ctx.creature, time(nullptr));
          foraging::rebuildBrowseOrder(ctx.now.tm_mon + 1, ctx.weather.postRain);
        } else if (ev.type == events::EventType::AnimalSighting) {
          journal::bumpAnimalSightings();
        } else if (ev.type == events::EventType::WeatherEvent) {
          journal::bumpWeatherEvents();
        } else if (ev.type == events::EventType::TrailMishap ||
                   ev.type == events::EventType::TrailTreasure ||
                   ev.type == events::EventType::MarmotEncounter) {
          journal::bumpOtherEvents();
        }
        events::resolve(ev, ctx.creature, time(nullptr));
        creature::evaluate(ctx.creature, ctx.now, ctx.weather, (Stage)ctx.stage);
        creature::save(ctx.creature);
        ctx.eventType = (uint8_t)events::EventType::None;
        showPendingMinigameUnlocks();
        display::renderView(View::Main, ctx, forageIdx);
      }
      break;
    }
    case View::Foraging: {
      if (foraging::browsableCount() == 0) break;  // nothing discovered yet, nothing to eat
      events::PendingEvent ev;
      ev.type = (events::EventType)ctx.eventType;
      ev.dataId = ctx.eventDataId;
      ev.exact = ctx.eventExact != 0;
      const Forageable& current = foraging::speciesAtRank(forageIdx);
      int month = ctx.now.tm_mon + 1;
      creature::feedForaged(ctx.creature, time(nullptr), foraging::inSeason(current, month),
                            current.kind);
      journal::markEaten(foraging::indexAtRank(forageIdx));
      journal::save();
      if (events::eventMatchesSpecies(ev, current)) {
        events::resolve(ev, ctx.creature, time(nullptr));
        ctx.eventType = (uint8_t)events::EventType::None;
      }
      creature::evaluate(ctx.creature, ctx.now, ctx.weather, (Stage)ctx.stage);
      creature::save(ctx.creature);
      display::renderView(View::Foraging, ctx, forageIdx);
      break;
    }
    default:
      break;
  }
}

// Foraging-view species stepping, shared by a plain RIGHT tap and the
// accelerating hold-to-scroll in loop(). Every FORAGE_SCROLL_REFRESH steps
// forces a full refresh instead of the usual partial one, since Foraging is
// the highest-scroll-volume view (hold-to-scroll can rack up dozens of
// partial refreshes in seconds) and ghosting accumulates with each one.
static const int FORAGE_SCROLL_REFRESH = 25;
static int forageScrollCount = 0;
// step defaults to 1 for a plain tap; a sustained hold passes a larger value
// (see forageHoldStepSize()) so scrolling speeds up by covering more ground
// per redraw rather than by redrawing faster -- the panel's own refresh time
// bounds how often loop() can even get back here, so shrinking the repeat
// interval alone (see RIGHT_HOLD_FLOOR_MS) can't make a held RIGHT feel any
// faster once that floor is reached. Wraps past the last species back to the
// first (and vice versa isn't possible -- Foraging only scrolls forward,
// LEFT always leaves the view, see retreatView()) instead of clamping, so a
// long hold can't get stuck sitting at species 200.
static void advanceForageIdx(int step = 1) {
  int count = foraging::browsableCount();
  if (count <= 0) return;
  forageIdx = (forageIdx + step) % count;
  forageScrollCount++;
  bool forceFullRefresh = forageScrollCount >= FORAGE_SCROLL_REFRESH;
  if (forceFullRefresh) forageScrollCount = 0;
  display::renderView(View::Foraging, ctx, forageIdx, forceFullRefresh);
}

// Escalating jump size for a held RIGHT in Foraging: covers more species per
// redraw the longer the hold lasts, in steps of 1/2/3/5/10 as requested --
// tuned to actually speed up browsing given the panel-redraw bottleneck
// noted above, rather than the earlier approach of just shrinking the
// repeat interval.
static int forageHoldStepSize(int holdSteps) {
  if (holdSteps < 5) return 1;
  if (holdSteps < 10) return 2;
  if (holdSteps < 16) return 3;
  if (holdSteps < 24) return 5;
  return 10;
}

/**
 * Blocks until ENTER is pressed and released, or the idle window runs out
 * (in which case it sleeps, exactly like the birth/transition screens do --
 * walking away mid-screen shouldn't burn battery). Shared by every
 * acknowledge-this-and-continue reveal.
 */
static void waitForEnterOrSleep() {
  uint32_t waitStart = millis();
  while (digitalRead(PIN_BTN_ENTER) != HIGH) {
    if (millis() - waitStart > INACTIVITY_SLEEP_MS) {
      goToSleep();
      return;  // only reached under DEV_MODE_NO_SLEEP, where goToSleep() is a no-op
    }
    delay(15);
  }
  // Wait for release too, so the press that acknowledged this screen can't
  // also register as an edge on whatever comes next.
  while (digitalRead(PIN_BTN_ENTER) == HIGH) delay(15);
  bEnter.prev = false;
}

/**
 * First December wake of the year: settle up the Winter Stash that Snack
 * Hunt has been filling (see minigames::stashResolveDue()). Making the goal
 * sends the marmot into the den fat and content; falling short costs it,
 * though nowhere near enough to be fatal on its own -- the stockpile is a
 * long-term goal to aim at, not a second way to lose the game.
 */
static void resolveWinterStashIfDue() {
  int year = ctx.now.tm_year + 1900, month = ctx.now.tm_mon + 1;
  if (!minigames::stashResolveDue(year, month)) return;

  time_t nowUtc = time(nullptr);
  int64_t daysAlive = ctx.creature.birthDate > 0 ? (nowUtc - ctx.creature.birthDate) / 86400 : 0;

  // Settle the year silently for a marmot that has barely existed. Powering
  // a fresh device on in December otherwise put a full-screen "you didn't
  // stockpile for winter" ceremony between the naming screen and the
  // marmot's own home view, aimed at a marmot minutes old. Banking the year
  // here still stops it firing again this December, and next winter resolves
  // normally.
  if (daysAlive < WINTER_TOO_YOUNG_DAYS) {
    minigames::stashResolve(year);
    return;
  }

  int points = minigames::stashPoints();
  bool made = minigames::stashResolve(year);

  // A marmot born in late autumn had a season, just not much of one, so it
  // isn't punished for a goal it couldn't have reached -- it's told the
  // winter came early instead.
  bool grace = !made && daysAlive < minigames::WINTER_GRACE_DAYS;

  display::renderWinterStash(made, grace, points, ctx.creature.name);
  waitForEnterOrSleep();

  int delta = made ? 25 : (grace ? 0 : -15);
  int h = (int)ctx.creature.happiness + delta;
  ctx.creature.happiness = (uint8_t)std::max(1, std::min(100, h));
  if (made) {
    int e = (int)ctx.creature.energy + 25;
    ctx.creature.energy = (uint8_t)std::min(100, e);
  }
  creature::save(ctx.creature);
}

/**
 * Shows one "new minigame unlocked" reveal per game that just became
 * available, then redraws whatever was on screen. Called both at wake (for
 * growth- and journal-driven unlocks that happened since last time) and
 * right after a Discovery resolves, since that can cross the 25/50-species
 * thresholds mid-session.
 */
static void showPendingMinigameUnlocks() {
  uint8_t pending = minigames::pendingUnlocks((Stage)ctx.stage);
  if (pending == 0) return;
  for (int i = 0; i < (int)minigames::Game::COUNT; i++) {
    if (!(pending & (1 << i))) continue;
    display::renderMinigameUnlock((minigames::Game)i, ctx.creature.name);
    minigames::markAnnounced((minigames::Game)i);
    waitForEnterOrSleep();
  }
}

/**
 * Ends a minigame run: records the high score (see minigames::finishRun())
 * and, the first time in this wake session, pays out the happiness/
 * lastPlayed reward for a non-zero score.
 */
/**
 * Which bar each game feeds. Every lethal stat has a game attached, so no
 * bar can run down with nothing the player can do about it: Snack Hunt turns
 * up food, Marmot Says is company, Burrow Maze is exercise, and the two
 * species games are the only things that feed curiosity.
 */
static creature::Stat statForGame(minigames::Game g) {
  switch (g) {
    case minigames::Game::Snack:
      return creature::Stat::Hunger;
    case minigames::Game::Maze:
      return creature::Stat::Energy;
    case minigames::Game::Memory:
    case minigames::Game::Quiz:
      return creature::Stat::Curiosity;
    default:
      return creature::Stat::Happiness;
  }
}

static void endMinigameRun() {
  minigames::finishRun(mg);
  // Every finished run resets lastPlayed (which lifts the happiness/energy
  // ceilings), but the stat top-up itself is once per wake -- pass 0 after
  // that so replaying can't farm it.
  int score = minigameRewarded ? 0 : mg.score;
  creature::rewardMinigame(ctx.creature, time(nullptr), score, statForGame(mg.game));
  if (score > 0) minigameRewarded = true;
  creature::evaluate(ctx.creature, ctx.now, ctx.weather, (Stage)ctx.stage);
  creature::save(ctx.creature);
}

/**
 * Starts a fresh run of `g` from the game menu (or from the Over screen's
 * "play again"). Refuses to start a locked game, or one whose question pool
 * turned out to be empty -- the caller stays on the menu in that case,
 * where the row already says what it's waiting on.
 */
static bool startMinigame(minigames::Game g) {
  if (!minigames::isUnlocked(g, (Stage)ctx.stage)) return false;
  mg.game = g;
  mg.score = 0;
  mg.newBest = false;
  mg.won = false;
  switch (g) {
    case minigames::Game::Snack:
      // Day index, so only the first run of each calendar day stocks the
      // stash (see minigames::startSnackRun()).
      minigames::startSnackRun(mg, (int64_t)time(nullptr) / 86400);
      mg.screen = minigames::Screen::Prompt;
      break;
    case minigames::Game::Simon:
      minigames::startSimonRun(mg);
      mg.screen = minigames::Screen::Sequence;
      mgNextFrameMs = millis() + MG_FRAME_MS;
      break;
    case minigames::Game::Memory:
      minigames::startMemoryBoard(mg);
      mg.screen = minigames::Screen::Prompt;
      break;
    case minigames::Game::Maze:
      minigames::startMaze(mg);
      mg.screen = minigames::Screen::Prompt;
      break;
    default:
      if (!minigames::startQuizRound(mg)) return false;
      mg.screen = minigames::Screen::Prompt;
      break;
  }
  renderCurrent(true);
  return true;
}

/**
 * The Minigames view's button handling, which mirrors how Foraging takes
 * over RIGHT: on the game menu LEFT cycles the selection (it's otherwise
 * dead, since Minigames is the leftmost view and retreatView() no-ops
 * there) while RIGHT still pages on to Status; once a run starts the game
 * owns all three buttons, and SETTINGS/KEY1 quits back to the menu (handled
 * in loop() with the rest of the back-button dispatch).
 *
 * Returns true if anything happened, so the caller only resets the idle
 * timer on real activity rather than on every poll.
 */
static bool handleMinigamesInput() {
  using minigames::Screen;

  if (mg.screen == Screen::Menu) {
    bool acted = false;
    if (pressed(bLeft)) {
      mg.menuSel = (mg.menuSel + 1) % (int)minigames::Game::COUNT;
      renderCurrent();
      acted = true;
    }
    if (pressed(bRight)) {
      advanceView();
      return true;
    }
    if (pressed(bEnter)) {
      // A locked (or unaskable) game just doesn't start -- its menu row
      // already says what it's waiting on, so there's nothing to add.
      acted = startMinigame((minigames::Game)mg.menuSel);
    }
    return acted;
  }

  if (mg.screen == Screen::Sequence) {
    // Non-interactive frames: swallow any presses (so a mashed button
    // doesn't queue up an answer) and advance on the frame timer.
    pressed(bLeft);
    pressed(bRight);
    pressed(bEnter);
    if (millis() < mgNextFrameMs) return false;
    if (mg.game == minigames::Game::Simon) {
      mg.simon.showIdx++;
      if (mg.simon.showIdx >= mg.simon.len) {
        // Playback just ran a burst of partial refreshes over the same
        // region (one per call, plus a photo), which is exactly how
        // ghosting accumulates -- and a ghost of the *last call shown* on
        // the answer screen would be a genuine hint. Clean the panel before
        // handing the turn over.
        mg.screen = Screen::Prompt;
        mgNextFrameMs = millis() + MG_FRAME_MS;
        renderCurrent(true);
        return true;
      }
    } else if (mg.game == minigames::Game::Memory) {
      // The beat where both cards are face-up before a mismatch flips back.
      if (minigames::memoryResolveTurn(mg)) {
        mg.score++;
        if (minigames::memoryBoardCleared(mg)) minigames::startMemoryBoard(mg);
      } else if (mg.memory.missesLeft <= 0) {
        endMinigameRun();
        renderCurrent(true);
        return true;
      }
      mg.screen = Screen::Prompt;
    }
    mgNextFrameMs = millis() + MG_FRAME_MS;
    renderCurrent();
    return true;
  }

  if (mg.screen == Screen::Reveal) {
    if (!pressed(bEnter)) return false;
    if (mg.game == minigames::Game::Snack) {
      // Snack Hunt's reveal is the bushes lifted; ENTER moves on to the
      // next set, or ends the run once the picks are spent.
      if (mg.snack.picksLeft > 0) {
        minigames::startSnackRound(mg);
        mg.screen = Screen::Prompt;
        // Same reasoning as the pick itself: bushes settling back down is a
        // small change, not a new screen. The run-over screen below is a
        // real page turn and still gets the full refresh.
        renderCurrent();
        return true;
      }
      endMinigameRun();
    } else {
      // A correct answer always has more questions behind it -- the pool the
      // quiz draws from is whatever's discovered, which can't shrink mid-run.
      minigames::startQuizRound(mg);
      mg.screen = Screen::Prompt;
    }
    renderCurrent(true);
    return true;
  }

  if (mg.screen == Screen::Over) {
    if (!pressed(bEnter)) return false;
    if (!startMinigame(mg.game)) {
      mg.screen = Screen::Menu;
      renderCurrent();
    }
    return true;
  }

  // Screen::Prompt -- the player's turn, per game.
  switch (mg.game) {
    case minigames::Game::Quiz: {
      bool acted = false;
      if (pressed(bLeft)) {
        mg.choice.sel = (mg.choice.sel + 2) % 3;
        acted = true;
      }
      if (pressed(bRight)) {
        mg.choice.sel = (mg.choice.sel + 1) % 3;
        acted = true;
      }
      if (pressed(bEnter)) {
        if (minigames::choiceCorrect(mg, mg.choice.sel)) {
          mg.score++;
          mg.screen = Screen::Reveal;  // the photo, as the reward
        } else {
          endMinigameRun();
        }
        renderCurrent(true);  // both outcomes are a full-page change
        return true;
      }
      if (acted) renderCurrent();
      return acted;
    }
    case minigames::Game::Memory: {
      const int cards = minigames::MemoryRound::CARDS;
      bool acted = false;
      if (pressed(bLeft)) {
        mg.memory.sel = (mg.memory.sel + cards - 1) % cards;
        acted = true;
      }
      if (pressed(bRight)) {
        mg.memory.sel = (mg.memory.sel + 1) % cards;
        acted = true;
      }
      if (pressed(bEnter) && minigames::memoryFlip(mg)) {
        // Second card of the turn: hold both face-up on a timed frame so
        // there's something to actually memorize before it flips back.
        if (minigames::memoryTurnPending(mg)) {
          mg.screen = Screen::Sequence;
          mgNextFrameMs = millis() + MG_FRAME_MS * 2;
        }
        renderCurrent();
        return true;
      }
      if (acted) renderCurrent();
      return acted;
    }
    case minigames::Game::Snack: {
      const int bushes = minigames::SnackRound::BUSHES;
      bool acted = false;
      if (pressed(bLeft)) {
        mg.snack.sel = (mg.snack.sel + bushes - 1) % bushes;
        acted = true;
      }
      if (pressed(bRight)) {
        mg.snack.sel = (mg.snack.sel + 1) % bushes;
        acted = true;
      }
      if (pressed(bEnter)) {
        // Whatever turns up is banked into the persisted stash immediately
        // (see minigames::snackPick()) -- a run can be abandoned mid-way
        // without losing what it already found.
        mg.score += minigames::snackPick(mg);
        mg.screen = Screen::Reveal;
        // Partial, not a forced full refresh: only the bushes and the line
        // under them change, and a full-panel flash between "pick" and
        // "here's what was under it" breaks the illusion that the bushes
        // are being lifted rather than the screen being replaced.
        renderCurrent();
        return true;
      }
      if (acted) renderCurrent();
      return acted;
    }
    case minigames::Game::Maze: {
      int opts[4];
      int nOpts = minigames::mazeOptions(mg, opts);
      bool acted = false;
      if (pressed(bLeft) && nOpts > 1) {
        mg.maze.sel = (mg.maze.sel + nOpts - 1) % nOpts;
        acted = true;
      }
      if (pressed(bRight) && nOpts > 1) {
        mg.maze.sel = (mg.maze.sel + 1) % nOpts;
        acted = true;
      }
      if (pressed(bEnter)) {
        minigames::mazeAdvance(mg);
        if (minigames::mazeSolved(mg)) {
          mg.score++;
          minigames::startMaze(mg);
          renderCurrent(true);
        } else if (mg.maze.movesLeft <= 0) {
          endMinigameRun();
          renderCurrent(true);
        } else {
          renderCurrent();
        }
        return true;
      }
      if (acted) renderCurrent();
      return acted;
    }
    case minigames::Game::Simon: {
      // All three read in one pass (not short-circuited) so every button's
      // edge state stays current -- see pressed()'s prev bookkeeping.
      bool l = pressed(bLeft), r = pressed(bRight), e = pressed(bEnter);
      int button = l ? 0 : r ? 1 : e ? 2 : -1;
      if (button < 0) return false;
      if (!minigames::simonPress(mg, (uint8_t)button)) {
        endMinigameRun();
        renderCurrent(true);
      } else if (minigames::simonRoundComplete(mg)) {
        mg.score++;
        if (mg.score >= minigames::SIMON_WIN_ROUNDS) {
          // Beat the game rather than failed out of it.
          mg.won = true;
          endMinigameRun();
          renderCurrent(true);
          return true;
        }
        minigames::startSimonRound(mg);
        mg.screen = Screen::Sequence;
        mgNextFrameMs = millis() + MG_FRAME_MS;
        renderCurrent(true);
      } else {
        renderCurrent();
      }
      return true;
    }
    default:
      return false;  // every game is handled above
  }
}

static const char* textEntryPrompt(TextEntryPurpose p) {
  switch (p) {
    case TextEntryPurpose::MarmotName:
      return "Name your marmot:";
    case TextEntryPurpose::WifiSsid:
      return "WiFi network name:";
    case TextEntryPurpose::WifiPassword:
      return "WiFi password:";
    default:
      return "";
  }
}

static void renderCurrentTextEntry() {
  display::renderTextEntry(textEntryPrompt(tePurpose), teState);
}

// Begins the shared text-entry overlay for one purpose. initial pre-fills
// the buffer (e.g. re-showing an in-progress SSID isn't needed today, but
// keeps the helper general).
static void startTextEntry(TextEntryPurpose purpose, const char* initial) {
  tePurpose = purpose;
  textentry::init(teState, initial, purpose == TextEntryPurpose::MarmotName);
  renderCurrentTextEntry();
}

// Applies whatever DONE-triggered action a given purpose implies once the
// picker commits. Each case decides what happens next -- another text-entry
// step (SSID -> password), returning to a menu, or just resuming the view.
static void finishTextEntry() {
  switch (tePurpose) {
    case TextEntryPurpose::MarmotName: {
      const std::string& name = teState.buffer.empty() ? "Marmot" : teState.buffer;
      strncpy(ctx.creature.name, name.c_str(), sizeof(ctx.creature.name) - 1);
      ctx.creature.name[sizeof(ctx.creature.name) - 1] = '\0';
      ctx.creature.named = true;
      creature::save(ctx.creature);
      tePurpose = TextEntryPurpose::None;
      renderCurrent();
      break;
    }
    case TextEntryPurpose::WifiSsid: {
      if (teState.buffer.empty()) {
        // Blank SSID -- treat as cancelling out of "Add Network" rather
        // than prompting for a password nobody's going to use.
        tePurpose = TextEntryPurpose::None;
        display::renderWifiMenu(wifiSelected, false);
        break;
      }
      pendingWifiSsid = teState.buffer;
      startTextEntry(TextEntryPurpose::WifiPassword, "");
      break;
    }
    case TextEntryPurpose::WifiPassword: {
      wifistore::add(pendingWifiSsid.c_str(), teState.buffer.c_str());
      tePurpose = TextEntryPurpose::None;
      wifiSelected = 0;
      display::renderWifiMenu(wifiSelected, false);
      break;
    }
    default:
      break;
  }
}

// Accelerating hold-to-scroll state for the text-entry keyboard's LEFT/
// RIGHT -- see HoldAccel above. Separate from Foraging's rightHeld/etc.
// since the two are never active at the same time but shouldn't share
// state (a hold carried over from one context into the other would fire a
// stray step).
static HoldAccel teRightAccel, teLeftAccel;

// Returns true if any button press was handled -- callers use this to reset
// their own idle/timeout tracking (see setup()'s birth-naming block, which
// runs before the normal loop() idle machinery is active).
static bool handleTextEntryInput() {
  bool activity = false;

  // SETTINGS/KEY1: immediate-enter while naming the marmot (commits
  // whatever's typed without needing to scroll the picker to DONE first),
  // or back-out-to-WiFi-menu while entering an SSID/password -- same
  // "KEY1 = back" convention as the rest of Settings, just abandoning the
  // whole add-network flow in one press rather than stepping field by
  // field.
  if (pressed(bSettings)) {
    if (tePurpose == TextEntryPurpose::MarmotName) {
      finishTextEntry();
    } else {
      tePurpose = TextEntryPurpose::None;
      display::renderWifiMenu(wifiSelected, false);
    }
    return true;
  }

  bool rightDown = digitalRead(PIN_BTN_RIGHT) == HIGH;
  if (holdAccelStep(teRightAccel, rightDown, TE_HOLD_INITIAL_MS, TE_HOLD_FLOOR_MS,
                    TE_HOLD_ACCEL_MS)) {
    textentry::moveNext(teState);
    renderCurrentTextEntry();
    activity = true;
  }
  bRight.prev = rightDown;

  bool leftDown = digitalRead(PIN_BTN_LEFT) == HIGH;
  if (holdAccelStep(teLeftAccel, leftDown, TE_HOLD_INITIAL_MS, TE_HOLD_FLOOR_MS,
                    TE_HOLD_ACCEL_MS)) {
    textentry::movePrev(teState);
    renderCurrentTextEntry();
    activity = true;
  }
  bLeft.prev = leftDown;

  if (pressed(bEnter)) {
    activity = true;
    if (textentry::commit(teState)) {
      finishTextEntry();
    } else {
      renderCurrentTextEntry();
    }
  }
  return activity;
}

// WiFi Networks sub-screen under Settings -- RIGHT cycles the selected
// network plus a trailing "Add Network" row, ENTER acts (confirm-delete for
// a network, start SSID entry for "Add Network"), LEFT exits back to the
// main Settings menu (or cancels a pending delete-confirm).
// LEFT/RIGHT/ENTER only cycle and select within these menus now -- "back"
// (to the previous screen, at whatever nesting depth) is the dedicated
// SETTINGS/KEY1 button instead, handled once in loop() rather than
// duplicated as a LEFT case in every handler (see the settings-button
// dispatch at the top of loop()).
static void handleWifiMenuInput() {
  int addRow = wifistore::count();
  if (wifiRemoveConfirm) {
    if (pressed(bEnter)) {
      wifistore::remove(wifiSelected);
      wifiRemoveConfirm = false;
      wifiSelected = 0;
      display::renderWifiMenu(wifiSelected, false);
    }
    return;
  }
  if (pressed(bRight)) {
    wifiSelected = (wifiSelected + 1) % (addRow + 1);
    display::renderWifiMenu(wifiSelected, false);
  }
  if (pressed(bLeft)) {
    wifiSelected = (wifiSelected - 1 + addRow + 1) % (addRow + 1);
    display::renderWifiMenu(wifiSelected, false);
  }
  if (pressed(bEnter)) {
    if (wifiSelected == addRow) {
      startTextEntry(TextEntryPurpose::WifiSsid, "");
    } else {
      wifiRemoveConfirm = true;
      display::renderWifiMenu(wifiSelected, true);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_BTN_LEFT, INPUT_PULLDOWN);
  pinMode(PIN_BTN_RIGHT, INPUT_PULLDOWN);
  pinMode(PIN_BTN_ENTER, INPUT_PULLDOWN);
  pinMode(PIN_BTN_SETTINGS, INPUT_PULLUP);
  // 11db attenuation for the full ~3.3V range -- the battery divider's
  // midpoint reaches ~2.1V at a full 4.2V charge, comfortably inside it.
  analogSetPinAttenuation(PIN_BATT_ADC, ADC_11db);
  log_i("Woke");

#if DEV_MODE_EVENT_CYCLE
  // Review-only loop -- see DEV_MODE_EVENT_CYCLE's doc comment in config.h.
  // Bypasses WiFi/game state entirely; ENTER advances to the next event,
  // wrapping back to the first after the last. Reuses the real
  // encounter-screen renderer (Main view takeover) by just populating ctx
  // the way a real spawned event would.
  display::begin();
  ctx.stage = (uint8_t)Stage::Adult;
  strncpy(ctx.creature.name, "Marmot", sizeof(ctx.creature.name) - 1);
  ctx.creature.mood = Mood::Excited;

  // One (EventType, count) bucket per demo group, in display order -- a
  // flat demo index walks through each bucket's dataId range in turn
  // before moving to the next bucket, so every distinct entry in the small
  // curated pools gets its own screen while AnimalSighting/Discovery/
  // ForagingFind (tied to the much larger animal/species tables) just get
  // one representative each.
  struct DemoBucket {
    events::EventType type;
    int count;
  };
  const DemoBucket kBuckets[] = {
      {events::EventType::AnimalSighting, 1},
      {events::EventType::Discovery, 1},
      {events::EventType::TrailMishap, events::mishapCount()},
      {events::EventType::WeatherEvent, events::weatherCount()},
      {events::EventType::BabyCare, events::babyCareCount()},
      {events::EventType::TrailTreasure, events::treasureCount()},
      {events::EventType::MarmotEncounter, events::encounterCount()},
      {events::EventType::ForagingFind, 2},  // category find, then an exact-species find
  };
  const int kBucketCount = sizeof(kBuckets) / sizeof(kBuckets[0]);
  int kTotal = 0;
  for (int b = 0; b < kBucketCount; b++) kTotal += kBuckets[b].count;

  int demoIdx = 0;
  while (true) {
    int remaining = demoIdx;
    int bucket = 0;
    while (remaining >= kBuckets[bucket].count) {
      remaining -= kBuckets[bucket].count;
      bucket++;
    }
    ctx.eventType = (uint8_t)kBuckets[bucket].type;
    ctx.eventDataId = (uint8_t)remaining;
    // ForagingFind's second demo entry (bucket count == 2) is the
    // exact-species variant; everything else is never exact.
    ctx.eventExact =
        (kBuckets[bucket].type == events::EventType::ForagingFind && remaining == 1) ? 1 : 0;

    display::renderView(View::Main, ctx, 0);
    log_i("Showing event %d/%d (type %d, dataId %d) -- press ENTER for next", demoIdx + 1, kTotal,
          ctx.eventType, ctx.eventDataId);
    while (digitalRead(PIN_BTN_ENTER) != HIGH) delay(15);
    while (digitalRead(PIN_BTN_ENTER) == HIGH) delay(15);  // wait for release
    demoIdx = (demoIdx + 1) % kTotal;
  }
#endif

  ctx.netOk = net::connectStrongest();
  if (ctx.netOk) {
    if (!net::syncTime(ctx.now)) log_w("Using prior clock");
    ctx.weather = net::fetchWeather();
  } else {
    log_w("Offline; using cached state");
    ctx.weather.valid = false;
  }
  net::shutdown();

  bool firstBoot = buildContext();

  display::begin();

  if (marmotDeathCause != DeathCause::None) {
    // Terminal -- preempts birth/transition/normal-view entirely. Blocks on
    // ENTER (with the same battery-safety timeout as the transition screen
    // below) before wiping everything and rebooting into a fresh birth.
    display::renderDeath(marmotDeathCause);
    uint32_t waitStart = millis();
    while (digitalRead(PIN_BTN_ENTER) != HIGH) {
      if (millis() - waitStart > INACTIVITY_SLEEP_MS) goToSleep();
      delay(15);
    }
    doResetGame();  // never returns -- esp_restart()
  }

  if (firstBoot) {
#if DEV_MODE_SKIP_GROWTH
    // DEV_MODE_SKIP_GROWTH already jumps straight to Adult content --
    // skipping the interactive birth/naming flow too (auto-named "Marmot")
    // means a dev testing Adult-only views doesn't have to click through
    // naming via the physical buttons on every fresh boot/reset.
    strncpy(ctx.creature.name, "Marmot", sizeof(ctx.creature.name) - 1);
    ctx.creature.name[sizeof(ctx.creature.name) - 1] = '\0';
    ctx.creature.named = true;
    creature::save(ctx.creature);
#else
    // Blank the panel first -- this is the very first frame after
    // display::begin(), so it gets a guaranteed full refresh (see
    // EpdGFX::endFrame() in CLAUDE.md's display notes), wiping any stale
    // content cleanly rather than partial-refreshing over it. Gives a brief
    // window to power off with a genuinely blank screen (e.g. right after a
    // fresh flash) before the birth reveal draws.
    display::renderPowerOff();
    delay(3000);
    display::renderBirth();
    // Require an explicit ENTER before jumping into naming -- otherwise the
    // reveal flashes by and the player's straight into text entry with no
    // chance to actually look at it. Same bounded-wait shape as the
    // transition screen below.
    uint32_t birthWaitStart = millis();
    while (digitalRead(PIN_BTN_ENTER) != HIGH) {
      if (millis() - birthWaitStart > INACTIVITY_SLEEP_MS) goToSleep();
      delay(15);
    }
    // Name the marmot right after the birth reveal, before the normal loop()
    // idle machinery exists yet -- so this uses its own bounded wait rather
    // than lastActivityMs, resetting the timeout on every keypress
    // (handleTextEntryInput()'s return value) rather than only at the end,
    // since naming can legitimately take longer than one idle window.
    startTextEntry(TextEntryPurpose::MarmotName, "");
    uint32_t waitStart = millis();
    while (tePurpose != TextEntryPurpose::None) {
      if (handleTextEntryInput()) waitStart = millis();
      if (millis() - waitStart > INACTIVITY_SLEEP_MS) {
        strncpy(ctx.creature.name, "Marmot", sizeof(ctx.creature.name) - 1);
        ctx.creature.name[sizeof(ctx.creature.name) - 1] = '\0';
        ctx.creature.named = true;
        creature::save(ctx.creature);
        tePurpose = TextEntryPurpose::None;
        break;
      }
      delay(15);
    }
#endif
  }
  if (pendingTransition) {
    display::renderTransition(transitionToStage, ctx.creature.name);
    // Block here until acknowledged -- this is a rare, celebratory
    // one-time screen, not the normal interactive loop, so a plain wait is
    // simplest. Bounded by INACTIVITY_SLEEP_MS so walking away mid-screen
    // sleeps normally instead of burning battery forever; lastSeenStage was
    // already persisted in buildContext() (so power loss can't repeat this
    // forever), so walking away just means missing the reveal, not an
    // infinite loop -- a much smaller downside.
    uint32_t waitStart = millis();
    while (digitalRead(PIN_BTN_ENTER) != HIGH) {
      if (millis() - waitStart > INACTIVITY_SLEEP_MS) goToSleep();
      delay(15);
    }
    pendingTransition = false;
  }
  resolveWinterStashIfDue();
  showPendingMinigameUnlocks();
  renderCurrent();
  log_i("Mood: %s", creature::moodName(ctx.creature.mood));

  lastActivityMs = millis();
}

// Settings overlay button handling -- takes over LEFT/RIGHT/ENTER entirely
// while active (mirrors how Foraging already takes over RIGHT today).
static void handleSettingsInput() {
  const int n = display::SETTINGS_OPTION_COUNT;
  if (confirmPending) {
    if (pressed(bEnter)) {
      if (selectedOption == display::SETTINGS_RESET_GAME) {
        doResetGame();  // never returns -- esp_restart()
      } else {
        doPowerOff();  // never returns -- deep sleep with no wake source
      }
    }
    return;
  }
  if (pressed(bRight)) {
    selectedOption = (selectedOption + 1) % n;
    display::renderSettings(selectedOption, confirmPending, ctx.batteryPercent);
  }
  if (pressed(bLeft)) {
    selectedOption = (selectedOption + n - 1) % n;
    display::renderSettings(selectedOption, confirmPending, ctx.batteryPercent);
  }
  if (pressed(bEnter)) {
    if (selectedOption == display::SETTINGS_ACHIEVEMENTS) {
      inAchievements = true;
      display::renderAchievements(ctx);
    } else if (selectedOption == display::SETTINGS_WIFI) {
      inWifiMenu = true;
      wifiSelected = 0;
      wifiRemoveConfirm = false;
      display::renderWifiMenu(wifiSelected, false);
    } else {
      // Reset Game and Power Off are both destructive/hard-to-undo, so both
      // go through the same yes/no confirm sub-screen before acting.
      confirmPending = true;
      display::renderSettings(selectedOption, confirmPending, ctx.batteryPercent);
    }
  }
}

void loop() {
  if (tePurpose != TextEntryPurpose::None) {
    handleTextEntryInput();
    lastActivityMs = millis();
  } else if (pressed(bSettings)) {
    // SETTINGS/KEY1 is "back" at whatever depth we're currently at, one
    // level per press -- confirm sub-screen -> its parent menu -> Settings
    // itself -> exit Settings entirely. Pressing it from outside Settings
    // enters it fresh, same as before -- except mid-minigame, where the
    // run itself is the thing to back out of first.
    if (!inSettings && currentView == View::Minigames && mg.screen != minigames::Screen::Menu) {
      mg.screen = minigames::Screen::Menu;
      renderCurrent();
    } else if (!inSettings) {
      inSettings = true;
      selectedOption = 0;
      confirmPending = false;
      display::renderSettings(selectedOption, confirmPending, ctx.batteryPercent);
    } else if (inAchievements) {
      inAchievements = false;
      display::renderSettings(selectedOption, false, ctx.batteryPercent);
    } else if (inWifiMenu) {
      if (wifiRemoveConfirm) {
        wifiRemoveConfirm = false;
        display::renderWifiMenu(wifiSelected, false);
      } else {
        inWifiMenu = false;
        display::renderSettings(selectedOption, false, ctx.batteryPercent);
      }
    } else if (confirmPending) {
      confirmPending = false;
      display::renderSettings(selectedOption, confirmPending, ctx.batteryPercent);
    } else {
      inSettings = false;
      renderCurrent();
    }
    lastActivityMs = millis();
  } else if (inAchievements) {
    // Read-only wall -- LEFT/RIGHT/ENTER do nothing here, BACK is the only
    // way out (handled above). Still drain their edge state so a press held
    // through the transition doesn't fire once we're back in a menu.
    pressed(bLeft);
    pressed(bRight);
    pressed(bEnter);
  } else if (inWifiMenu) {
    handleWifiMenuInput();
    lastActivityMs = millis();
  } else if (inSettings) {
    handleSettingsInput();
    lastActivityMs = millis();
  } else if (currentView == View::Minigames) {
    if (handleMinigamesInput()) lastActivityMs = millis();
  } else {
    if (pressed(bLeft)) {
      retreatView();
      lastActivityMs = millis();
    }
    if (currentView == View::Foraging) {
      // Accelerating hold-to-scroll: a fresh press steps once immediately;
      // holding repeats with a shrinking interval down to a floor, and each
      // repeat's jump size grows (see forageHoldStepSize()) since the panel's
      // own redraw time -- not the repeat interval -- is what actually
      // bounds how fast this can go.
      bool rightDown = digitalRead(PIN_BTN_RIGHT) == HIGH;
      if (rightDown && !rightHeld) {
        rightHeld = true;
        rightHoldSteps = 0;
        advanceForageIdx();
        rightNextStepMs = millis() + RIGHT_HOLD_INITIAL_MS;
        lastActivityMs = millis();
      } else if (rightDown && rightHeld && millis() >= rightNextStepMs) {
        rightHoldSteps++;
        uint32_t interval = RIGHT_HOLD_INITIAL_MS > rightHoldSteps * RIGHT_HOLD_ACCEL_MS
                                ? RIGHT_HOLD_INITIAL_MS - rightHoldSteps * RIGHT_HOLD_ACCEL_MS
                                : RIGHT_HOLD_FLOOR_MS;
        if (interval < RIGHT_HOLD_FLOOR_MS) interval = RIGHT_HOLD_FLOOR_MS;
        advanceForageIdx(forageHoldStepSize(rightHoldSteps));
        rightNextStepMs = millis() + interval;
        lastActivityMs = millis();
      } else if (!rightDown) {
        rightHeld = false;
      }
      bRight.prev = rightDown;
    } else if (pressed(bRight)) {
      advanceView();
      lastActivityMs = millis();
    }
    if (pressed(bEnter)) {
      onEnter();
      lastActivityMs = millis();
    }
  }

#if DEV_MODE_NO_SLEEP
  static int lastLeft = -1, lastRight = -1, lastEnter = -1, lastSettings = -1;
  int left = digitalRead(PIN_BTN_LEFT);
  int right = digitalRead(PIN_BTN_RIGHT);
  int enter = digitalRead(PIN_BTN_ENTER);
  int settings = digitalRead(PIN_BTN_SETTINGS);
  if (left != lastLeft) {
    log_i("LEFT  (GPIO%d) = %d", PIN_BTN_LEFT, left);
    lastLeft = left;
  }
  if (right != lastRight) {
    log_i("RIGHT (GPIO%d) = %d", PIN_BTN_RIGHT, right);
    lastRight = right;
  }
  if (enter != lastEnter) {
    log_i("ENTER (GPIO%d) = %d", PIN_BTN_ENTER, enter);
    lastEnter = enter;
  }
  if (settings != lastSettings) {
    log_i("SETTINGS (GPIO%d) = %d (active-low: 0 = pressed)", PIN_BTN_SETTINGS, settings);
    lastSettings = settings;
  }
#endif

  // A run in progress gets the longer window (see MINIGAME_IDLE_SLEEP_MS):
  // its state is RAM-only, so sleeping mid-run throws the run away.
  bool inRun = currentView == View::Minigames && mg.screen != minigames::Screen::Menu;
  uint32_t idleWindow = inRun ? MINIGAME_IDLE_SLEEP_MS : INACTIVITY_SLEEP_MS;

  static bool loggedIdle = false;
  if (millis() - lastActivityMs > idleWindow) {
    if (!loggedIdle) {
      log_i("Idle; sleeping");
      loggedIdle = true;
    }
    goToSleep();
  } else {
    loggedIdle = false;
  }

  delay(15);
}
