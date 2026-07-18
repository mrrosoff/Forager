// model.h — shared data types passed between modules.
#pragma once

#include <Arduino.h>
#include <time.h>

struct MoonInfo {
  float       phase;      // 0..1 (0/1 = new, 0.5 = full)
  uint8_t     illumPct;
  const char* name;
  int         daysToFull;
  int         daysToNew;
  bool        isFull;
  bool        isNew;
};

struct WeatherData {
  bool  valid;            // false if fetch/parse failed
  float tempC;
  float rainLast24hMm;
  bool  postRain;         // recently wet == good foraging
  char  condition[40];
};

struct Forageable {
  const char* name;
  const char* kind;       // "mushroom" / "green" / "berry" / "flower"
  const char* note;
  uint8_t     spriteId;
};

enum class Mood : uint8_t {
  Excited, Content, Sleepy, Hungry,
  Glowing,   // full moon
  Dormant,   // deep winter
};

struct CreatureState {
  Mood    mood;
  uint8_t hunger;         // 0 = full, 100 = starving
  uint8_t happiness;      // 0..100
  time_t  lastFed;        // epoch, 0 = never
};

enum class View : uint8_t { Main = 0, Moon, Foraging, Status, COUNT };

struct AppContext {
  struct tm     now;      // local time
  MoonInfo      moon;
  WeatherData   weather;
  Forageable    featured;
  Forageable    secondary;
  CreatureState creature;
  bool          netOk;
};
