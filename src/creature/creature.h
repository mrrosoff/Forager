// creature.h — the forest spirit's state, persistence, and mood logic.
#pragma once

#include "model.h"

namespace creature {

// Load persisted state (hunger/happiness/lastFed) from NVS flash.
// Survives power loss via the inline switch; defaults to a fresh creature.
void load(CreatureState& s);

void save(const CreatureState& s);

/**
 * Recompute hunger from elapsed time and derive the current mood from the
 * full context (season + weather + hunger). Updates s in place and returns
 * the chosen mood.
 *
 * `stage` scales how fast everything decays: a Baby has only two of the five
 * minigames unlocked and a Juvenile three, so the bars they can't yet do
 * anything about drain more slowly (see decayScale()). Without that, a young
 * marmot is asked to maintain four bars with two tools.
 */
Mood evaluate(CreatureState& s, const struct tm& now, const WeatherData& weather, Stage stage);

/**
 * A resolved Discovery event feeds curiosity -- finding a new species is the
 * single most curiosity-shaped thing in the game, and it's the one source
 * available from birth, before either species minigame unlocks.
 */
void rewardDiscovery(CreatureState& s, time_t now);

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

// Which bar a minigame feeds. Every lethal stat has a game attached (see
// main.cpp's statForGame()), so no bar can run down with nothing the player
// can do about it.
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
void rewardMinigame(CreatureState& s, time_t now, int score, Stat stat);

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
