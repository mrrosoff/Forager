# CLAUDE.md

Working notes for this repo that aren't in the README. See README.md for the
product description, hardware table, pin map, views, growth/death mechanics,
settings behavior, build/flash/format/lint commands, and behavior overview.

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
- `include/bitmaps/{animals,badges,marmot,species}/` are generated art -- one
  PROGMEM header per photo/icon (see the sourcing pipeline notes below), plus
  a generated `species/species_index.h` that ties all 250 species headers
  together with a name-keyed lookup table (that one's regenerated wholesale,
  not hand-edited, since it's a large auto-derived index rather than a small
  curated set like the animal/marmot/badges folders).
- Pure-data tables (no runtime/`epd` dependency) live in their own headers,
  `#include`d only from the one `.cpp` that uses them, instead of inline --
  keeps the logic-heavy files scannable. Established pattern:
  `foraging/foraging_species.h` (the 250-entry species table, out of
  `foraging.cpp`), `events/events_data.h` (mishap/weather/treasure/encounter/
  baby-care pools, out of `events.cpp`), `display/display_marmot_art.h` +
  `display_animal_art.h` + `display_thoughts.h` (pose-art tables and
  thought-bubble phrase pools, out of `display.cpp`). These headers rely on
  being included *after* whatever bitmap headers or macros (e.g.
  `foraging_species.h`'s `MO(month)` bitmask macro, defined by `foraging.cpp`
  before including it) they reference -- they're textually inlined, so
  include order at the call site matters, same as any other header split
  this way. Actual rendering/drawing logic (tightly coupled to the shared
  `epd` instance) stays in `display.cpp` rather than being split further --
  splitting that would require exposing `epd` via an extern in a shared
  internal header, which hasn't been worth the risk yet.

## NVS persistence

Everything persisted lives in one `Preferences` namespace, `"forager"`,
opened independently (own `p.begin()`/`p.end()` block) in each of
`creature.cpp`, `events.cpp`, and `journal.cpp` — no shared wrapper. Current
keys: `hunger`, `happy`, `energy`, `lastFed`, `lastPlayed`, `birthDate`,
`streak`, `streakDay`, `lastStage`, `name` (creature.cpp); `evType`, `evData`,
`evExact`, `evLastAt`, `lastWake`, `engage` (events.cpp — pending-event +
spawn-cooldown state, plus the engagement streak counter); `eaten`,
`discovered` (journal.cpp, two parallel 32-byte species bitsets), plus
simple lifetime counters (`animalSee`, `weatherEv`, `otherEv`, also
journal.cpp). A single `Preferences::clear()` on `"forager"` wipes all of it
at once, which is exactly what Settings → Reset Game relies on.

## Display: official Waveshare driver, 1-bit + dithering

The panel is driven via Waveshare's own **`epd4in2_V2`** reference driver
(vendored in `src/epd_official/`, not a PlatformIO registry package), wrapped
in `src/epd_official/epd_adapter.h`'s `EpdGFX` class so it looks like a plain
Adafruit_GFX display to the rest of `display.cpp` (fillCircle, drawBitmap,
print, etc. all work unchanged). The driver itself was trimmed early on to
drop unused 4-gray/fast-refresh code paths (`Init_4Gray`, `Lut`/`LUT_ALL`,
`Init_Fast`, `Display_Fast`, etc.) that were dead weight given the decision
below.

We tried the panel's true 4-level grayscale earlier (via the third-party
`GxEPD2_4G` fork) but dropped it: a 4-grey full refresh is inherently
multi-pass and flickery, and this driver can't mix a grey full-refresh with a
fast b/w partial refresh in the same session without corrupting the panel
(confirmed by reproducing solid-grey corruption when we tried). So the whole
UI is **plain 1-bit black/white**, with grey shading *approximated* via
ordered (Bayer 4x4) dithering (see `dFillRect`/`dFillCircle`/etc. in
`display.cpp`).

**Full vs. partial refresh, and why forcing "full" alone isn't enough**:
`EpdGFX::endFrame(partial)` forces a full refresh on the first frame after
`epd.begin()` regardless of what's requested, and honors `partial` after
that (`display::renderView()`'s normal calls all request partial). But the
vendor driver's `Display_Partial()` reconfigures panel registers (border
waveform, RAM window) via its own `Reset()`, and `Display()` (the "full"
path) never restores that state afterward -- so simply calling
`endFrame(false)` mid-session, after any partial refreshes have already
happened, runs on top of a controller left in a dirty partial-mode
configuration and does **not** actually clear the panel (confirmed by
reproducing persistent ghosting under freshly-drawn content on real
hardware, even across two such "full" passes in a row). The real fix,
used by `renderView(..., forceFullRefresh=true)` (the mid-session
guaranteed-event trigger, see events below): re-run `epd.begin()` (the full
`Init()` sequence) immediately before drawing, putting the controller back
in the same known-clean state every normal wake's first frame already
relies on.

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

Every bitmap in `include/bitmaps/{animals,marmot,species}/` was generated
from a real sourced photo, not hand-drawn or procedural. The pipeline
(scratch dir, not checked in): Wikipedia opensearch to disambiguate a name
(bare species names are surprisingly homonym-prone — "Chanterelle" alone
resolves to a guitar string, "Huckleberry" to Mark Twain's novel, "Wolf" to
a comic book writer), then either the images actually embedded on that
specific Wikipedia article, or a Commons category/search for the species,
scored by a "border variance" heuristic (lower = cleaner/plainer background)
to auto-pick the candidate most likely to dither cleanly at small size. Crop
to content bbox, gamma curve (~0.5–0.6) to prevent stipple/dark photos
collapsing to solid black when dithered small, Floyd-Steinberg dither via
`.convert("1")`, pack MSB-first into a `static const uint8_t X_BITMAP[N]
PROGMEM` array.

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
unblocked the baby/juvenile art after the obvious search terms came up
empty. Real "sleeping/lounging pup" photos in particular don't turn up at
all — Baby/Juvenile's sleep screen reuses their normal standing pose rather
than force a bad match.

**Badge icons are a different pipeline** from the photo bitmaps above:
sourced as flat, openly-licensed icons (game-icons.net, OOjs UI, Bootstrap
icons, Creative Tail — all via Wikimedia Commons) rather than photos, then
thresholded (not gamma-dithered like the photo pipeline — full
photo-texture dithering turns to unrecognizable speckle at 46px, the badge
circle's size) into a two-tone look: dark regions solid black, mid-tone
regions get the same Bayer-dither pattern `dFillCircle`/etc. use elsewhere,
so there's still a solid black "anchor" shape keeping each icon
recognizable instead of an all-over stipple.

## Hardware gotchas

- Deep-sleep wake uses `esp_sleep_enable_ext1_wakeup()` (ANY_HIGH) across all
  three of LEFT/RIGHT/ENTER, which only works on RTC-capable GPIOs (0-21 on
  the S3). GPIO11-20 are ADC2 channels that share hardware with the WiFi/BT
  radio and read flaky/self-oscillating once that radio block has ever been
  powered, even after `WiFi.mode(WIFI_OFF)` (confirmed by reproducing the
  oscillation on GPIO13 twice) — avoid that range for any button/wake pin.
- SETTINGS/KEY1's inverted polarity (see README's hardware section) is
  handled via a `Btn::activeHigh` bool field on the shared button-debounce
  struct in `main.cpp` — `pressed()` XORs the raw digitalRead against it, so
  LEFT/RIGHT/ENTER (activeHigh=true) and SETTINGS (activeHigh=false) share
  the same debounce/edge-detect code path.
- `forageIdx` (the Foraging view's browse position) is a plain RAM global,
  not persisted — it resets to 0 every sleep/wake cycle by design, unrelated
  to the relevance-sorted browse order which is also rebuilt fresh each wake.
- `checkForEvent()`'s spawn-check is explicitly skipped on `firstBoot`: it
  treats a never-set `evLastAt` as "cooldown already elapsed" (nothing to
  measure from yet), so without the guard a newborn marmot could roll
  straight into a pending event before the player's even finished naming it.

## Dev-mode flags (config.h)

- `DEV_MODE_NO_SLEEP` — disables deep sleep entirely so the board stays on
  USB serial while iterating.
- `DEV_MODE_SKIP_GROWTH` — `creature::computeStage()` always returns Adult.
  Also **implicitly unlocks Adult-only event types** (ForagingFind, TrailMishap,
  TrailTreasure) regardless of the marmot's real age, since those are gated
  on the same computed stage — a newborn will immediately see Adult-only
  content and events if this is left on, which reads as a bug if you forget
  it's set (confirmed happening: an immediate post-naming ForagingFind that
  couldn't be acknowledged via ENTER, by design, since ForagingFind only
  resolves by eating a matching species on Foraging).
- Both flip to `0` before any real gameplay testing session — flip back to
  `1` only while actively iterating on stage/growth-adjacent or hardware
  timing-adjacent features, and remember to flip back before handing the
  device off for a normal playtest.
- A third flag, `DEV_MODE_EVENT_CYCLE`, exists in `main.cpp`/`config.h` as a
  review-only loop (bypasses WiFi/game state, shows every distinct
  wake-time-event flavor back-to-back, ENTER advances) for proofreading
  event text/layout on real hardware. Off (`0`) by default; flipping it on
  measurably shrinks the compiled binary since the rest of `setup()`/`loop()`
  becomes unreachable dead code the linker strips — don't be alarmed by the
  smaller flash-usage percentage while it's on, and always flip it back off
  before committing.

## Time persists across deep sleep

The ESP32 RTC clock keeps running through deep sleep, so NTP only truly matters
on the first wake after a power-cycle (the inline BAT switch). On later wakes
`time()` is already valid; we still attempt a refresh when online.

## Commits

Do **not** add a Claude co-author / "Generated with" trailer to commits in this
repo (owner preference).
