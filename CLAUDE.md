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
  a generated `species/species_index.h` that ties all species headers
  together with a name-keyed lookup table (that one's regenerated wholesale,
  not hand-edited, since it's a large auto-derived index rather than a small
  curated set like the animal/marmot/badges folders). It still has entries
  for a handful of names that were cut from `foraging_species.h` in the
  250->200 trim (see below) -- harmless dead PROGMEM data, `species_bitmaps::
  find()` just never gets called with those names anymore, but worth
  regenerating wholesale next time the pipeline runs rather than leaving
  as-is indefinitely.
- Pure-data tables (no runtime/`epd` dependency) live in their own headers,
  `#include`d only from the one `.cpp` that uses them, instead of inline --
  keeps the logic-heavy files scannable. Established pattern:
  `foraging/foraging_species.h` (the 200-entry species table, out of
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

## Species table: name length and the 250->200 trim

`renderForaging()` in `display.cpp` draws each species' display name
centered at text size 2 (`textCentered(0, SCREEN_W, y, f.name, 2)`), with no
wrapping or shrink-to-fit -- the built-in 5x7 font advances 6px/char at size
1, so 12px/char at size 2. SCREEN_W is 300, so any name at or above ~25
characters overflows and reads as cut off on real hardware (confirmed: two
25+ char names -- `"Boletus edulis (King Bolete flush)"` and `"Angel's Wing
Bolete (Suillus lakei)"` -- were visibly clipped). When adding or renaming a
species, keep `name` at 23 characters or fewer (274px, comfortable margin);
anything longer needs a shorter display name (move the detail, e.g. a Latin
binomial or parenthetical, into `note` instead).

The species table was trimmed from 250 to exactly 200 entries in one pass:
every name over the length budget was either shortened (e.g. `"Cortinarius
(Deadly Webcap)"` -> `"Deadly Webcap"`) or, if it turned out to be a
near-duplicate of another entry anyway, deleted rather than renamed. That
covered about 20 of the 50 cuts; the rest were genuine near-duplicate pairs
found by fuzzy name/description matching (same species under two names, e.g.
`"Manila Clam"`/`"Manila Littleneck Clam"`, `"Bladderwrack"`/`"Rockweed"`,
`"Stinging Nettle"`/`"Nettle Tips"`) plus the `sap`/`cambium`-kind entries
(`Bigleaf Maple Sap`, `Western Larch Gum`, `Lodgepole Pine Cambium`, `Western
Hemlock Cambium`) and a chunk of the most obscure, least visually-distinct
mushroom variants (redundant boletes, crust/jelly/coral fungi, puffballs --
the `mushroom` kind was 89/250 entries pre-trim, wildly overrepresented).
Kept species were chosen to preserve one representative per duplicate pair,
prioritizing whichever entry already had real sourced-photo art in
`include/bitmaps/species/` (see the sourcing pipeline notes below) over one
that didn't. `species_index.h` was *not* regenerated as part of this trim
(see above) -- it still has bitmap entries for a few now-deleted names.

## NVS persistence

Everything persisted lives in one `Preferences` namespace, `"forager"`,
opened independently (own `p.begin()`/`p.end()` block) in each of
`creature.cpp`, `events.cpp`, `journal.cpp`, and `minigames.cpp` — no shared
wrapper. Current keys: `hunger`, `happy`, `energy`, `lastFed`, `lastPlayed`,
`birthDate`, `streak`, `streakDay`, `lastStage`, `name`, `named`, `curio`,
`lastCur` (creature.cpp
— `curio`/`lastCur` are the curiosity stat and its own decay clock);
`evType`, `evData`, `evExact`, `evLastAt`, `lastWake`, `engage` (events.cpp —
pending-event + spawn-cooldown state, plus the engagement streak counter);
`eaten`, `discovered` (journal.cpp, two parallel 32-byte species bitsets),
plus simple lifetime counters (`animalSee`, `weatherEv`, `otherEv`, also
journal.cpp); `hsSnack`, `hsSimon`, `hsMemory`, `hsMaze`, `hsQuiz`,
`mgSeen`, `stashG`, `stashL`, `stashT`, `stashF`, `stashYr`, `stashDay`
(minigames.cpp —
one high score per game, indexed by the `Game` enum so that array's order
tracks the enum's; a bitmask of which games have already shown their unlock
screen; and the Winter Stash counts plus the last year settled);
`wifiCount`, `wifiNets`, `wifiSeeded` (wifistore.cpp — the runtime-editable
network list, plus the marker that stops it re-seeding from `secrets.h`); and
`wxValid`, `wxTemp`, `wxRain`, `wxWet`, `wxCond`, `wxAt` (net.cpp — the
cached weather reading and when it was fetched).

A single `Preferences::clear()` on `"forager"` wipes all of it at once, which
is what Settings → Reset Game relies on. **The WiFi keys are the one
exception**: `doResetGame()` reloads them, clears, then calls
`wifistore::persist()` to write them back, because they're device setup rather
than game state — otherwise a reset would drop the networks the owner added
and hand them back whatever `secrets.h` happened to be compiled in.

### Hunger is derived, not stored

`creature::evaluate()` **recomputes** `hunger` outright from `lastFed` every
time it runs (`agingHunger()`), so assigning to `s.hunger` does nothing that
survives — and every caller re-evaluates immediately after, so in practice the
write is dead on the next line. Anything that means to move hunger has to go
through `creature::shiftHunger()`, which shifts `lastFed` instead. This bit
three separate places before it was noticed (Snack Hunt's reward, TrailMishap's
penalty, and `feedForaged()`'s subtraction, which was masked by its own
`lastFed = now`). The other three bars are ceiling-clamps keyed to
`lastPlayed`/`lastCurious`, so they don't have this problem — direct writes to
happiness/energy/curiosity are fine.

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

## Minigames: turn-based only, and where the state lives

`src/minigames/` is rules + state with no rendering and no button reading --
`display::renderMinigames()` draws whatever screen the state says, and
main.cpp's `handleMinigamesInput()` owns the buttons. Two consequences worth
knowing before adding a sixth game:

- **Nothing real-time.** A panel refresh is hundreds of milliseconds, so
  every game is turn-based; the closest thing to pressure is Burrow Maze's
  rising meltwater, and even that is counted in moves rather than seconds --
  nothing advances until a button is pressed. The two non-interactive playback screens (Marmot Says' sequence,
  Forest Memory's look-at-both-cards beat) run off a `MG_FRAME_MS` timer in
  loop() rather than a
  blocking `delay()` loop, so BACK still quits mid-playback.
- **View::Minigames doesn't go through `display::renderView()`** -- it needs
  the game state, which `AppContext` deliberately doesn't carry. main.cpp's
  `renderCurrent()` is the dispatcher every "redraw where we are" call site
  goes through instead.

**Nothing here can be scored on reaction time.** Several designs were tried
and cut on exactly this point (a lane-dodger, a runner): a panel refresh is
hundreds of milliseconds, so "moves on its own, press in time" is unplayable
no matter how the constants are tuned. Every game in the roster was
*already* turn-based before it got put on this device, which is the filter
any sixth game has to pass too.

**A shell game was also tried and cut, for a related reason**: three cups
only works if you can *watch* them move, and rendering the swaps as discrete
labelled frames ("burrows 1 and 2 traded, then 2 and 3") turns it into
mental permutation arithmetic rather than the game anyone means by a shell
game. Motion is not available here, so games that are *about* motion are
out, not just games that are fast.

Games unlocked at birth (Snack Hunt, Marmot Says) are excluded from the
unlock *reveal* -- see `unlockedAtBirth()` in minigames.cpp. First boot
already runs a birth reveal plus the naming flow, and announcing two games
that were never locked was pure ceremony. `mgSeen` therefore only ever
records the three progress-gated ones.

The unlock ladder (see `minigames::isUnlocked()`) mixes two axes: Snack Hunt
and Marmot Says from birth, Forest Memory at Juvenile, Burrow Maze at Adult,
Species Quiz at 50 discovered species. Locked games are **not shown on the
menu at all** -- `visibleGameCount()`/`visibleGameAt()` in minigames.cpp
collapse the menu to just the unlocked ones, and the menu's row index means
"nth unlocked game", not a `Game` enum value, so both the renderer and
main.cpp's LEFT/ENTER handling have to go through those two helpers. (They
used to stay visible with their unlock requirement in place of the blurb;
that read as a wall of locks on a newborn's menu, so the requirement text and
its `unlockHint()` helper are gone.) Crossing a threshold fires a
one-time reveal (`display::renderMinigameUnlock()`), driven by the persisted
`mgSeen` bitmask rather than by "is it unlocked right now", and checked both
at wake and immediately after a Discovery resolves -- that's the one thing
that can cross the species threshold mid-session.

Three further constraints worth knowing:

- **The games' small art is flat silhouettes, and that was settled
  empirically.** Photos were tried first, at the owner's request, and the
  result is recorded here so it isn't retried blind: 63 Commons candidates
  across 7 subjects, scored by border variance, run through the marmot
  pipeline (gamma 0.55 + Floyd-Steinberg) produced *nothing usable* at 56px
  **or at 112px**, and thresholding them instead gave blobs. The reason
  matters -- the marmot photos work because a marmot is one form with a
  strong outline, whereas grass, leaves, wood and cone scales **are**
  texture, so there's no silhouette for dithering to preserve. Photos remain
  the right choice above ~112px with a clear subject: Simon's marmot, the
  Quiz's species art, and Snack Hunt's predator reveal all use them.
  The silhouettes come from named icon sets on Commons, fetched by exact
  filename (a text search for "X silhouette" returns landscape photos and is
  useless): **Noto Emoji v2.034**, which is Google's monochrome release, and
  **Font Awesome 5 solid**. See `include/bitmaps/tokens/`, where each header
  records its source glyph.
- **Forest Memory's card faces are flat silhouettes, never species photos**
  (`drawForestIcon()`). A dithered photo at card size is unreadable speckle
  -- the same finding that made the achievement badges flat icons -- and a
  memory game where two faces can be confused is broken. Four of the six are
  traced bitmaps in `include/bitmaps/tokens/` (from a reference image the
  owner supplied; note these did **not** come through the Commons sourcing
  pipeline the marmot/species/animal art uses, so don't assume the same
  licensing applies if this ever ships beyond a personal device). The other
  two are drawn to match. When adding a face, check it against the existing
  five at card size first: the original sheet had two ferns and two
  rosettes that were indistinguishable at 44px, which would have made the
  game unwinnable-by-skill.
- **Burrow Maze never asks for a direction.** Three buttons can't steer in
  four, and turn-turn-step spends presses on rotating rather than deciding,
  which is miserable at one refresh per press. Instead `mazeOptions()` lists
  the tunnels actually leaving the current cell, LEFT/RIGHT cycle that short
  list, ENTER commits, and `mazeAdvance()` walks the entire corridor to the
  next junction in one go. One press is one *decision*, not one cell.
- **Snack Hunt banks into NVS on every pick**, not at the end of a run (see
  `snackPick()`), so abandoning a run mid-way never loses what it found --
  the stash is a long-term stockpile, not a run score.
- **No minigame draws from the foraging species table except the Quiz.**
  Snack Hunt hands out generic den-stuffing (grass/leaves/twigs/flower) and
  Forest Memory uses procedural forest tokens, both deliberately: the
  Foraging view is where real species live, and a minigame paying out
  "Chanterelle x3" would blur two systems that mean different things.
- **The menu rows and unlock screens carry no icons.** Per-game emblems were
  drawn and then pulled: at the ~14px a menu row affords, procedural icons
  read as smudges beside crisp text and made the list look worse. Both
  screens are typographic instead (name at size 2 over a hairline, size-3
  name in a framed panel on the reveal).

**The Winter Stash** is the one piece of state here that isn't a per-session
score: Snack Hunt's finds accumulate in `stashG`/`stashL`/`stashT`/`stashF`
across days, and the first December wake settles up
(`stashResolveDue()`/`stashResolve()`, driven from main.cpp's
`resolveWinterStashIfDue()`), applying a happiness/energy swing and clearing
the pile for the next year. `stashYr` records the year already settled so a
second December wake doesn't re-run it. The shortfall penalty is deliberately
mild -- this is a goal to aim at, not a second way to kill the marmot -- and
a marmot younger than `WINTER_GRACE_DAYS` at winter is exempt entirely,
since a November hatchling never had a season to gather in.

**Gathering is capped to one run per calendar day** (`stashDay`, checked in
`startSnackRun()`). Without it the whole seasonal arc collapses: "play again"
on the run-over screen is instant, so a determined evening would clear a
three-week goal in one sitting. Later runs stay fully playable and still post
high scores -- they just don't stock the pile, and the screen says so rather
than silently discarding the finds. The day is claimed at the *start* of a
run so quitting a bad run can't re-roll it, and
`DEV_MODE_UNLOCK_MINIGAMES` bypasses the cap so testing isn't gated on the
calendar.

The Species Quiz's questions come from `minigames/quiz_facts.h`, a
name-keyed bank of ~220 clues covering all 200 species, **deliberately not**
the species table's `note`/`harvestTip` -- the Foraging view prints those
verbatim, so quizzing on them would just test whether the player can page
one screen left. Clues must never contain their own species' name; there's a
one-off check for that worth re-running when entries are added.

## Hardware gotchas

- **WiFi will not associate at the default 20dBm TX power on this board.**
  Every attempt ends in `AUTH_EXPIRE` (and eventually `AUTH_FAIL`) against
  *any* AP -- confirmed against both the house network and a phone hotspot at
  -40dBm -- with a correct password. The tell that sends you the wrong way is
  that scanning keeps working perfectly the whole time and reports a strong
  RSSI: a scan only needs the receive path, and it's *transmit* that the cheap
  PA/antenna (in a tight enclosure, with hot glue near the antenna) can't do
  cleanly at full power. `net::connectStrongest()` drops to 8.5dBm, after
  which it associates in ~1.6s.
  **The ordering is the subtle part**: `WiFi.mode(WIFI_STA)` does not actually
  start the STA, so `WiFi.setTxPower()` on its own silently no-ops (it logs
  `Neither AP or STA has been started` and the power reads back as 20dBm).
  `esp_wifi_start()` must come first. Note ../Lander's `connectWifi()` sets TX
  power *without* that call, so its version is a no-op too -- don't copy that
  ordering back over. `connectStrongest()` logs the actual readback for this
  reason; if it ever says 20.00 again, the ordering broke, not the network.
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
  It also resets to 0 on every LEFT/RIGHT transition into or out of the
  Foraging view (`retreatView()`/`advanceView()` in `main.cpp`), so leaving
  for Main and paging back in always lands on species 1 rather than wherever
  you left off.
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
- `DEV_MODE_UNLOCK_MINIGAMES` — `minigames::isUnlocked()` returns true for
  everything, so the whole games menu is playable without waiting on growth
  stage or 50 discovered species. It does **not** touch the unlock *reveal*
  screens: those run off the persisted `mgSeen` bitmask, so after a Reset
  Game they all fire back-to-back on the next wake — annoying if you just
  want to play, handy if you're proofreading them.
- `DEV_MODE_SHOW_ALL_CONTENT` — Snack Hunt deals bushes from a fixed list
  covering every possible content (empty, each stash kind, every critter,
  every predator) instead of rolling, and Forest Memory deals every card
  face-up. Purely for proofreading the art without grinding for a rare
  outcome; it makes Memory trivially winnable, which is the point.
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

## The radio runs before sleep, not at wake

A scan plus connect takes ~17s, and `display::begin()` used to sit behind it,
so every button press left the sleep screen up for most of that. Now a wake
reads weather from NVS (`net::cachedWeather()`) and touches no radio at all;
`refreshNetworkBeforeSleep()` does the NTP + weather pass from `goToSleep()`,
*after* the sleep screen is drawn and the player has walked away, and only
when `net::refreshDue()` says the cache is older than `WEATHER_MAX_AGE_HOURS`.
That also cuts the biggest power draw on the device to a few times a day.

Two thresholds, deliberately different: `WEATHER_MAX_AGE_HOURS` (6) triggers a
refresh, `WEATHER_USABLE_HOURS` (24) is when `cachedWeather()` stops returning
the reading at all and reports `valid == false`. Stale conditions are worse
than none — a `postRain` from three days ago would keep boosting mushroom
relevance long after the ground dried — so one missed refresh degrades
quietly, but a long offline stretch reads as "no weather" rather than as
yesterday's.

The one case that still blocks at wake is an unset clock (`net::clockUnset()`,
i.e. after a power-cycle with no NTP since): nothing time-derived means
anything until that's fixed, so it's worth the wait. `AppContext::netOk` is
now only meaningful inside that branch, and nothing reads it.

## Time persists across deep sleep

The ESP32 RTC clock keeps running through deep sleep, so NTP only truly matters
on the first wake after a power-cycle (the inline BAT switch). On later wakes
`time()` is already valid; we still attempt a refresh when online.

## Commits

Do **not** add a Claude co-author / "Generated with" trailer to commits in this
repo (owner preference).
