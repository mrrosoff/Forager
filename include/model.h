// model.h — shared data types passed between modules.
#pragma once

#include <Arduino.h>
#include <time.h>

struct MoonInfo {
  float phase;  // 0..1 (0/1 = new, 0.5 = full)
  uint8_t illumPct;
  const char* name;
  int daysToFull;
  int daysToNew;
  bool isFull;
  bool isNew;
};

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
  uint8_t spriteId;
  Biome biome;
};

enum class Mood : uint8_t {
  Excited,
  Content,
  Sleepy,
  Hungry,
  Annoyed,
  Glowing,  // full moon
  Dormant,  // deep winter
};

struct CreatureState {
  Mood mood;
  uint8_t hunger;     // 0 = full, 100 = starving
  uint8_t happiness;  // 0..100
  time_t lastFed;     // epoch, 0 = never
};

enum class View : uint8_t { Main = 0, Foraging, Status, COUNT };

struct AppContext {
  struct tm now;  // local time
  MoonInfo moon;
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
};
