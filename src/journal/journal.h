// journal.h — tracks which foraging species have been eaten at least once.
#pragma once

namespace journal {

/**
 * Pulls the eaten-species bitset from NVS into RAM. Call once per wake,
 * before any hasEaten()/markEaten() calls (mirrors creature::load()).
 */
void load();

// Persists the current bitset back to NVS. Call after markEaten().
void save();

// Marks a species (by its raw foraging::speciesAt() index) as eaten. Also
// marks it discovered, since eating something means you've found it.
void markEaten(int speciesIndex);

bool hasEaten(int speciesIndex);

// Total distinct species eaten so far, for the Status/Achievements views.
int totalEaten();

// A species only shows up in the Foraging browse list once discovered via
// a Discovery encounter (see events::EventType::Discovery) -- the full
// 250-species reference isn't handed over on day one.
void markDiscovered(int speciesIndex);

bool isDiscovered(int speciesIndex);

int totalDiscovered();

// Simple lifetime counters for the Achievements badges that aren't tied to
// a species bitset -- bumped once per resolved event of the matching kind
// (see main.cpp's onEnter()). Persisted alongside the bitsets.
void bumpAnimalSightings();
int totalAnimalSightings();

void bumpWeatherEvents();
int totalWeatherEvents();

// TrailMishap + TrailTreasure + MarmotEncounter combined -- "other events"
// as a single bucket rather than three thin badges.
void bumpOtherEvents();
int totalOtherEvents();

}  // namespace journal
