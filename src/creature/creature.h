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

const char* moodName(Mood m);

}  // namespace creature
