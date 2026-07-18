#include "journal.h"

#include <Preferences.h>

namespace journal {

static const char* NVS_NS = "forager";

// 256 bits -- comfortably covers the current 250-species database with a
// little headroom; bump if the species count ever grows past that.
static const int kBytes = 32;
static uint8_t bits[kBytes];

void load() {
  Preferences p;
  p.begin(NVS_NS, /*readOnly=*/true);
  size_t got = p.getBytes("eaten", bits, kBytes);
  if (got != (size_t)kBytes) {
    for (int i = 0; i < kBytes; i++) bits[i] = 0;
  }
  p.end();
}

void save() {
  Preferences p;
  p.begin(NVS_NS, /*readOnly=*/false);
  p.putBytes("eaten", bits, kBytes);
  p.end();
}

void markEaten(int speciesIndex) {
  if (speciesIndex < 0 || speciesIndex >= kBytes * 8) return;
  bits[speciesIndex / 8] |= (uint8_t)(1 << (speciesIndex % 8));
}

bool hasEaten(int speciesIndex) {
  if (speciesIndex < 0 || speciesIndex >= kBytes * 8) return false;
  return (bits[speciesIndex / 8] & (1 << (speciesIndex % 8))) != 0;
}

int totalEaten() {
  int count = 0;
  for (int i = 0; i < kBytes; i++) {
    uint8_t b = bits[i];
    while (b) {
      count += b & 1;
      b >>= 1;
    }
  }
  return count;
}

}  // namespace journal
