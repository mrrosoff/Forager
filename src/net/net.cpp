#include "net.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <sys/time.h>
#include <time.h>

#include "config.h"
#include "wifistore.h"

namespace net {

// Strongest known network in range, or -1 if none. Returns an index into
// wifistore, not the raw scan results.
static int pickStrongestKnown() {
  int n = WiFi.scanNetworks(/*async=*/false, /*show_hidden=*/false);
  if (n <= 0) return -1;

  int bestCred = -1;
  int32_t bestRssi = -1000;

  for (int i = 0; i < n; i++) {
    String foundSsid = WiFi.SSID(i);
    int32_t rssi = WiFi.RSSI(i);
    for (int c = 0; c < wifistore::count(); c++) {
      if (foundSsid == wifistore::at(c).ssid && rssi > bestRssi) {
        bestRssi = rssi;
        bestCred = c;
      }
    }
  }

  WiFi.scanDelete();
  return bestCred;
}

bool connectStrongest() {
  wifistore::load();
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);

  WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);

  // This board can't authenticate at the default 20dBm against any AP -- see
  // CLAUDE.md's hardware gotchas. esp_wifi_start() is load-bearing:
  // WiFi.mode() doesn't start the STA, so setTxPower() would silently no-op
  // and leave the power at 20dBm. The readback is logged because a 20.00 there
  // means this ordering broke, not that the network did.
  esp_wifi_start();
  delay(100);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  int8_t txPower = 0;
  esp_wifi_get_max_tx_power(&txPower);
  log_i("TX power reads back as %.2f dBm", txPower / 4.0);

  int idx = pickStrongestKnown();
  if (idx < 0) {
    log_w("No known WiFi networks in range");
    return false;
  }

  const wifistore::Network& net = wifistore::at(idx);
  log_i("Connecting to '%s' (strongest known)", net.ssid);
  WiFi.begin(net.ssid, net.password);

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start > WIFI_TIMEOUT_MS) {
      log_w("WiFi connect timed out");
      WiFi.disconnect(true);
      return false;
    }
    delay(100);
  }
  log_i("WiFi connected, IP %s", WiFi.localIP().toString().c_str());
  return true;
}

bool syncTime(struct tm& localNow) {
  configTzTime(TZ_SEATTLE, NTP_SERVER);

  // Wait for the clock to actually advance past a sane epoch (2021+).
  uint32_t start = millis();
  time_t now = 0;
  while ((now = time(nullptr)) < 1609459200) {  // 2021-01-01
    if (millis() - start > 8000) {
      log_w("NTP sync timed out");
      return false;
    }
    delay(100);
  }
  localtime_r(&now, &localNow);
  return true;
}

WeatherData fetchWeather() {
  WeatherData w{};
  w.valid = false;

  if (WiFi.status() != WL_CONNECTED) {
    log_w("Weather: not connected");
    return w;
  }

  HTTPClient http;
  http.setTimeout(8000);
  if (!http.begin(WEATHER_URL)) {
    log_w("Weather: http.begin failed for %s", WEATHER_URL);
    return w;
  }
  http.addHeader("User-Agent", "Forager/1.0");
  // wttr.in replies chunked, and getStream() hands the chunk-size lines
  // straight to the parser -- which yields an empty document and no parse
  // error, i.e. a silent no-op. HTTP/1.0 gets a plain Content-Length body.
  http.useHTTP10(true);

  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    log_w("Weather HTTP %d", code);
    http.end();
    return w;
  }

  // wttr.in j1 is large; filter to just the fields we use while streaming.
  JsonDocument filter;
  filter["current_condition"][0]["temp_C"] = true;
  filter["current_condition"][0]["weatherDesc"][0]["value"] = true;
  filter["current_condition"][0]["precipMM"] = true;
  filter["weather"][0]["hourly"][0]["precipMM"] = true;

  JsonDocument doc;
  DeserializationError err =
      deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
  http.end();

  if (err) {
    log_w("Weather JSON parse failed: %s", err.c_str());
    return w;
  }

  JsonObject cur = doc["current_condition"][0];
  if (cur.isNull()) {
    log_w("Weather: no current_condition (doc %u bytes, len %d)", (unsigned)doc.memoryUsage(),
          http.getSize());
    return w;
  }

  w.tempC = cur["temp_C"].as<float>();
  w.condition = cur["weatherDesc"][0]["value"] | "Unknown";

  float rain = cur["precipMM"].as<float>();
  for (JsonObject hr : doc["weather"][0]["hourly"].as<JsonArray>())
    rain += hr["precipMM"].as<float>();
  w.rainLast24hMm = rain;
  w.postRain = rain >= 1.0f;  // damp ground == good foraging

  w.valid = true;
  return w;
}

void shutdown() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

// Epoch the clock has to beat to count as NTP-set (2021-01-01), same check
// syncTime() waits on.
static const time_t kSaneEpoch = 1609459200;

bool clockUnset(time_t now) { return now < kSaneEpoch; }

void rememberClock() {
  time_t now = time(nullptr);
  if (clockUnset(now)) return;
  Preferences p;
  p.begin("forager", /*readOnly=*/false);
  p.putULong64("clockAt", (uint64_t)now);
  p.end();
}

void restoreClock() {
  Preferences p;
  p.begin("forager", /*readOnly=*/true);
  uint64_t saved = p.getULong64("clockAt", 0);
  p.end();
  time_t seed = saved > (uint64_t)kSaneEpoch ? (time_t)saved : kSaneEpoch;
  struct timeval tv = {};
  tv.tv_sec = seed;
  settimeofday(&tv, nullptr);
  log_i("Clock seeded from %s", saved ? "last known time" : "build floor");
}

// Is anything stored, and when? Kept separate from the payload so a miss
// costs one read instead of five NOT_FOUND lookups.
static bool cacheStamp(uint64_t& fetchedAt) {
  Preferences p;
  p.begin("forager", /*readOnly=*/true);
  bool stored = p.getBool("wxValid", false);
  fetchedAt = p.getULong64("wxAt", 0);
  p.end();
  return stored && fetchedAt != 0;
}

// A reading older than `withinHours` -- or one we can't date, because the
// clock is unset or has moved backwards -- doesn't describe `now`.
static bool cacheFresh(time_t now, uint64_t fetchedAt, uint32_t withinHours) {
  if (clockUnset(now) || (uint64_t)now < fetchedAt) return false;
  return (uint64_t)now - fetchedAt < (uint64_t)withinHours * 3600ULL;
}

WeatherData cachedWeather(time_t now) {
  WeatherData w{};
  uint64_t fetchedAt = 0;
  if (!cacheStamp(fetchedAt) || !cacheFresh(now, fetchedAt, WEATHER_USABLE_HOURS)) return w;

  Preferences p;
  p.begin("forager", /*readOnly=*/true);
  w.tempC = p.getFloat("wxTemp", 0.0f);
  w.rainLast24hMm = p.getFloat("wxRain", 0.0f);
  w.postRain = p.getBool("wxWet", false);
  char buf[48] = {0};
  p.getString("wxCond", buf, sizeof(buf));
  p.end();
  w.condition = buf;
  w.valid = true;
  return w;
}

void saveWeather(const WeatherData& w) {
  if (!w.valid) return;  // never overwrite good cached data with a failed fetch
  Preferences p;
  p.begin("forager", /*readOnly=*/false);
  p.putBool("wxValid", true);
  p.putFloat("wxTemp", w.tempC);
  p.putFloat("wxRain", w.rainLast24hMm);
  p.putBool("wxWet", w.postRain);
  p.putString("wxCond", w.condition.c_str());
  p.putULong64("wxAt", (uint64_t)time(nullptr));
  p.end();
}

bool refreshDue(time_t now) {
  uint64_t fetchedAt = 0;
  return !cacheStamp(fetchedAt) || !cacheFresh(now, fetchedAt, WEATHER_MAX_AGE_HOURS);
}

}  // namespace net
