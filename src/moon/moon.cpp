#include "moon.h"
#include <math.h>

namespace moon {

// Mean synodic month length (new moon to new moon), days.
static const double SYNODIC = 29.53058867;

// Reference new moon: 2000-01-06 18:14 UTC == Julian Day 2451550.1.
static const double KNOWN_NEW_JD = 2451550.1;

static double toJulian(time_t utc) {
  // Unix epoch (1970-01-01) is Julian Day 2440587.5.
  return (double)utc / 86400.0 + 2440587.5;
}

static const char* phaseName(double age) {
  // age in days since new moon, 0..SYNODIC.
  if (age < 1.0)               return "New Moon";
  if (age < SYNODIC / 4 - 1)   return "Waxing Crescent";
  if (age < SYNODIC / 4 + 1)   return "First Quarter";
  if (age < SYNODIC / 2 - 1)   return "Waxing Gibbous";
  if (age < SYNODIC / 2 + 1)   return "Full Moon";
  if (age < 3 * SYNODIC / 4 - 1) return "Waning Gibbous";
  if (age < 3 * SYNODIC / 4 + 1) return "Last Quarter";
  if (age < SYNODIC - 1)       return "Waning Crescent";
  return "New Moon";
}

MoonInfo compute(time_t utc) {
  MoonInfo m{};

  double jd = toJulian(utc);
  double daysSinceNew = jd - KNOWN_NEW_JD;
  double age = fmod(daysSinceNew, SYNODIC);
  if (age < 0) age += SYNODIC;

  double frac = age / SYNODIC;            // 0..1
  m.phase = (float)frac;

  // Illuminated fraction: (1 - cos(2*pi*frac)) / 2.
  double illum = (1.0 - cos(2.0 * M_PI * frac)) / 2.0;
  m.illumPct = (uint8_t)lround(illum * 100.0);

  m.name = phaseName(age);

  // Days to next full (age == SYNODIC/2) and next new (age == 0/SYNODIC).
  double toFull = (SYNODIC / 2.0) - age;
  if (toFull < 0) toFull += SYNODIC;
  double toNew = SYNODIC - age;
  if (toNew >= SYNODIC) toNew -= SYNODIC;

  m.daysToFull = (int)lround(toFull);
  m.daysToNew  = (int)lround(toNew);

  m.isFull = fabs(age - SYNODIC / 2.0) < 1.0;
  m.isNew  = (age < 1.0) || (age > SYNODIC - 1.0);

  return m;
}

}  // namespace moon
