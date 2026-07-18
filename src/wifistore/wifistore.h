// wifistore.h — runtime-editable WiFi network list, persisted in NVS so
// networks can be added/removed from the Settings menu without reflashing.
// Seeded once from the compiled-in secrets.h list on first-ever boot, so an
// existing setup keeps working after this feature lands.
#pragma once

namespace wifistore {

static const int MAX_NETWORKS = 5;

struct Network {
  char ssid[32];
  char password[64];
};

// Pulls the saved list from NVS into RAM. Call once per wake, before
// count()/at(). Seeds from the compiled-in secrets.h WIFI_NETWORKS list the
// very first time this ever runs (nothing saved yet), so a fresh flash still
// connects without a manual Settings step.
void load();

int count();
const Network& at(int index);

// Returns false if the list is already at MAX_NETWORKS or ssid is empty.
bool add(const char* ssid, const char* password);

void remove(int index);

}  // namespace wifistore
