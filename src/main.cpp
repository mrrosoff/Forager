#include <Arduino.h>

#include "config.h"
#include "creature.h"
#include "display.h"
#include "events.h"
#include "foraging.h"
#include "model.h"
#include "moon.h"
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
static Btn bRight{PIN_BTN_RIGHT, false, 0};
static Btn bEnter{PIN_BTN_ENTER, false, 0};

static void goToSleep() {
#if DEV_MODE_NO_SLEEP
  return;
#else
  // Always return to Main before sleeping, so the bistable e-ink panel
  // shows the home screen (not whatever view was last open) until next
  // woken, and the next wake starts from Main too.
  currentView = View::Main;
  display::renderView(currentView, ctx, forageIdx);
  display::hibernate();
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_BTN_ENTER, 1);
  esp_sleep_enable_timer_wakeup(FORCE_REFRESH_INTERVAL_US);
  esp_deep_sleep_start();
#endif
}

static void buildContext() {
  time_t nowUtc = time(nullptr);
  localtime_r(&nowUtc, &ctx.now);
  int month = ctx.now.tm_mon + 1;

  ctx.moon = moon::compute(nowUtc);
  ctx.featured = foraging::featured(month);

  creature::load(ctx.creature);
  creature::evaluate(ctx.creature, ctx.now, ctx.moon, ctx.weather);
  creature::save(ctx.creature);

  // Spawn-check runs exactly once per wake, here -- there's no live
  // background timer since the device is asleep the rest of the time.
  events::PendingEvent ev = events::checkForEvent(nowUtc, month);
  ctx.eventType = (uint8_t)ev.type;
  ctx.eventDataId = ev.dataId;
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

// RIGHT moves forward through Main -> Foraging -> Status with no wrapping;
// there is no LEFT/back -- pressing RIGHT on the last view does nothing.
static void advanceView() {
  int n = (int)View::COUNT;
  if ((int)currentView >= n - 1) return;
  currentView = (View)((int)currentView + 1);
  display::renderView(currentView, ctx, forageIdx);
}

// ENTER's action depends on the current view: resolve a pending sighting or
// (failing that) do nothing on Main, feed on Status, page through species on
// Foraging (does not wrap -- there's no LEFT to page back, and forageIdx
// resets to 0 on the next sleep/wake anyway).
static void onEnter() {
  switch (currentView) {
    case View::Main: {
      events::PendingEvent ev;
      ev.type = (events::EventType)ctx.eventType;
      ev.dataId = ctx.eventDataId;
      if (ev.type != events::EventType::None) {
        events::resolve(ev, ctx.creature, time(nullptr));
        creature::evaluate(ctx.creature, ctx.now, ctx.moon, ctx.weather);
        creature::save(ctx.creature);
        ctx.eventType = (uint8_t)events::EventType::None;
        display::renderView(View::Main, ctx, forageIdx);
      }
      break;
    }
    case View::Status:
      creature::feed(ctx.creature, time(nullptr));
      creature::evaluate(ctx.creature, ctx.now, ctx.moon, ctx.weather);
      creature::save(ctx.creature);
      display::renderView(View::Status, ctx, forageIdx);
      break;
    case View::Foraging:
      if (forageIdx < foraging::speciesCount() - 1) {
        forageIdx++;
        display::renderView(View::Foraging, ctx, forageIdx);
      }
      break;
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_BTN_RIGHT, INPUT_PULLDOWN);
  pinMode(PIN_BTN_ENTER, INPUT_PULLDOWN);
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

  buildContext();

  display::begin();
  display::renderView(currentView, ctx, forageIdx);
  log_i("Mood: %s", creature::moodName(ctx.creature.mood));

  lastActivityMs = millis();
}

void loop() {
  if (pressed(bRight)) {
    advanceView();
    lastActivityMs = millis();
  }
  if (pressed(bEnter)) {
    onEnter();
    lastActivityMs = millis();
  }

#if DEV_MODE_NO_SLEEP
  static int lastRight = -1, lastEnter = -1;
  int right = digitalRead(PIN_BTN_RIGHT);
  int enter = digitalRead(PIN_BTN_ENTER);
  if (right != lastRight) {
    log_i("RIGHT (GPIO%d) = %d", PIN_BTN_RIGHT, right);
    lastRight = right;
  }
  if (enter != lastEnter) {
    log_i("ENTER (GPIO%d) = %d", PIN_BTN_ENTER, enter);
    lastEnter = enter;
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
