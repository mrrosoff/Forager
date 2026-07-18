# Forager

A battery-powered e-ink shelf artifact: a foraging tamagotchi. A hoary marmot
lives on the e-ink display and reflects real-world Seattle foraging
conditions — season, weather — and shows you what's worth hunting for right
now across PNW forests, mountains, and coastline. "Feed" it by confirming you
actually went out foraging.

It sleeps almost all the time. Pressing ENTER wakes it from deep sleep; it
pulls fresh time/weather over WiFi, updates the creature, and renders the
e-ink display. After a minute of no interaction it drops back into deep
sleep, holding the last image on the screen at zero power.

## Getting started

1. **Clone and configure secrets**:
   ```sh
   cp include/secrets.example.h include/secrets.h
   # edit include/secrets.h with your WiFi network(s)
   ```
2. **Wire the hardware** — see [Hardware](#hardware) and the pin map below.
3. **Build and flash** (see [Build](#build)):
   ```sh
   ~/.platformio/penv/bin/pio run -t upload
   ```
4. Press ENTER to wake it.

## Hardware

| Part | Detail |
|------|--------|
| MCU | ESP32-S3 Super Mini (onboard LiPo charging via USB-C) |
| Display | Waveshare 4.2" e-ink, 400×300 physical / 300×400 logical portrait, SPI, 1-bit black/white (official Waveshare `epd4in2_V2` driver) |
| Buttons | 2× tactile (RIGHT / ENTER), INPUT_PULLDOWN |
| Battery | LiPo 3.7 V, soldered to BAT+/BAT- |
| Power switch | SPST slide switch inline on BAT+ |

### Pin map (peripheral → ESP32-S3 GPIO)

| Signal | GPIO |
|--------|------|
| E-ink SCK | 12 |
| E-ink MOSI (DIN) | 11 |
| E-ink CS | 10 |
| E-ink DC | 9 |
| E-ink RST | 8 |
| E-ink BUSY | 7 |
| Button ENTER (deep-sleep wake) | 4 |
| Button RIGHT | 2 |

There's no LEFT button and no PIR/light sensor on this build — navigation is
one-directional (RIGHT only, no wrap) and ENTER is the sole wake source.
Buttons are wired to 3V3 through the power switch with `INPUT_PULLDOWN`, so a
press reads HIGH and no external resistors are needed. ENTER must sit on an
RTC-capable GPIO (0-21 on the S3) for `esp_sleep_enable_ext0_wakeup()` to
work; it avoids GPIO3 (a strapping pin) and GPIO11-20 (ADC2 — flaky whenever
the WiFi radio has ever been powered). RIGHT is on GPIO2 (ADC1, proven
reliable).

## Behavior

- **Deep sleep** between interactions; the screen retains its last image at no
  power (e-ink is bistable).
- **Wake on ENTER**, or a 24h timer backstop so the panel refreshes at least
  once a day even with zero button presses.
- **On wake**: connect WiFi → NTP time → fetch Seattle weather → recompute
  mood → render.
- **Auto-sleep** after 60s of no button presses — right before sleeping, the
  current view is re-rendered once more (and the view resets to Main) so the
  bistable panel always shows something fresh, not a stale mid-interaction
  frame.

## Views

RIGHT moves forward through the views (no wraparound, no LEFT/back); ENTER
acts on the current view (shown in the nav bar at the bottom of every
screen).

1. **Main** — the marmot on its habitat stage, current mood (reflected in its
   expression), and a weather glyph. ENTER does nothing here.
2. **Foraging** — pages through the full PNW species reference one at a time
   (ENTER advances, stops at the last entry): a large icon, name, biome tag,
   season note, safety caution when relevant, and a harvest tip.
3. **Status** — hunger, happiness, energy (time-of-day), curiosity
   (weather-driven), and last-fed time. ENTER here = "I went foraging"
   (feeds the creature: resets hunger, boosts happiness).

## The creature

A hoary marmot with moods driven by season, weather, and how long since it
was last fed — each with a distinct expression:

`excited` · `content` · `sleepy` · `hungry` · `annoyed` (neglected too long) ·
`glowing` (full moon) · `dormant` (deep winter).

## Data sources

- **Time** — NTP (no RTC module); Pacific time with US DST rules.
- **Weather** — [wttr.in](https://wttr.in) JSON for Seattle; recent rainfall
  and temperature. Recent rain ⇒ good foraging, feeds the Curiosity stat.
- **Foraging reference** — 200+ PNW species spanning forest, mountain, and
  coastal biomes, hardcoded with season, safety notes, and harvest tips (see
  `src/foraging/foraging.cpp`).

## Project layout

```
platformio.ini          Build config, board, libs, per-module include paths
.clang-format            Formatting rules (see Formatting below)
.clang-tidy               Linter rules (see Linting below)
include/                Shared headers (auto on the compiler path)
  config.h                Pin map + behavioral tunables
  model.h                 Shared data types (Forageable, CreatureState, …)
  sprites.h               Sprite IDs shared by foraging DB and renderer
  marmot_bitmap.h          Generated marmot artwork (PROGMEM bitmap)
  wifi_creds.h            WifiCred type
  secrets.example.h       Copy to secrets.h and fill in WiFi networks
src/
  main.cpp                State machine: wake → fetch → render → sleep
  moon/                   Local moon-phase math (used for the Glowing mood)
  net/                    WiFi (strongest-known), NTP, weather fetch + parse
  foraging/               PNW species reference (forest/mountain/coast)
  creature/               Mood logic, hunger/happiness, persisted across sleep
  display/                Rendering: views, sprites, dithered shading, nav bar
  epd_official/           Vendored Waveshare epd4in2_V2 driver (do not lint/format)
```

## WiFi setup

Copy the template and list every network the shelf might see — home, a studio,
a phone hotspot. On each wake it scans and connects to whichever **known
network has the strongest signal**, so it just works wherever it sits.

```sh
cp include/secrets.example.h include/secrets.h
# edit include/secrets.h
```

`include/secrets.h` is gitignored.

## Build

`pio` isn't on PATH — use the PlatformIO penv binary:

```sh
~/.platformio/penv/bin/pio run              # build
~/.platformio/penv/bin/pio run -t upload    # flash over USB-C
~/.platformio/penv/bin/pio device monitor   # serial @ 115200
```

Or open the folder in VSCode with the **PlatformIO IDE** extension
(recommended in `.vscode/extensions.json`) and use its build/upload buttons.

## Display rendering

The panel is driven in plain **1-bit black/white** via the official Waveshare
`epd4in2_V2` driver (vendored in `src/epd_official/`, wrapped in a small
Adafruit_GFX adapter so drawing code works unchanged). The panel is
physically 400×300 landscape; `epd.setRotation(3)` plus a rotation-aware
`drawPixel()` in the adapter remap everything to a 300×400 portrait logical
canvas for the actual enclosure orientation.

This panel's 4-grey hardware mode forces a multi-pass flicker on every full
refresh and can't be safely mixed with fast partial refresh, so grey shading
(species icons, habitat ground) is approximated instead with **ordered
(Bayer) dithering**. The marmot itself is a hardcoded PROGMEM bitmap
(`include/marmot_bitmap.h`, generated from reference artwork and dithered at
generation time) rather than procedural shapes, for real fur texture. Every
wake still gets exactly one full refresh (on the first frame after
`epd.begin()`), matching Waveshare's guidance to always full-refresh on wake.

## Formatting

Reformat with `clang-format -i $(find src include -name '*.cpp' -o -name '*.h' | grep -v epd_official)` (repo root `.clang-format`; skips vendored `src/epd_official/`).

## Linting

Generate `compile_commands.json` with `~/.platformio/penv/bin/pio run -t compiledb`, then run `clang-tidy` against files under `src/` (excluding `src/epd_official/`) using the repo root `.clang-tidy` config.
