#include "display.h"

#include <SPI.h>
#include <math.h>

#include <algorithm>
#include <string>

#include "bitmaps/animals/bald_eagle_bitmap.h"
#include "bitmaps/animals/banana_slug_bitmap.h"
#include "bitmaps/animals/barred_owl_bitmap.h"
#include "bitmaps/animals/beaver_bitmap.h"
#include "bitmaps/animals/black_bear_bitmap.h"
#include "bitmaps/animals/bobcat_bitmap.h"
#include "bitmaps/animals/chickadee_bitmap.h"
#include "bitmaps/animals/cougar_bitmap.h"
#include "bitmaps/animals/coyote_bitmap.h"
#include "bitmaps/animals/deer_bitmap.h"
#include "bitmaps/animals/douglas_squirrel_bitmap.h"
#include "bitmaps/animals/elk_bitmap.h"
#include "bitmaps/animals/gray_wolf_bitmap.h"
#include "bitmaps/animals/great_blue_heron_bitmap.h"
#include "bitmaps/animals/harbor_seal_bitmap.h"
#include "bitmaps/animals/mountain_goat_bitmap.h"
#include "bitmaps/animals/orca_bitmap.h"
#include "bitmaps/animals/osprey_bitmap.h"
#include "bitmaps/animals/pacific_tree_frog_bitmap.h"
#include "bitmaps/animals/pileated_woodpecker_bitmap.h"
#include "bitmaps/animals/raccoon_bitmap.h"
#include "bitmaps/animals/red_fox_bitmap.h"
#include "bitmaps/animals/river_otter_bitmap.h"
#include "bitmaps/animals/snowshoe_hare_bitmap.h"
#include "bitmaps/animals/stellers_jay_bitmap.h"
#include "bitmaps/badges/berries_bitmap.h"
#include "bitmaps/badges/complete_bitmap.h"
#include "bitmaps/badges/halfway_bitmap.h"
#include "bitmaps/badges/herbalist_bitmap.h"
#include "bitmaps/badges/mushrooms_bitmap.h"
#include "bitmaps/badges/naturalist_bitmap.h"
#include "bitmaps/badges/storms_bitmap.h"
#include "bitmaps/badges/wanderer_bitmap.h"
#include "bitmaps/badges/wildlife_bitmap.h"
#include "bitmaps/marmot/marmot_baby2_bitmap.h"
#include "bitmaps/marmot/marmot_baby2_small_bitmap.h"
#include "bitmaps/marmot/marmot_baby3_bitmap.h"
#include "bitmaps/marmot/marmot_baby4_bitmap.h"
#include "bitmaps/marmot/marmot_baby4_small_bitmap.h"
#include "bitmaps/marmot/marmot_baby6_bitmap.h"
#include "bitmaps/marmot/marmot_baby6_small_bitmap.h"
#include "bitmaps/marmot/marmot_baby7_bitmap.h"
#include "bitmaps/marmot/marmot_baby7_small_bitmap.h"
#include "bitmaps/marmot/marmot_baby_bitmap.h"
#include "bitmaps/marmot/marmot_baby_small_bitmap.h"
#include "bitmaps/marmot/marmot_bitmap.h"
#include "bitmaps/marmot/marmot_death_bitmap.h"
#include "bitmaps/marmot/marmot_juvenile2_bitmap.h"
#include "bitmaps/marmot/marmot_juvenile2_small_bitmap.h"
#include "bitmaps/marmot/marmot_juvenile3_bitmap.h"
#include "bitmaps/marmot/marmot_juvenile3_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant10_bitmap.h"
#include "bitmaps/marmot/marmot_variant10_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant11_bitmap.h"
#include "bitmaps/marmot/marmot_variant11_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant13_bitmap.h"
#include "bitmaps/marmot/marmot_variant13_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant15_bitmap.h"
#include "bitmaps/marmot/marmot_variant15_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant16_bitmap.h"
#include "bitmaps/marmot/marmot_variant16_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant17_bitmap.h"
#include "bitmaps/marmot/marmot_variant17_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant18_bitmap.h"
#include "bitmaps/marmot/marmot_variant18_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant20_bitmap.h"
#include "bitmaps/marmot/marmot_variant20_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant21_bitmap.h"
#include "bitmaps/marmot/marmot_variant21_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant22_bitmap.h"
#include "bitmaps/marmot/marmot_variant22_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant24_bitmap.h"
#include "bitmaps/marmot/marmot_variant24_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant25_bitmap.h"
#include "bitmaps/marmot/marmot_variant25_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant26_bitmap.h"
#include "bitmaps/marmot/marmot_variant26_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant27_bitmap.h"
#include "bitmaps/marmot/marmot_variant27_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant28_bitmap.h"
#include "bitmaps/marmot/marmot_variant28_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant5_bitmap.h"
#include "bitmaps/marmot/marmot_variant5_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant6_bitmap.h"
#include "bitmaps/marmot/marmot_variant6_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant7_bitmap.h"
#include "bitmaps/marmot/marmot_variant7_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant8_bitmap.h"
#include "bitmaps/marmot/marmot_variant8_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant9_bitmap.h"
#include "bitmaps/marmot/marmot_variant9_small_bitmap.h"
#include "bitmaps/species/species_index.h"
#include "bitmaps/tokens/token_beetle.h"
#include "bitmaps/tokens/token_bloom.h"
#include "bitmaps/tokens/token_branch.h"
#include "bitmaps/tokens/token_bush.h"
#include "bitmaps/tokens/token_cricket.h"
#include "bitmaps/tokens/token_eyes.h"
#include "bitmaps/tokens/token_fern.h"
#include "bitmaps/tokens/token_grass.h"
#include "bitmaps/tokens/token_leaf.h"
#include "bitmaps/tokens/token_leafpile.h"
#include "bitmaps/tokens/token_lizard.h"
#include "bitmaps/tokens/token_marmot.h"
#include "bitmaps/tokens/token_spider.h"
#include "bitmaps/tokens/token_toadstool.h"
#include "bitmaps/tokens/token_tree.h"
#include "bitmaps/tokens/token_wood.h"
#include "config.h"
#include "creature.h"
#include "epd_adapter.h"
#include "events.h"
#include "foraging.h"
#include "journal.h"
#include "textentry.h"
#include "wifistore.h"

namespace display {

// Logical portrait canvas after rotation (see epd.setRotation(3) in
// begin()). The panel is physically 400x300 landscape; EpdGFX's
// rotation-aware drawPixel() remaps these logical coordinates onto that
// fixed physical buffer, so all layout code below targets 300x400.
static const int SCREEN_W = 300, SCREEN_H = 400;

// Waveshare's official epd4in2_V2 driver, wrapped in an Adafruit_GFX
// adapter (see epd_adapter.h). Plain 1-bit black/white: 4-grey mode forces
// a multi-pass flicker on every full refresh and can't be safely mixed with
// fast partial refresh on this hardware, so grayscale is approximated with
// dithering instead of using the panel's true grey levels.
static EpdGFX epd;

static const uint16_t C_BLACK = EPD_GFX_BLACK;
static const uint16_t C_WHITE = EPD_GFX_WHITE;

enum Shade : uint8_t { SHADE_BLACK, SHADE_DARK, SHADE_LIGHT, SHADE_WHITE };

static const uint8_t BAYER4[4][4] = {
    {0, 8, 2, 10},
    {12, 4, 14, 6},
    {3, 11, 1, 9},
    {15, 7, 13, 5},
};

static inline bool ditherBlack(int x, int y, Shade shade) {
  uint8_t t = BAYER4[y & 3][x & 3];
  return shade == SHADE_DARK ? t < 11 : t < 5;
}

static void dFillRect(int x, int y, int w, int h, Shade shade) {
  if (shade == SHADE_BLACK) {
    epd.fillRect(x, y, w, h, C_BLACK);
    return;
  }
  if (shade == SHADE_WHITE) return;
  for (int yy = y; yy < y + h; yy++)
    for (int xx = x; xx < x + w; xx++)
      if (ditherBlack(xx, yy, shade)) epd.drawPixel(xx, yy, C_BLACK);
}

static void dFillVLine(int x, int y, int h, Shade shade) {
  if (shade == SHADE_BLACK) {
    epd.drawFastVLine(x, y, h, C_BLACK);
    return;
  }
  if (shade == SHADE_WHITE) return;
  for (int yy = y; yy < y + h; yy++)
    if (ditherBlack(x, yy, shade)) epd.drawPixel(x, yy, C_BLACK);
}

static void dFillHLine(int x, int y, int w, Shade shade) {
  if (shade == SHADE_BLACK) {
    epd.drawFastHLine(x, y, w, C_BLACK);
    return;
  }
  if (shade == SHADE_WHITE) return;
  for (int xx = x; xx < x + w; xx++)
    if (ditherBlack(xx, y, shade)) epd.drawPixel(xx, y, C_BLACK);
}

static void dFillCircle(int cx, int cy, int r, Shade shade) {
  if (shade == SHADE_BLACK) {
    epd.fillCircle(cx, cy, r, C_BLACK);
    return;
  }
  if (shade == SHADE_WHITE) return;
  for (int yy = -r; yy <= r; yy++) {
    int dx = (int)sqrtf((float)(r * r - yy * yy));
    for (int xx = -dx; xx <= dx; xx++)
      if (ditherBlack(cx + xx, cy + yy, shade)) epd.drawPixel(cx + xx, cy + yy, C_BLACK);
  }
}

static void dFillRoundRect(int x, int y, int w, int h, int r, Shade shade) {
  if (shade == SHADE_BLACK) {
    epd.fillRoundRect(x, y, w, h, r, C_BLACK);
    return;
  }
  if (shade == SHADE_WHITE) return;
  for (int yy = y; yy < y + h; yy++) {
    for (int xx = x; xx < x + w; xx++) {
      int ccx = -1, ccy = -1;
      if (xx < x + r && yy < y + r) {
        ccx = x + r;
        ccy = y + r;
      } else if (xx >= x + w - r && yy < y + r) {
        ccx = x + w - r - 1;
        ccy = y + r;
      } else if (xx < x + r && yy >= y + h - r) {
        ccx = x + r;
        ccy = y + h - r - 1;
      } else if (xx >= x + w - r && yy >= y + h - r) {
        ccx = x + w - r - 1;
        ccy = y + h - r - 1;
      }
      if (ccx >= 0) {
        int dx = xx - ccx, dy = yy - ccy;
        if (dx * dx + dy * dy > r * r) continue;
      }
      if (ditherBlack(xx, yy, shade)) epd.drawPixel(xx, yy, C_BLACK);
    }
  }
}

// Blocky built-in 5x7 font scaled by `size`, for a chunky pixel-art look.
static void textAt(int x, int y, const char* s, uint8_t size = 1) {
  epd.setFont(nullptr);
  epd.setTextSize(size);
  epd.setTextColor(C_BLACK);
  epd.setCursor(x, y);
  epd.print(s);
}

static void textCentered(int x0, int w, int y, const char* s, uint8_t size = 1) {
  epd.setFont(nullptr);
  epd.setTextSize(size);
  int16_t bx, by;
  uint16_t bw, bh;
  epd.getTextBounds(s, 0, y, &bx, &by, &bw, &bh);
  textAt(x0 + (w - (int)bw) / 2 - bx, y, s, size);
}

static void textAt(int x, int y, const std::string& s, uint8_t size = 1) {
  textAt(x, y, s.c_str(), size);
}

static void textCentered(int x0, int w, int y, const std::string& s, uint8_t size = 1) {
  textCentered(x0, w, y, s.c_str(), size);
}

// Word-wraps `s` into lines of at most `maxChars` characters (breaking on
// spaces), drawing each line `size`*8+2 px apart starting at (x,y). Returns
// the y coordinate just past the last line, for stacking content below it.
static int textWrapped(int x, int y, int maxChars, const char* s, uint8_t size = 1) {
  int lineH = size * 8 + 2;
  std::string rest = s;
  while (!rest.empty()) {
    int n = 0;
    int lastSpace = -1;
    while (n < (int)rest.size() && n < maxChars) {
      if (rest[n] == ' ') lastSpace = n;
      n++;
    }
    if (n < (int)rest.size() && lastSpace >= 0) n = lastSpace;  // break at last space in range
    textAt(x, y, rest.substr(0, n), size);
    y += lineH;
    rest.erase(0, n);
    while (!rest.empty() && rest.front() == ' ') rest.erase(0, 1);
  }
  return y;
}

// Full-screen creature stage: the marmot bitmap already includes its own
// rock ledge, so there's no separate habitat/ground drawing here anymore --
// just the horizontal span the creature is centered within.
static const int STAGE_X = 10, STAGE_Y = 34, STAGE_W = 280, STAGE_H = 330;

// Small weather glyph (sun / rain / cloud) so the creature's world reflects
// current conditions, independent of mood.
static void drawWeatherGlyph(int x, int y, const WeatherData& w) {
  if (!w.valid) return;
  if (w.postRain) {
    dFillCircle(x + 12, y + 8, 9, SHADE_LIGHT);
    epd.drawCircle(x + 12, y + 8, 9, C_BLACK);
    for (int i = 0; i < 3; i++) epd.drawLine(x + 4 + i * 8, y + 16, x + 2 + i * 8, y + 24, C_BLACK);
  } else if (w.tempC >= 18.0f) {
    epd.drawCircle(x + 12, y + 10, 7, C_BLACK);
    for (int a = 0; a < 360; a += 45) {
      float ra = a * (float)M_PI / 180.0f;
      epd.drawLine(x + 12 + (int)(9 * cosf(ra)), y + 10 + (int)(9 * sinf(ra)),
                   x + 12 + (int)(14 * cosf(ra)), y + 10 + (int)(14 * sinf(ra)), C_BLACK);
    }
  } else {
    dFillCircle(x + 8, y + 12, 8, SHADE_LIGHT);
    dFillCircle(x + 18, y + 10, 10, SHADE_LIGHT);
    epd.drawCircle(x + 8, y + 12, 8, C_BLACK);
    epd.drawCircle(x + 18, y + 10, 10, C_BLACK);
  }
}

// Simple battery gauge: outline + a small terminal nub + a black fill
// proportional to percent (0-100), floored at a 20%-equivalent width so a
// near-empty reading still shows a visible sliver instead of disappearing
// into the outline at this small icon size.
static const int BATT_ICON_W = 20, BATT_ICON_H = 10, BATT_NUB_W = 2;

static void drawBatteryIcon(int x, int y, uint8_t percent) {
  epd.drawRect(x, y, BATT_ICON_W, BATT_ICON_H, C_BLACK);
  epd.fillRect(x + BATT_ICON_W, y + BATT_ICON_H / 2 - 2, BATT_NUB_W, 4, C_BLACK);
  int fillW = (BATT_ICON_W - 4) * std::max<int>(percent, 20) / 100;
  epd.fillRect(x + 2, y + 2, fillW, BATT_ICON_H - 4, C_BLACK);
}

#include "display_marmot_art.h"

// A hoary marmot -- hardcoded, dithered pen-and-ink-style/photo bitmaps (see
// include/bitmaps/marmot/) rather than live procedural shapes, for real fur
// texture and a recognizable silhouette. No procedural eye/nose/sparkle
// overlay -- just the bitmap.
// Deep sleep wipes ordinary RAM, so a function-local static naturally
// re-rolls the first time it's read each wake and then holds steady for
// the rest of the session -- shared helper for every per-wake pose pool
// (baby/juvenile/adult) below.
static int8_t pickVariant(int8_t& cache, int count) {
  if (cache < 0) cache = (int8_t)random(count);
  return cache;
}

// small=true picks from the genuinely-smaller-canvas bitmap pools (see
// their declarations above) instead of runtime-scaling the full-size ones
// -- nearest-neighbor scaling an already-dithered 1-bit image aliases the
// dither pattern into noise, so the Status view needs real small assets.
// Returns the top pixel row the art was drawn at, so callers can position
// things (like the Main view's thought bubble) relative to the actual pose
// height instead of a fixed offset that leaves a gap above shorter poses.
// The pixel rectangle a pose actually occupies. Poses differ a lot in width
// and height, so anything positioned relative to the marmot (the thought
// bubble) has to read this rather than assume a fixed spot.
struct ArtBox {
  int x, y, w, h;
};

static int drawCreature(int cx, int groundY, Mood mood, Stage stage, bool small = false,
                        ArtBox* box = nullptr) {
  if (stage == Stage::Baby || stage == Stage::Juvenile) {
    static int8_t babyVariant = -1;
    static int8_t juvenileVariant = -1;
    int idx = (stage == Stage::Baby) ? pickVariant(babyVariant, kBabyVariantCount)
                                     : pickVariant(juvenileVariant, kJuvenileVariantCount);
    const MarmotArt& art = (stage == Stage::Baby)
                               ? (small ? kBabyVariantsSmall[idx] : kBabyVariants[idx])
                               : (small ? kJuvenileVariantsSmall[idx] : kJuvenileVariants[idx]);
    int bx = cx - art.w / 2;
    int by = groundY - art.groundY;
    epd.drawBitmap(bx, by, art.bitmap, art.w, art.h, C_BLACK, C_WHITE);
    if (box) *box = {bx, by, art.w, art.h};
    return by;
  }

  static int8_t excitedVariant = -1;
  static int8_t contentVariant = -1;
  static int8_t sleepyVariant = -1;
  static int8_t annoyedVariant = -1;
  static int8_t hungryVariant = -1;
  static int8_t scaredVariant = -1;
  const MarmotArt* art;
  switch (mood) {
    case Mood::Excited: {
      int idx = pickVariant(excitedVariant, kMarmotExcitedCount);
      art = small ? &kMarmotExcitedSmall[idx] : &kMarmotExcited[idx];
      break;
    }
    case Mood::Sleepy:
    case Mood::Dormant: {
      int idx = pickVariant(sleepyVariant, kMarmotSleepyCount);
      art = small ? &kMarmotSleepySmall[idx] : &kMarmotSleepy[idx];
      break;
    }
    case Mood::Annoyed: {
      int idx = pickVariant(annoyedVariant, kMarmotAnnoyedCount);
      art = small ? &kMarmotAnnoyedSmall[idx] : &kMarmotAnnoyed[idx];
      break;
    }
    case Mood::Hungry: {
      int idx = pickVariant(hungryVariant, kMarmotHungryCount);
      art = small ? &kMarmotHungrySmall[idx] : &kMarmotHungry[idx];
      break;
    }
    case Mood::Scared: {
      int idx = pickVariant(scaredVariant, kMarmotScaredCount);
      art = small ? &kMarmotScaredSmall[idx] : &kMarmotScared[idx];
      break;
    }
    case Mood::Content:
    default: {
      int idx = pickVariant(contentVariant, kMarmotContentCount);
      art = small ? &kMarmotContentSmall[idx] : &kMarmotContent[idx];
      break;
    }
  }
  int bx = cx - art->w / 2;
  int by = groundY - art->groundY;
  epd.drawBitmap(bx, by, art->bitmap, art->w, art->h, C_BLACK, C_WHITE);
  if (box) *box = {bx, by, art->w, art->h};
  return by;
}

// Bottom nav bar shown on every view: what LEFT/RIGHT/ENTER do from here.
static const int NAV_Y = SCREEN_H - 14;

// leftIsDown swaps the usual "<" prefix for a down-arrow glyph: on a
// vertical list (the minigames menu) LEFT steps *down* the list rather than
// navigating back a view, and "<Pick" reads as "go left to somewhere",
// which is exactly what it doesn't do. The built-in font has no arrow
// glyph, so it's drawn as a triangle.
static void drawNavBar(const char* leftLbl, const char* enterLbl, const char* rightLbl,
                       bool leftIsDown = false) {
  epd.drawFastHLine(4, NAV_Y - 4, SCREEN_W - 8, C_BLACK);
  if (leftIsDown) {
    epd.fillTriangle(4, NAV_Y + 1, 13, NAV_Y + 1, 8, NAV_Y + 8, C_BLACK);
    textAt(17, NAV_Y, leftLbl, 1);
  } else {
    std::string left = std::string("<") + leftLbl;
    textAt(4, NAV_Y, left, 1);
  }
  textCentered(0, SCREEN_W, NAV_Y, enterLbl, 1);
  std::string right = std::string(rightLbl) + ">";
  int16_t bx, by;
  uint16_t bw, bh;
  epd.setFont(nullptr);
  epd.setTextSize(1);
  epd.getTextBounds(right.c_str(), 0, NAV_Y, &bx, &by, &bw, &bh);
  textAt(SCREEN_W - 4 - (int)bw, NAV_Y, right, 1);
}

// Per-animal reference-photo artwork for the encounter screen, indexed the
// same way as events::animalIndex() -- i.e. positionally matching kAnimals[]
// in events.cpp. Not every animal has art (some source photos didn't dither
// into a recognizable silhouette); nullptr means "fall back to the marmot".
#include "display_animal_art.h"

// A pending wildlife sighting takes over the Main view until ENTER
// resolves it (see events::checkForEvent / onEnter() in main.cpp).
static void renderEncounter(const AppContext& ctx, const events::PendingEvent& ev) {
  bool negative = events::eventIsNegative(ev);
  textCentered(0, SCREEN_W, 16, events::eventTitle(ev.type, negative), 2);

  const AnimalArt* art = nullptr;
  if (ev.type == events::EventType::AnimalSighting) {
    uint8_t idx = events::animalIndex(ev);
    if (idx < kAnimalArtCount && kAnimalArt[idx].bitmap != nullptr) art = &kAnimalArt[idx];
  }
  const species_bitmaps::SpeciesBitmap* speciesArt = nullptr;
  if (ev.type == events::EventType::Discovery) {
    speciesArt = species_bitmaps::find(events::eventName(ev));
  }

  int y;
  if (art != nullptr) {
    // Real reference-photo art for this animal -- show it instead of the
    // marmot for this screen; heights vary per photo, so lay out everything
    // below it relative to its actual bottom edge rather than a fixed offset.
    int bx = (SCREEN_W - art->w) / 2, by = 32;
    epd.drawBitmap(bx, by, art->bitmap, art->w, art->h, C_BLACK, C_WHITE);
    y = by + art->h + 10;
  } else if (speciesArt != nullptr) {
    int bx = (SCREEN_W - speciesArt->w) / 2, by = 40;
    epd.drawBitmap(bx, by, speciesArt->bitmap, speciesArt->w, speciesArt->h, C_BLACK, C_WHITE);
    y = by + speciesArt->h + 10;
  } else {
    // Full-size marmot art is 220px tall, bottom-aligned at stageGroundY --
    // groundY=240 put its top edge at y=20, overlapping the y=16 title.
    // There's slack in the bottom of this screen (text below rarely
    // reaches the nav bar), so push it down instead of shrinking the title.
    int stageCx = SCREEN_W / 2, stageGroundY = 260;
    drawCreature(stageCx, stageGroundY, negative ? Mood::Scared : Mood::Excited, (Stage)ctx.stage);
    y = stageGroundY + 15;
  }

  textAt(16, y, events::eventName(ev), 2);
  y += 24;
  y = textWrapped(16, y, 40, events::eventNote(ev), 1) + 8;
  if (negative) {
    y += 6;
    textAt(16, y, "Stay alert!", 1);
    y += 16;
  }
  // A ForagingFind is the one event ENTER does NOT resolve -- it clears by
  // eating something matching on the Foraging view (see onEnter()). So it
  // gets instructions instead of an effect preview, and the nav bar must not
  // offer "Acknowledge": a screen that advertises a button which does nothing,
  // and never says what does work, reads as a locked-up game.
  bool isFind = ev.type == events::EventType::ForagingFind;
  if (isFind) {
    y += 4;
    textAt(16, y, "To settle this:", 1);
    y += 14;
    y = textWrapped(16, y, 40,
                    ev.exact ? "Find this exact species on the Foraging list and eat it."
                             : "Eat anything of this kind from the Foraging list.",
                    1);
  } else {
    textAt(16, y, events::eventEffectPreview(ev), 1);
  }

  drawNavBar("Status", isFind ? "" : "Acknowledge", "Foraging");
}

// Short, wholesome flavor lines for the occasional Main-view thought
// bubble -- pure decoration, no mechanical effect. Content matches how
// coherent a marmot's inner monologue should sound at each growth stage:
// baby is pure gibberish, juvenile is simple/disjointed single concepts,
// adult is the only stage with actual complete thoughts.
#include "display_thoughts.h"

// A rounded speech-cloud with two small trailing circles, shown above the
// marmot's head about 2 in 3 wakes -- re-rolled once per wake (cached in a
// function-local static, same pattern as pickVariant()) so it doesn't
// flicker between renders of the same view.
// Picks which pool a thought should come from. Baby/Juvenile stay fixed
// (their pools are already stage-flavored, not stat-flavored); Adult reacts
// to whichever bar is worst so the marmot's inner monologue actually
// reflects hunger/loneliness/exhaustion instead of always sounding content.
// Thresholds mirror creature::evaluate()'s Mood cutoffs (s.hunger >= 70,
// s.happiness < 20) plus a matching cutoff for energy, which Mood doesn't
// otherwise consider.
static void pickThoughtPool(Stage stage, const CreatureState& creature, const char* const** outPool,
                            int* outCount) {
  if (stage == Stage::Baby) {
    *outPool = kThoughtsBaby;
    *outCount = kThoughtsBabyCount;
    return;
  }
  if (stage == Stage::Juvenile) {
    *outPool = kThoughtsJuvenile;
    *outCount = kThoughtsJuvenileCount;
    return;
  }
  if (creature.hunger >= 70) {
    *outPool = kThoughtsAdultHungry;
    *outCount = kThoughtsAdultHungryCount;
    return;
  }
  if (creature.happiness < 20) {
    *outPool = kThoughtsAdultSad;
    *outCount = kThoughtsAdultSadCount;
    return;
  }
  if (creature.energy < 20) {
    *outPool = kThoughtsAdultTired;
    *outCount = kThoughtsAdultTiredCount;
    return;
  }
  *outPool = kThoughtsAdult;
  *outCount = kThoughtsAdultCount;
}

/**
 * Places the bubble up and to the right of the marmot's head, with the tail
 * circles rising that way -- the classic comic reading, and consistent no
 * matter which pose is up.
 *
 * What moves per pose is the *origin*. Poses vary by ~80px in width and
 * ~200px in height (a standing marmot vs. a wide lounging one), so the head
 * is in a very different place each time; anchoring off the art box's
 * top-right region tracks it, where the old fixed x=235 anchor left the tail
 * pointing at nothing and the bubble looking centred over a low, wide pose.
 *
 * The box is filled white and drawn after the creature, so it erases what it
 * covers -- overlapping the pose's upper right is fine and reads as being in
 * front, but it's clamped to stay on the panel and below the header rows.
 */
static void maybeDrawThoughtBubble(const ArtBox& art, Stage stage, const CreatureState& creature) {
  static int8_t roll = -1;
  static int8_t thoughtIdx = -1;
  const char* const* pool;
  int count;
  pickThoughtPool(stage, creature, &pool, &count);
  if (roll < 0) {
    roll = (int8_t)random(3);
    thoughtIdx = (int8_t)random(count);
  }
  if (roll == 0) return;

  const char* thought = pool[thoughtIdx % count];

  // Wrapped to two lines when it's long. A one-line bubble for a 32-character
  // thought is ~200px wide, which crowds out any room to sit beside the pose;
  // splitting at the space nearest the middle roughly halves the width.
  std::string line1 = thought, line2;
  if (line1.size() > 18) {
    size_t mid = line1.size() / 2, best = std::string::npos;
    for (size_t i = 0; i < line1.size(); i++) {
      if (line1[i] != ' ') continue;
      if (best == std::string::npos ||
          (i > mid ? i - mid : mid - i) < (best > mid ? best - mid : mid - best)) {
        best = i;
      }
    }
    if (best != std::string::npos) {
      line2 = line1.substr(best + 1);
      line1 = line1.substr(0, best);
    }
  }

  epd.setFont(nullptr);
  epd.setTextSize(1);
  int16_t bx, by;
  uint16_t bw, bh, bw2 = 0, bh2 = 0;
  epd.getTextBounds(line1.c_str(), 0, 0, &bx, &by, &bw, &bh);
  if (!line2.empty()) epd.getTextBounds(line2.c_str(), 0, 0, &bx, &by, &bw2, &bh2);
  int textW = std::max((int)bw, (int)bw2);
  int lineH = (int)bh + 4;
  int textH = line2.empty() ? (int)bh : lineH + (int)bh2;

  const int padX = 8, padY = 6, margin = 4;
  const int topLimit = 46;  // below the name (y=10) and date (y=30) rows
  int rw = textW / 2 + padX, rh = textH / 2 + padY;

  // Origin: the pose's head, taken as the upper-right of its bounding box --
  // both coordinates move with the art rather than being fixed.
  int headX = art.x + art.w * 3 / 4;
  int headY = art.y + art.h / 8;

  // Bubble up and to the right of that, clamped onto the panel.
  int cx = headX + 24 + rw;
  int cy = headY - 26 - rh;
  cx = std::max(margin + rw, std::min(SCREEN_W - margin - rw, cx));
  cy = std::max(topLimit + rh, cy);

  epd.fillRoundRect(cx - rw, cy - rh, rw * 2, rh * 2, 8, C_WHITE);
  epd.drawRoundRect(cx - rw, cy - rh, rw * 2, rh * 2, 8, C_BLACK);
  if (line2.empty()) {
    textAt(cx - (int)bw / 2, cy - (int)bh / 2, line1, 1);
  } else {
    textAt(cx - (int)bw / 2, cy - textH / 2, line1, 1);
    textAt(cx - (int)bw2 / 2, cy - textH / 2 + lineH, line2, 1);
  }

  // Tail: two circles stepping up-and-right from the head to the bubble's
  // lower-left, growing as they go.
  int toX = cx - rw + 6, toY = cy + rh - 2;
  for (int i = 1; i <= 2; i++) {
    int tx = headX + (toX - headX) * i / 3;
    int ty = headY + (toY - headY) * i / 3;
    int r = 2 + i;
    epd.fillCircle(tx, ty, r, C_WHITE);
    epd.drawCircle(tx, ty, r, C_BLACK);
  }
}

static void renderMain(const AppContext& ctx) {
  events::PendingEvent ev;
  ev.type = (events::EventType)ctx.eventType;
  ev.dataId = ctx.eventDataId;
  if (ev.type != events::EventType::None) {
    renderEncounter(ctx, ev);
    return;
  }

  textAt(8, 10, ctx.creature.name, 2);
  drawWeatherGlyph(SCREEN_W - 100, 8, ctx.weather);

  // Battery flush against the top-right corner (icon's right edge, nub
  // included, lands at SCREEN_W - 8, matching the right margin other views
  // use), with the weather glyph bumped further left to make room.
  drawBatteryIcon(SCREEN_W - 30, 10, ctx.batteryPercent);
  textAt(SCREEN_W - 60, 12, std::to_string(ctx.batteryPercent) + "%", 1);

  char buf[24];
  strftime(buf, sizeof(buf), "%a %b %d", &ctx.now);
  textAt(8, 30, buf, 1);

  // No separate ground/habitat drawing -- the marmot bitmap already has its
  // own rock ledge baked in, and a second ground line under it just clashed.
  int stageCx = STAGE_X + STAGE_W / 2 - 20;
  int stageGroundY = STAGE_Y + STAGE_H - 24;
  ArtBox art{stageCx, STAGE_Y, 0, 0};
  drawCreature(stageCx, stageGroundY, ctx.creature.mood, (Stage)ctx.stage, /*small=*/false, &art);
  maybeDrawThoughtBubble(art, (Stage)ctx.stage, ctx.creature);

  drawNavBar("Status", "", "Foraging");
}

static const char* const MONTH_ABBR[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

// Renders a species' active months as compact ranges (e.g. "Jun-Aug, Oct")
// instead of an unlabeled dot calendar.
static std::string seasonText(const Forageable& f) {
  std::string out;
  bool first = true;
  int m = 1;
  while (m <= 12) {
    if (!foraging::inSeason(f, m)) {
      m++;
      continue;
    }
    int start = m;
    while (m <= 12 && foraging::inSeason(f, m)) m++;
    int end = m - 1;
    if (!first) out += ", ";
    out += MONTH_ABBR[start - 1];
    if (start != end) {
      out += "-";
      out += MONTH_ABBR[end - 1];
    }
    first = false;
  }
  return out;
}

static void renderForaging(const AppContext& ctx, int speciesIdx) {
  if (foraging::browsableCount() == 0) {
    textCentered(0, SCREEN_W, 160, "Nothing discovered yet", 1);
    textCentered(0, SCREEN_W, 180, "Explore to find your", 1);
    textCentered(0, SCREEN_W, 194, "first plant or animal!", 1);
    drawNavBar("Main", "", "");
    return;
  }

  const Forageable& f = foraging::speciesAtRank(speciesIdx);
  int month = ctx.now.tm_mon + 1;
  bool active = foraging::inSeason(f, month);

  events::PendingEvent ev;
  ev.type = (events::EventType)ctx.eventType;
  ev.dataId = ctx.eventDataId;
  ev.exact = ctx.eventExact != 0;
  bool isMatch = events::eventMatchesSpecies(ev, f);

  bool eaten = journal::hasEaten(foraging::indexAtRank(speciesIdx));
  std::string posBuf = std::to_string(speciesIdx + 1) + "/" +
                       std::to_string(foraging::browsableCount()) + (eaten ? " (eaten)" : "");
  int16_t pbx, pby;
  uint16_t pbw, pbh;
  epd.setFont(nullptr);
  epd.setTextSize(1);
  epd.getTextBounds(posBuf.c_str(), 0, 6, &pbx, &pby, &pbw, &pbh);
  textAt(SCREEN_W - 8 - (int)pbw, 6, posBuf, 1);

  // Large centered icon -- the species art is the focal point of the view.
  // Species without a sourced photo yet (see include/bitmaps/species/) just
  // get an empty placeholder frame here rather than a procedural sprite.
  const float iconScale = 2.25f;
  int iconSize = (int)(64 * iconScale);
  int iconX = (SCREEN_W - iconSize) / 2, iconY = 6;
  const species_bitmaps::SpeciesBitmap* art = species_bitmaps::find(f.name);
  if (art) {
    int bx = (SCREEN_W - art->w) / 2;
    int by = iconY + (iconSize - art->h) / 2;
    epd.drawBitmap(bx, by, art->bitmap, art->w, art->h, C_BLACK, C_WHITE);
  } else {
    epd.drawRect(iconX, iconY, iconSize, iconSize, C_BLACK);
    textCentered(iconX, iconSize, iconY + iconSize / 2 - 4, "?", 2);
  }

  int y = iconY + iconSize + 8;
  textCentered(0, SCREEN_W, y, f.name, 2);
  y += 22;

  std::string kindLine = std::string(f.kind) + " - " + foraging::biomeName(f.biome);
  textCentered(0, SCREEN_W, y, kindLine, 1);
  y += 16;

  if (active) {
    textCentered(0, SCREEN_W, y, "IN SEASON", 1);
    y += 16;
  }

  epd.drawFastHLine(8, y, SCREEN_W - 16, C_BLACK);
  y += 10;

  textAt(8, y, "NOTE:", 1);
  y = textWrapped(8, y + 12, 46, f.note, 1) + 8;

  if (f.caution[0]) {
    textAt(8, y, "CAUTION:", 1);
    y = textWrapped(8, y + 12, 46, f.caution, 1) + 8;
  }

  textAt(8, y, "TIP:", 1);
  y = textWrapped(8, y + 12, 46, f.harvestTip, 1) + 8;

  textAt(8, y, "SEASON: " + seasonText(f), 1);

  if (isMatch) textCentered(0, SCREEN_W, y + 16, "MATCHES ACTIVE FIND!", 1);

  drawNavBar("Main", "Eat", "Scroll");
}

/**
 * Curiosity as shown on the bar: the persisted stat (raised by the species
 * games and by discoveries, decaying on its own clock) plus a transient
 * bonus for weather worth going out in. Only the base is saved, so a wet
 * week can't inflate the stat -- and it used to be *only* this weather term,
 * which meant the bar sat among three stats you can affect while being
 * something you couldn't touch at all.
 */
static uint8_t curiosityDisplay(const CreatureState& c, const WeatherData& w) {
  int bonus = 0;
  if (w.valid) bonus = w.postRain ? 25 : (w.tempC >= 8.0f && w.tempC <= 22.0f ? 10 : 0);
  int v = (int)c.curiosity + bonus;
  return (uint8_t)(v > 100 ? 100 : v);
}

static const char* stageName(Stage s) {
  switch (s) {
    case Stage::Baby:
      return "Baby";
    case Stage::Juvenile:
      return "Juvenile";
    default:
      return "Adult";
  }
}

static void renderStatus(const AppContext& ctx) {
  textAt(8, 10, std::string(ctx.creature.name) + " - " + stageName((Stage)ctx.stage), 2);

  // Smaller than the Main view's mascot -- Status needs the vertical room
  // for four bars plus weather/streak text below it.
  int stageCx = SCREEN_W / 2, stageGroundY = 168;
  drawCreature(stageCx, stageGroundY, ctx.creature.mood, (Stage)ctx.stage, /*small=*/true);

  auto bar = [&](int y, const char* label, uint8_t pct) {
    textAt(20, y, label, 1);
    int bx = 20, bw = SCREEN_W - 40, by = y + 12;
    dFillRect(bx, by, bw, 14, SHADE_LIGHT);
    epd.drawRect(bx, by, bw, 14, C_BLACK);
    epd.fillRect(bx + 2, by + 2, (bw - 4) * pct / 100, 10, C_BLACK);
  };
  bar(196, "Fullness", 100 - ctx.creature.hunger);
  bar(232, "Happiness", ctx.creature.happiness);
  bar(268, "Energy", ctx.creature.energy);
  bar(304, "Curiosity", curiosityDisplay(ctx.creature, ctx.weather));

  if (ctx.weather.valid) {
    char buf[48];
    snprintf(buf, sizeof(buf), "%s, %.0fC", ctx.weather.condition.c_str(), ctx.weather.tempC);
    textAt(20, 338, buf, 1);
  }

  textAt(20, 350,
         "Streak: " + std::to_string(ctx.creature.feedStreakDays) + "d | Eaten: " +
             std::to_string(journal::totalEaten()) + "/" + std::to_string(foraging::speciesCount()),
         1);

  // The winter stockpile lives here too, not only inside Snack Hunt -- it's
  // the one goal in the game that spans weeks, so it shouldn't take a trip
  // into a minigame to check on.
  textAt(20, 362,
         "Winter stash: " + std::to_string(minigames::stashPoints()) + "/" +
             std::to_string(minigames::WINTER_STASH_GOAL),
         1);

  drawNavBar("Minigames", "", "Main");
}

// Badge progress line: "[X] Name (n/threshold)" -- a filled box once count
// reaches threshold. No per-badge icons; matches the existing blocky-font,
// text-only aesthetic used everywhere else.
// Simple procedural glyphs (GFX primitives, not photo bitmaps) drawn inside
// an unlocked badge circle -- one per badge, kept recognizable at ~50px.
enum class BadgeIcon { Mushroom, Berries, Leaf, Paw, Binoculars, Cloud, Compass, HalfMoon, Star };

static void drawBadgeIcon(BadgeIcon icon, int cx, int cy, int r) {
  switch (icon) {
    case BadgeIcon::Mushroom:
      epd.fillCircle(cx, cy - r / 4, r * 2 / 3, C_BLACK);
      epd.fillRect(cx - r / 5, cy - r / 4, r * 2 / 5, r, C_BLACK);
      break;
    case BadgeIcon::Berries:
      epd.fillCircle(cx - r / 3, cy - r / 4, r / 3, C_BLACK);
      epd.fillCircle(cx + r / 3, cy - r / 4, r / 3, C_BLACK);
      epd.fillCircle(cx, cy + r / 3, r / 3, C_BLACK);
      break;
    case BadgeIcon::Leaf:
      epd.fillTriangle(cx, cy - r * 2 / 3, cx - r * 2 / 3, cy + r / 2, cx + r * 2 / 3, cy + r / 2,
                       C_BLACK);
      epd.drawLine(cx, cy - r / 2, cx, cy + r / 2, C_WHITE);
      break;
    case BadgeIcon::Paw:
      epd.fillCircle(cx, cy + r / 4, r / 2, C_BLACK);
      epd.fillCircle(cx - r / 2, cy - r / 3, r / 5, C_BLACK);
      epd.fillCircle(cx, cy - r / 2, r / 5, C_BLACK);
      epd.fillCircle(cx + r / 2, cy - r / 3, r / 5, C_BLACK);
      break;
    case BadgeIcon::Binoculars:
      epd.fillCircle(cx - r / 3, cy, r / 3, C_BLACK);
      epd.fillCircle(cx + r / 3, cy, r / 3, C_BLACK);
      epd.fillRect(cx - r / 4, cy - r / 2, r / 2, r / 4, C_BLACK);
      break;
    case BadgeIcon::Cloud:
      epd.fillCircle(cx - r / 3, cy, r / 3, C_BLACK);
      epd.fillCircle(cx + r / 3, cy, r / 3, C_BLACK);
      epd.fillCircle(cx, cy - r / 5, r / 2, C_BLACK);
      epd.fillRect(cx - r / 2, cy, r, r / 3, C_BLACK);
      break;
    case BadgeIcon::Compass:
      epd.drawCircle(cx, cy, r * 2 / 3, C_BLACK);
      epd.drawCircle(cx, cy, r * 2 / 3 - 1, C_BLACK);
      epd.fillTriangle(cx, cy - r / 2, cx - r / 5, cy, cx + r / 5, cy, C_BLACK);
      epd.fillTriangle(cx, cy + r / 2, cx - r / 5, cy, cx + r / 5, cy, C_BLACK);
      break;
    case BadgeIcon::HalfMoon:
      epd.fillCircle(cx, cy, r * 2 / 3, C_BLACK);
      epd.fillRect(cx, cy - r * 2 / 3, r * 2 / 3 + 1, r * 4 / 3, C_WHITE);
      break;
    case BadgeIcon::Star:
      for (int i = 0; i < 5; i++) {
        float a1 = (float)i * 2 * PI / 5 - PI / 2;
        float a2 = (float)(i + 2) * 2 * PI / 5 - PI / 2;
        epd.drawLine(cx + (int)(cosf(a1) * r * 0.8f), cy + (int)(sinf(a1) * r * 0.8f),
                     cx + (int)(cosf(a2) * r * 0.8f), cy + (int)(sinf(a2) * r * 0.8f), C_BLACK);
      }
      break;
  }
}

struct BadgeDef {
  const char* name;
  BadgeIcon icon;
  int count;
  int threshold;
  // Real dithered-photo art (a cropped/thresholded scan of an actual
  // embroidered patch) overrides the procedural icon when set -- nullptr
  // falls back to drawBadgeIcon(). Sourced one badge at a time as real
  // patch photos matching the theme turn up (see
  // assets/source_photos/badges/), not all 9 at once.
  const uint8_t* bitmap;
  int bitmapW;
  int bitmapH;
};

// Medal-style rim: an outer ring, an inset inner ring, and short radial tick
// marks between them (like a coin's reeded edge) -- reads as a proper medal
// instead of a plain outline circle.
static void drawMedalRim(int cx, int cy, int r) {
  epd.drawCircle(cx, cy, r, C_BLACK);
  epd.drawCircle(cx, cy, r - 4, C_BLACK);
  const int ticks = 24;
  for (int i = 0; i < ticks; i++) {
    float a = (float)i * 2.0f * (float)M_PI / ticks;
    int x0 = cx + (int)((r - 1) * cosf(a)), y0 = cy + (int)((r - 1) * sinf(a));
    int x1 = cx + (int)((r - 4) * cosf(a)), y1 = cy + (int)((r - 4) * sinf(a));
    epd.drawLine(x0, y0, x1, y1, C_BLACK);
  }
}

static void drawBadge(const BadgeDef& b, int cx, int cy) {
  const int r = 26;
  bool unlocked = b.count >= b.threshold;
  drawMedalRim(cx, cy, r);
  if (unlocked) {
    if (b.bitmap) {
      epd.drawBitmap(cx - b.bitmapW / 2, cy - b.bitmapH / 2, b.bitmap, b.bitmapW, b.bitmapH,
                     C_BLACK);
    } else {
      drawBadgeIcon(b.icon, cx, cy, r);
    }
  } else {
    textCentered(cx - r, r * 2, cy - 4, "?", 1);
  }
  textCentered(cx - 45, 90, cy + r + 4, b.name, 1);
  int shown = b.count < b.threshold ? b.count : b.threshold;
  textCentered(cx - 45, 90, cy + r + 16, std::to_string(shown) + "/" + std::to_string(b.threshold),
               1);
}

void renderAchievements(const AppContext& ctx) {
  epd.beginFrame();
  textAt(8, 10, "Achievements", 2);

  // Locked until full-grown -- badges/streak tracking still runs underneath
  // the whole time (see journal.cpp/creature.cpp), this just gates the
  // reveal so there's something to look forward to at Adult.
  if ((Stage)ctx.stage != Stage::Adult) {
    textCentered(0, SCREEN_W, 160, "Locked", 2);
    textCentered(0, SCREEN_W, 190, "Come back once your marmot", 1);
    textCentered(0, SCREEN_W, 204, "is fully grown.", 1);
    textCentered(0, SCREEN_W, SCREEN_H - 22, "BACK = previous menu", 1);
    epd.endFrame(true);
    return;
  }

  int total = journal::totalEaten();
  textCentered(0, SCREEN_W, 46,
               "Species: " + std::to_string(total) + "/" +
                   std::to_string(foraging::speciesCount()) +
                   "   Streak: " + std::to_string(ctx.creature.feedStreakDays) + "d",
               1);

  int herbalist = foraging::countEatenOfKind("green") + foraging::countEatenOfKind("herb") +
                  foraging::countEatenOfKind("flower") + foraging::countEatenOfKind("fern") +
                  foraging::countEatenOfKind("root") + foraging::countEatenOfKind("shoot");

  // 3 foraging (food types), 2 animals, 2 other events, 2 completionist.
  const BadgeDef badges[9] = {
      {"Mushrooms", BadgeIcon::Mushroom, foraging::countEatenOfKind("mushroom"), 15,
       BADGE_MUSHROOMS_BITMAP, BADGE_MUSHROOMS_W, BADGE_MUSHROOMS_H},
      {"Berries", BadgeIcon::Berries, foraging::countEatenOfKind("berry"), 10, BADGE_BERRIES_BITMAP,
       BADGE_BERRIES_W, BADGE_BERRIES_H},
      {"Herbalist", BadgeIcon::Leaf, herbalist, 15, BADGE_HERBALIST_BITMAP, BADGE_HERBALIST_W,
       BADGE_HERBALIST_H},
      {"Wildlife", BadgeIcon::Paw, journal::totalAnimalSightings(), 10, BADGE_WILDLIFE_BITMAP,
       BADGE_WILDLIFE_W, BADGE_WILDLIFE_H},
      {"Naturalist", BadgeIcon::Binoculars, journal::totalAnimalSightings(), 25,
       BADGE_NATURALIST_BITMAP, BADGE_NATURALIST_W, BADGE_NATURALIST_H},
      {"Storms", BadgeIcon::Cloud, journal::totalWeatherEvents(), 10, BADGE_STORMS_BITMAP,
       BADGE_STORMS_W, BADGE_STORMS_H},
      {"Wanderer", BadgeIcon::Compass, journal::totalOtherEvents(), 15, BADGE_WANDERER_BITMAP,
       BADGE_WANDERER_W, BADGE_WANDERER_H},
      {"Halfway", BadgeIcon::HalfMoon, total, foraging::speciesCount() / 2, BADGE_HALFWAY_BITMAP,
       BADGE_HALFWAY_W, BADGE_HALFWAY_H},
      {"Complete", BadgeIcon::Star, total, foraging::speciesCount(), BADGE_COMPLETE_BITMAP,
       BADGE_COMPLETE_W, BADGE_COMPLETE_H},
  };

  const int cols[3] = {60, 150, 240};
  const int rows[3] = {110, 210, 310};
  for (int i = 0; i < 9; i++) {
    drawBadge(badges[i], cols[i % 3], rows[i / 3]);
  }

  textCentered(0, SCREEN_W, SCREEN_H - 22, "BACK = previous menu", 1);
  epd.endFrame(true);
}

// ------------------------------------------------------------- Minigames

// A selectable row: filled black with knocked-out white text when picked,
// a plain outline otherwise -- the same inversion the text-entry keyboard
// uses for its highlighted key, which reads far better on this panel than a
// dithered highlight behind small text.
static void optionBox(int x, int y, int w, int h, const std::string& label, bool selected,
                      uint8_t size = 1) {
  epd.setFont(nullptr);
  epd.setTextSize(size);
  int16_t bx, by;
  uint16_t bw, bh;
  epd.getTextBounds(label.c_str(), 0, 0, &bx, &by, &bw, &bh);
  int tx = x + (w - (int)bw) / 2;
  int ty = y + (h - (int)bh) / 2;
  if (selected) {
    epd.fillRoundRect(x, y, w, h, 6, C_BLACK);
    epd.setTextColor(C_WHITE);
    epd.setCursor(tx, ty);
    epd.print(label.c_str());
    epd.setTextColor(C_BLACK);
  } else {
    epd.drawRoundRect(x, y, w, h, 6, C_BLACK);
    textAt(tx, ty, label, size);
  }
}

/**
 * One call in a Marmot Says sequence: the icon this run has assigned to that
 * button, on a rounded plate. Icons rather than arrow glyphs because a
 * scaled-up "<" is a thin stroke on a big empty field -- weak art, and
 * genuinely harder to hold in memory for a second than a distinct shape.
 * The three are chosen to differ in silhouette (dome, spike, rosette), not
 * just in detail.
 */
static void drawSimonCall(uint8_t iconId, int cx, int cy, int size) {
  int half = size / 2;
  epd.drawRoundRect(cx - half, cy - half, size, size, size / 5, C_BLACK);
  epd.drawRoundRect(cx - half + 1, cy - half + 1, size - 2, size - 2, size / 5, C_BLACK);
  const uint8_t* bmp;
  int w, h;
  switch (iconId) {
    case 0:
      bmp = TOKEN_TOADSTOOL_BITMAP;
      w = TOKEN_TOADSTOOL_W;
      h = TOKEN_TOADSTOOL_H;
      break;
    case 1:
      bmp = TOKEN_WOOD_BITMAP;
      w = TOKEN_WOOD_W;
      h = TOKEN_WOOD_H;
      break;
    default:
      bmp = TOKEN_LEAF_BITMAP;
      w = TOKEN_LEAF_W;
      h = TOKEN_LEAF_H;
      break;
  }
  epd.drawBitmap(cx - w / 2, cy - h / 2, bmp, w, h, C_BLACK, C_WHITE);
}

static void renderMinigameMenu(const AppContext& ctx, const minigames::State& s) {
  textAt(8, 10, "Minigames", 2);
  textAt(8, 36, std::string("A break with ") + ctx.creature.name, 1);

  // Locked games don't get a row at all -- see visibleGameAt() -- so every
  // row drawn here is playable.
  int count = minigames::visibleGameCount((Stage)ctx.stage);
  for (int i = 0; i < count; i++) {
    minigames::Game g = minigames::visibleGameAt((Stage)ctx.stage, i);
    const int x = 12, w = SCREEN_W - 24, h = 56;
    int y = 58 + i * 62;
    epd.drawRoundRect(x, y, w, h, 10, C_BLACK);
    if (i == s.menuSel) {
      // Double border plus a solid left tab -- an outline alone is too
      // subtle at a glance, and a dithered fill behind size-1 text muddies
      // it (see optionBox()'s note).
      epd.drawRoundRect(x + 2, y + 2, w - 4, h - 4, 8, C_BLACK);
      epd.fillRect(x + 4, y + 10, 6, h - 20, C_BLACK);
    }
    // No emblems: the rows carry themselves on type alone -- name at size 2
    // over a size-1 subtitle, with the best score right-aligned on the name's
    // baseline and a hairline separating the two lines. Small procedural
    // icons were tried here and pulled: at 14px they read as smudges next to
    // crisp text, which made the list look worse, not better.
    const int textX = x + 16;
    textAt(textX, y + 9, minigames::gameName(g), 2);
    std::string best = "Best " + std::to_string(minigames::highScore(g));
    int16_t bx, by;
    uint16_t bw, bh;
    epd.setFont(nullptr);
    epd.setTextSize(1);
    epd.getTextBounds(best.c_str(), 0, 0, &bx, &by, &bw, &bh);
    textAt(x + w - 12 - (int)bw, y + 15, best, 1);
    epd.drawFastHLine(textX, y + 31, w - 28, C_BLACK);
    textAt(textX, y + 38, minigames::gameBlurb(g), 1);
  }

  drawNavBar("Next", "Play", "Status", /*leftIsDown=*/true);
}

/**
 * Forest Memory's card faces -- six procedural forest tokens, drawn big and
 * simple. Deliberately not species photos: a dithered photo shrunk to card
 * size is unreadable speckle (the same reason the achievement badges are
 * flat icons, see CLAUDE.md's art notes), and a memory game is unplayable
 * if two faces can be confused.
 */
static void drawForestIcon(int id, int cx, int cy, int r) {
  (void)r;  // every face is a fixed-size bitmap
  // Six faces chosen for distinct SILHOUETTES -- frond, cone, dome, log,
  // flower, animal -- not just distinct subjects. Three earlier faces were
  // cut for failing exactly that test once dithered: a eucalyptus sprig whose
  // round leaves hollowed into a scatter of rings, a succulent rosette that
  // became a speckled blob, and an agave whose blades read as a hand.
  const uint8_t* bmp;
  int bw, bh;
  switch (id) {
    case 0:
      bmp = TOKEN_FERN_BITMAP;
      bw = TOKEN_FERN_W;
      bh = TOKEN_FERN_H;
      break;
    case 1:
      bmp = TOKEN_TREE_BITMAP;
      bw = TOKEN_TREE_W;
      bh = TOKEN_TREE_H;
      break;
    case 2:
      bmp = TOKEN_TOADSTOOL_BITMAP;
      bw = TOKEN_TOADSTOOL_W;
      bh = TOKEN_TOADSTOOL_H;
      break;
    case 3:
      bmp = TOKEN_WOOD_BITMAP;
      bw = TOKEN_WOOD_W;
      bh = TOKEN_WOOD_H;
      break;
    case 4:
      bmp = TOKEN_BLOOM_BITMAP;
      bw = TOKEN_BLOOM_W;
      bh = TOKEN_BLOOM_H;
      break;
    default:
      bmp = TOKEN_BRANCH_BITMAP;
      bw = TOKEN_BRANCH_W;
      bh = TOKEN_BRANCH_H;
      break;
  }
  epd.drawBitmap(cx - bw / 2, cy - bh / 2, bmp, bw, bh, C_BLACK, C_WHITE);
}

static void renderMemory(const minigames::State& s) {
  textAt(8, 10, "Forest Memory", 2);
  textAt(SCREEN_W - 84, 14, "Pairs: " + std::to_string(s.score), 1);
  epd.drawFastHLine(8, 34, SCREEN_W - 16, C_BLACK);

  const int cols = 4, rows = minigames::MemoryRound::CARDS / 4;
  const int cw = 66, ch = 84, gapX = 6, gapY = 8;
  int gridW = cols * cw + (cols - 1) * gapX;
  int x0 = (SCREEN_W - gridW) / 2, y0 = 60;

  for (int i = 0; i < minigames::MemoryRound::CARDS; i++) {
    int x = x0 + (i % cols) * (cw + gapX);
    int y = y0 + (i / cols) * (ch + gapY);
#if DEV_MODE_SHOW_ALL_CONTENT
    // Every card open, so all six faces can be compared side by side --
    // which is the only way to be sure none of them look alike.
    bool faceUp = true;
#else
    bool faceUp = s.memory.matched[i] || i == s.memory.firstFlip || i == s.memory.secondFlip;
#endif
    epd.drawRoundRect(x, y, cw, ch, 8, C_BLACK);
    if (i == s.memory.sel) epd.drawRoundRect(x + 2, y + 2, cw - 4, ch - 4, 6, C_BLACK);
    if (faceUp) {
      drawForestIcon(s.memory.face[i], x + cw / 2, y + ch / 2, 22);
      // A matched pair stays up for good -- underline it so it reads as
      // done rather than as still in play.
      if (s.memory.matched[i]) epd.fillRect(x + 12, y + ch - 12, cw - 24, 3, C_BLACK);
    } else {
      // Face-down back: a dithered field with a knocked-out diamond, so a
      // turned card is obviously a *back* rather than an empty slot.
      dFillRoundRect(x + 6, y + 6, cw - 12, ch - 12, 6, SHADE_LIGHT);
      int mx = x + cw / 2, my = y + ch / 2, d = 13;
      epd.fillTriangle(mx, my - d, mx - d, my, mx + d, my, C_WHITE);
      epd.fillTriangle(mx, my + d, mx - d, my, mx + d, my, C_WHITE);
      epd.drawLine(mx, my - d, mx - d, my, C_BLACK);
      epd.drawLine(mx, my - d, mx + d, my, C_BLACK);
      epd.drawLine(mx, my + d, mx - d, my, C_BLACK);
      epd.drawLine(mx, my + d, mx + d, my, C_BLACK);
    }
  }

  // Miss budget as pips, so "how much rope is left" is glanceable.
  int pipY = y0 + rows * (ch + gapY) + 12;
  textAt(x0, pipY, "Misses left:", 1);
  for (int i = 0; i < minigames::MemoryRound::MISS_BUDGET; i++) {
    int px = x0 + 80 + i * 16;
    if (i < s.memory.missesLeft) {
      epd.fillCircle(px, pipY + 3, 5, C_BLACK);
    } else {
      epd.drawCircle(px, pipY + 3, 5, C_BLACK);
    }
  }

  drawNavBar("Prev", "Flip", "Next");
}

/**
 * A bush, drawn as a solid dark clump of leaves so it reads at a glance as
 * something opaque you can't see under. `lift` (0 = sitting on the ground,
 * up to a full lift) raises and tilts it aside for the reveal, which is the
 * whole point of the screen: you should see the bush *move* and the ground
 * under it appear, not just have text tell you what happened.
 */
static void drawBush(int cx, int baseY, int w, int lift) {
  (void)w;  // sized by the art now
  // Leans as it lifts, so the two states aren't the same shape shifted up.
  epd.drawBitmap(cx - TOKEN_BUSH_W / 2 - lift / 5, baseY - lift - TOKEN_BUSH_H, TOKEN_BUSH_BITMAP,
                 TOKEN_BUSH_W, TOKEN_BUSH_H, C_BLACK, C_WHITE);
}

/**
 * What's under a lifted bush -- den-stuffing rather than anything from the
 * foraging species table (see StashKind's note in minigames.h). Kept to
 * four shapes that stay distinct in silhouette at ~16px: blades, a leaf,
 * crossed twigs, a flower.
 */
static void drawStashIcon(uint8_t kind, uint8_t variant, int cx, int cy) {
  const uint8_t* bmp;
  int w, h;
  if (kind == minigames::KIND_SCARE) {
    // Only a pair of eyes in the grid -- the animal itself gets the whole
    // run-over screen at photo size (see snackPredatorArt()).
    bmp = TOKEN_EYES_BITMAP;
    w = TOKEN_EYES_W;
    h = TOKEN_EYES_H;
  } else if (kind == minigames::KIND_CRITTER) {
    switch (variant) {
      case 0:
        bmp = TOKEN_BEETLE_BITMAP;
        w = TOKEN_BEETLE_W;
        h = TOKEN_BEETLE_H;
        break;
      case 1:
        bmp = TOKEN_LIZARD_BITMAP;
        w = TOKEN_LIZARD_W;
        h = TOKEN_LIZARD_H;
        break;
      case 2:
        bmp = TOKEN_SPIDER_BITMAP;
        w = TOKEN_SPIDER_W;
        h = TOKEN_SPIDER_H;
        break;
      default:
        bmp = TOKEN_CRICKET_BITMAP;
        w = TOKEN_CRICKET_W;
        h = TOKEN_CRICKET_H;
        break;
    }
  } else {
    switch (kind) {
      case minigames::STASH_LEAVES:
        bmp = TOKEN_LEAFPILE_BITMAP;
        w = TOKEN_LEAFPILE_W;
        h = TOKEN_LEAFPILE_H;
        break;
      case minigames::STASH_TWIGS:
        bmp = TOKEN_WOOD_BITMAP;
        w = TOKEN_WOOD_W;
        h = TOKEN_WOOD_H;
        break;
      case minigames::STASH_FLOWER:
        bmp = TOKEN_BLOOM_BITMAP;
        w = TOKEN_BLOOM_W;
        h = TOKEN_BLOOM_H;
        break;
      case minigames::STASH_GRASS:
        bmp = TOKEN_GRASS_BITMAP;
        w = TOKEN_GRASS_W;
        h = TOKEN_GRASS_H;
        break;
      default:
        return;  // KIND_NONE draws nothing
    }
  }
  epd.drawBitmap(cx - w / 2, cy - h / 2, bmp, w, h, C_BLACK, C_WHITE);
}

// The predator behind a scare, at real photo size for the run-over screen.
// Order matches minigames::predatorName().
struct SnackPredator {
  const uint8_t* bitmap;
  int w, h;
};
static SnackPredator snackPredatorArt(int i) {
  switch (i) {
    case 0:
      return {ANIMAL_COYOTE_BITMAP, ANIMAL_COYOTE_W, ANIMAL_COYOTE_H};
    case 1:
      // Was the bobcat, swapped out: its source photo dithered pale and
      // low-contrast, and at 170px next to the coyote/fox/owl it read as a
      // vague smudge. The cougar's art is the strongest of the predators
      // available, and it's just as real a marmot predator.
      return {ANIMAL_COUGAR_BITMAP, ANIMAL_COUGAR_W, ANIMAL_COUGAR_H};
    case 2:
      return {ANIMAL_RED_FOX_BITMAP, ANIMAL_RED_FOX_W, ANIMAL_RED_FOX_H};
    default:
      return {ANIMAL_BARRED_OWL_BITMAP, ANIMAL_BARRED_OWL_W, ANIMAL_BARRED_OWL_H};
  }
}

// The stash progress bar, shown under Snack Hunt -- the long-term goal the
// game actually feeds, so it's on screen the whole time rather than buried.
static void drawStashBar(int y) {
  int points = minigames::stashPoints();
  int goal = minigames::WINTER_STASH_GOAL;
  textAt(16, y, "Winter stash", 1);
  std::string frac = std::to_string(points) + "/" + std::to_string(goal);
  int16_t bx, by;
  uint16_t bw, bh;
  epd.setFont(nullptr);
  epd.setTextSize(1);
  epd.getTextBounds(frac.c_str(), 0, 0, &bx, &by, &bw, &bh);
  textAt(SCREEN_W - 16 - (int)bw, y, frac, 1);
  int barX = 16, barW = SCREEN_W - 32, barY = y + 14;
  dFillRect(barX, barY, barW, 12, SHADE_LIGHT);
  epd.drawRect(barX, barY, barW, 12, C_BLACK);
  int fill = points >= goal ? barW - 4 : (barW - 4) * points / goal;
  epd.fillRect(barX + 2, barY + 2, fill, 8, C_BLACK);
}

static void renderSnack(const minigames::State& s) {
  textAt(8, 10, "Snack Hunt", 2);
  textAt(SCREEN_W - 84, 14, "Picks: " + std::to_string(s.snack.picksLeft), 1);
  epd.drawFastHLine(8, 34, SCREEN_W - 16, C_BLACK);

  bool revealed = s.screen == minigames::Screen::Reveal;
  // A practice run says so up front rather than silently not counting --
  // see SnackRound::banking.
  textCentered(0, SCREEN_W, 46,
               revealed ? "Under the bushes:"
                        : (s.snack.banking ? "Rummage under which bush?" : "Practice -- already"),
               1);
  if (!revealed && !s.snack.banking) {
    textCentered(0, SCREEN_W, 60, "gathered today.", 1);
  }

  // 2x2 of bushes. On the reveal every bush shifts ASIDE rather than
  // straight up, and all four open, not just the one picked. Sideways is
  // forced by arithmetic: the bush art is 58px tall and a find is 44px, so
  // stacking them needs ~110px of the 100px a cell has -- and lifting far
  // enough for both ran the top row into the header and the bottom row into
  // the row above it. There's plenty of width, so the bush leans left and
  // the find sits on bare ground to its right.
  const int cellW = 130, cellH = 100;
  int x0 = (SCREEN_W - cellW * 2) / 2, y0 = 74;
  for (int i = 0; i < minigames::SnackRound::BUSHES; i++) {
    int cx = x0 + (i % 2) * cellW + cellW / 2;
    int baseY = y0 + (i / 2) * cellH + cellH - 24;
    int bushX = revealed ? cx - 30 : cx;
    int findX = cx + 28;

    if (revealed) {
      // Bare scrape where the bush was sitting, with the find on it. Outline
      // only, no dithered fill: at size 1 the label's strokes and the Bayer
      // pattern are both 1px, so any fill behind the text fights it.
      epd.drawRoundRect(findX - 26, baseY - 12, 52, 16, 8, C_BLACK);
      std::string label = s.snack.kind[i] == minigames::KIND_NONE
                              ? "Empty"
                              : "x" + std::to_string(s.snack.amount[i]);
      if (s.snack.kind[i] != minigames::KIND_NONE) {
        drawStashIcon(s.snack.kind[i], s.snack.amount[i], findX, baseY - 34);
      }
      textCentered(findX - 30, 60, baseY - 8, label, 1);
    }

    drawBush(bushX, baseY, 52, revealed ? 8 : 0);

    if (!revealed && i == s.snack.sel) {
      epd.fillTriangle(cx - 8, baseY + 20, cx + 8, baseY + 20, cx, baseY + 8, C_BLACK);
    }
    if (revealed && i == s.snack.picked) {
      // Mark the one actually chosen, since all four are now open. Sized to
      // the cell's real content extents -- bush from cx-31, find art out to
      // cx+54, tallest icon 44px above baseY-12 -- rather than to a guessed
      // inset, which left the bush and the icon tops poking outside the box.
      // drawBush() subtracts half the art width from the x it's given, so a
      // bush drawn at cx-30 actually starts at cx-61 -- the box has to reach
      // further left than the arithmetic first suggests.
      int rowTop = y0 + (i / 2) * cellH;
      epd.drawRoundRect(cx - 68, rowTop + 12, 126, 76, 8, C_BLACK);
      epd.drawRoundRect(cx - 67, rowTop + 13, 124, 74, 7, C_BLACK);
    }
  }

  if (revealed) {
    int i = s.snack.picked;
    uint8_t k = s.snack.kind[i];
    std::string line;
    bool banked = false;
    if (k == minigames::KIND_SCARE) {
      line = std::string(minigames::predatorName(s.snack.amount[i])) + "! You bolt for home.";
    } else if (k == minigames::KIND_CRITTER) {
      line = "Just " + std::string(minigames::critterName(s.snack.amount[i])) + ".";
    } else if (k == minigames::KIND_NONE) {
      line = "Nothing under that one.";
    } else {
      line =
          "Found " + std::to_string(s.snack.amount[i]) + "x " + minigames::stashKindName(k) + "!";
      banked = true;
    }
    textCentered(0, SCREEN_W, 284, line, 1);
    if (!s.snack.banking && banked) {
      textCentered(0, SCREEN_W, 300, "(practice -- not stashed)", 1);
    }
    drawStashBar(320);
    drawNavBar("", s.snack.picksLeft > 0 ? "Next bush" : "Done", "");
  } else {
    drawStashBar(300);
    drawNavBar("Prev", "Rummage", "Next");
  }
}

// Compass labels for the maze's option list -- the player is picking a
// tunnel, not a heading, but naming them keeps the list unambiguous.
static const char* mazeDirName(int dir) {
  switch (dir) {
    case 0:
      return "up";
    case 1:
      return "right";
    case 2:
      return "down";
    default:
      return "left";
  }
}

static void renderMaze(const minigames::State& s) {
  textAt(8, 10, "Burrow Maze", 2);
  textAt(SCREEN_W - 84, 8, "Solved: " + std::to_string(s.score), 1);
  textAt(SCREEN_W - 84, 20, std::to_string(s.maze.n) + "x" + std::to_string(s.maze.n), 1);
  epd.drawFastHLine(8, 34, SCREEN_W - 16, C_BLACK);

  // The grid grows with the score, so the cell size has to shrink to keep
  // the whole board inside a fixed 240px band -- otherwise a 7x7 would push
  // the option row down through the nav bar. 46px is the cap so a 5x5 looks
  // the same as it always did.
  const int n = s.maze.n;
  const int cell = std::min(42, 210 / n);
  int gx = (SCREEN_W - n * cell) / 2, gy = 46;
  int gh = n * cell;

  // Walls are drawn 2px thick as packed earth, so the tunnels read as dug
  // space rather than as a wireframe grid.
  for (int y = 0; y < n; y++) {
    for (int x = 0; x < n; x++) {
      int px = gx + x * cell, py = gy + y * cell;
      uint8_t w = s.maze.wall[y][x];
      // Trail already dug, so a long backtrack is visibly a backtrack --
      // and shaded darker once the water has had time to reach it, which is
      // what makes the flood something you can see rather than just a bar.
      if (s.maze.seen[y][x]) {
        bool flooded = s.maze.movesLeft <= minigames::mazeBudgetFor(n) / 3;
        dFillRect(px + 3, py + 3, cell - 6, cell - 6, flooded ? SHADE_DARK : SHADE_LIGHT);
      }
      if (w & minigames::MazeRound::WALL_N) epd.fillRect(px, py, cell, 2, C_BLACK);
      if (w & minigames::MazeRound::WALL_W) epd.fillRect(px, py, 2, cell, C_BLACK);
      if (x == n - 1 && (w & minigames::MazeRound::WALL_E)) {
        epd.fillRect(px + cell - 1, py, 2, cell, C_BLACK);
      }
      if (y == n - 1 && (w & minigames::MazeRound::WALL_S)) {
        epd.fillRect(px, py + cell - 1, cell, 2, C_BLACK);
      }
    }
  }

  // The way out: an actual gap knocked through the outer wall with the
  // tunnel mouth beyond it, rather than a black blob filling the corner
  // cell. The wall loop above already drew that segment, so it's cleared
  // first -- a goal you can see daylight through reads as an exit.
  int ex = gx + (n - 1) * cell, ey = gy + (n - 1) * cell;
  int gap = cell / 2, gapX = ex + cell / 2 - gap / 2;
  epd.fillRect(gapX, gy + gh - 2, gap, 4, C_WHITE);
  epd.fillRect(gapX, gy + gh - 2, 2, 4, C_BLACK);
  epd.fillRect(gapX + gap - 2, gy + gh - 2, 2, 4, C_BLACK);
  // Mouth flaring out below the wall line.
  epd.drawLine(gapX, gy + gh, gapX - 5, gy + gh + 9, C_BLACK);
  epd.drawLine(gapX + gap, gy + gh, gapX + gap + 5, gy + gh + 9, C_BLACK);
  dFillRect(gapX - 4, gy + gh + 2, gap + 8, 7, SHADE_LIGHT);
  textCentered(ex, cell, ey + 6, "OUT", 1);

  // A silhouette token rather than the photo art: even the small marmot
  // poses are 112px, and a cell here is 34-46px.
  {
    int mx = gx + s.maze.x * cell + cell / 2;
    int my = gy + s.maze.y * cell + cell / 2;
    epd.drawBitmap(mx - TOKEN_MARMOT_W / 2, my - TOKEN_MARMOT_H / 2, TOKEN_MARMOT_BITMAP,
                   TOKEN_MARMOT_W, TOKEN_MARMOT_H, C_BLACK, C_WHITE);
  }

  int opts[4];
  int nOpts = minigames::mazeOptions(s, opts);
  // Breathing room between the board and the controls. There's slack at the
  // bottom of the panel, so the whole lower block sits lower rather than
  // crowding the maze.
  int y = gy + gh + 22;
  textCentered(0, SCREEN_W, y, "Which tunnel?", 1);
  // The option list is short by construction (a cell has at most four
  // exits), so every choice fits on one row of boxes.
  int boxW = 66, totalW = nOpts * boxW;
  int bx = (SCREEN_W - totalW) / 2;
  for (int i = 0; i < nOpts; i++) {
    optionBox(bx + i * boxW + 3, y + 16, boxW - 6, 26, mazeDirName(opts[i]),
              i == (s.maze.sel % nOpts));
  }

  // The rising meltwater is the fail state, so it's drawn as water gaining
  // on you -- a bar that FILLS as moves run out, not one that drains. A
  // depleting "moves left" gauge is bookkeeping; a rising flood is a threat.
  int budget = minigames::mazeBudgetFor(n);
  int left = s.maze.movesLeft < 0 ? 0 : s.maze.movesLeft;
  int used = budget - left;
  bool low = left <= 4;
  // Clear of the option boxes above (which end at y+42): the label sits at
  // barY-12, so the bar has to start at least 56 below `y`.
  int barY = y + 68, barX = 40, barW = SCREEN_W - 80;
  textCentered(0, SCREEN_W, barY - 12,
               low ? "The water's at your tail!"
                   : "Meltwater rising -- " + std::to_string(left) + " moves ahead",
               1);
  epd.drawRect(barX, barY, barW, 10, C_BLACK);
  int flood = (barW - 4) * used / (budget > 0 ? budget : 1);
  dFillRect(barX + 2, barY + 2, flood, 6, low ? SHADE_BLACK : SHADE_DARK);

  drawNavBar("Prev", "Dig", "Next");
}

static void renderQuizPrompt(const minigames::State& s) {
  textAt(8, 10, "Species Quiz", 2);
  textAt(SCREEN_W - 96, 14, "Score: " + std::to_string(s.score), 1);
  epd.drawFastHLine(8, 34, SCREEN_W - 16, C_BLACK);

  // The clue sits in its own framed panel: it's the whole question, and on
  // a page that's otherwise three answer boxes it needs to read as the
  // thing being asked rather than as a caption.
  textCentered(0, SCREEN_W, 44, "Which one is this?", 1);
  epd.drawRoundRect(10, 60, SCREEN_W - 20, 160, 10, C_BLACK);
  textWrapped(22, 78, 42, minigames::quizFact(s), 1);

  for (int i = 0; i < 3; i++) {
    optionBox(16, 236 + i * 42, SCREEN_W - 32, 34, minigames::choiceOption(s, i).name,
              i == s.choice.sel);
  }

  drawNavBar("Prev", "Answer", "Next");
}

static void renderQuizReveal(const minigames::State& s) {
  textCentered(0, SCREEN_W, 12, "Correct!", 2);

  const Forageable& f = minigames::choiceAnswer(s);
  const int iconSize = 144;
  int iconY = 50;
  const species_bitmaps::SpeciesBitmap* art = species_bitmaps::find(f.name);
  if (art) {
    epd.drawBitmap((SCREEN_W - art->w) / 2, iconY + (iconSize - art->h) / 2, art->bitmap, art->w,
                   art->h, C_BLACK, C_WHITE);
  } else {
    // Same placeholder frame the Foraging view uses for species without a
    // sourced photo yet (see include/bitmaps/species/).
    epd.drawRect((SCREEN_W - iconSize) / 2, iconY, iconSize, iconSize, C_BLACK);
    textCentered(0, SCREEN_W, iconY + iconSize / 2 - 4, "?", 2);
  }

  int y = iconY + iconSize + 12;
  textCentered(0, SCREEN_W, y, f.name, 2);
  y += 26;
  textCentered(0, SCREEN_W, y, std::string(f.kind) + " - " + foraging::biomeName(f.biome), 1);
  y += 22;
  textWrapped(14, y, 44, f.harvestTip, 1);

  textCentered(0, SCREEN_W, SCREEN_H - 34, "Score: " + std::to_string(s.score), 1);
  drawNavBar("", "Next", "");
}

static void renderSimon(const minigames::State& s) {
  textAt(8, 10, "Marmot Says", 2);
  textAt(SCREEN_W - 96, 14, "Round: " + std::to_string(s.score + 1), 1);
  epd.drawFastHLine(8, 34, SCREEN_W - 16, C_BLACK);

  if (s.screen == minigames::Screen::Sequence) {
    textCentered(0, SCREEN_W, 56, "The marmot whistles...", 1);
    // The real photo art, not the procedural glyph: this is the one screen
    // in the games with room for it (the small pose set is 112x136), and
    // the marmot is the thing doing the whistling, so it should look like
    // the marmot everywhere else in the app looks.
    const MarmotArt& art = kMarmotExcitedSmall[0];
    epd.drawBitmap(10, 150, art.bitmap, art.w, art.h, C_BLACK, C_WHITE);
    drawSimonCall(s.simon.icon[s.simon.seq[s.simon.showIdx]], 208, 210, 72);
    textCentered(
        0, SCREEN_W, 300,
        "Call " + std::to_string(s.simon.showIdx + 1) + " of " + std::to_string(s.simon.len), 1);
    drawNavBar("", "", "");
    return;
  }

  textCentered(0, SCREEN_W, 56, "Your turn -- whistle it back", 1);

  // A small reference of which button is which call, so the mapping never
  // has to be remembered separately from the sequence itself.
  // Which button is which call, this run. Reshuffled per run, so this is a
  // legend the player actually needs rather than decoration.
  drawSimonCall(s.simon.icon[0], 58, 132, 56);
  drawSimonCall(s.simon.icon[2], SCREEN_W / 2, 132, 56);
  drawSimonCall(s.simon.icon[1], SCREEN_W - 58, 132, 56);
  textCentered(28, 60, 168, "LEFT", 1);
  textCentered(SCREEN_W / 2 - 30, 60, 168, "ENTER", 1);
  textCentered(SCREEN_W - 88, 60, 168, "RIGHT", 1);

  // One pip per call, filled as far as the player has got right.
  int n = s.simon.len;
  int spacing = n > 12 ? 16 : 24;
  int x0 = SCREEN_W / 2 - (n - 1) * spacing / 2;
  for (int i = 0; i < n; i++) {
    if (i < s.simon.inputIdx) {
      epd.fillCircle(x0 + i * spacing, 240, 7, C_BLACK);
    } else {
      epd.drawCircle(x0 + i * spacing, 240, 7, C_BLACK);
    }
  }
  textCentered(0, SCREEN_W, 280,
               std::to_string(s.simon.inputIdx) + " of " + std::to_string(n) + " repeated", 1);
  drawNavBar("Left", "Enter", "Right");
}

static void renderMinigameOver(const minigames::State& s) {
  textAt(8, 10, minigames::gameName(s.game), 2);
  epd.drawFastHLine(8, 34, SCREEN_W - 16, C_BLACK);

  // A Snack Hunt run cut short by a predator gets its own layout: the
  // animal art is 170px tall, which simply doesn't fit alongside the
  // standard score block, and the scare is the more interesting ending.
  bool bolted = s.game == minigames::Game::Snack && s.snack.picked >= 0 &&
                s.snack.kind[s.snack.picked] == minigames::KIND_SCARE;
  if (bolted) {
    int which = s.snack.amount[s.snack.picked];
    textCentered(0, SCREEN_W, 48, "You ran for it!", 2);
    textCentered(0, SCREEN_W, 78, std::string(minigames::predatorName(which)) + " was waiting.", 1);
    SnackPredator art = snackPredatorArt(which);
    epd.drawBitmap((SCREEN_W - art.w) / 2, 96, art.bitmap, art.w, art.h, C_BLACK, C_WHITE);
    textCentered(0, SCREEN_W, 274, "Everything you found is safe", 1);
    textCentered(0, SCREEN_W, 288, "in the stash.", 1);
    drawStashBar(308);
    textCentered(0, SCREEN_W, SCREEN_H - 34, "BACK = pick another game", 1);
    drawNavBar("", "Play again", "");
    return;
  }

  textCentered(0, SCREEN_W, 60,
               s.won ? "You beat it!" : (s.score > 0 ? "Nice run!" : "That's the game"), 2);
  if (s.won) {
    textCentered(0, SCREEN_W, 88,
                 "All " + std::to_string(minigames::SIMON_WIN_ROUNDS) + " calls, start to finish.",
                 1);
  }
  textCentered(0, SCREEN_W, 110, "Score", 1);
  textCentered(0, SCREEN_W, 130, std::to_string(s.score), 4);
  textCentered(0, SCREEN_W, 190, "Best: " + std::to_string(minigames::highScore(s.game)), 1);
  if (s.newBest) textCentered(0, SCREEN_W, 212, "NEW BEST!", 2);

  // Both species games close with the answer they were asking for -- being
  // told you're wrong and never learning what was right is the one way a
  // quiz can waste the player's time.
  if (s.game == minigames::Game::Quiz) {
    textCentered(0, SCREEN_W, 260, "It was:", 1);
    textWrapped(8, 280, 23, minigames::choiceAnswer(s).name, 2);
  } else if (s.game == minigames::Game::Snack) {
    // Snack Hunt's run score is beside the point -- the stash it fed is
    // what carries over, so that's what the summary leads with.
    drawStashBar(258);
    textCentered(0, SCREEN_W, 300,
                 minigames::stashPoints() >= minigames::WINTER_STASH_GOAL
                     ? "Enough to see out the winter."
                     : "Keep gathering before winter.",
                 1);
  }

  textCentered(0, SCREEN_W, SCREEN_H - 34, "BACK = pick another game", 1);
  drawNavBar("", "Play again", "");
}

/**
 * Winter settling-up for the Snack Hunt stockpile, shown once on the first
 * December wake (see minigames::stashResolveDue()). `made` is whether the
 * pile reached WINTER_STASH_GOAL; the caller applies the matching
 * happiness/energy change and then clears the pile for next year.
 */
void renderWinterStash(bool made, bool grace, int points, const char* name) {
  epd.beginFrame();
  textCentered(0, SCREEN_W, 20, "WINTER", 2);
  textCentered(0, SCREEN_W, 48, "The snow has come.", 1);

  const MarmotArt& art = kMarmotSleepy[0];
  int cx = SCREEN_W / 2;
  int topY = 74;
  epd.drawBitmap(cx - art.w / 2, topY, art.bitmap, art.w, art.h, C_BLACK, C_WHITE);

  int y = topY + art.h + 10;
  textCentered(0, SCREEN_W, y, std::string(name) + " stashed " + std::to_string(points), 1);
  textCentered(0, SCREEN_W, y + 16,
               "of " + std::to_string(minigames::WINTER_STASH_GOAL) + " needed.", 1);

  y += 44;
  if (made) {
    textCentered(0, SCREEN_W, y, "A full larder!", 2);
    textCentered(0, SCREEN_W, y + 28, "It dens up fat and content,", 1);
    textCentered(0, SCREEN_W, y + 44, "and sleeps easy till spring.", 1);
  } else if (grace) {
    // Born too close to winter to have gathered a season's worth -- no
    // penalty, since it was never a goal this marmot could reach.
    textCentered(0, SCREEN_W, y, "Winter came early.", 2);
    textCentered(0, SCREEN_W, y + 28, "Too young to have gathered", 1);
    textCentered(0, SCREEN_W, y + 44, "much. Next year, then.", 1);
  } else {
    textCentered(0, SCREEN_W, y, "A thin larder.", 2);
    textCentered(0, SCREEN_W, y + 28, "It dens up hungry, and the", 1);
    textCentered(0, SCREEN_W, y + 44, "winter will be a long one.", 1);
  }

  textCentered(0, SCREEN_W, SCREEN_H - 30, "Press ENTER", 1);
  epd.endFrame(true);
}

/**
 * "You've unlocked a new game" reveal, shown once per game the first time
 * its condition is met (see minigames::pendingUnlocks()). Borrows the
 * growth-transition screen's deliberate full-refresh flashes -- the flicker
 * sells the moment here the same way it does there.
 */
void renderMinigameUnlock(minigames::Game g, const char* name) {
  for (int i = 0; i < 2; i++) {
    epd.beginFrame();
    epd.fillScreen(C_BLACK);
    epd.endFrame(true);
    delay(180);
    epd.beginFrame();
    epd.fillScreen(C_WHITE);
    epd.endFrame(true);
    delay(180);
  }

  epd.beginFrame();

  // Banner: inverted bar rather than plain text, so the reveal announces
  // itself instead of looking like another view.
  epd.fillRect(0, 30, SCREEN_W, 34, C_BLACK);
  epd.setFont(nullptr);
  epd.setTextSize(2);
  {
    int16_t bx, by;
    uint16_t bw, bh;
    epd.getTextBounds("NEW MINIGAME", 0, 0, &bx, &by, &bw, &bh);
    epd.setTextColor(C_WHITE);
    epd.setCursor((SCREEN_W - (int)bw) / 2, 39);
    epd.print("NEW MINIGAME");
    epd.setTextColor(C_BLACK);
  }
  textCentered(0, SCREEN_W, 78, std::string(name) + " has something new to do", 1);

  // The name itself is the art here -- no emblem. Every game name fits at
  // size 3 (18px/char, longest is 13 characters against a 300px panel), and
  // a framed panel gives it the weight the medal used to.
  const int panelY = 116, panelH = 128;
  epd.drawRoundRect(16, panelY, SCREEN_W - 32, panelH, 12, C_BLACK);
  epd.drawRoundRect(19, panelY + 3, SCREEN_W - 38, panelH - 6, 10, C_BLACK);
  textCentered(0, SCREEN_W, panelY + 34, minigames::gameName(g), 3);
  epd.drawFastHLine(48, panelY + 74, SCREEN_W - 96, C_BLACK);
  textCentered(0, SCREEN_W, panelY + 90, minigames::gameBlurb(g), 1);

  textCentered(0, SCREEN_W, 276, "Unlocked. Find it under", 1);
  textCentered(0, SCREEN_W, 292, "Minigames, left of Status.", 1);
  textCentered(0, SCREEN_W, SCREEN_H - 30, "Press ENTER", 1);
  epd.endFrame(true);
}

void renderMinigames(const AppContext& ctx, const minigames::State& s, bool forceFullRefresh) {
  // Same forced-full-refresh mechanics as renderView() -- a run racks up a
  // lot of partial refreshes, so callers periodically ask for a clean one.
  if (forceFullRefresh) epd.reinitPanel();
  epd.beginFrame();
  switch (s.screen) {
    case minigames::Screen::Menu:
      renderMinigameMenu(ctx, s);
      break;
    case minigames::Screen::Over:
      renderMinigameOver(s);
      break;
    default:
      // Everything else dispatches on the GAME, not the screen. Screen is
      // shared vocabulary -- Snack Hunt and the Quiz both use Reveal, Simon
      // and Memory both use Sequence -- so a screen-first switch sent Snack
      // Hunt's bush reveal to the Quiz's renderer, which then drew a species
      // card out of a ChoiceRound the run had never filled in (landing on
      // species 0, Dead Man's Fingers, every single time). Each renderer
      // handles its own screens.
      switch (s.game) {
        case minigames::Game::Snack:
          renderSnack(s);
          break;
        case minigames::Game::Simon:
          renderSimon(s);
          break;
        case minigames::Game::Memory:
          renderMemory(s);
          break;
        case minigames::Game::Maze:
          renderMaze(s);
          break;
        default:
          if (s.screen == minigames::Screen::Reveal) {
            renderQuizReveal(s);
          } else {
            renderQuizPrompt(s);
          }
          break;
      }
      break;
  }
  epd.endFrame(!forceFullRefresh);
}

void begin() {
  // epd.begin() owns SPI setup internally (custom SCK/MOSI pins are
  // configured in epd_official/epdif.cpp's IfInit()).
  epd.begin();
  epd.setRotation(3);
  epd.setTextWrap(false);
}

void renderView(View v, const AppContext& ctx, int speciesIdx, bool forceFullRefresh) {
  // Requesting partial refresh, not full: endFrame() forces a full refresh
  // on the wake's first frame regardless (see epd_adapter.h) and honors
  // `partial` after that -- unless the caller wants a forced full draw.
  // forceFullRefresh re-inits just the panel registers (reinitPanel(), not
  // the SPI-touching epd.begin() -- calling that mid-session wedged the
  // bus and hung the device, see epd_adapter.h) to actually clear ghosting
  // instead of drawing on top of leftover partial-refresh state.
  if (forceFullRefresh) epd.reinitPanel();
  epd.beginFrame();
  switch (v) {
    case View::Main:
      renderMain(ctx);
      break;
    case View::Foraging:
      renderForaging(ctx, speciesIdx);
      break;
    case View::Status:
      renderStatus(ctx);
      break;
    default:
      // View::Minigames included -- it draws through renderMinigames()
      // instead, since it needs game state renderView() isn't given.
      break;
  }
  epd.endFrame(!forceFullRefresh);
}

/**
 * One-time "birth" reveal, shown only on the very first-ever boot (see
 * main.cpp) before the normal view takes over. A short two-frame sequence
 * using the same beginFrame()/endFrame() full-refresh pattern as every other
 * screen -- no new animation machinery needed.
 */
void renderBirth() {
  epd.beginFrame();
  textCentered(0, SCREEN_W, SCREEN_H / 2 - 10, "...", 3);
  epd.endFrame(true);
  delay(1400);

  epd.beginFrame();
  const MarmotArt& baby = kBabyVariants[0];
  int cx = SCREEN_W / 2;
  int by = SCREEN_H / 2 + 60;
  int bx = cx - baby.w / 2;
  int topY = by - baby.groundY;
  epd.drawBitmap(bx, topY, baby.bitmap, baby.w, baby.h, C_BLACK, C_WHITE);
  textCentered(0, SCREEN_W, topY - 30, "A baby marmot is born!", 1);
  textCentered(0, SCREEN_W, SCREEN_H - 30, "Press ENTER", 1);
  epd.endFrame(true);
}

void renderTransition(Stage newStage, const char* name) {
  // A couple of deliberate full-refresh flashes -- normally something this
  // codebase avoids (see the dithering/no-partial-refresh notes elsewhere),
  // but here the flicker itself sells the "leveling up" moment.
  for (int i = 0; i < 2; i++) {
    epd.beginFrame();
    epd.fillScreen(C_BLACK);
    epd.endFrame(true);
    delay(180);
    epd.beginFrame();
    epd.fillScreen(C_WHITE);
    epd.endFrame(true);
    delay(180);
  }

  epd.beginFrame();
  textCentered(0, SCREEN_W, 20, std::string(name) + " grew up!", 1);

  const MarmotArt& art = (newStage == Stage::Juvenile) ? kJuvenileVariants[0] : kMarmotContent[0];
  int cx = SCREEN_W / 2;
  int by = 60 + art.groundY;
  int bx = cx - art.w / 2;
  int topY = by - art.groundY;
  epd.drawBitmap(bx, topY, art.bitmap, art.w, art.h, C_BLACK, C_WHITE);

  const char* label = (newStage == Stage::Juvenile) ? "Juvenile" : "Adult";
  textCentered(0, SCREEN_W, by + 20, label, 2);
  textCentered(0, SCREEN_W, SCREEN_H - 30, "Press ENTER", 1);
  epd.endFrame(true);
}

// One line per DeathCause, picked at random each time so the ending doesn't
// always read the same way -- the cause itself (which bar bottomed out)
// still always matches what actually happened.
static const char* const kDeathStarved[] = {
    "Starved after too long without food.",
    "Never found enough to eat.",
    "Went too long between meals.",
};
static const char* const kDeathHeartbroken[] = {
    "Heartbroken from being ignored.",
    "Gave up after too long alone.",
    "Lost interest in everything, unloved.",
};
static const char* const kDeathExhausted[] = {
    "Exhausted from never resting easy.",
    "Wore out with no one checking in.",
    "Ran out of energy, alone too long.",
};

static const char* deathReasonLine(DeathCause cause) {
  switch (cause) {
    case DeathCause::Starved:
      return kDeathStarved[random(3)];
    case DeathCause::Heartbroken:
      return kDeathHeartbroken[random(3)];
    case DeathCause::Exhausted:
      return kDeathExhausted[random(3)];
    default:
      return "Neglected for too long.";
  }
}

void renderDeath(DeathCause cause) {
  epd.beginFrame();
  // The art is 280px tall, so its placement is what everything else has to
  // fit around: at the old y=82 its bottom edge landed at 362, *through*
  // the first line of the sign-off text at 356. Title and image both move
  // up, leaving a clear 14px gap above the text block.
  textCentered(0, SCREEN_W, 20, "Your marmot has died.", 2);
  textCentered(0, SCREEN_W, 44, deathReasonLine(cause), 1);
  int bx = (SCREEN_W - MARMOT_DEATH_W) / 2, by = 60;
  epd.drawBitmap(bx, by, MARMOT_DEATH_BITMAP, MARMOT_DEATH_W, MARMOT_DEATH_H, C_BLACK, C_WHITE);
  textCentered(0, SCREEN_W, SCREEN_H - 46, "Press ENTER to start over", 1);
  textCentered(0, SCREEN_W, SCREEN_H - 32, "with a new baby marmot.", 1);
  epd.endFrame(true);
}

/**
 * Settings overlay -- triggered by the dedicated SETTINGS button (see
 * main.cpp), not part of the normal View cycle. selected: 0 = Power Off,
 * 1 = Reset Game. confirmPending shows a yes/no sub-screen for Reset Game,
 * since it's destructive.
 */
void renderSettings(int selected, bool confirmPending, uint8_t batteryPercent) {
  epd.beginFrame();
  textAt(8, 10, "Settings", 2);
  drawBatteryIcon(SCREEN_W - 52, 10, batteryPercent);
  textAt(SCREEN_W - 52 - 30, 12, std::to_string(batteryPercent) + "%", 1);

  if (confirmPending) {
    if (selected == SETTINGS_RESET_GAME) {
      textCentered(0, SCREEN_W, 160, "Really reset the game?", 1);
      textCentered(0, SCREEN_W, 180, "This erases all progress.", 1);
    } else {
      textCentered(0, SCREEN_W, 160, "Power off?", 1);
      textCentered(0, SCREEN_W, 180, "Only the switch wakes it.", 1);
    }
    drawNavBar("", "", "");
    textCentered(0, SCREEN_W, SCREEN_H - 34, "ENTER = Yes, BACK = No", 1);
  } else {
    const char* options[SETTINGS_OPTION_COUNT] = {"Achievements", "WiFi Networks", "Reset Game",
                                                  "Power Off"};
    for (int i = 0; i < SETTINGS_OPTION_COUNT; i++) {
      textAt(20, 90 + i * 30, std::string(i == selected ? "> " : "  ") + options[i], 1);
    }
    drawNavBar("Prev", "Select", "Next");
    textCentered(0, SCREEN_W, SCREEN_H - 34, "BACK = previous menu", 1);
  }

  epd.endFrame(true);
}

// Short label for a grid cell -- printable characters show as themselves,
// the control/toggle sentinels get a word so an empty-looking or sentinel
// cell isn't mistaken for a rendering glitch.
static std::string keyLabel(char c) {
  if (c == textentry::BACKSPACE) return "DEL";
  if (c == textentry::DONE) return "OK";
  if (c == textentry::SHIFT) return "SHIFT";
  if (c == textentry::SYMBOLS) return "SYM";
  if (c == ' ') return "SPACE";
  return std::string(1, c);
}

// A real on-screen keyboard grid (not a single blown-up character) -- every
// key on the active page (letters/SHIFT-caps/SYMBOLS, see textentry.h) is
// visible at once, LEFT/RIGHT sweep the highlighted cell through the grid
// in the same row-major order the flat charset is stored in, so cursor
// motion always matches what's drawn without any separate 2D-navigation
// bookkeeping. Each key is sized to fit its own label (so "SPACE"/"SHIFT"
// aren't clipped) rather than a fixed column width, then each row is
// centered as a whole -- which is also what gives the letter rows their
// realistic QWERTY stagger.
void renderTextEntry(const char* prompt, const textentry::State& s) {
  epd.beginFrame();
  textAt(8, 10, prompt, 1);
  textAt(8, 34, s.buffer, 2);

  const int cellH = 24;
  const int keyPad = 8;
  const int minKeyW = 22;
  const int gridY = 66;

  epd.setFont(nullptr);
  epd.setTextSize(1);

  int idx = 0;
  for (int row = 0; row < textentry::rowCount(s); row++) {
    int rowLen = textentry::rowLen(s, row);
    int widths[16];
    int rowW = 0;
    for (int col = 0; col < rowLen; col++) {
      std::string label = keyLabel(textentry::charsetAt(s, idx + col));
      int16_t bx, by;
      uint16_t bw, bh;
      epd.getTextBounds(label.c_str(), 0, 0, &bx, &by, &bw, &bh);
      widths[col] = std::max((int)bw + keyPad, minKeyW);
      rowW += widths[col];
    }
    int cx = (SCREEN_W - rowW) / 2;
    int cy = gridY + row * cellH;
    for (int col = 0; col < rowLen; col++, idx++) {
      char c = textentry::charsetAt(s, idx);
      std::string label = keyLabel(c);
      int w = widths[col];
      int16_t bx, by;
      uint16_t bw, bh;
      epd.getTextBounds(label.c_str(), 0, 0, &bx, &by, &bw, &bh);
      bool toggledOn = (c == textentry::SHIFT && s.caps) || (c == textentry::SYMBOLS && s.symbols);
      if (idx == s.pickerIndex) {
        epd.fillRect(cx + 1, cy + 1, w - 2, cellH - 2, C_BLACK);
        epd.setTextColor(C_WHITE);
        epd.setCursor(cx + (w - (int)bw) / 2, cy + (cellH - (int)bh) / 2);
        epd.print(label.c_str());
        epd.setTextColor(C_BLACK);
      } else {
        if (toggledOn) epd.drawRect(cx + 1, cy + 1, w - 2, cellH - 2, C_BLACK);
        textCentered(cx, w, cy + 6, label, 1);
      }
      cx += w;
    }
  }

  drawNavBar("Prev", "Pick", "Next");
  epd.endFrame(true);
}

void renderWifiMenu(int selected, bool confirmRemove) {
  epd.beginFrame();
  textAt(8, 10, "WiFi Networks", 2);

  int count = wifistore::count();
  if (confirmRemove && selected >= 0 && selected < count) {
    std::string msg = std::string("Remove '") + wifistore::at(selected).ssid + "'?";
    textCentered(0, SCREEN_W, 160, msg, 1);
    drawNavBar("", "", "");
    textCentered(0, SCREEN_W, SCREEN_H - 34, "ENTER = Yes, BACK = No", 1);
    epd.endFrame(true);
    return;
  }

  textAt(20, 32, "Used to set the clock and fetch weather.", 1);
  textAt(20, 44, "The marmot works offline without it.", 1);

  int y = 64;
  if (count == 0) {
    textAt(20, y, "(no saved networks)", 1);
    y += 20;
  } else {
    for (int i = 0; i < count; i++) {
      textAt(20, y, std::string(i == selected ? "> " : "  ") + wifistore::at(i).ssid, 1);
      y += 18;
    }
  }
  textAt(20, y, std::string(selected == count ? "> " : "  ") + "Add Network", 1);

  drawNavBar("Prev", "Select", "Next");
  textCentered(0, SCREEN_W, SCREEN_H - 34, "BACK = previous menu", 1);
  epd.endFrame(true);
}

/**
 * Brief goodbye screen shown right before a Power Off deep-sleep with no
 * wake source armed -- only the physical power switch brings the device
 * back after this.
 */
void renderConnecting() {
  epd.beginFrame();
  textCentered(0, SCREEN_W, SCREEN_H / 2 - 20, "Setting the clock", 2);
  textCentered(0, SCREEN_W, SCREEN_H / 2 + 6, "This only happens on a cold start.", 1);
  epd.endFrame(true);
}

void renderPowerOff() {
  // Genuinely off (no wake source armed) -- a blank panel is the correct
  // resting state here, not a lingering message that'd sit on an
  // unpowered e-ink display until the physical switch cycles it.
  epd.beginFrame();
  epd.endFrame(true);
}

void renderSleep(Stage stage) {
  epd.beginFrame();
  // Always the first variant for the stage (not the per-wake random pick
  // drawCreature() uses for the live views) so this screen reads
  // consistently as "asleep". No dedicated lounging/sleeping photo exists
  // for Baby/Juvenile (real hoary marmot pup photos in that specific pose
  // don't turn up in the wild -- see CLAUDE.md's sourcing notes), so those
  // stages reuse their normal standing pose here rather than showing the
  // wrong-stage Adult art.
  const MarmotArt* art = stage == Stage::Baby       ? &kBabyVariants[0]
                         : stage == Stage::Juvenile ? &kJuvenileVariants[0]
                                                    : &kMarmotSleepy[0];
  int cx = SCREEN_W / 2;
  int by = SCREEN_H / 2 + 40;
  int bx = cx - art->w / 2;
  int topY = by - art->groundY;
  epd.drawBitmap(bx, topY, art->bitmap, art->w, art->h, C_BLACK, C_WHITE);

  // Stack the Z's up and to the right of the head. The Adult sleepy pose's
  // canvas has a lot of empty headroom above the actual marmot (a wide
  // lounging shot on a portrait canvas), so it anchors off the ground line
  // instead of the art's top edge; Baby/Juvenile's standing poses don't
  // have that headroom, so anchor those off the art's actual top edge.
  // Baby's pose sits lowest in its canvas of the three, so its Z's start
  // much further down -- at the Juvenile offset they floated well clear of
  // the head with a visible gap. Juvenile and Adult get a smaller nudge in
  // the same direction for the same reason, just less of one.
  int headX = std::min(bx + art->w - 40, SCREEN_W - 50);
  int zOffset = stage == Stage::Baby ? 66 : 34;
  int headY = stage == Stage::Adult ? std::max(by - 86, 44) : std::max(topY + zOffset, 44);
  textAt(headX, headY, "Z", 3);
  textAt(headX + 22, headY - 24, "Z", 2);
  textAt(headX + 38, headY - 44, "Z", 1);

  epd.endFrame(true);
}

void hibernate() { epd.sleep(); }

}  // namespace display
