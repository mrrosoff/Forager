// creature.h — the forest spirit's state, persistence, and mood logic.
#pragma once

#include "model.h"

namespace creature {

// Load persisted state (hunger/happiness/lastFed) from NVS flash.
// Survives power loss via the inline switch; defaults to a fresh creature.
void load(CreatureState& s);

void save(const CreatureState& s);

// Recompute hunger from elapsed time and derive the current mood from the
// full context (season + weather + hunger). Updates s in place and returns
// the chosen mood.
Mood evaluate(CreatureState& s, const struct tm& now, const WeatherData& weather);

// Eating a species on the Foraging view feeds the creature directly --
// this is the player-driven counterpart to events::resolve()'s ForagingFind
// bonus (which stacks on top when the eaten species also happens to match
// a pending find). inSeason gives a small extra happiness bump.
void feedForaged(CreatureState& s, time_t now, bool inSeason);

/**
 * Growth stage from real elapsed time since birth (see BABY_STAGE_DAYS /
 * JUVENILE_STAGE_DAYS in config.h). birthDate == 0 (never born) is treated
 * as Baby -- callers detect the birth-sequence case separately by checking
 * birthDate directly, this just needs to not divide by a negative duration.
 */
Stage computeStage(time_t birthDate, time_t now);

/**
 * Tracks/applies the neglect consequence (see NEGLECT_* in config.h):
 * updates s.neglectSince based on whether hunger/happiness are both
 * currently in neglect territory, and returns true once that's been
 * continuously true for NEGLECT_DAYS -- the marmot has wandered off for
 * good. Callers should treat a true return as terminal (show a "ran away"
 * screen, then reset) rather than something to recover from in place.
 */
bool updateNeglect(CreatureState& s, time_t now);

const char* moodName(Mood m);

}  // namespace creature
