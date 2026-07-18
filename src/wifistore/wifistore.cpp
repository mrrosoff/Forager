#include "wifistore.h"

#include <Preferences.h>

#include <cstring>

#include "secrets.h"

namespace wifistore {

static const char* NVS_NS = "forager";

static Network networks[MAX_NETWORKS];
static int networkCount = 0;

static void save() {
  Preferences p;
  p.begin(NVS_NS, /*readOnly=*/false);
  p.putUChar("wifiCount", (uint8_t)networkCount);
  p.putBytes("wifiNets", networks, sizeof(networks));
  p.end();
}

void load() {
  Preferences p;
  p.begin(NVS_NS, /*readOnly=*/true);
  bool seeded = p.isKey("wifiSeeded");
  networkCount = p.getUChar("wifiCount", 0);
  if (networkCount > MAX_NETWORKS) networkCount = MAX_NETWORKS;
  size_t got = p.getBytes("wifiNets", networks, sizeof(networks));
  p.end();
  if (got != sizeof(networks)) networkCount = 0;  // corrupt/missing -> empty, not garbage

  if (!seeded) {
    // First-ever boot: import the compiled-in secrets.h list so an existing
    // setup doesn't need a manual Settings step just to keep working.
    for (size_t i = 0; i < WIFI_NETWORK_COUNT && networkCount < MAX_NETWORKS; i++) {
      Network& n = networks[networkCount];
      strncpy(n.ssid, WIFI_NETWORKS[i].ssid, sizeof(n.ssid) - 1);
      n.ssid[sizeof(n.ssid) - 1] = '\0';
      strncpy(n.password, WIFI_NETWORKS[i].password, sizeof(n.password) - 1);
      n.password[sizeof(n.password) - 1] = '\0';
      networkCount++;
    }
    save();
    Preferences p2;
    p2.begin(NVS_NS, /*readOnly=*/false);
    p2.putBool("wifiSeeded", true);
    p2.end();
  }
}

int count() { return networkCount; }

const Network& at(int index) { return networks[index]; }

bool add(const char* ssid, const char* password) {
  if (networkCount >= MAX_NETWORKS || !ssid || !ssid[0]) return false;
  Network& n = networks[networkCount];
  strncpy(n.ssid, ssid, sizeof(n.ssid) - 1);
  n.ssid[sizeof(n.ssid) - 1] = '\0';
  strncpy(n.password, password ? password : "", sizeof(n.password) - 1);
  n.password[sizeof(n.password) - 1] = '\0';
  networkCount++;
  save();
  return true;
}

void remove(int index) {
  if (index < 0 || index >= networkCount) return;
  for (int i = index; i < networkCount - 1; i++) networks[i] = networks[i + 1];
  networkCount--;
  save();
}

}  // namespace wifistore
