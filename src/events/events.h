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
};

struct PendingEvent {
  EventType type = EventType::None;
  uint8_t dataId = 0;
};

// Call once per wake, after time is known. Returns any already-pending
// (unresolved) event, or rolls a chance to spawn a new one if the cooldown
// since the last resolved event has elapsed.
PendingEvent checkForEvent(time_t now, int month);

// Short header shown at the top of the encounter view, e.g. "SIGHTING".
const char* eventTitle(EventType type, bool negative);
// The animal/category/mishap/weather name (ForagingFind shows the required
// category, e.g. "Mushroom", not a specific species -- see eventCategory()).
const char* eventName(const PendingEvent& ev);
// One-line flavor note.
const char* eventNote(const PendingEvent& ev);
// For ForagingFind only: the Forageable::kind required to resolve it (e.g.
// "mushroom") -- browse to any species of that kind on the Foraging view and
// press ENTER there to deliver it. Empty string for other event types.
const char* eventCategory(const PendingEvent& ev);
// True for a predator sighting, a mishap, or bad weather -- drives a worse
// mood/expression instead of an excited one.
bool eventIsNegative(const PendingEvent& ev);

// Applies this event's stat effect to the creature, clears the pending
// event, and stamps the cooldown so the next one waits its turn.
void resolve(const PendingEvent& ev, CreatureState& creature, time_t now);

}  // namespace events
