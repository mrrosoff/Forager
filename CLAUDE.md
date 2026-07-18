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

A clean build lands around RAM 19% / Flash 29%. Libraries are already cached
under `.pio/libdeps`.

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
- Tests go in top-level `test/`, never under `src/`.
- `include/secrets.h` is gitignored; `include/secrets.example.h` is the template.

## Display "greys" are dithered, not hardware grayscale

The 4.2" b/w panel via `GxEPD2_BW` is 1-bit. There is no reliable 4-level
grayscale path for it in mainline GxEPD2. Midtones are faked with an ordered
(Bayer 4x4) dither in `display.cpp` (`shadeHLine`/`shadeDisc`/`shadeDome`,
levels `SH_LIGHT/MID/DARK`). If you ever move to a true grayscale panel, that's
where to swap.

Panel revision is selected by `EPD_PANEL_GDEY042T81` in `include/config.h`.
Wrong choice = ghosting or blank screen.

## Hardware gotchas

- **GPIO3 (ENTER) is an ESP32-S3 strapping pin.** It's used here as a
  pulldown button input, which is fine at runtime, but don't hold ENTER during
  boot/flash — it can affect boot mode. If flashing gets flaky, that's the
  first suspect.
- Buttons are `INPUT_PULLDOWN`, wired to 3V3 **through the power switch**, so
  pressed reads HIGH and buttons are dead when the switch is off (expected).
- PIR wake uses `ext0` on `GPIO4` (wake on HIGH). Only one ext0 pin is allowed.
- `BH1750` failing to init returns a large lux sentinel so a dead sensor never
  traps the device permanently asleep.

## Time persists across deep sleep

The ESP32 RTC clock keeps running through deep sleep, so NTP only truly matters
on the first wake after a power-cycle (the inline BAT switch). On later wakes
`time()` is already valid; we still attempt a refresh when online.

## Creature state

Persisted in NVS (`Preferences`, namespace `forager`) so it survives the power
switch, unlike RTC memory. Hunger is recomputed from `lastFed` vs.
`HUNGER_PERIOD_HOURS` each wake; ENTER on the Status view = "I went foraging"
(feed).

## Commits

Do **not** add a Claude co-author / "Generated with" trailer to commits in this
repo (owner preference).
