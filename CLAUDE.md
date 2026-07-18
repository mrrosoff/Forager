# CLAUDE.md

Working notes for this repo that aren't in the README. See README.md for the
product description, hardware table, pin map, views, and behavior overview.

## Build / flash

`pio` is not on PATH. Use the PlatformIO penv binary:

```sh
~/.platformio/penv/bin/pio run              # build
~/.platformio/penv/bin/pio run -t upload    # flash over USB-C
~/.platformio/penv/bin/pio device monitor   # serial @ 115200
```

A clean build lands around RAM 19% / Flash 44%. The `huge_app.csv` partition
table (single large app partition, no OTA slot, minimal SPIFFS) is what makes
that comfortable — an earlier default partition table left only ~1.3MB for
the app and got tight once the foraging-species and animal-sighting photo
bitmaps piled up; switching partition schemes freed the project from that
constraint. Libraries are already cached under `.pio/libdeps`.

## Editor diagnostics are not the build

The clang/IntelliSense language server reports `Arduino.h file not found` and
`unknown type uint8_t` until PlatformIO generates `.vscode/c_cpp_properties.json`
(run a build or "PlatformIO: Rebuild IntelliSense Index"). These are **not**
real errors — trust `pio run`, not the inline squiggles.

## Layout conventions (mirrors ../Lander)

- Shared headers in `include/` — auto on the compiler path, no `-I` needed.
- Each module is a folder under `src/` holding its own `.h` + `.cpp`. Module
  dirs are added as include paths in `platformio.ini` so a module includes its
  own header as `#include "foraging.h"`.
- No automated tests -- this project doesn't have a test suite.
- `include/secrets.h` is gitignored; `include/secrets.example.h` is the template.
- `include/bitmaps/{animals,marmot,species}/` are generated art -- one PROGMEM
  header per photo (see the sourcing pipeline notes below), plus a generated
  `species/species_index.h` that ties all 250 species headers together with
  a name-keyed lookup table (that one's regenerated wholesale, not hand-edited,
  since it's a large auto-derived index rather than a small curated set like
  the animal/marmot folders).

## NVS persistence

Everything persisted lives in one `Preferences` namespace, `"forager"`,
opened independently (own `p.begin()`/`p.end()` block) in each of
`creature.cpp`, `events.cpp`, and `journal.cpp` — no shared wrapper. Current
keys: `hunger`, `happy`, `lastFed`, `birthDate`, `streak`, `streakDay`
(creature.cpp); `evType`, `evData`, `evExact`, `evLastAt` (events.cpp,
pending-event + spawn-cooldown state); `eaten` (journal.cpp, a 32-byte
eaten-species bitset). A single `Preferences::clear()` on `"forager"` wipes
all of it at once, which is exactly what Settings → Reset Game relies on.

## Display: official Waveshare driver, 1-bit + dithering

The panel is driven via Waveshare's own **`epd4in2_V2`** reference driver
(vendored in `src/epd_official/`, not a PlatformIO registry package), wrapped
in `src/epd_official/epd_adapter.h`'s `EpdGFX` class so it looks like a plain
Adafruit_GFX display to the rest of `display.cpp` (fillCircle, drawBitmap,
print, etc. all work unchanged). The driver itself was trimmed this session
to drop unused 4-gray/fast-refresh code paths (`Init_4Gray`, `Lut`/`LUT_ALL`,
`Init_Fast`, `Display_Fast`, etc.) that were dead weight given the decision
below.

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

Panel revision selected by `EPD_PANEL_GDEY042T81` in `include/config.h` —
currently unused by `epd_official` (which only supports that revision), kept
as documentation of the assumption.

## Sourcing real-photo art (marmot poses, animal sightings, species icons)

Every bitmap in `include/bitmaps/` (animal sightings, marmot growth-stage
poses, foraging species icons) was generated from a real sourced photo, not
hand-drawn or procedural. The pipeline (scratch dir, not checked in):
Wikipedia opensearch to disambiguate a name (bare species names are
surprisingly homonym-prone — "Chanterelle" alone resolves to a guitar
string, "Huckleberry" to Mark Twain's novel, "Wolf" to a comic book writer),
then either the images actually embedded on that specific Wikipedia article,
or a Commons category/search for the species, scored by a "border variance"
heuristic (lower = cleaner/plainer background) to auto-pick the candidate
most likely to dither cleanly at small size. Crop to content bbox, gamma
curve (~0.5–0.6) to prevent stipple/dark photos collapsing to solid black
when dithered small, Floyd-Steinberg dither via `.convert("1")`, pack
MSB-first into a `static const uint8_t X_BITMAP[N] PROGMEM` array.

This pipeline has a real failure mode worth knowing before re-running it:
busy/textured backgrounds (rock, bark, brush, spotted fur, grass) don't
survive dithering at this size even when the border-variance heuristic
scores them well, and some species (hoary marmot chief among them) have a
stubbornly low hit rate no matter how many candidates get tried — getting to
the current photo set took manually reviewing dozens of rejected candidates
per hard case, not just trusting the heuristic. When the automated pick
fails, a manual crop to isolate just the subject against whatever plain
patch of background is available (sky, snow, blurred bokeh, plain gravel)
plus a stronger gamma boost sometimes rescues an otherwise-noisy photo — this
is how the marmot's baby/juvenile/adult pose variety and about a third of
the harder species photos were ultimately produced, not by the automatic
scoring path alone. Widening the search to zoo/park photos and closely
related species (e.g. yellow-bellied marmot for young-marmot poses, since
hoary marmot pup photos specifically had almost nothing usable) was what
unblocked the baby/juvenile art after the obvious search terms came up empty.

## Hardware gotchas

- Deep-sleep wake (`ext0`) only works on RTC-capable GPIOs (0-21 on the S3),
  so ENTER lives on **GPIO4**. GPIO11-20 are ADC2 channels that share
  hardware with the WiFi/BT radio and read flaky/self-oscillating once that
  radio block has ever been powered, even after `WiFi.mode(WIFI_OFF)`
  (confirmed by reproducing the oscillation on GPIO13 twice) — avoid that
  range for any button/wake pin. LEFT/RIGHT are on GPIO1/2 (ADC1, proven
  reliable).
- The SETTINGS button (GPIO5, the display board's onboard "KEY0") is
  **provisional** — the user recalled the pin number but it hasn't been
  confirmed against the actual board silkscreen/schematic. Flag this if
  asked to touch button/pin config again; don't assume it's verified.
- Buttons are `INPUT_PULLDOWN`, wired to 3V3 **through the power switch**, so
  pressed reads HIGH and buttons are dead when the switch is off (expected).
- `forageIdx` (the Foraging view's browse position) is a plain RAM global,
  not persisted — it resets to 0 every sleep/wake cycle by design, unrelated
  to the relevance-sorted browse order which is also rebuilt fresh each wake.

## Time persists across deep sleep

The ESP32 RTC clock keeps running through deep sleep, so NTP only truly matters
on the first wake after a power-cycle (the inline BAT switch). On later wakes
`time()` is already valid; we still attempt a refresh when online.

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
