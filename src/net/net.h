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

/**
 * Last fetched weather, out of NVS -- weather is cached rather than
 * re-fetched every wake, so a wake costs no radio at all (see refreshDue()).
 * Returns valid==false if nothing was ever stored, if the reading is older
 * than WEATHER_USABLE_HOURS, or if the clock can't date it: stale conditions
 * are worse than none, so callers fall back to having no weather.
 */
WeatherData cachedWeather(time_t now);

void saveWeather(const WeatherData& w);

// True if the cache is missing or older than WEATHER_MAX_AGE_HOURS. Checked
// before sleeping, not at wake, so the radio never delays a button press.
bool refreshDue(time_t now);

// True if the system clock reads as never-set (i.e. an unsynced 1970 epoch).
bool clockUnset(time_t now);

// Stores the current time so a power-cycle can pick up near where it left
// off. No-op while the clock is unset.
void rememberClock();

/**
 * Seeds the system clock from rememberClock()'s last value, or from a sane
 * build-era floor if there isn't one. The result is behind reality by however
 * long the device was off, but it is never 1970 -- which matters because the
 * epoch-0 case corrupts anything time-derived (hunger freezes on the "never
 * fed" sentinel, the feed streak counts every feed as a new day). Corrected
 * properly at the next sync.
 */
void restoreClock();

}  // namespace net
