#include <Arduino.h>
#include <Wire.h>
#include <BH1750.h>

#include "config.h"
#include "model.h"
#include "moon.h"
#include "net.h"
#include "foraging.h"
#include "creature.h"
#include "display.h"

static BH1750 lightMeter;

static AppContext ctx;
static View currentView = View::Main;
static uint32_t lastActivityMs = 0;
static uint8_t animFrame = 0;
static uint32_t lastAnimMs = 0;

struct Btn {
  int pin;
  bool prev;
  uint32_t lastEdge;
};
static Btn bLeft{PIN_BTN_LEFT, false, 0};
static Btn bRight{PIN_BTN_RIGHT, false, 0};
static Btn bEnter{PIN_BTN_ENTER, false, 0};

static void goToSleep() {
  display::hibernate();
  esp_sleep_enable_ext0_wakeup(PIN_PIR, 1);
  esp_deep_sleep_start();
}

static float readLux() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  if (!lightMeter.begin(BH1750::ONE_TIME_HIGH_RES_MODE)) {
    log_w("BH1750 not found; assuming lit");
    return 9999.0f;  // sensor failed: don't trap the device asleep
  }
  delay(180);
  return lightMeter.readLightLevel();
}

static void buildContext() {
  time_t nowUtc = time(nullptr);
  localtime_r(&nowUtc, &ctx.now);
  int month = ctx.now.tm_mon + 1;

  ctx.moon = moon::compute(nowUtc);
  ctx.featured = foraging::featured(month);
  ctx.secondary = foraging::secondary(month);

  creature::load(ctx.creature);
  creature::evaluate(ctx.creature, ctx.now, ctx.moon, ctx.weather);
  creature::save(ctx.creature);
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

static void cycleView(int dir) {
  int n = (int)View::COUNT;
  currentView = (View)(((int)currentView + dir + n) % n);
  display::renderView(currentView, ctx);
}

static void onEnter() {
  if (currentView == View::Status) {
    creature::feed(ctx.creature, time(nullptr));
    creature::evaluate(ctx.creature, ctx.now, ctx.moon, ctx.weather);
    creature::save(ctx.creature);
    display::flashFed(ctx);
    delay(900);
    display::renderView(View::Status, ctx);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_BTN_LEFT, INPUT_PULLDOWN);
  pinMode(PIN_BTN_RIGHT, INPUT_PULLDOWN);
  pinMode(PIN_BTN_ENTER, INPUT_PULLDOWN);
  pinMode(PIN_PIR, INPUT);

  float lux = readLux();
  if (lux < DARK_LUX_THRESHOLD) {
    log_i("Dark (%.0f lux); back to sleep", lux);
    goToSleep();
  }
  log_i("Woke, %.0f lux", lux);

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
  display::renderView(currentView, ctx);
  log_i("Mood: %s", creature::moodName(ctx.creature.mood));

  lastActivityMs = millis();
  lastAnimMs = millis();
}

void loop() {
  if (pressed(bLeft))  { cycleView(-1); lastActivityMs = millis(); }
  if (pressed(bRight)) { cycleView(+1); lastActivityMs = millis(); }
  if (pressed(bEnter)) { onEnter();     lastActivityMs = millis(); }

  if (millis() - lastAnimMs > ANIM_TICK_MS) {
    animFrame++;
    display::animateCreature(currentView, ctx, animFrame);
    lastAnimMs = millis();
  }

  if (millis() - lastActivityMs > INACTIVITY_SLEEP_MS) {
    log_i("Idle; sleeping");
    goToSleep();
  }

  delay(15);
}
