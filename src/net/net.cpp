#include "net.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <time.h>

#include "config.h"
#include "secrets.h"

namespace net {

// Strongest known network in range, or -1 if none. Returns index into WIFI_NETWORKS.
static int pickStrongestKnown() {
  int n = WiFi.scanNetworks(/*async=*/false, /*show_hidden=*/false);
  if (n <= 0) return -1;

  int bestCred = -1;
  int32_t bestRssi = -1000;

  for (int i = 0; i < n; i++) {
    String foundSsid = WiFi.SSID(i);
    int32_t rssi = WiFi.RSSI(i);
    for (size_t c = 0; c < WIFI_NETWORK_COUNT; c++) {
      if (foundSsid == WIFI_NETWORKS[c].ssid && rssi > bestRssi) {
        bestRssi = rssi;
        bestCred = (int)c;
      }
    }
  }

  WiFi.scanDelete();
  return bestCred;
}

bool connectStrongest() {
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);

  int idx = pickStrongestKnown();
  if (idx < 0) {
    log_w("No known WiFi networks in range");
    return false;
  }

  log_i("Connecting to '%s' (strongest known)", WIFI_NETWORKS[idx].ssid);
  WiFi.begin(WIFI_NETWORKS[idx].ssid, WIFI_NETWORKS[idx].password);

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

  if (WiFi.status() != WL_CONNECTED) return w;

  HTTPClient http;
  http.setTimeout(8000);
  if (!http.begin(WEATHER_URL)) return w;
  http.addHeader("User-Agent", "Forager/1.0");

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
  if (cur.isNull()) return w;

  w.tempC = cur["temp_C"].as<float>();
  const char* desc = cur["weatherDesc"][0]["value"] | "Unknown";
  strncpy(w.condition, desc, sizeof(w.condition) - 1);
  w.condition[sizeof(w.condition) - 1] = '\0';

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

}  // namespace net
