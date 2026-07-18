# CLAUDE.md

Working notes for this repo that aren't in the README. See README.md for the
product description, hardware table, pin map, and behavior overview.

## Build / flash

`pio` is not on PATH. Use the PlatformIO penv binary:

```sh
~/.platformio/penv/bin/pio run              # build
~/.platformio/penv/bin/pio run -t upload    # flash over USB-C
~/.platformio/penv/bin/pio device monitor   # serial @ 115200
```

A clean build lands around RAM 19% / Flash 74% (the foraging species
database, ~220 entries, accounts for a good chunk of that). Libraries are
already cached under `.pio/libdeps`.

## Editor diagnostics are not the build

The clang/IntelliSense language server reports `Arduino.h file not found` and
`unknown type uint8_t` until PlatformIO generates `.vscode/c_cpp_properties.json`
(run a build or "PlatformIO: Rebuild IntelliSense Index"). These are **not**
real errors — trust `pio run`, not the inline squiggles.

## Layout conventions (mirrors ../Lander)

- Shared headers in `include/` — auto on the compiler path, no `-I` needed.
- Each module is a folder under `src/` holding its own `.h` + `.cpp`. Module
  dirs are added as include paths in `platformio.ini` so a module includes its
  own header as `#include "moon.h"`.
- No automated tests -- this project doesn't have a test suite.
- `include/secrets.h` is gitignored; `include/secrets.example.h` is the template.

## Display: official Waveshare driver, 1-bit + dithering

The panel is driven via Waveshare's own **`epd4in2_V2`** reference driver
(vendored in `src/epd_official/`, not a PlatformIO registry package), wrapped
in `src/epd_official/epd_adapter.h`'s `EpdGFX` class so it looks like a plain
Adafruit_GFX display to the rest of `display.cpp` (fillCircle, drawBitmap,
print, etc. all work unchanged).

We tried the panel's true 4-level grayscale earlier (via the third-party
`GxEPD2_4G` fork) but dropped it: a 4-grey full refresh is inherently
multi-pass and flickery, and this driver can't mix a grey full-refresh with a
fast b/w partial refresh in the same session without corrupting the panel
(confirmed by reproducing solid-grey corruption when we tried). So the whole
UI is **plain 1-bit black/white**, with grey shading *approximated* via
ordered (Bayer 4x4) dithering (see `dFillRect`/`dFillCircle`/etc. in
`display.cpp`), and every render is a full-window refresh — no partial
refresh, no idle animation loop.

**Rotation**: the panel is physically 400×300 landscape, but the enclosure
mounts it portrait. `EpdGFX::drawPixel()` is rotation-aware (standard
Adafruit_GFX convention: remaps logical x/y through `getRotation()` onto the
fixed physical buffer), and `display::begin()` calls `epd.setRotation(3)`.
All layout code in `display.cpp` targets the resulting 300×400 logical
canvas (`SCREEN_W`/`SCREEN_H` constants) — do not use `config.h`'s
`EPD_WIDTH`/`EPD_HEIGHT` (400/300) for layout, those describe the physical
buffer only.

**The marmot** (`drawCreature()` in `display.cpp`) is a hardcoded PROGMEM
bitmap (`include/marmot_bitmap.h`), not procedural shapes — generated from
reference photos via a Pillow pipeline (smooth grayscale art at 4x
supersample, then Floyd-Steinberg dithered down to the embed resolution,
which naturally produces a pen-and-ink stipple texture). The generation
script lived in a scratch dir during development and isn't checked in;
regenerating the art means re-running that pipeline by hand. Eyes and a
small brow/mouth accent are drawn procedurally on top at runtime (see the
`MARMOT_EYE_X/Y`, `MARMOT_NOSE_X/Y` anchor constants in the header) so the
same base bitmap serves every mood.

Panel revision selected by `EPD_PANEL_GDEY042T81` in `include/config.h` —
currently unused by `epd_official` (which only supports that revision), kept
as documentation of the assumption.

## Hardware gotchas

- **No PIR, no light sensor** on this build — ENTER is the sole wake source.
  Deep-sleep wake (`ext0`) only works on RTC-capable GPIOs (0-21 on the S3),
  so ENTER lives on **GPIO4** (freed up once the PIR was removed). RIGHT is
  on GPIO2. There is no LEFT button.
- GPIO11-20 are ADC2 channels that share hardware with the WiFi/BT radio and
  read flaky/self-oscillating once that radio block has ever been powered,
  even after `WiFi.mode(WIFI_OFF)` — confirmed by reproducing the
  oscillation on GPIO13 twice. Avoid that range for any button/wake pin.
- Buttons are `INPUT_PULLDOWN`, wired to 3V3 **through the power switch**, so
  pressed reads HIGH and buttons are dead when the switch is off (expected).
- Navigation is intentionally one-directional: RIGHT advances
  Main → Foraging → Status with no wraparound and no LEFT/back. The Foraging
  species list *does* still wrap via ENTER (the list is long and there's no
  other way to page backward), but it also resets to species 0 on every
  sleep/wake cycle since `forageIdx` is a plain RAM global, not persisted.

## Time persists across deep sleep

The ESP32 RTC clock keeps running through deep sleep, so NTP only truly matters
on the first wake after a power-cycle (the inline BAT switch). On later wakes
`time()` is already valid; we still attempt a refresh when online.

## Creature state

Persisted in NVS (`Preferences`, namespace `forager`) so it survives the power
switch, unlike RTC memory. Hunger is recomputed from `lastFed` vs.
`HUNGER_PERIOD_HOURS` each wake; ENTER on the Status view = "I went foraging"
(feed). Energy and Curiosity (shown on Status) are *not* persisted — they're
computed live from time-of-day and current weather respectively.

## Formatting

Reformat with `clang-format -i $(find src include -name '*.cpp' -o -name '*.h' | grep -v epd_official)` (repo root `.clang-format`; skips vendored `src/epd_official/`).

## Linting

Regenerate `compile_commands.json` with
`~/.platformio/penv/bin/pio run -t compiledb`, then run
`clang-tidy -p . $(find src -name '*.cpp' -not -path 'src/epd_official/*')`
(repo root `.clang-tidy`; the Xtensa-specific compiler flags PlatformIO emits
aren't understood by clang/clang-tidy, so expect `unknown argument` noise on
those unless you strip them first).

## Commits

Do **not** add a Claude co-author / "Generated with" trailer to commits in this
repo (owner preference).
