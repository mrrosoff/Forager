// config.h — hardware pin map and behavioral tunables. Secrets live in secrets.h.
#pragma once

#include <Arduino.h>

// Match your panel: newer GDEY042T81 (fast partial refresh) vs older GDEW042T2.
// Currently unused by the epd_official driver (src/epd_official/), which only
// supports the GDEY042T81 revision; kept as documentation of the assumption.
#define EPD_PANEL_GDEY042T81 1

// Dev-mode: disable deep sleep entirely so the board stays on USB serial
// while iterating. Flip to 0 before shipping.
#define DEV_MODE_NO_SLEEP 0

static const int PIN_EPD_SCK = 12;
static const int PIN_EPD_MOSI = 11;
static const int PIN_EPD_CS = 10;
static const int PIN_EPD_DC = 9;
static const int PIN_EPD_RST = 8;
static const int PIN_EPD_BUSY = 7;

static const int EPD_WIDTH = 400;
static const int EPD_HEIGHT = 300;

// Buttons wired to 3V3 through the switch, INPUT_PULLDOWN: pressed == HIGH.
// No PIR/light sensor on this board -- ENTER is the sole wake source, so it
// must sit on an RTC-capable GPIO (0-21 on the S3; ext0/ext1 deep-sleep wake
// doesn't work on any other pin). It avoids GPIO3 (a strapping pin) and
// GPIO11-20 (ADC2 -- shares hardware with the WiFi/BT radio and reads
// flaky/self-oscillating once that radio block has ever been powered, even
// after WiFi.mode(WIFI_OFF); confirmed by reproducing the oscillation on
// GPIO13 twice). GPIO4 is RTC-capable, ADC1 (unaffected by the radio
// conflict), and free now that there's no PIR -- ENTER lives there so
// esp_sleep_enable_ext0_wakeup() can use it directly. LEFT/RIGHT stay on
// GPIO1/2 (also ADC1, proven reliable) for in-session bidirectional
// navigation (clamped at each end, no wraparound).
static const int PIN_BTN_LEFT = 1;
static const int PIN_BTN_RIGHT = 2;
static const int PIN_BTN_ENTER = 4;

/**
 * The Waveshare display board's onboard "KEY0" button, provisionally wired
 * to GPIO5 (ADC1, not a strapping pin, so it's safe by the same criteria as
 * LEFT/RIGHT/ENTER above) -- CONFIRM against the actual board silkscreen
 * before flashing, this was not fully verified against hardware.
 */
static const int PIN_BTN_SETTINGS = 5;

static const uint32_t INACTIVITY_SLEEP_MS = 60UL * 1000UL;
static const uint32_t BTN_DEBOUNCE_MS = 40;
static const uint32_t WIFI_TIMEOUT_MS = 12UL * 1000UL;

// Waveshare's own datasheet recommends refreshing the panel at least once
// every 24h even with zero interaction, to avoid image burn-in from an
// extremely long-static display. ENTER is the primary wake source; this
// timer is a backstop for when nobody presses it.
static const uint64_t FORCE_REFRESH_INTERVAL_US = 24ULL * 60 * 60 * 1000000ULL;

static const char* const NTP_SERVER = "pool.ntp.org";
static const char* const TZ_SEATTLE = "PST8PDT,M3.2.0,M11.1.0";
static const char* const WEATHER_URL = "https://wttr.in/Seattle?format=j1";

static const uint32_t HUNGER_PERIOD_HOURS = 72;

/**
 * Growth-stage thresholds (real elapsed days since birth, see
 * creature::computeStage()): Baby for the first BABY_STAGE_DAYS, Juvenile
 * through JUVENILE_STAGE_DAYS, Adult after that.
 */
static const uint32_t BABY_STAGE_DAYS = 2;
static const uint32_t JUVENILE_STAGE_DAYS = 7;
