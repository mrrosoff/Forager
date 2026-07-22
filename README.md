# Forager

A battery-powered e-ink shelf artifact: a foraging tamagotchi. A hoary
marmot lives on the display, born the first time you power it on, and grows
up as you feed it real Pacific Northwest species — reflecting real Seattle
weather/season and what's worth foraging right now. It sleeps almost all
the time; ENTER wakes it, it refreshes, and it drops back to deep sleep
after 60s idle. Neglect it for about a week and it dies.

## Getting started

```sh
cp include/secrets.example.h include/secrets.h   # fill in WiFi networks
~/.platformio/penv/bin/pio run -t upload          # build + flash
```

Wire the hardware per the pin map below, then press ENTER. First-ever wake
runs a birth sequence and prompts you to name the marmot.

## Hardware

| Part | Detail |
|------|--------|
| MCU | ESP32-S3 Super Mini (onboard LiPo charging via USB-C) |
| Display | Waveshare 4.2" e-ink, 400×300 physical / 300×400 logical portrait, SPI, 1-bit (official `epd4in2_V2` driver) |
| Buttons | 4× tactile (LEFT / RIGHT / ENTER / SETTINGS) |
| Battery | LiPo 3.7V, soldered to BAT+/BAT- |
| Power switch | SPST slide switch inline on BAT+ |

| Signal | GPIO |
|--------|------|
| E-ink SCK / MOSI / CS / DC / RST / BUSY | 12 / 11 / 10 / 9 / 8 / 7 |
| ENTER (deep-sleep wake) | 4 |
| SETTINGS | 5 |
| LEFT / RIGHT | 1 / 2 |

LEFT/RIGHT/ENTER read `INPUT_PULLDOWN`, active-HIGH, and wake the board from
deep sleep (`esp_sleep_enable_ext1_wakeup`, ANY_HIGH); ENTER needs an
RTC-capable GPIO. SETTINGS rides the display module's own KEY1 button
instead — wired switch-to-GND (`INPUT_PULLUP`, active-LOW, opposite polarity
from the rest) and not a wake source, so Settings only works once awake.

## Behavior

- **Sleep/wake**: deep sleep between interactions (screen holds its image at
  zero power); ENTER wakes it, or a 24h timer backstop. On wake: WiFi → NTP
  → weather → recompute mood/growth/death → check for an event → render.
- **Growing up**: three stages (Baby / Juvenile / Adult) based on distinct
  species eaten, not elapsed time. Species only appear in the Foraging list
  once discovered via a wake-time event.
- **Staying alive**: Hunger, Happiness, and Energy are persisted bars that
  each decay over about a week without feeding/interaction; any one maxing
  out (Hunger) or bottoming out (Happiness/Energy) kills the marmot and
  resets to a fresh birth.
- **Views** (LEFT/RIGHT cycle, ENTER acts): Achievements ← Status ← **Main**
  → Foraging. Main shows the marmot + mood + weather + pending events;
  Foraging pages the discovered species list and eats on ENTER; Status shows
  the raw stats; Achievements (Adult only) shows 9 unlockable badges.
- **Wake-time events**: roughly every 6h of wall-clock time, a chance of a
  Discovery/sighting/find/mishap/weather/treasure/encounter event takes over
  the Main view until resolved. Frequent use raises the odds; browsing all
  views quickly guarantees one.
- **Settings** (via SETTINGS button): WiFi Networks (add/remove, on-screen
  keyboard), Reset Game (wipes progress, confirm required), Power Off (true
  off, confirm required).

See `CLAUDE.md` for implementation details (NVS layout, display driver
quirks, hardware gotchas, art-sourcing pipeline) not covered here.

## Data sources

- **Time**: NTP, no RTC module, Pacific time with DST.
- **Weather**: [wttr.in](https://wttr.in) JSON for Seattle.
- **Foraging reference**: 200 PNW species (`src/foraging/foraging_species.h`),
  most paired with a real sourced photo.
- **Animal sightings**: 25 PNW animals, 21 with a real photo.
- **Achievement badges**: 9 flat icons from openly-licensed Commons sets.

## Project layout

```
platformio.ini          Build config, board, libs, per-module include paths
include/                Shared headers (config.h pins/tunables, model.h types,
                         secrets.example.h, bitmaps/{animals,badges,marmot,species})
src/
  main.cpp                State machine: wake -> fetch -> render -> sleep
  net/                    WiFi, NTP, weather fetch + parse
  foraging/               PNW species reference table
  creature/               Mood/growth/death logic, persisted stats
  events/                 Wake-time interaction events + flavor text
  journal/                Eaten/discovered species tracking, counters
  display/                Rendering: views, dithering, nav bar, settings
  textentry/              Shared on-screen QWERTY keyboard
  wifistore/              Saved WiFi credentials (NVS-backed)
  epd_official/           Vendored Waveshare driver (do not lint/format)
enclosure/               OpenSCAD 3D-printable case (see file header)
```

## Build / Format / Lint

```sh
~/.platformio/penv/bin/pio run                                        # build
~/.platformio/penv/bin/pio run -t upload                              # flash
~/.platformio/penv/bin/pio device monitor                             # serial @ 115200
clang-format -i $(find src include -name '*.cpp' -o -name '*.h' | grep -v epd_official)
~/.platformio/penv/bin/pio run -t compiledb && clang-tidy ...         # against src/, excluding epd_official/
```

Or use the PlatformIO IDE extension in VSCode (`.vscode/extensions.json`).
