// net.h — WiFi connection, time sync (NTP), and weather fetch.
#pragma once

#include "model.h"

namespace net {

// Scan for known networks and connect to the strongest in-range one.
// Returns true on success. Blocks up to WIFI_TIMEOUT_MS.
bool connectStrongest();

// Sync the system clock from NTP (requires an active connection).
// Returns true once time is valid. Fills out `localNow` with local time.
bool syncTime(struct tm& localNow);

// Fetch + parse current weather for Seattle. Returns a struct with
// valid==false on any failure (caller should fall back to cached data).
WeatherData fetchWeather();

// Tear down WiFi to save power before sleeping.
void shutdown();

}  // namespace net
