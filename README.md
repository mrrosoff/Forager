# Forager

A battery-powered e-ink shelf artifact: a foraging tamagotchi. A hoary
marmot lives on the e-ink display, is born the first time you power it on,
and grows up as you feed it real Pacific Northwest species. It reflects
real-world Seattle conditions — season, weather — and shows you what's worth
foraging for right now across PNW forests, mountains, and coastline. Eating
any species in the foraging list feeds the marmot directly.

It sleeps almost all the time. Pressing ENTER wakes it from deep sleep; it
pulls fresh time/weather over WiFi, updates the creature, and renders the
e-ink display. After a minute of no interaction it drops back into deep
sleep, holding the last image on the screen at zero power. Neglect it for
about a week and it dies — feeding it keeps it alive.

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
   sequence and prompts you to name the marmot.

## Hardware

| Part | Detail |
|------|--------|
| MCU | ESP32-S3 Super Mini (onboard LiPo charging via USB-C) |
| Display | Waveshare 4.2" e-ink, 400×300 physical / 300×400 logical portrait, SPI, 1-bit black/white (official Waveshare `epd4in2_V2` driver) |
| Buttons | 4× tactile (LEFT / RIGHT / ENTER / SETTINGS) |
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
| Button SETTINGS | 5 |
| Button LEFT | 1 |
| Button RIGHT | 2 |

LEFT/RIGHT/ENTER are dedicated tactile buttons wired to 3V3 through the power
switch with `INPUT_PULLDOWN`, so a press reads HIGH and no external resistors
are needed; they read dead when the switch is off (expected). ENTER must sit
on an RTC-capable GPIO (0-21 on the S3) for wake-from-sleep to work; it
avoids GPIO3 (a strapping pin) and GPIO11-20 (ADC2 — flaky whenever the WiFi
radio has ever been powered). LEFT/RIGHT are on GPIO1/2 (ADC1, proven
reliable). All three wake the board from deep sleep via
`esp_sleep_enable_ext1_wakeup()` (ANY_HIGH).

SETTINGS is different: it rides the display module's own onboard **KEY1**
button (confirmed against a real Waveshare Pico-ePaper-4.2 board — the
button pinned "KEY0" is a *different* one, on the board's other header row).
Like every Waveshare board button, KEY1 is wired switch-to-GND, not through
the power switch to 3V3, so it needs `INPUT_PULLUP` and reads **LOW** when
pressed — the opposite polarity from the other three buttons. It is not a
wake source (not in the ext1 mask), so Settings is only reachable once the
device is already awake.

## Behavior

- **Deep sleep** between interactions; the screen retains its last image at no
  power (e-ink is bistable).
- **Wake on ENTER**, or a 24h timer backstop so the panel refreshes at least
  once a day even with zero button presses.
- **On wake**: connect WiFi → NTP time → fetch Seattle weather → recompute
  mood/growth stage/death check → check for a new event → render.
- **Auto-sleep** after 60s of no button presses — right before sleeping, a
  dedicated sleeping-marmot screen (with drifting Zzz's, no chrome) is drawn
  so the bistable panel shows something intentional, not a stale mid-
  interaction frame.

## Growing up

The marmot is born the first time the device is ever powered on (birthDate
is stamped then) and grows through three stages based on **distinct species
foraged** (`journal::totalEaten()`), not elapsed time, recomputed once per
wake:

| Stage | When |
|-------|------|
| Baby | Fewer than 5 species eaten |
| Juvenile | 5-14 species eaten |
| Adult | 15+ species eaten |

**Foraging is unlocked from birth** — growth itself depends on foraging, so
a Baby has to be able to do it. A species only shows up in the Foraging
browse list once "discovered" via a Discovery encounter (a common wake-time
event type, resolved on Main via ENTER like a sighting) — the full
250-species reference isn't handed over on day one. While Baby, wake-time
events lean toward always-positive care moments (naps, first steps,
snuggles) instead of trail mishaps, since those don't fit a baby-at-home
story, but Discovery still rolls at every stage. Juvenile and Adult have
their own rotating pools of real marmot-pose photos; Baby does too, sourced
from marmot pup/family photos rather than reusing adult art.

## Staying alive

Three persisted bars drive whether the marmot survives:

- **Hunger** — ramps up over about a week without feeding.
- **Happiness** — decays the longer it's been since anything was
  acknowledged (feeding, or resolving a wake-time event).
- **Energy** — decays on the same clock as Happiness, restored more by
  protein/fat-rich food kinds (shellfish, crab, nuts, ...) than a plain
  green.

Eating any species always fills Hunger; which food kind it is also nudges
Happiness or Energy — sweet/treat kinds (berry, sap, nut) lean Happiness,
hearty/protein kinds lean Energy, everything else gives a bit of both.

If Hunger maxes out, or Happiness or Energy bottoms out at 0, the marmot
dies — checked every wake. Since all three only reach those extremes after
roughly a week of zero feeding, this is a real neglect consequence, not a
one-bad-wake surprise: normal play never comes close. The death screen shows
a randomized cause line (starved / heartbroken / exhausted, matching
whichever bar actually triggered it) and resets the whole game back to a
fresh birth on the next ENTER.

## Views

LEFT/RIGHT step between views (clamped at each end, no wraparound); ENTER
acts on the current view (labeled in the nav bar at the bottom of every
screen). Order: **Achievements ← Status ← Main → Foraging**.

1. **Main** — the marmot, its current mood (reflected in which pose/art gets
   picked), a weather glyph, and an occasional thought bubble. When a
   wake-time event is pending (see below), this view is taken over by the
   encounter screen instead; ENTER resolves it there.
2. **Foraging** — pages through the *discovered* subset of the 250-species
   PNW reference: a real photo (where sourced), name, biome tag, season
   note, safety caution when relevant, and a harvest tip. **ENTER eats the
   species on screen**, feeding the marmot directly (and resolving a
   matching ForagingFind event for a bonus, if one's pending). RIGHT scrolls
   forward through the list, accelerating the longer it's held.
3. **Status** — fullness, happiness, energy, curiosity (weather-driven),
   feed streak, and species-eaten count.
4. **Achievements** *(Adult only)* — total species eaten, feed streak, and 9
   medal-style badges (Mushrooms, Berries, Herbalist, Wildlife, Naturalist,
   Storms, Wanderer, Halfway, Complete) that unlock as their underlying
   counts (species by kind, animal sightings, weather/other events) cross a
   threshold — locked badges show a plain `?`.

## Wake-time events

Roughly once every 6 hours of wall-clock time (checked only on wake, no live
timer), there's a 1-in-3 chance of a new event: a species Discovery
(unlocks one species into the Foraging list — deliberately the most
common), an animal sighting, a foraging find (needing a specific kind of
food, or occasionally one exact species), a trail mishap, a weather event,
a trail treasure, a marmot encounter, or — while the marmot is a baby — a
simple care moment instead of a find or mishap. Pending events take over the
Main view until resolved (ForagingFind is the exception: it resolves
silently by eating a matching species on the Foraging view instead of via
ENTER).

Both the cooldown and the roll odds scale with how actively the device is
being used: waking it repeatedly within 10 minutes builds up to 3 stacked
"engagement" bonus levels (shorter cooldown, better odds), dropping back to
baseline the moment a wake follows a longer gap. Separately, navigating
between 8 views within 90 seconds guarantees an event on the spot
(bypassing the cooldown entirely), as long as one wasn't already pending or
resolved in the last 3 minutes — an actively-browsing session earns an
event rather than waiting on background odds. A fresh marmot never rolls an
event on its very first wake, before naming.

## The creature

A hoary marmot with moods driven by season, weather, and how long since it
was last fed:

`excited` · `content` · `sleepy` · `hungry` · `annoyed` (neglected too long) ·
`dormant` (deep winter).

## Settings

Press SETTINGS from any screen to open an overlay. LEFT/RIGHT cycle through
options, ENTER selects; the SETTINGS/KEY1 button is "back" at every nesting
depth (a confirm sub-screen backs to its parent menu, which backs to
Settings, which exits back to the previous view):

- **WiFi Networks** — list saved networks, add a new one (SSID then
  password, entered via an on-screen QWERTY grid keyboard with SHIFT and
  symbols pages), or remove one.
- **Reset Game** — wipes all saved progress and reboots into a fresh birth.
  Requires a second ENTER on a confirm screen first, since it's destructive.
- **Power Off** — deep-sleeps with no wake source armed at all. Only the
  physical power switch brings the device back (a true off, not just a
  longer sleep). Also requires a confirm screen.

## Data sources

- **Time** — NTP (no RTC module); Pacific time with US DST rules.
- **Weather** — [wttr.in](https://wttr.in) JSON for Seattle; recent rainfall
  and temperature. Recent rain ⇒ good foraging, feeds the Curiosity stat.
- **Foraging reference** — 250 PNW species spanning forest, mountain, and
  coastal biomes, hardcoded with season, safety notes, and harvest tips (see
  `src/foraging/foraging_species.h`), 248 of them paired with a real sourced
  photo.
- **Animal sightings** — 25 PNW animals, 21 with a real sourced photo.
- **Achievement badges** — 9 flat icons sourced from openly-licensed
  Wikimedia Commons icon sets, thresholded (not gamma-dithered — full
  photo-style dithering doesn't survive at this size) into a two-tone
  solid+stippled look at the exact badge resolution.

## Project layout

```
platformio.ini          Build config, board, libs, per-module include paths
.clang-format            Formatting rules (see Formatting below)
.clang-tidy               Linter rules (see Linting below)
include/                Shared headers (auto on the compiler path)
  config.h                Pin map + behavioral tunables
  model.h                 Shared data types (Forageable, CreatureState, Stage, View, DeathCause, …)
  wifi_creds.h            WifiCred type
  secrets.example.h       Copy to secrets.h and fill in WiFi networks
  bitmaps/
    animals/                One PROGMEM header per animal-sighting photo
    badges/                 One PROGMEM header per achievement-badge icon
    marmot/                 Marmot pose art: mood/baby/juvenile/adult/death variants
    species/                One PROGMEM header per foraging-species photo, + a
                             generated species_index.h lookup table
src/
  main.cpp                State machine: wake → fetch → render → sleep; button handling
  net/                    WiFi (strongest-known), NTP, weather fetch + parse
  foraging/               PNW species reference (forest/mountain/coast); foraging_species.h holds the data table
  creature/               Mood/growth-stage/death logic, hunger/happiness/energy/streak, persisted across sleep
  events/                 Wake-time interaction events; events_data.h holds the flavor-text pools
  journal/                Tracks eaten + discovered species (two bitsets), lifetime sighting/event counters
  display/                Rendering: views, dithered shading, nav bar, settings overlay; data tables split into display_*_art.h / display_thoughts.h
  textentry/              Shared on-screen QWERTY keyboard (marmot naming, WiFi SSID/password)
  wifistore/               Saved WiFi credentials (NVS-backed)
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

`include/secrets.h` is gitignored. Networks can also be added/removed
on-device via Settings → WiFi Networks.

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
shapes, for real fur/feather/texture detail. Every wake gets a full refresh
on the first frame after `epd.begin()`; later view changes within the same
wake use the panel's faster partial-refresh path, except for a couple of
deliberately full-refreshed moments (growth transitions, a guaranteed
mid-session event) that re-initialize the panel controller first to avoid
ghosting.

## Formatting

Reformat with `clang-format -i $(find src include -name '*.cpp' -o -name '*.h' | grep -v epd_official)` (repo root `.clang-format`; skips vendored `src/epd_official/`).

## Linting

Generate `compile_commands.json` with `~/.platformio/penv/bin/pio run -t compiledb`, then run `clang-tidy` against files under `src/` (excluding `src/epd_official/`) using the repo root `.clang-tidy` config.
