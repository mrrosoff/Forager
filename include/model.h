// model.h — shared data types passed between modules.
#pragma once

#include <Arduino.h>
#include <time.h>

struct WeatherData {
  bool valid;  // false if fetch/parse failed
  float tempC;
  float rainLast24hMm;
  bool postRain;  // recently wet == good foraging
  char condition[40];
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
  uint8_t energy;     // 0..100 -- decays like happiness, refilled by feeding
                       // (more so by protein/fat-rich food kinds; see
                       // creature::feedEffectForKind())
  time_t lastFed;      // epoch, 0 = never

  /**
   * Epoch of the last "play" interaction -- feeding or resolving a wake-time
   * event -- distinct from lastFed. Drives a happiness decay independent of
   * hunger (see creature::evaluate()): you can keep the marmot fed and
   * still neglect it by never resolving events.
   */
  time_t lastPlayed;

  time_t birthDate;  // epoch, 0 = never born yet (first-ever boot sentinel)

  /**
   * Consecutive days fed at least once, for the Status/Achievements streak
   * display. lastStreakDay is day-truncated (see creature::feedForaged()).
   */
  uint16_t feedStreakDays;
  time_t lastStreakDay;

  /**
   * Mirrors Stage -- the last growth stage the player has acknowledged (see
   * main.cpp's transition-screen check). Lets a new wake detect "the marmot
   * just grew up" by comparing this against the freshly computed stage,
   * without needing a separate persisted flag.
   */
  uint8_t lastSeenStage;

  // Chosen at birth via the on-screen text entry (see textentry.h);
  // defaults to "Marmot" if the player leaves it blank.
  char name[16];
};

/**
 * Achievements sits LEFT of Status, Status LEFT of Main, Foraging RIGHT of
 * Main -- the numeric order here is what LEFT(-1)/RIGHT(+1) step through in
 * main.cpp. Achievements goes leftmost (not appended after Foraging) because
 * Foraging's RIGHT button is fully consumed by accelerating hold-to-scroll,
 * so RIGHT can never reach a view past it.
 */
enum class View : uint8_t { Achievements = 0, Status, Main, Foraging, COUNT };

struct AppContext {
  struct tm now;  // local time
  WeatherData weather;
  Forageable featured;
  CreatureState creature;
  bool netOk;

  // A pending interaction event (see src/events/), shown as a takeover of
  // the Main view until ENTER resolves it. eventType mirrors
  // events::EventType (0 = None); kept as raw fields here (rather than an
  // events:: type) so model.h doesn't depend on that module -- events.h
  // itself depends on model.h for CreatureState.
  uint8_t eventType = 0;
  uint8_t eventDataId = 0;
  uint8_t eventExact = 0;

  /**
   * Mirrors Stage; computed once per wake in main.cpp's buildContext() from
   * creature.birthDate, and used to gate Foraging visibility and the
   * baby-appropriate event pool.
   */
  uint8_t stage = 0;
};
