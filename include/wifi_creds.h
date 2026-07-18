// wifi_creds.h — the WifiCred type, kept separate so secrets.h (gitignored)
// and the net module can share it without committing credentials.
#pragma once

struct WifiCred {
  const char* ssid;
  const char* password;
};
