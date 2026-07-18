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

/**
 * How much a food kind ("mushroom", "berry", "shellfish", ...) tops up
 * happiness/energy on top of the base hunger fill -- sweet/treat kinds
 * (berry, sap, nut) lean toward happiness, protein/hearty kinds (shellfish,
 * crab, shrimp, snail, chiton, urchin) lean toward energy, and plain greens
 * give a smaller amount of both. Exposed so main.cpp's onEnter() doesn't
 * need to duplicate this table when it's just showing an eat-effect preview.
 */
struct FeedEffect {
  uint8_t happinessBoost;
  uint8_t energyBoost;
};
FeedEffect feedEffectForKind(const char* kind);

// Eating a species on the Foraging view feeds the creature directly --
// this is the player-driven counterpart to events::resolve()'s ForagingFind
// bonus (which stacks on top when the eaten species also happens to match
// a pending find). inSeason gives a small extra happiness bump; kind
// selects the FeedEffect (see above) applied to happiness/energy.
void feedForaged(CreatureState& s, time_t now, bool inSeason, const char* kind);

/**
 * Growth stage from distinct species foraged so far (journal::totalEaten(),
 * passed in rather than read directly so this module doesn't need to depend
 * on journal.h) -- see BABY_STAGE_SPECIES / ADULT_STAGE_SPECIES in config.h.
 */
Stage computeStage(int speciesEaten);

/**
 * DeathCause::None unless a bar has bottomed out (see DEATH_* in
 * config.h): hunger at DEATH_HUNGER_THRESHOLD (fully starved), or
 * happiness/energy down at DEATH_HAPPINESS_THRESHOLD/DEATH_ENERGY_THRESHOLD.
 * All three only reach those extremes after multiple days of zero feeding
 * (hunger ramps over HUNGER_PERIOD_HOURS, happiness/energy decay over
 * PLAY_PERIOD_HOURS/ENERGY_PERIOD_HOURS -- all a full week), so this is a
 * real-neglect consequence, not a one-bad-wake gotcha. Hunger is checked
 * first since it's the most "final" of the three; callers should treat a
 * non-None return as terminal (show a death screen, then reset) rather
 * than something to recover from in place.
 */
DeathCause checkDeath(const CreatureState& s);

const char* moodName(Mood m);

}  // namespace creature
