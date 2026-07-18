# Forager

A battery-powered e-ink shelf artifact: a foraging tamagotchi. A hoary
marmot lives on the e-ink display, is born the first time you power it on,
and grows up over real elapsed time. It reflects real-world Seattle
conditions — season, weather — and shows you what's worth foraging for right
now across PNW forests, mountains, and coastline. Eating any species in the
foraging list feeds the marmot directly.

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
4. Press ENTER to wake it. The very first wake ever runs a short birth
   sequence and starts the marmot as a baby.

## Hardware

| Part | Detail |
|------|--------|
| MCU | ESP32-S3 Super Mini (onboard LiPo charging via USB-C) |
| Display | Waveshare 4.2" e-ink, 400×300 physical / 300×400 logical portrait, SPI, 1-bit black/white (official Waveshare `epd4in2_V2` driver) |
| Buttons | 4× tactile (LEFT / RIGHT / ENTER / SETTINGS), INPUT_PULLDOWN |
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
| Button SETTINGS | 5 *(provisional — see below)* |
| Button LEFT | 1 |
| Button RIGHT | 2 |

Buttons are wired to 3V3 through the power switch with `INPUT_PULLDOWN`, so a
press reads HIGH and no external resistors are needed; they read dead when
the switch is off (expected). ENTER must sit on an RTC-capable GPIO (0-21 on
the S3) for `esp_sleep_enable_ext0_wakeup()` to work; it avoids GPIO3 (a
strapping pin) and GPIO11-20 (ADC2 — flaky whenever the WiFi radio has ever
been powered). LEFT/RIGHT are on GPIO1/2 (ADC1, proven reliable). SETTINGS is
wired to the display board's onboard "KEY0" button on GPIO5 — **confirm this
against your board's actual silkscreen/schematic before flashing**, it
hasn't been verified against real hardware yet.

## Behavior

- **Deep sleep** between interactions; the screen retains its last image at no
  power (e-ink is bistable).
- **Wake on ENTER**, or a 24h timer backstop so the panel refreshes at least
  once a day even with zero button presses.
- **On wake**: connect WiFi → NTP time → fetch Seattle weather → recompute
  mood/growth stage → check for a new event → render.
- **Auto-sleep** after 60s of no button presses — right before sleeping, a
  dedicated sleeping-marmot screen (with drifting Zzz's, no chrome) is drawn
  so the bistable panel shows something intentional, not a stale mid-
  interaction frame.

## Growing up

The marmot is born the first time the device is ever powered on (birthDate
is stamped then) and grows through three stages based on real elapsed time,
recomputed once per wake:

| Stage | When |
|-------|------|
| Baby | First 2 days |
| Juvenile | Through day 7 |
| Adult | Day 7 onward |

The **Foraging view is hidden until Juvenile** — pressing RIGHT from Main
stops there instead of continuing on, and while Baby, wake-time events are
simple always-positive care moments (naps, first steps, snuggles) instead of
foraging finds or trail mishaps, since neither fits a baby-at-home story.
Juvenile and Adult have their own rotating pools of real marmot-pose
photos; Baby does too (3 poses), sourced from marmot pup/family photos
rather than reusing adult art.

## Views

LEFT/RIGHT step between views (clamped at each end, no wraparound); ENTER
acts on the current view (labeled in the nav bar at the bottom of every
screen). Order: **Achievements ← Status ← Main → Foraging**.

1. **Main** — the marmot, its current mood (reflected in which pose/art gets
   picked), and a weather glyph. When a wake-time event is pending (see
   below), this view is taken over by the encounter screen instead; ENTER
   resolves it there.
2. **Foraging** *(Juvenile+ only)* — pages through the full 250-species PNW
   reference: a real photo (where sourced), name, biome tag, season note,
   safety caution when relevant, and a harvest tip. **ENTER eats the species
   on screen**, feeding the marmot directly (and resolving a matching
   ForagingFind event for a bonus, if one's pending). RIGHT scrolls forward
   through the list, accelerating the longer it's held.
3. **Status** — fullness, happiness, energy (time-of-day), curiosity
   (weather-driven), feed streak, and species-eaten count.
4. **Achievements** — total species eaten, feed streak, and progress badges
   (Mushroom Hunter, Berry Picker, Herbalist, Coastal/Mountain/Tidepool
   Forager, Completionist) based on what's been eaten.

## Wake-time events

Roughly once every 6 hours of wall-clock time (checked only on wake, no live
timer), there's a 1-in-3 chance of a new event: an animal sighting, a
foraging find (needing a specific kind of food, or occasionally one exact
species), a trail mishap, a weather event, or — while the marmot is a baby —
a simple care moment instead of a find or mishap. Pending events take over
the Main view until resolved.

## The creature

A hoary marmot with moods driven by season, weather, and how long since it
was last fed:

`excited` · `content` · `sleepy` · `hungry` · `annoyed` (neglected too long) ·
`dormant` (deep winter).

## Settings

Press SETTINGS from any screen to open an overlay (RIGHT cycles the option,
ENTER selects, LEFT exits without acting):

- **Power Off** — deep-sleeps with no wake source armed at all. Only the
  physical power switch brings the device back (a true off, not just a
  longer sleep).
- **Reset Game** — wipes all saved progress and reboots into a fresh birth.
  Requires a second ENTER on a confirm screen first, since it's destructive.

## Data sources

- **Time** — NTP (no RTC module); Pacific time with US DST rules.
- **Weather** — [wttr.in](https://wttr.in) JSON for Seattle; recent rainfall
  and temperature. Recent rain ⇒ good foraging, feeds the Curiosity stat.
- **Foraging reference** — 250 PNW species spanning forest, mountain, and
  coastal biomes, hardcoded with season, safety notes, and harvest tips (see
  `src/foraging/foraging.cpp`), 248 of them paired with a real sourced photo.
- **Animal sightings** — 25 PNW animals, 21 with a real sourced photo.

## Project layout

```
platformio.ini          Build config, board, libs, per-module include paths
.clang-format            Formatting rules (see Formatting below)
.clang-tidy               Linter rules (see Linting below)
include/                Shared headers (auto on the compiler path)
  config.h                Pin map + behavioral tunables
  model.h                 Shared data types (Forageable, CreatureState, Stage, View, …)
  wifi_creds.h            WifiCred type
  secrets.example.h       Copy to secrets.h and fill in WiFi networks
  bitmaps/
    animals/                One PROGMEM header per animal-sighting photo
    marmot/                 Marmot pose art: base/hungry/baby/juvenile/adult variants
    species/                One PROGMEM header per foraging-species photo, + a
                             generated species_index.h lookup table
src/
  main.cpp                State machine: wake → fetch → render → sleep; button handling
  net/                    WiFi (strongest-known), NTP, weather fetch + parse
  foraging/               PNW species reference (forest/mountain/coast)
  creature/               Mood/growth-stage logic, hunger/happiness/streak, persisted across sleep
  events/                 Wake-time interaction events (sightings, finds, mishaps, weather, baby care)
  journal/                Tracks which species have been eaten, for streak/achievements
  display/                Rendering: views, dithered shading, nav bar, settings overlay
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
is approximated instead with **ordered (Bayer) dithering**. The marmot and
every animal/species icon are hardcoded PROGMEM bitmaps generated from real
reference photos (dithered at generation time) rather than procedural
shapes, for real fur/feather/texture detail. Every wake still gets exactly
one full refresh (on the first frame after `epd.begin()`), matching
Waveshare's guidance to always full-refresh on wake.

## Formatting

Reformat with `clang-format -i $(find src include -name '*.cpp' -o -name '*.h' | grep -v epd_official)` (repo root `.clang-format`; skips vendored `src/epd_official/`).

## Linting

Generate `compile_commands.json` with `~/.platformio/penv/bin/pio run -t compiledb`, then run `clang-tidy` against files under `src/` (excluding `src/epd_official/`) using the repo root `.clang-tidy` config.
