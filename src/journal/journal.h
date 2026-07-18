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

// Marks a species (by its raw foraging::speciesAt() index) as eaten.
void markEaten(int speciesIndex);

bool hasEaten(int speciesIndex);

// Total distinct species eaten so far, for the Status/Achievements views.
int totalEaten();

}  // namespace journal
