// events.h — periodic wake-time interactions (animal sightings, foraging
// finds, trail mishaps, weather events) that wait for the user to resolve
// them. There is no live background timer: the device is asleep almost
// always, so a spawn check only ever runs once, right after waking, against
// how much time has passed since the last one.
#pragma once

#include "model.h"

namespace events {

enum class EventType : uint8_t {
  None,
  AnimalSighting,
  ForagingFind,
  TrailMishap,
  WeatherEvent,
  // Baby-stage only: simple, always-positive care moments (naps, first
  // steps, snuggles) resolved on Main via ENTER like AnimalSighting --
  // never rolled once Foraging unlocks (Juvenile+).
  BabyCare,
};

struct PendingEvent {
  EventType type = EventType::None;
  uint8_t dataId = 0;
  // ForagingFind only: true if dataId names one exact required species
  // instead of just a kind category (rarer, harder version of the event).
  bool exact = false;
};

/**
 * Call once per wake, after time is known. Returns any already-pending
 * (unresolved) event, or rolls a chance to spawn a new one if the cooldown
 * since the last resolved event has elapsed. stage gates which event types
 * can be rolled: Baby excludes ForagingFind (Foraging is hidden, so it
 * could never be resolved) and TrailMishap (doesn't fit a baby-at-home
 * narrative), rolling BabyCare instead.
 */
PendingEvent checkForEvent(time_t now, int month, Stage stage);

// Short header shown at the top of the encounter view, e.g. "SIGHTING".
const char* eventTitle(EventType type, bool negative);
// The animal/category/mishap/weather name. For ForagingFind: the required
// kind (e.g. "Mushroom") normally, or the exact species name when
// ev.exact is set -- see eventCategory()/eventMatchesSpecies().
const char* eventName(const PendingEvent& ev);
// One-line flavor note.
const char* eventNote(const PendingEvent& ev);
// For ForagingFind only: the Forageable::kind required when not exact.
// Empty string for other event types or exact-species finds.
const char* eventCategory(const PendingEvent& ev);
// True if eating `f` on the Foraging view satisfies this ForagingFind --
// exact-species finds require a name match, category finds a kind match.
// Always false for non-ForagingFind events.
bool eventMatchesSpecies(const PendingEvent& ev, const Forageable& f);
// True for a predator sighting, a mishap, or bad weather -- drives a worse
// mood/expression instead of an excited one.
bool eventIsNegative(const PendingEvent& ev);

// For AnimalSighting only: the index into the animal table (0-based, stable
// across builds -- matches kAnimals[] declaration order in events.cpp). Lets
// the display layer look up per-animal artwork without events.cpp needing to
// know anything about bitmaps. Meaningless for other event types.
uint8_t animalIndex(const PendingEvent& ev);

// Applies this event's stat effect to the creature, clears the pending
// event, and stamps the cooldown so the next one waits its turn.
void resolve(const PendingEvent& ev, CreatureState& creature, time_t now);

}  // namespace events
