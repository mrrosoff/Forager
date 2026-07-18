#include <Arduino.h>
#include <Preferences.h>

#include <algorithm>

#include "config.h"
#include "creature.h"
#include "display.h"
#include "events.h"
#include "foraging.h"
#include "journal.h"
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
 * selectedOption: 0 = WiFi Networks, 1 = Reset Game, 2 = Power Off.
 */
static bool inSettings = false;
static int selectedOption = 0;
static bool confirmPending = false;

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
static char pendingWifiSsid[32];  // holds the SSID while its password is typed next

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
  uint64_t wakeMask =
      (1ULL << PIN_BTN_LEFT) | (1ULL << PIN_BTN_RIGHT) | (1ULL << PIN_BTN_ENTER);
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

// Returns true the very first time this ever runs (birthDate == 0, no prior
// save) -- setup() uses this to show the one-time birth sequence. Sets
// birthDate immediately so a later wake never re-triggers it.
static bool buildContext() {
  time_t nowUtc = time(nullptr);
  localtime_r(&nowUtc, &ctx.now);
  int month = ctx.now.tm_mon + 1;

  ctx.featured = foraging::featured(month);

  creature::load(ctx.creature);
  bool firstBoot = (ctx.creature.birthDate == 0);
  if (firstBoot) ctx.creature.birthDate = nowUtc;

  creature::evaluate(ctx.creature, ctx.now, ctx.weather);
  creature::save(ctx.creature);

  journal::load();

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

// LEFT/RIGHT step backward/forward through Achievements <-> Status <-> Main
// <-> Foraging, clamped at each end (no wraparound). Foraging is reachable
// from birth now (growth is gated by species foraged, so a Baby has to be
// able to forage). Achievements is hidden entirely until Adult (see
// renderAchievements()'s old "Locked" message, replaced by just not
// navigating there at all).
static bool viewReachable(View v) {
  if (v == View::Achievements && (Stage)ctx.stage != Stage::Adult) return false;
  return true;
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
  if (!viewReachable(prev)) return;
  if (currentView == View::Foraging) forageIdx = DEV_MODE_SPECIES_BROWSE_START;
  currentView = prev;
  bool spawned = bumpScreenChangeAndMaybeSpawn();
  display::renderView(currentView, ctx, forageIdx, spawned);
}

static void advanceView() {
  int n = (int)View::COUNT;
  if ((int)currentView >= n - 1) return;
  View next = (View)((int)currentView + 1);
  if (!viewReachable(next)) return;
  if (next == View::Foraging) forageIdx = DEV_MODE_SPECIES_BROWSE_START;
  currentView = next;
  bool spawned = bumpScreenChangeAndMaybeSpawn();
  display::renderView(currentView, ctx, forageIdx, spawned);
}

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
        creature::evaluate(ctx.creature, ctx.now, ctx.weather);
        creature::save(ctx.creature);
        ctx.eventType = (uint8_t)events::EventType::None;
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
      creature::feedForaged(ctx.creature, time(nullptr), foraging::inSeason(current, month), current.kind);
      journal::markEaten(foraging::indexAtRank(forageIdx));
      journal::save();
      if (events::eventMatchesSpecies(ev, current)) {
        events::resolve(ev, ctx.creature, time(nullptr));
        ctx.eventType = (uint8_t)events::EventType::None;
      }
      creature::evaluate(ctx.creature, ctx.now, ctx.weather);
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

static void renderCurrentTextEntry() { display::renderTextEntry(textEntryPrompt(tePurpose), teState); }

// Begins the shared text-entry overlay for one purpose. initial pre-fills
// the buffer (e.g. re-showing an in-progress SSID isn't needed today, but
// keeps the helper general).
static void startTextEntry(TextEntryPurpose purpose, const char* initial) {
  tePurpose = purpose;
  textentry::init(teState, initial);
  renderCurrentTextEntry();
}

// Applies whatever DONE-triggered action a given purpose implies once the
// picker commits. Each case decides what happens next -- another text-entry
// step (SSID -> password), returning to a menu, or just resuming the view.
static void finishTextEntry() {
  switch (tePurpose) {
    case TextEntryPurpose::MarmotName: {
      const char* name = teState.buffer[0] ? teState.buffer : "Marmot";
      strncpy(ctx.creature.name, name, sizeof(ctx.creature.name) - 1);
      ctx.creature.name[sizeof(ctx.creature.name) - 1] = '\0';
      creature::save(ctx.creature);
      tePurpose = TextEntryPurpose::None;
      display::renderView(currentView, ctx, forageIdx);
      break;
    }
    case TextEntryPurpose::WifiSsid: {
      if (!teState.buffer[0]) {
        // Blank SSID -- treat as cancelling out of "Add Network" rather
        // than prompting for a password nobody's going to use.
        tePurpose = TextEntryPurpose::None;
        display::renderWifiMenu(wifiSelected, false);
        break;
      }
      strncpy(pendingWifiSsid, teState.buffer, sizeof(pendingWifiSsid) - 1);
      pendingWifiSsid[sizeof(pendingWifiSsid) - 1] = '\0';
      startTextEntry(TextEntryPurpose::WifiPassword, "");
      break;
    }
    case TextEntryPurpose::WifiPassword: {
      wifistore::add(pendingWifiSsid, teState.buffer);
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
        creature::save(ctx.creature);
        tePurpose = TextEntryPurpose::None;
        break;
      }
      delay(15);
    }
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
  display::renderView(currentView, ctx, forageIdx);
  log_i("Mood: %s", creature::moodName(ctx.creature.mood));

  lastActivityMs = millis();
}

// Settings overlay button handling -- takes over LEFT/RIGHT/ENTER entirely
// while active (mirrors how Foraging already takes over RIGHT today).
static void handleSettingsInput() {
  if (confirmPending) {
    if (pressed(bEnter)) {
      if (selectedOption == 1) {
        doResetGame();  // never returns -- esp_restart()
      } else {
        doPowerOff();  // never returns -- deep sleep with no wake source
      }
    }
    return;
  }
  if (pressed(bRight)) {
    selectedOption = (selectedOption + 1) % 3;
    display::renderSettings(selectedOption, confirmPending);
  }
  if (pressed(bLeft)) {
    selectedOption = (selectedOption + 2) % 3;
    display::renderSettings(selectedOption, confirmPending);
  }
  if (pressed(bEnter)) {
    if (selectedOption == 0) {
      inWifiMenu = true;
      wifiSelected = 0;
      wifiRemoveConfirm = false;
      display::renderWifiMenu(wifiSelected, false);
    } else {
      // Reset Game and Power Off are both destructive/hard-to-undo, so both
      // go through the same yes/no confirm sub-screen before acting.
      confirmPending = true;
      display::renderSettings(selectedOption, confirmPending);
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
    // enters it fresh, same as before.
    if (!inSettings) {
      inSettings = true;
      selectedOption = 0;
      confirmPending = false;
      display::renderSettings(selectedOption, confirmPending);
    } else if (inWifiMenu) {
      if (wifiRemoveConfirm) {
        wifiRemoveConfirm = false;
        display::renderWifiMenu(wifiSelected, false);
      } else {
        inWifiMenu = false;
        display::renderSettings(selectedOption, false);
      }
    } else if (confirmPending) {
      confirmPending = false;
      display::renderSettings(selectedOption, confirmPending);
    } else {
      inSettings = false;
      display::renderView(currentView, ctx, forageIdx);
    }
    lastActivityMs = millis();
  } else if (inWifiMenu) {
    handleWifiMenuInput();
    lastActivityMs = millis();
  } else if (inSettings) {
    handleSettingsInput();
    lastActivityMs = millis();
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

  static bool loggedIdle = false;
  if (millis() - lastActivityMs > INACTIVITY_SLEEP_MS) {
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
