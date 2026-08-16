// model.h — shared data types passed between modules.
#pragma once

#include <Arduino.h>
#include <time.h>

#include <string>

struct WeatherData {
  bool valid;  // false if fetch/parse failed
  float tempC;
  float rainLast24hMm;
  bool postRain;  // recently wet == good foraging
  std::string condition;
};

// Rough PNW foraging region so the same monthly calendar can span very
// different terrain -- lowland/urban forest, high mountain/subalpine,
// and the marine shoreline/tidepool zone.
enum class Biome : uint8_t {
  Forest = 0,  // lowland forest, river bottoms, urban edges
  Mountain,    // subalpine/alpine slopes, high conifer country
  Coast,       // beaches, tidepools, estuaries, dunes
};

struct Forageable {
  const char* name;
  const char* kind;  // "mushroom" / "green" / "berry" / "flower" / "seaweed"
  const char* note;
  const char* caution;     // edibility/safety warning; "" if none
  const char* harvestTip;  // practical how-to-find/pick note
  uint16_t monthMask;      // bit (1 << (month-1)) set for each active month
  Biome biome;
};

enum class Mood : uint8_t {
  Excited,
  Content,
  Sleepy,
  Hungry,
  Annoyed,
  Dormant,  // deep winter
  Scared,   // transient reaction to a bad encounter, not a persistent state
};

/**
 * Real-elapsed-time-since-birth growth stage, recomputed once per wake (see
 * creature::computeStage()) -- Foraging is hidden until Juvenile, and the
 * marmot's art pool grows with it (see drawCreature() in display.cpp).
 */
enum class Stage : uint8_t { Baby, Juvenile, Adult };

/**
 * Which bar bottomed out (see creature::checkDeath()) -- None means still
 * alive. Drives which flavor-text pool the death screen picks from (see
 * display::renderDeath()) so the ending reflects what actually killed the
 * marmot instead of always saying the same generic line.
 */
enum class DeathCause : uint8_t { None, Starved, Heartbroken, Exhausted };

struct CreatureState {
  Mood mood;
  uint8_t hunger;     // 0 = full, 100 = starving
  uint8_t happiness;  // 0..100
  uint8_t energy;     // 0..100
  // 0..100, and the one bar that is NOT lethal -- "has anything interesting
  // happened lately". Status adds a transient weather bonus at render time
  // (curiosityDisplay()); only this base value is persisted.
  uint8_t curiosity;

  // Epochs, 0 = never. hunger ramps from lastFed; happiness/energy clamp
  // against lastPlayed (feeding or resolving an event); curiosity against
  // lastCurious (a species game or a Discovery), so a marmot can be fed and
  // played with and still have seen nothing new.
  time_t lastFed;
  time_t lastPlayed;
  time_t lastCurious;
  time_t birthDate;

  // Consecutive days fed at least once; lastStreakDay is day-truncated.
  uint16_t feedStreakDays;
  time_t lastStreakDay;

  // Last stage the player has acknowledged -- compared against the freshly
  // computed stage each wake to fire the "grew up" screen once.
  uint8_t lastSeenStage;

  char name[16];  // textentry caps input to fit this; blank defaults to "Marmot"

  /**
   * True once naming actually completed. Distinct from birthDate != 0, which
   * is persisted before naming so growth timing starts at the real creation
   * moment -- keying firstBoot off birthDate would send a power-loss
   * mid-naming straight to Main with a default name.
   */
  bool named = false;
};

/**
 * Numeric order is what LEFT(-1)/RIGHT(+1) step through. Minigames is
 * leftmost rather than appended after Foraging for two reasons: Foraging
 * consumes RIGHT entirely for hold-to-scroll, so nothing can sit past it,
 * and being leftmost frees up LEFT (dead at the end of the cycle) for the
 * game menu's own selection stepping.
 */
enum class View : uint8_t { Minigames = 0, Status, Main, Foraging, COUNT };

struct AppContext {
  struct tm now;  // local time
  WeatherData weather;
  Forageable featured;
  CreatureState creature;

  // Pending event, mirroring events::EventType (0 = None). Raw fields rather
  // than an events:: type so model.h doesn't depend on that module -- events.h
  // already depends on this one.
  uint8_t eventType = 0;
  uint8_t eventDataId = 0;
  uint8_t eventExact = 0;

  // Both recomputed once per wake in buildContext(), neither persisted.
  uint8_t stage = 0;  // mirrors Stage
  // BATT_PERCENT_UNKNOWN when the sense line looks broken.
  uint8_t batteryPercent = 0;
};

static const uint8_t BATT_PERCENT_UNKNOWN = 255;
