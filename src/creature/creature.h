// creature.h — the forest spirit's state, persistence, and mood logic.
#pragma once

#include "model.h"

namespace creature {

void load(CreatureState& s);
void save(const CreatureState& s);

/**
 * Ages every bar and derives the mood from season + weather + hunger,
 * updating s in place. `stage` scales the decay: a Baby has two of the five
 * games unlocked and a Juvenile three, so bars they can't yet act on drain
 * more slowly (decayScale()).
 */
Mood evaluate(CreatureState& s, const struct tm& now, const WeatherData& weather, Stage stage);

// Curiosity's only source available from birth, before either species game.
void rewardDiscovery(CreatureState& s, time_t now);

// What a food kind tops up beyond hunger: berry/sap/nut lean happiness,
// shellfish/crab/shrimp/snail/chiton/urchin lean energy, greens give a little
// of both. Exposed so onEnter()'s eat-preview doesn't duplicate the table.
struct FeedEffect {
  uint8_t happinessBoost;
  uint8_t energyBoost;
};
FeedEffect feedEffectForKind(const char* kind);

// Eating on the Foraging view. The player-driven counterpart to
// events::resolve()'s ForagingFind bonus, which stacks when the eaten species
// also satisfies a pending find.
void feedForaged(CreatureState& s, time_t now, bool inSeason, const char* kind);

// Which bar a game feeds -- every lethal stat has one, so no bar can run down
// with nothing the player can do about it.
enum class Stat : uint8_t { Hunger, Happiness, Energy, Curiosity };

/**
 * Finishing a minigame run counts as attention.
 *
 * `lastPlayed` is reset on EVERY finished run, score or no score -- that's
 * what lifts the happiness and energy decay ceilings (see evaluate()), so
 * simply sitting down to play is enough to hold off both. On top of that, a
 * run with a non-zero score tops up the one bar `stat` names, scaled to the
 * score and capped below what a single feed gives.
 *
 * Callers pass score 0 to get the lastPlayed reset without the top-up --
 * main.cpp does that after the first scoring run of a wake, so a good run
 * can't be replayed for unlimited stat.
 */
void rewardMinigame(CreatureState& s, time_t now, int score, Stat stat, Stage stage);

/**
 * Moves hunger by `deltaPoints` (positive = hungrier), durably. Hunger is
 * derived, not stored: evaluate() recomputes it from `lastFed` (see
 * agingHunger()), so assigning to `s.hunger` is discarded on the next
 * evaluate() -- shifting `lastFed`, as this does, is what sticks. The ramp is
 * stage-scaled, hence `stage`.
 */
void shiftHunger(CreatureState& s, time_t now, int deltaPoints, Stage stage);

// Species count passed in rather than read from journal.h, so this module
// doesn't depend on it.
Stage computeStage(int speciesEaten);

// None unless a bar has bottomed out. Only reachable after days of zero
// interaction, so a non-None return is terminal: show the death screen and
// reset, don't try to recover in place.
DeathCause checkDeath(const CreatureState& s);

const char* moodName(Mood m);

}  // namespace creature
