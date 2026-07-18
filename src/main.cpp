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

SET_LOOP_TASK_STACK_SIZE(16 * 1024);

static AppContext ctx;
static View currentView = View::Main;
static int forageIdx = 0;  // Foraging view's browse position; session-only
static uint32_t lastActivityMs = 0;

struct Btn {
  int pin;
  bool prev;
  uint32_t lastEdge;
};
static Btn bLeft{PIN_BTN_LEFT, false, 0};
static Btn bRight{PIN_BTN_RIGHT, false, 0};
static Btn bEnter{PIN_BTN_ENTER, false, 0};
static Btn bSettings{PIN_BTN_SETTINGS, false, 0};

/**
 * Settings overlay state (see loop()) -- not part of the View cycle, since
 * it's triggered by its own dedicated button rather than LEFT/RIGHT paging.
 * selectedOption: 0 = Power Off, 1 = Reset Game.
 */
static bool inSettings = false;
static int selectedOption = 0;
static bool confirmPending = false;

// Accelerating hold-to-scroll state for RIGHT on the Foraging view (see
// loop()): a held press repeats faster the longer it's held, down to a
// floor, rather than a fixed step size.
static bool rightHeld = false;
static uint32_t rightNextStepMs = 0;
static int rightHoldSteps = 0;
static const uint32_t RIGHT_HOLD_INITIAL_MS = 450;
static const uint32_t RIGHT_HOLD_FLOOR_MS = 60;
static const uint32_t RIGHT_HOLD_ACCEL_MS = 40;

static void goToSleep() {
#if DEV_MODE_NO_SLEEP
  return;
#else
  // Always return to Main next wake, and show the sleeping marmot now --
  // the bistable e-ink panel keeps displaying whatever we draw here,
  // unpowered, until the next wake redraws it.
  currentView = View::Main;
  display::renderSleep();
  display::hibernate();
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_BTN_ENTER, 1);
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

  Stage stage = creature::computeStage(ctx.creature.birthDate, nowUtc);
  ctx.stage = (uint8_t)stage;

  // Spawn-check runs exactly once per wake, here -- there's no live
  // background timer since the device is asleep the rest of the time.
  events::PendingEvent ev = events::checkForEvent(nowUtc, month, stage);
  ctx.eventType = (uint8_t)ev.type;
  ctx.eventDataId = ev.dataId;
  ctx.eventExact = ev.exact ? 1 : 0;

  // Foraging browse order is relevance-ranked (season + rain) with per-wake
  // randomization, so it's not the same order every time either.
  foraging::rebuildBrowseOrder(month, ctx.weather.postRain);

  return firstBoot;
}

static bool pressed(Btn& b) {
  bool now = digitalRead(b.pin) == HIGH;
  bool fired = false;
  if (now && !b.prev && (millis() - b.lastEdge) > BTN_DEBOUNCE_MS) {
    fired = true;
    b.lastEdge = millis();
  }
  b.prev = now;
  return fired;
}

// LEFT/RIGHT step backward/forward through Achievements <-> Status <-> Main
// <-> Foraging, clamped at each end (no wraparound). Foraging is hidden
// during Baby stage (nothing to eat there could be resolved yet) -- treat
// Main as if it were the rightmost boundary in that case.
static bool viewReachable(View v) {
  if (v == View::Foraging && (Stage)ctx.stage == Stage::Baby) return false;
  return true;
}

static void retreatView() {
  if ((int)currentView <= 0) return;
  currentView = (View)((int)currentView - 1);
  display::renderView(currentView, ctx, forageIdx);
}

static void advanceView() {
  int n = (int)View::COUNT;
  if ((int)currentView >= n - 1) return;
  View next = (View)((int)currentView + 1);
  if (!viewReachable(next)) return;
  currentView = next;
  display::renderView(currentView, ctx, forageIdx);
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
        events::resolve(ev, ctx.creature, time(nullptr));
        creature::evaluate(ctx.creature, ctx.now, ctx.weather);
        creature::save(ctx.creature);
        ctx.eventType = (uint8_t)events::EventType::None;
        display::renderView(View::Main, ctx, forageIdx);
      }
      break;
    }
    case View::Foraging: {
      events::PendingEvent ev;
      ev.type = (events::EventType)ctx.eventType;
      ev.dataId = ctx.eventDataId;
      ev.exact = ctx.eventExact != 0;
      const Forageable& current = foraging::speciesAtRank(forageIdx);
      int month = ctx.now.tm_mon + 1;
      creature::feedForaged(ctx.creature, time(nullptr), foraging::inSeason(current, month));
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
// accelerating hold-to-scroll in loop().
static void advanceForageIdx() {
  int maxIdx = foraging::speciesCount() - 1;
  forageIdx = std::min(forageIdx + 1, maxIdx);
  display::renderView(View::Foraging, ctx, forageIdx);
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_BTN_LEFT, INPUT_PULLDOWN);
  pinMode(PIN_BTN_RIGHT, INPUT_PULLDOWN);
  pinMode(PIN_BTN_ENTER, INPUT_PULLDOWN);
  pinMode(PIN_BTN_SETTINGS, INPUT_PULLDOWN);
  log_i("Woke");

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
  if (firstBoot) display::renderBirth();
  display::renderView(currentView, ctx, forageIdx);
  log_i("Mood: %s", creature::moodName(ctx.creature.mood));

  lastActivityMs = millis();
}

// Settings overlay button handling -- takes over LEFT/RIGHT/ENTER entirely
// while active (mirrors how Foraging already takes over RIGHT today).
static void handleSettingsInput() {
  if (confirmPending) {
    if (pressed(bEnter)) {
      doResetGame();  // never returns -- esp_restart()
    }
    if (pressed(bLeft)) {
      confirmPending = false;
      display::renderSettings(selectedOption, confirmPending);
    }
    return;
  }
  if (pressed(bRight)) {
    selectedOption = (selectedOption + 1) % 2;
    display::renderSettings(selectedOption, confirmPending);
  }
  if (pressed(bEnter)) {
    if (selectedOption == 0) {
      doPowerOff();  // never returns -- deep sleep with no wake source
    } else {
      confirmPending = true;
      display::renderSettings(selectedOption, confirmPending);
    }
  }
  if (pressed(bLeft)) {
    inSettings = false;
    display::renderView(currentView, ctx, forageIdx);
  }
}

void loop() {
  if (pressed(bSettings) && !inSettings) {
    inSettings = true;
    selectedOption = 0;
    confirmPending = false;
    display::renderSettings(selectedOption, confirmPending);
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
      // holding repeats with a shrinking interval down to a floor.
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
        advanceForageIdx();
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
    log_i("SETTINGS (GPIO%d) = %d", PIN_BTN_SETTINGS, settings);
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
