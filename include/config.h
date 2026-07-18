// config.h — hardware pin map and behavioral tunables. Secrets live in secrets.h.
#pragma once

#include <Arduino.h>

// Match your panel: newer GDEY042T81 (fast partial refresh) vs older GDEW042T2.
#define EPD_PANEL_GDEY042T81 1

static const int PIN_EPD_SCK  = 12;
static const int PIN_EPD_MOSI = 11;
static const int PIN_EPD_CS   = 10;
static const int PIN_EPD_DC   = 9;
static const int PIN_EPD_RST  = 8;
static const int PIN_EPD_BUSY = 7;

static const int EPD_WIDTH  = 400;
static const int EPD_HEIGHT = 300;

static const int PIN_I2C_SDA = 5;
static const int PIN_I2C_SCL = 6;

static const gpio_num_t PIN_PIR = GPIO_NUM_4;  // deep-sleep wake source

// Buttons wired to 3V3 through the switch, INPUT_PULLDOWN: pressed == HIGH.
static const int PIN_BTN_LEFT  = 1;
static const int PIN_BTN_RIGHT = 2;
static const int PIN_BTN_ENTER = 3;

static const float DARK_LUX_THRESHOLD = 10.0f;
static const uint32_t INACTIVITY_SLEEP_MS = 60UL * 1000UL;
static const uint32_t BTN_DEBOUNCE_MS = 40;
static const uint32_t ANIM_TICK_MS = 1500;
static const uint32_t WIFI_TIMEOUT_MS = 12UL * 1000UL;

static const char* const NTP_SERVER = "pool.ntp.org";
static const char* const TZ_SEATTLE = "PST8PDT,M3.2.0,M11.1.0";
static const char* const WEATHER_URL = "https://wttr.in/Seattle?format=j1";

static const uint32_t HUNGER_PERIOD_HOURS = 72;
