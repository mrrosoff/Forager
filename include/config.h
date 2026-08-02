// config.h — hardware pin map and behavioral tunables. Secrets live in secrets.h.
#pragma once

#include <Arduino.h>

// Match your panel: newer GDEY042T81 (fast partial refresh) vs older GDEW042T2.
// Currently unused by the epd_official driver (src/epd_official/), which only
// supports the GDEY042T81 revision; kept as documentation of the assumption.
#define EPD_PANEL_GDEY042T81 1

// Dev flags -- all ship at 0. CLAUDE.md's "Dev-mode flags" section covers
// what each one does to the rest of the system, including the surprises
// (SKIP_GROWTH also unlocks Adult-only events; UNLOCK_MINIGAMES doesn't
// touch the unlock reveals; EVENT_CYCLE strips the real setup()/loop()).
#define DEV_MODE_NO_SLEEP 0            // never deep-sleep; stay on USB serial
#define DEV_MODE_SKIP_GROWTH 0         // computeStage() always returns Adult
#define DEV_MODE_UNLOCK_MINIGAMES 0    // every game playable, ignoring the ladder
#define DEV_MODE_SHOW_ALL_CONTENT 0    // deal every bush/card instead of rolling
#define DEV_MODE_UNLOCK_SPECIES 0      // every species discovered, for browsing
#define DEV_MODE_ALPHABETIZE_BROWSE 0  // browse A-Z instead of by relevance
#define DEV_MODE_SPECIES_BROWSE_START 0  // 0-based start index for that browse
#define DEV_MODE_EVENT_CYCLE 0         // loop every event screen for proofreading

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

/**
 * Battery sense -- a 200k/200k divider from BAT+ to GND, tapped into GPIO6
 * (ADC1, so unaffected by the radio conflict that rules out GPIO11-20; not a
 * strapping pin). Halves the cell, so 4.2V full sits at 2.1V, inside the
 * ADC's usable range.
 *
 * 400k total also means ~100k of source impedance at the tap, well above the
 * ~10k the ADC wants -- readBatteryVolts() oversamples to compensate. The
 * voltage-to-percent curve lives there too, since a LiPo is nowhere near
 * linear.
 */
static const int PIN_BATT_ADC = 6;
static const float BATT_DIVIDER_RATIO = 2.0f;

static const uint32_t INACTIVITY_SLEEP_MS = 120UL * 1000UL;

// Longer window mid-run: run state is RAM-only, so sleeping over a 7x7 maze
// throws the run away. Only applies between a run's first and last press.
static const uint32_t MINIGAME_IDLE_SLEEP_MS = 300UL * 1000UL;

// Younger than this at winter and the stash screen is skipped entirely, not
// just its penalty (that's minigames::WINTER_GRACE_DAYS) -- a marmot born
// days ago shouldn't open with a "you failed to stockpile" ceremony.
static const int WINTER_TOO_YOUNG_DAYS = 7;
static const uint32_t BTN_DEBOUNCE_MS = 40;
static const uint32_t WIFI_TIMEOUT_MS = 12UL * 1000UL;

// Waveshare's own datasheet recommends refreshing the panel at least once
// every 24h even with zero interaction, to avoid image burn-in from an
// extremely long-static display. ENTER is the primary wake source; this
// timer is a backstop for when nobody presses it.
static const uint64_t FORCE_REFRESH_INTERVAL_US = 24ULL * 60 * 60 * 1000000ULL;

// Cache older than this triggers a refresh (before sleep, never at wake).
static const uint32_t WEATHER_MAX_AGE_HOURS = 6;
// Older than this and it stops being used at all -- a stale postRain would
// keep boosting mushroom relevance days after the ground dried.
static const uint32_t WEATHER_USABLE_HOURS = 24;

static const char* const NTP_SERVER = "pool.ntp.org";
static const char* const TZ_SEATTLE = "PST8PDT,M3.2.0,M11.1.0";
static const char* const WEATHER_URL = "https://wttr.in/Seattle?format=j1";

// Decay clocks. All 1 week except curiosity, which is slower because it
// isn't lethal and shouldn't nag. Hunger ramps from lastFed; the other three
// are ceiling-clamps from lastPlayed/lastCurious (see creature::evaluate()).
// A week is the outer edge of the death timeline, not a "gets hungry" ramp,
// hence the length.
static const uint32_t HUNGER_PERIOD_HOURS = 168;
static const uint32_t PLAY_PERIOD_HOURS = 168;
static const uint32_t ENERGY_PERIOD_HOURS = 168;
static const uint32_t CURIOSITY_PERIOD_HOURS = 240;

// Growth is distinct species eaten (journal::totalEaten()), not elapsed time.
static const int BABY_STAGE_SPECIES = 5;
static const int ADULT_STAGE_SPECIES = 15;

// Any one bar bottoming out kills the marmot, checked every wake rather than
// on a separate neglect clock. Curiosity is deliberately absent.
static const uint8_t DEATH_HUNGER_THRESHOLD = 100;
static const uint8_t DEATH_HAPPINESS_THRESHOLD = 0;
static const uint8_t DEATH_ENERGY_THRESHOLD = 0;

// Mid-session event trigger: this many view changes inside the window
// guarantee an event, unless one resolved within the recency gate. Lets an
// actively-browsing session earn one instead of waiting on idle odds.
static const int SCREEN_CHANGE_EVENT_TRIGGER = 8;
static const uint32_t SCREEN_CHANGE_WINDOW_MS = 90UL * 1000UL;
static const uint32_t EVENT_RECENCY_GATE_SECONDS = 180;
