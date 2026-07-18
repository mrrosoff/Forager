#include "journal.h"

#include <Preferences.h>

namespace journal {

static const char* NVS_NS = "forager";

// 256 bits -- comfortably covers the current 250-species database with a
// little headroom; bump if the species count ever grows past that.
static const int kBytes = 32;
static uint8_t bits[kBytes];
static uint8_t discoveredBits[kBytes];

static int countBits(const uint8_t* arr) {
  int count = 0;
  for (int i = 0; i < kBytes; i++) {
    uint8_t b = arr[i];
    while (b) {
      count += b & 1;
      b >>= 1;
    }
  }
  return count;
}

void load() {
  Preferences p;
  p.begin(NVS_NS, /*readOnly=*/true);
  size_t got = p.getBytes("eaten", bits, kBytes);
  if (got != (size_t)kBytes) {
    for (int i = 0; i < kBytes; i++) bits[i] = 0;
  }
  size_t gotDiscovered = p.getBytes("discovered", discoveredBits, kBytes);
  if (gotDiscovered != (size_t)kBytes) {
    for (int i = 0; i < kBytes; i++) discoveredBits[i] = 0;
  }
  // Migration safety: anything already eaten (from before discovery-gating
  // existed) counts as discovered too, so an upgrade doesn't suddenly hide
  // species a player had already found.
  for (int i = 0; i < kBytes; i++) discoveredBits[i] |= bits[i];
  p.end();
}

void save() {
  Preferences p;
  p.begin(NVS_NS, /*readOnly=*/false);
  p.putBytes("eaten", bits, kBytes);
  p.putBytes("discovered", discoveredBits, kBytes);
  p.end();
}

void markDiscovered(int speciesIndex) {
  if (speciesIndex < 0 || speciesIndex >= kBytes * 8) return;
  discoveredBits[speciesIndex / 8] |= (uint8_t)(1 << (speciesIndex % 8));
}

bool isDiscovered(int speciesIndex) {
  if (speciesIndex < 0 || speciesIndex >= kBytes * 8) return false;
  return (discoveredBits[speciesIndex / 8] & (1 << (speciesIndex % 8))) != 0;
}

int totalDiscovered() { return countBits(discoveredBits); }

void markEaten(int speciesIndex) {
  if (speciesIndex < 0 || speciesIndex >= kBytes * 8) return;
  bits[speciesIndex / 8] |= (uint8_t)(1 << (speciesIndex % 8));
  markDiscovered(speciesIndex);
}

bool hasEaten(int speciesIndex) {
  if (speciesIndex < 0 || speciesIndex >= kBytes * 8) return false;
  return (bits[speciesIndex / 8] & (1 << (speciesIndex % 8))) != 0;
}

int totalEaten() { return countBits(bits); }

// Self-contained NVS read-modify-write, unlike the bitsets above -- these
// are bumped once per resolved event (infrequent), so there's no need to
// batch them into the caller's save() the way markEaten()/markDiscovered()
// do for the higher-frequency per-species bits.
static void bumpCounter(const char* key) {
  Preferences p;
  p.begin(NVS_NS, /*readOnly=*/false);
  uint16_t n = p.getUShort(key, 0);
  p.putUShort(key, n + 1);
  p.end();
}

static int readCounter(const char* key) {
  Preferences p;
  p.begin(NVS_NS, /*readOnly=*/true);
  uint16_t n = p.getUShort(key, 0);
  p.end();
  return n;
}

void bumpAnimalSightings() { bumpCounter("animalSee"); }
int totalAnimalSightings() { return readCounter("animalSee"); }

void bumpWeatherEvents() { bumpCounter("weatherEv"); }
int totalWeatherEvents() { return readCounter("weatherEv"); }

void bumpOtherEvents() { bumpCounter("otherEv"); }
int totalOtherEvents() { return readCounter("otherEv"); }

}  // namespace journal
