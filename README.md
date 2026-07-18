# Forager

A battery-powered e-ink shelf artifact: a foraging tamagotchi. A little PNW
forest spirit — a mycelium creature — lives on the e-ink display and reflects
real-world Seattle foraging conditions. It knows the moon phase, the season,
and whether it just rained (good foraging), and it shows you what's worth
hunting for right now. "Feed" it by confirming you actually went out foraging.

It sleeps almost all the time. A PIR sensor wakes it when you walk past; it
checks the room isn't dark, pulls fresh time/weather over WiFi, updates the
creature, and animates on the e-ink with partial refresh. After a minute of no
interaction it drops back into deep sleep, holding the last image on the screen
at zero power.

## Hardware

| Part | Detail |
|------|--------|
| MCU | ESP32-S3 Super Mini (onboard LiPo charging via USB-C) |
| Display | Waveshare 4.2" e-ink, b/w, 400×300, SPI, partial refresh (GxEPD2) |
| Light sensor | BH1750 ambient light (I2C 0x23) |
| Motion | HC-SR501 PIR — deep-sleep wake source |
| Buttons | 3× tactile (LEFT / RIGHT / ENTER), INPUT_PULLDOWN |
| Battery | LiPo 3.7 V, soldered to BAT+/BAT- |
| Power switch | SPST slide switch inline on BAT+ |

### Pin map (peripheral → ESP32-S3 GPIO)

| Signal | GPIO |
|--------|------|
| E-ink SCK | 12 |
| E-ink MOSI | 11 |
| E-ink CS | 10 |
| E-ink DC | 9 |
| E-ink RST | 8 |
| E-ink BUSY | 7 |
| BH1750 SDA | 5 |
| BH1750 SCL | 6 |
| PIR (wake) | 4 |
| Button LEFT | 1 |
| Button RIGHT | 2 |
| Button ENTER | 3 |

Buttons are wired to 3V3 through the power switch with `INPUT_PULLDOWN`, so a
press reads HIGH and no external resistors are needed.

## Behavior

- **Deep sleep** between interactions; the screen retains its last image at no
  power.
- **Wake on motion** via the PIR on GPIO 4 (RTC wake source).
- **Dark check**: on wake the BH1750 is read; below ~10 lux the room is dark, so
  it goes straight back to sleep without lighting up.
- **On wake (lit room)**: connect WiFi → NTP time → fetch Seattle weather →
  recompute moon + creature → render.
- **Auto-sleep** after 60 s of no button presses.

## Views

Cycle with LEFT / RIGHT; ENTER acts on the current view.

1. **Main** — creature + mood + moon phase + current forageable.
2. **Moon** — large moon render, phase name, days until full / new.
3. **Foraging** — featured species, season notes, current conditions.
4. **Status** — hunger, happiness, last fed. ENTER here = "I went foraging"
   (feeds the creature, resets hunger, boosts happiness).

## The creature

A mycelium forest spirit with moods driven by moon phase, season, weather, and
how long since it was last fed:

`excited` · `content` · `sleepy` · `hungry` · `glowing` (full moon) ·
`dormant` (deep winter).

## Data sources

- **Time** — NTP (no RTC module); Pacific time with US DST rules.
- **Moon** — computed locally from the date, no API.
- **Weather** — [wttr.in](https://wttr.in) JSON for Seattle; recent rainfall and
  temperature. Recent rain ⇒ good foraging.
- **Foraging calendar** — hardcoded PNW/Seattle species by month (see
  `src/foraging/`).

## Project layout

```
platformio.ini          Build config, board, libs, per-module include paths
include/                Shared headers (auto on the compiler path)
  config.h                Pin map + behavioral tunables
  model.h                 Shared data types (MoonInfo, WeatherData, …)
  sprites.h               Sprite IDs shared by foraging DB and renderer
  wifi_creds.h            WifiCred type
  secrets.example.h       Copy to secrets.h and fill in WiFi networks
src/
  main.cpp                State machine: wake → fetch → render → sleep
  moon/                   Local moon-phase math
  net/                    WiFi (strongest-known), NTP, weather fetch + parse
  foraging/               PNW/Seattle species calendar
  creature/               Mood logic, hunger/happiness, persisted across sleep
  display/                GxEPD2 rendering, views, sprites, partial-refresh anim
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

Open the folder in VSCode with the **PlatformIO IDE** extension (recommended in
`.vscode/extensions.json`) and use the build/upload buttons, or from the CLI:

```sh
pio run                 # build
pio run -t upload       # flash over USB-C
pio device monitor      # serial @ 115200
```

## Display panel note

Set `EPD_PANEL_GDEY042T81` in `include/config.h` to match your module. Newer
Waveshare 4.2" revisions (GDEY042T81) support fast partial refresh; the older
GDEW042T2 has limited partial refresh. The wrong choice shows ghosting or a
blank screen.
