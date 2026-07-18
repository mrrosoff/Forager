#include "creature.h"

#include <Preferences.h>

#include <algorithm>

#include "config.h"

namespace creature {

static const char* NVS_NS = "forager";

void load(CreatureState& s) {
  Preferences p;
  p.begin(NVS_NS, /*readOnly=*/true);
  s.hunger = p.getUChar("hunger", 30);
  s.happiness = p.getUChar("happy", 60);
  s.lastFed = (time_t)p.getULong64("lastFed", 0);
  s.birthDate = (time_t)p.getULong64("birthDate", 0);
  s.feedStreakDays = p.getUShort("streak", 0);
  s.lastStreakDay = (time_t)p.getULong64("streakDay", 0);
  p.end();
  s.mood = Mood::Content;  // recomputed by evaluate()
}

void save(const CreatureState& s) {
  Preferences p;
  p.begin(NVS_NS, /*readOnly=*/false);
  p.putUChar("hunger", s.hunger);
  p.putUChar("happy", s.happiness);
  p.putULong64("lastFed", (uint64_t)s.lastFed);
  p.putULong64("birthDate", (uint64_t)s.birthDate);
  p.putUShort("streak", s.feedStreakDays);
  p.putULong64("streakDay", (uint64_t)s.lastStreakDay);
  p.end();
}

// Recompute hunger as a 0..100 ramp over HUNGER_PERIOD_HOURS since last fed.
static void agingHunger(CreatureState& s, time_t now) {
  if (s.lastFed == 0 || now <= s.lastFed) return;
  double hrs = (double)(now - s.lastFed) / 3600.0;
  double frac = hrs / (double)HUNGER_PERIOD_HOURS;
  frac = std::max(0.0, std::min(1.0, frac));
  s.hunger = (uint8_t)(frac * 100.0);

  // Happiness drifts toward neutral as hunger climbs; very hungry = unhappy.
  if (s.hunger > 70 && s.happiness > 25) s.happiness -= 1;
}

Mood evaluate(CreatureState& s, const struct tm& now, const WeatherData& weather) {
  agingHunger(s, mktime(const_cast<struct tm*>(&now)));

  int month = now.tm_mon + 1;  // 1..12
  int hour = now.tm_hour;
  bool deepWinter = (month == 12 || month == 1 || month == 2);
  bool cold = weather.valid && weather.tempC <= 4.0f;
  bool night = (hour < 7) || (hour >= 21);
  bool goodForaging = weather.valid && weather.postRain;

  Mood m;
  if (deepWinter && cold && s.hunger < 70) {
    m = Mood::Dormant;  // hibernating through the cold dark
  } else if (s.hunger >= 70) {
    m = Mood::Hungry;
  } else if (s.happiness < 20) {
    m = Mood::Annoyed;  // neglected too long
  } else if (goodForaging || s.hunger < 20 || s.happiness >= 80) {
    m = Mood::Excited;  // fresh rain or just fed = let's go
  } else if (night) {
    m = Mood::Sleepy;
  } else {
    m = Mood::Content;
  }

  s.mood = m;
  return m;
}

// Whole-day index (not calendar-local, just a consistent 24h bucket) --
// good enough for a streak counter, no need for exact local-midnight math.
static int64_t dayIndex(time_t t) { return (int64_t)t / 86400; }

void feedForaged(CreatureState& s, time_t now, bool inSeason) {
  s.hunger = s.hunger > 25 ? s.hunger - 25 : 0;
  int h = (int)s.happiness + (inSeason ? 15 : 10);
  s.happiness = (uint8_t)(h > 100 ? 100 : h);
  s.lastFed = now;

  int64_t today = dayIndex(now);
  int64_t lastDay = dayIndex(s.lastStreakDay);
  if (s.lastStreakDay == 0 || today - lastDay == 1) {
    s.feedStreakDays++;
    s.lastStreakDay = now;
  } else if (today - lastDay > 1) {
    s.feedStreakDays = 1;
    s.lastStreakDay = now;
  }
  // today - lastDay == 0 (already fed today): streak unchanged, but still
  // bump lastStreakDay so later feeds today don't re-trigger the >1 branch.
  else {
    s.lastStreakDay = now;
  }
}

/**
 * Growth stage from real elapsed time since birth (see BABY_STAGE_DAYS /
 * JUVENILE_STAGE_DAYS in config.h).
 */
Stage computeStage(time_t birthDate, time_t now) {
  if (birthDate == 0 || now <= birthDate) return Stage::Baby;
  double days = (double)(now - birthDate) / 86400.0;
  if (days < (double)BABY_STAGE_DAYS) return Stage::Baby;
  if (days < (double)JUVENILE_STAGE_DAYS) return Stage::Juvenile;
  return Stage::Adult;
}

const char* moodName(Mood m) {
  switch (m) {
    case Mood::Excited:
      return "excited";
    case Mood::Content:
      return "content";
    case Mood::Sleepy:
      return "sleepy";
    case Mood::Hungry:
      return "hungry";
    case Mood::Annoyed:
      return "annoyed";
    case Mood::Dormant:
      return "dormant";
  }
  return "content";
}

}  // namespace creature
