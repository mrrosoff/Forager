# Forager

A hoary marmot that lives on a shelf. It is an e-ink tamagotchi in a small
3D-printed enclosure, and it is born the first time you power it on. You keep
it alive by feeding it real Pacific Northwest species, and what it offers you
depends on the season, the month, and the actual weather in Seattle.
Chanterelles show up after an autumn rain and not in March. Neglect it for
about a week and it dies.

Most of its life is deep sleep. Press ENTER, it wakes in a couple of seconds,
redraws the panel, and drops back to sleep after two minutes of being ignored.
A charge lasts a long time because the radio almost never runs.

<p align="center">
  <img src="assets/device.jpg" alt="Forager: a hand-held e-ink marmot tamagotchi in a green 3D-printed enclosure" width="480">
</p>

## Getting started

You need [PlatformIO](https://platformio.org/) and a WiFi network. Copy
`include/secrets.example.h` to `include/secrets.h` and list one or more
networks. On each sync it scans and joins whichever known network is
strongest, so you can leave both your house and your phone hotspot in there.

```cpp
static const WifiCred WIFI_NETWORKS[] = {
    {"home-ssid",  "home-pass"},
    {"phone-ssid", "hotspot-pass"},
};
```

Wire it up per the pin table in `include/config.h`, then build and flash over
the XIAO's USB-C port.

```sh
pio run              # build
pio run -t upload    # flash
pio device monitor   # serial monitor (115200)
```

esptool will claim it reset the board, but on USB-serial-JTAG it doesn't
actually start the app. Tap RESET after a flash.

There is no power switch, because the enclosure lost it. A brand new device
sits on a blank screen with its buttons armed until you press LEFT and RIGHT
together, which is the power-on gesture. That only ever happens once per
device. After that it runs the birth sequence and asks you to name the marmot.

Everything else configures itself. Weather comes from
[wttr.in](https://wttr.in/) and the clock from NTP, and neither one needs a
key.

## How it lives

**Four bars, three of them lethal.** Hunger, Happiness and Energy all decay
over roughly a week and will kill the marmot at either extreme. Curiosity is a
fourth bar that just sulks. Young marmots decay slower, which is the grace
period for figuring out what the buttons do. Every bar has something that
fills it. Eating and Snack Hunt cover Hunger, Marmot Says and resolved events
cover Happiness, Burrow Maze covers Energy, and the species games and
Discoveries cover Curiosity.

**It grows up on variety, not on time.** Baby to Juvenile to Adult is driven
by how many *distinct* species it has eaten, so a marmot fed the same
huckleberry every day stays a baby.

**Four views, LEFT and RIGHT to walk between them.** Minigames, Status, Main,
Foraging, with Main in the middle. Main is the marmot itself, along with its
mood, the weather and anything currently demanding attention. Foraging pages
through what is in season right now, sorted by what is actually worth picking,
and ENTER eats it. Status is the raw numbers for when you want to know exactly
how bad things are.

**Events happen while you're away.** Every six hours or so there's a chance
the next wake opens on a discovery, an animal sighting, a trail mishap, a
weather turn, a treasure, or an encounter, and it holds Main until you deal
with it. Playing with the thing regularly raises the odds, which is the point.

**Five minigames, all turn-based.** A panel refresh takes most of a second, so
nothing here can be scored on reflexes. Every game waits for you. Snack Hunt
and Marmot Says are there from birth, Forest Memory arrives at Juvenile,
Burrow Maze at Adult, and the Species Quiz once you've discovered 50 species.
Locked games aren't listed at all. Each one keeps a high score and gets a
one-time reveal screen when it unlocks.

**Winter is coming, specifically.** Snack Hunt's finds pile up in a stash
across days, and only the first run of each day counts, so you can't clear the
goal in one determined evening. The first December wake settles the books
against a 120-point goal. Falling short stings a little. It is not a second
way to kill the marmot.

The SETTINGS button (the one on the display module) gets you Achievements,
WiFi networks, Reset Game and Power Off.

## Hardware

| Part | Detail |
|------|--------|
| MCU | Seeed XIAO ESP32S3, 8 MB flash, charges the cell over the same USB-C you flash through |
| Display | Waveshare 4.2" e-ink, 400×300 physical, mounted portrait for a 300×400 canvas |
| Buttons | 4× tactile, LEFT / RIGHT / ENTER, plus the display module's own KEY1 as SETTINGS |
| Battery | LiPo 3.7 V, soldered to BAT+/BAT- |

The XIAO breaks out exactly eleven GPIO and this needs exactly eleven, so
there is no spare and no room to change your mind. GPIO43/44 are neither
RTC-capable nor ADC, which rules them out for the three wake buttons and for
the battery divider, so they take display signals instead.

LEFT, RIGHT and ENTER are pulled down, read active-HIGH, and all three wake
the board out of deep sleep. SETTINGS rides the display board's KEY1, which is
wired to ground and therefore active-LOW, the opposite polarity from
everything else. It isn't a wake source, so Settings only exists once the
device is already awake.

One thing worth knowing before you debug the wrong end. This board will not
associate at the default 20 dBm. Scanning works fine the whole time and
reports a strong signal, because a scan only needs the receive path. Dropping
TX power to 8.5 dBm fixes it in about 1.6 seconds.

## Build, format, lint

```sh
pio run              # build
pio run -t upload    # flash
pio device monitor   # serial @ 115200

clang-format -i $(find src include -name '*.cpp' -o -name '*.h' | grep -v epd_official)
pio run -t compiledb    # then clang-tidy against src/, skipping epd_official/
```

Or just use the PlatformIO extension in VSCode, which
`.vscode/extensions.json` will offer you.
