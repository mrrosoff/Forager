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

// GPIO numbers, not the XIAO's silkscreen D-numbers (D8 is GPIO7). DC/BUSY
// take GPIO43/44, the only pads that are neither RTC nor ADC.
static const int PIN_EPD_SCK = 7;    // yellow  (pad D8)
static const int PIN_EPD_MOSI = 9;   // blue    (pad D10, "DIN")
static const int PIN_EPD_CS = 8;     // orange  (pad D9)
static const int PIN_EPD_DC = 43;    // green   (pad D6)
static const int PIN_EPD_RST = 4;    // white   (pad D3)
static const int PIN_EPD_BUSY = 44;  // purple  (pad D7)

static const int EPD_WIDTH = 400;
static const int EPD_HEIGHT = 300;

// Wired to 3V3 through the switch, INPUT_PULLDOWN: pressed == HIGH. All three
// wake from deep sleep, so all three need RTC-capable GPIOs (0-21 on the S3).
static const int PIN_BTN_LEFT = 1;   // pad D0
static const int PIN_BTN_ENTER = 2;  // pad D1
// GPIO3 is a strapping pin, safe here: INPUT_PULLDOWN sits low at reset.
static const int PIN_BTN_RIGHT = 3;  // pad D2

// The display board's onboard KEY1 (the one the enclosure cuts a hole for),
// wired switch-to-GND: INPUT_PULLUP, active LOW, unlike the other three.
static const int PIN_BTN_SETTINGS = 5;  // pad D4

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

/**
 * Below this the device parks itself: it draws a "charge me" screen and deep
 * sleeps with no wake source, so nothing can wake it to drain further. Only
 * charging revives it.
 *
 * Deliberately well above a LiPo's ~3.0V damage threshold, because the ADC
 * reads a little low through the 400k divider and it's better to park early
 * than to run a cell flat. Doing that once is what preceded the first board's
 * battery path failing.
 */
static const float BATT_CUTOFF_VOLTS = 3.40f;

// Below this the sense line is assumed broken rather than the cell flat --
// the chip browns out long before a cell reads this low, so running code
// can't really be seeing it. A floating GPIO6 (the D5 wire came off once)
// looks identical to a dead battery otherwise.
static const float BATT_SENSE_FAULT_VOLTS = 2.60f;

// How often a device parked on a flat cell wakes to re-read the battery.
static const uint64_t BATT_RECHECK_INTERVAL_US = 10ULL * 60 * 1000000ULL;

// Window to get LEFT+RIGHT held together when waking from parked. Generous on
// purpose -- it's a tolerance for pressing them slightly apart, not a reflex test.
static const uint32_t POWER_COMBO_WINDOW_MS = 1500;

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
