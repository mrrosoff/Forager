// config.h — hardware pin map and behavioral tunables. Secrets live in secrets.h.
#pragma once

#include <Arduino.h>

// Match your panel: newer GDEY042T81 (fast partial refresh) vs older GDEW042T2.
// Currently unused by the epd_official driver (src/epd_official/), which only
// supports the GDEY042T81 revision; kept as documentation of the assumption.
#define EPD_PANEL_GDEY042T81 1

// Dev-mode: disable deep sleep entirely so the board stays on USB serial
// while iterating. Flip to 0 before shipping.
#define DEV_MODE_NO_SLEEP 1

// Dev-mode: creature::computeStage() always returns Adult, skipping the
// real-time Baby/Juvenile wait so Adult-only content (Foraging,
// Achievements, the full adult pose pool) can be tested immediately. Flip
// to 0 before shipping.
#define DEV_MODE_SKIP_GROWTH 0

// Dev-mode: setup() shows every wake-time event's encounter screen
// back-to-back in a loop instead of the normal wake flow -- one
// representative AnimalSighting/Discovery/ForagingFind plus every distinct
// entry in the small curated pools (mishaps, weather, treasures,
// encounters, baby care), so the flavor text/layout can all be proofread
// on real hardware. ENTER advances to the next one. Flip to 0 before
// shipping.
#define DEV_MODE_EVENT_CYCLE 0

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

static const uint32_t INACTIVITY_SLEEP_MS = 120UL * 1000UL;
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

// 1 week -- see DEATH_* below: this is the outer edge of the death timeline,
// not a "gets hungry" ramp on its own, so it needs to be genuinely long.
static const uint32_t HUNGER_PERIOD_HOURS = 168;

/**
 * Happiness decays toward 0 the longer it's been since the marmot was last
 * "played with" (fed, or had a wake-time event resolved) -- see
 * creature::evaluate()'s boredom-ceiling clamp. Separate from hunger: you
 * can keep the marmot fed and still neglect it by never resolving events.
 * 1 week, same reasoning as HUNGER_PERIOD_HOURS.
 */
static const uint32_t PLAY_PERIOD_HOURS = 168;

/**
 * Growth-stage thresholds (distinct species foraged -- journal::totalEaten()
 * -- not elapsed time; see creature::computeStage()): Baby until
 * BABY_STAGE_SPECIES eaten, Juvenile until ADULT_STAGE_SPECIES eaten, Adult
 * after that. Foraging is unlocked from birth so a Baby can actually reach
 * the first threshold.
 */
static const int BABY_STAGE_SPECIES = 5;
static const int ADULT_STAGE_SPECIES = 15;

/**
 * Energy decays toward 0 the longer it's been since the marmot was last
 * "played with" (same trigger as happiness's boredom clock -- see
 * creature::evaluate()'s ceiling clamp) -- feeding restores it, with
 * protein/fat-rich food kinds (see creature::feedEffectForKind()) restoring
 * more than a plain green does. 1 week, same reasoning as
 * HUNGER_PERIOD_HOURS.
 */
static const uint32_t ENERGY_PERIOD_HOURS = 168;

/**
 * Death thresholds (see creature::checkDeath()): if hunger reaches
 * DEATH_HUNGER_THRESHOLD (fully starved), or happiness or energy drops to
 * DEATH_*_THRESHOLD (utterly miserable/exhausted), the marmot dies on the
 * spot -- checked every wake, not a multi-day timer, so it's a direct
 * consequence of a bar actually bottoming out rather than a separate
 * neglect clock. Feeding restores all three (see feedForaged()), so normal
 * play never approaches these -- only sustained neglect does.
 */
static const uint8_t DEATH_HUNGER_THRESHOLD = 100;
static const uint8_t DEATH_HAPPINESS_THRESHOLD = 0;
static const uint8_t DEATH_ENERGY_THRESHOLD = 0;

/**
 * Mid-session event trigger (see main.cpp's screen-change-count hook,
 * separate from events::checkForEvent()'s once-per-wake background roll):
 * SCREEN_CHANGE_EVENT_TRIGGER view changes (LEFT/RIGHT navigation, not
 * Foraging's within-view species scroll) within SCREEN_CHANGE_WINDOW_MS of
 * each other guarantee a new event, as long as one hasn't already been
 * resolved in the last EVENT_RECENCY_GATE_SECONDS -- an actively-browsing
 * session earns an event rather than waiting on the same odds as an idle
 * device.
 */
static const int SCREEN_CHANGE_EVENT_TRIGGER = 8;
static const uint32_t SCREEN_CHANGE_WINDOW_MS = 90UL * 1000UL;
static const uint32_t EVENT_RECENCY_GATE_SECONDS = 180;
