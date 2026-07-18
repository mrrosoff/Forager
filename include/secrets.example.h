// secrets.h.example — copy to secrets.h and fill in your WiFi credentials.
// secrets.h is gitignored so your credentials never get committed.
//
// List as many known networks as you like. On wake, the device scans for
// nearby APs and connects to whichever known network has the strongest signal.
#pragma once

#include "wifi_creds.h"

static const WifiCred WIFI_NETWORKS[] = {
    {"your-home-network",  "home-password"},
    {"your-phone-hotspot", "hotspot-password"},
};
static const size_t WIFI_NETWORK_COUNT =
    sizeof(WIFI_NETWORKS) / sizeof(WIFI_NETWORKS[0]);
