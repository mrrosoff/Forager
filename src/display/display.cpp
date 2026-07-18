#include "display.h"

#include <SPI.h>
#include <math.h>

#include <cstring>

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
#include "bitmaps/marmot/marmot_bitmap.h"
#include "bitmaps/marmot/marmot_hungry_bitmap.h"
#include "bitmaps/marmot/marmot_variant1_bitmap.h"
#include "bitmaps/marmot/marmot_variant2_bitmap.h"
#include "bitmaps/marmot/marmot_variant3_bitmap.h"
#include "bitmaps/marmot/marmot_variant4_bitmap.h"
#include "bitmaps/marmot/marmot_variant5_bitmap.h"
#include "bitmaps/species/species_index.h"
#include "config.h"
#include "creature.h"
#include "epd_adapter.h"
#include "events.h"
#include "foraging.h"

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

// Word-wraps `s` into lines of at most `maxChars` characters (breaking on
// spaces), drawing each line `size`*8+2 px apart starting at (x,y). Returns
// the y coordinate just past the last line, for stacking content below it.
static int textWrapped(int x, int y, int maxChars, const char* s, uint8_t size = 1) {
  char line[64];
  int lineH = size * 8 + 2;
  const char* p = s;
  while (*p) {
    int n = 0;
    int lastSpace = -1;
    while (p[n] && n < maxChars) {
      if (p[n] == ' ') lastSpace = n;
      n++;
    }
    if (p[n] && lastSpace >= 0) n = lastSpace;  // break at last space in range
    int copyLen = n < (int)sizeof(line) - 1 ? n : (int)sizeof(line) - 1;
    memcpy(line, p, copyLen);
    line[copyLen] = '\0';
    textAt(x, y, line, size);
    y += lineH;
    p += n;
    while (*p == ' ') p++;
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

struct MarmotArt {
  const uint8_t* bitmap;
  int w, h, groundY;
};

// Non-Hungry moods rotate through the base pose plus a handful of real
// hoary-marmot photos (see include/bitmaps/marmot/) for visual variety --
// picked once per wake below, not per render, so the creature doesn't
// change pose mid-session as the user pages between views.
static const MarmotArt kMarmotVariants[] = {
    {MARMOT_BITMAP, MARMOT_W, MARMOT_H, MARMOT_GROUND_Y},
    {MARMOT_VARIANT1_BITMAP, MARMOT_VARIANT1_W, MARMOT_VARIANT1_H, MARMOT_VARIANT1_H},
    {MARMOT_VARIANT2_BITMAP, MARMOT_VARIANT2_W, MARMOT_VARIANT2_H, MARMOT_VARIANT2_H},
    {MARMOT_VARIANT3_BITMAP, MARMOT_VARIANT3_W, MARMOT_VARIANT3_H, MARMOT_VARIANT3_H},
    {MARMOT_VARIANT4_BITMAP, MARMOT_VARIANT4_W, MARMOT_VARIANT4_H, MARMOT_VARIANT4_H},
    {MARMOT_VARIANT5_BITMAP, MARMOT_VARIANT5_W, MARMOT_VARIANT5_H, MARMOT_VARIANT5_H},
};
static const int kMarmotVariantCount = sizeof(kMarmotVariants) / sizeof(kMarmotVariants[0]);

// A hoary marmot -- hardcoded, dithered pen-and-ink-style/photo bitmaps (see
// include/bitmaps/marmot/) rather than live procedural shapes, for real fur
// texture and a recognizable silhouette. No procedural eye/nose/sparkle
// overlay -- just the bitmap.
static void drawCreature(int cx, int groundY, Mood mood) {
  if (mood == Mood::Hungry) {
    int bx = cx - MARMOT_HUNGRY_W / 2;
    int by = groundY - MARMOT_HUNGRY_GROUND_Y;
    epd.drawBitmap(bx, by, MARMOT_HUNGRY_BITMAP, MARMOT_HUNGRY_W, MARMOT_HUNGRY_H, C_BLACK,
                   C_WHITE);
    return;
  }
  // Deep sleep wipes ordinary RAM, so a function-local static naturally
  // re-rolls on the very first drawCreature() call each wake and then holds
  // steady for the rest of the session.
  static int8_t variant = -1;
  if (variant < 0) variant = (int8_t)random(kMarmotVariantCount);
  const MarmotArt& art = kMarmotVariants[variant];
  int bx = cx - art.w / 2;
  int by = groundY - art.groundY;
  epd.drawBitmap(bx, by, art.bitmap, art.w, art.h, C_BLACK, C_WHITE);
}

// Bottom nav bar shown on every view: what LEFT/RIGHT/ENTER do from here.
static const int NAV_Y = SCREEN_H - 14;

static void drawNavBar(const char* leftLbl, const char* enterLbl, const char* rightLbl) {
  epd.drawFastHLine(4, NAV_Y - 4, SCREEN_W - 8, C_BLACK);
  char buf[24];
  snprintf(buf, sizeof(buf), "<%s", leftLbl);
  textAt(4, NAV_Y, buf, 1);
  textCentered(0, SCREEN_W, NAV_Y, enterLbl, 1);
  snprintf(buf, sizeof(buf), "%s>", rightLbl);
  int16_t bx, by;
  uint16_t bw, bh;
  epd.setFont(nullptr);
  epd.setTextSize(1);
  epd.getTextBounds(buf, 0, NAV_Y, &bx, &by, &bw, &bh);
  textAt(SCREEN_W - 4 - (int)bw, NAV_Y, buf, 1);
}

// Per-animal reference-photo artwork for the encounter screen, indexed the
// same way as events::animalIndex() -- i.e. positionally matching kAnimals[]
// in events.cpp. Not every animal has art (some source photos didn't dither
// into a recognizable silhouette); nullptr means "fall back to the marmot".
struct AnimalArt {
  const uint8_t* bitmap;
  int w, h;
};
static const AnimalArt kAnimalArt[] = {
    {ANIMAL_DEER_BITMAP, ANIMAL_DEER_W, ANIMAL_DEER_H},                             // Deer
    {ANIMAL_MOUNTAIN_GOAT_BITMAP, ANIMAL_MOUNTAIN_GOAT_W, ANIMAL_MOUNTAIN_GOAT_H},  // Mountain Goat
    {ANIMAL_BALD_EAGLE_BITMAP, ANIMAL_BALD_EAGLE_W, ANIMAL_BALD_EAGLE_H},           // Bald Eagle
    {ANIMAL_RIVER_OTTER_BITMAP, ANIMAL_RIVER_OTTER_W, ANIMAL_RIVER_OTTER_H},        // River Otter
    {ANIMAL_RACCOON_BITMAP, ANIMAL_RACCOON_W, ANIMAL_RACCOON_H},                    // Raccoon
    {ANIMAL_ELK_BITMAP, ANIMAL_ELK_W, ANIMAL_ELK_H},                                // Elk
    {ANIMAL_BLACK_BEAR_BITMAP, ANIMAL_BLACK_BEAR_W, ANIMAL_BLACK_BEAR_H},           // Black Bear
    {ANIMAL_ORCA_BITMAP, ANIMAL_ORCA_W, ANIMAL_ORCA_H},                             // Orca
    {ANIMAL_COUGAR_BITMAP, ANIMAL_COUGAR_W, ANIMAL_COUGAR_H},                       // Cougar
    {ANIMAL_COYOTE_BITMAP, ANIMAL_COYOTE_W, ANIMAL_COYOTE_H},                       // Coyote
    {ANIMAL_DOUGLAS_SQUIRREL_BITMAP, ANIMAL_DOUGLAS_SQUIRREL_W,
     ANIMAL_DOUGLAS_SQUIRREL_H},  // Douglas Squirrel
    {ANIMAL_SNOWSHOE_HARE_BITMAP, ANIMAL_SNOWSHOE_HARE_W, ANIMAL_SNOWSHOE_HARE_H},  // Snowshoe Hare
    {ANIMAL_PILEATED_WOODPECKER_BITMAP, ANIMAL_PILEATED_WOODPECKER_W,
     ANIMAL_PILEATED_WOODPECKER_H},  // Pileated Woodpecker
    {ANIMAL_STELLERS_JAY_BITMAP, ANIMAL_STELLERS_JAY_W, ANIMAL_STELLERS_JAY_H},  // Steller's Jay
    {ANIMAL_BANANA_SLUG_BITMAP, ANIMAL_BANANA_SLUG_W, ANIMAL_BANANA_SLUG_H},     // Banana Slug
    {ANIMAL_PACIFIC_TREE_FROG_BITMAP, ANIMAL_PACIFIC_TREE_FROG_W,
     ANIMAL_PACIFIC_TREE_FROG_H},                              // Pacific Tree Frog
    {ANIMAL_BEAVER_BITMAP, ANIMAL_BEAVER_W, ANIMAL_BEAVER_H},  // Beaver
    {ANIMAL_GREAT_BLUE_HERON_BITMAP, ANIMAL_GREAT_BLUE_HERON_W,
     ANIMAL_GREAT_BLUE_HERON_H},                                        // Great Blue Heron
    {ANIMAL_OSPREY_BITMAP, ANIMAL_OSPREY_W, ANIMAL_OSPREY_H},           // Osprey
    {ANIMAL_CHICKADEE_BITMAP, ANIMAL_CHICKADEE_W, ANIMAL_CHICKADEE_H},  // Chestnut-backed Chickadee
    {ANIMAL_HARBOR_SEAL_BITMAP, ANIMAL_HARBOR_SEAL_W, ANIMAL_HARBOR_SEAL_H},  // Harbor Seal
    {ANIMAL_RED_FOX_BITMAP, ANIMAL_RED_FOX_W, ANIMAL_RED_FOX_H},              // Red Fox
    {ANIMAL_BARRED_OWL_BITMAP, ANIMAL_BARRED_OWL_W, ANIMAL_BARRED_OWL_H},     // Barred Owl
    {ANIMAL_GRAY_WOLF_BITMAP, ANIMAL_GRAY_WOLF_W, ANIMAL_GRAY_WOLF_H},        // Gray Wolf
    {ANIMAL_BOBCAT_BITMAP, ANIMAL_BOBCAT_W, ANIMAL_BOBCAT_H},                 // Bobcat
};
static const int kAnimalArtCount = sizeof(kAnimalArt) / sizeof(kAnimalArt[0]);

// A pending wildlife sighting takes over the Main view until ENTER
// resolves it (see events::checkForEvent / onEnter() in main.cpp).
static void renderEncounter(const AppContext& ctx, const events::PendingEvent& ev) {
  bool negative = events::eventIsNegative(ev);
  textCentered(0, SCREEN_W, 6, events::eventTitle(ev.type, negative), 2);

  const AnimalArt* art = nullptr;
  if (ev.type == events::EventType::AnimalSighting) {
    uint8_t idx = events::animalIndex(ev);
    if (idx < kAnimalArtCount && kAnimalArt[idx].bitmap != nullptr) art = &kAnimalArt[idx];
  }

  int y;
  if (art != nullptr) {
    // Real reference-photo art for this animal -- show it instead of the
    // marmot for this screen; heights vary per photo, so lay out everything
    // below it relative to its actual bottom edge rather than a fixed offset.
    int bx = (SCREEN_W - art->w) / 2, by = 26;
    epd.drawBitmap(bx, by, art->bitmap, art->w, art->h, C_BLACK, C_WHITE);
    y = by + art->h + 10;
  } else {
    int stageCx = SCREEN_W / 2, stageGroundY = 235;
    drawCreature(stageCx, stageGroundY, negative ? Mood::Annoyed : Mood::Excited);
    y = stageGroundY + 15;
  }

  textCentered(0, SCREEN_W, y, events::eventName(ev), 2);
  y += 24;
  y = textWrapped(16, y, 40, events::eventNote(ev), 1) + 8;
  if (negative) textCentered(0, SCREEN_W, y, "Stay alert!", 1);

  drawNavBar("Status", "Acknowledge", "Foraging");
}

static void renderMain(const AppContext& ctx) {
  events::PendingEvent ev;
  ev.type = (events::EventType)ctx.eventType;
  ev.dataId = ctx.eventDataId;
  if (ev.type != events::EventType::None) {
    renderEncounter(ctx, ev);
    return;
  }

  char buf[48];
  strftime(buf, sizeof(buf), "%a %b %d", &ctx.now);
  textAt(8, 6, buf, 2);
  drawWeatherGlyph(SCREEN_W - 40, 4, ctx.weather);

  // No separate ground/habitat drawing -- the marmot bitmap already has its
  // own rock ledge baked in, and a second ground line under it just clashed.
  int stageCx = STAGE_X + STAGE_W / 2 - 20;
  int stageGroundY = STAGE_Y + STAGE_H - 24;
  drawCreature(stageCx, stageGroundY, ctx.creature.mood);

  drawNavBar("Status", "", "Foraging");
}

static const char* const MONTH_ABBR[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

// Renders a species' active months as compact ranges (e.g. "Jun-Aug, Oct")
// instead of an unlabeled dot calendar.
static void seasonText(const Forageable& f, char* buf, size_t bufSize) {
  size_t written = 0;
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
    if (!first && written < bufSize) written += snprintf(buf + written, bufSize - written, ", ");
    if (written < bufSize) {
      if (start == end)
        written += snprintf(buf + written, bufSize - written, "%s", MONTH_ABBR[start - 1]);
      else
        written += snprintf(buf + written, bufSize - written, "%s-%s", MONTH_ABBR[start - 1],
                            MONTH_ABBR[end - 1]);
    }
    first = false;
  }
}

static void renderForaging(const AppContext& ctx, int speciesIdx) {
  const Forageable& f = foraging::speciesAtRank(speciesIdx);
  int month = ctx.now.tm_mon + 1;
  bool active = foraging::inSeason(f, month);

  events::PendingEvent ev;
  ev.type = (events::EventType)ctx.eventType;
  ev.dataId = ctx.eventDataId;
  bool isMatch =
      ev.type == events::EventType::ForagingFind && strcmp(f.kind, events::eventCategory(ev)) == 0;

  char posBuf[16];
  snprintf(posBuf, sizeof(posBuf), "%d/%d", speciesIdx + 1, foraging::speciesCount());
  int16_t pbx, pby;
  uint16_t pbw, pbh;
  epd.setFont(nullptr);
  epd.setTextSize(1);
  epd.getTextBounds(posBuf, 0, 6, &pbx, &pby, &pbw, &pbh);
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

  char kindLine[40];
  snprintf(kindLine, sizeof(kindLine), "%s - %s", f.kind, foraging::biomeName(f.biome));
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

  char seasonBuf[48];
  seasonText(f, seasonBuf, sizeof(seasonBuf));
  char seasonLine[64];
  snprintf(seasonLine, sizeof(seasonLine), "SEASON: %s", seasonBuf);
  textAt(8, y, seasonLine, 1);

  if (isMatch) textCentered(0, SCREEN_W, y + 16, "MATCHES ACTIVE FIND!", 1);

  drawNavBar("Main", isMatch ? "Feed!" : "Next", "+10");
}

// Energy: derived from time of day, not persisted -- low overnight, ramps
// up through the morning, peaks midday, tapers into the evening.
static uint8_t computeEnergy(const struct tm& now) {
  int hour = now.tm_hour;
  if (hour < 6 || hour >= 22) return 15;
  if (hour < 9) return 55;
  if (hour < 18) return 95;
  return 50;
}

// Curiosity: derived from current weather, not persisted -- fresh rain (good
// foraging conditions) makes for a curious creature; stale/offline data is
// neutral.
static uint8_t computeCuriosity(const WeatherData& w) {
  if (!w.valid) return 50;
  if (w.postRain) return 90;
  if (w.tempC >= 8.0f && w.tempC <= 22.0f) return 65;
  return 35;
}

static void renderStatus(const AppContext& ctx) {
  textAt(8, 6, "Status", 2);

  // No mood-name text or "Foraged X days ago" line anymore -- the mascot
  // moves down to fill the space those left behind.
  int stageCx = SCREEN_W / 2, stageGroundY = 230;
  drawCreature(stageCx, stageGroundY, ctx.creature.mood);

  auto bar = [&](int y, const char* label, uint8_t pct) {
    textAt(20, y, label, 1);
    int bx = 20, bw = SCREEN_W - 40, by = y + 12;
    dFillRect(bx, by, bw, 14, SHADE_LIGHT);
    epd.drawRect(bx, by, bw, 14, C_BLACK);
    epd.fillRect(bx + 2, by + 2, (bw - 4) * pct / 100, 10, C_BLACK);
  };
  bar(250, "Fullness", 100 - ctx.creature.hunger);
  bar(278, "Happiness", ctx.creature.happiness);
  bar(306, "Energy", computeEnergy(ctx.now));
  bar(334, "Curiosity", computeCuriosity(ctx.weather));

  if (ctx.weather.valid) {
    char buf[48];
    snprintf(buf, sizeof(buf), "%s, %.0fC", ctx.weather.condition, ctx.weather.tempC);
    textAt(20, 364, buf, 1);
  }

  drawNavBar("", "", "Main");
}

void begin() {
  // epd.begin() owns SPI setup internally (custom SCK/MOSI pins are
  // configured in epd_official/epdif.cpp's IfInit()).
  epd.begin();
  epd.setRotation(3);
  epd.setTextWrap(false);
}

void renderView(View v, const AppContext& ctx, int speciesIdx) {
  // Requesting partial refresh here, not full: EpdGFX::endFrame() forces a
  // full (flashy) refresh on the first frame after epd.begin() regardless
  // of what's requested (see epd_adapter.h), so every subsequent view
  // change in the same wake session gets the fast, flicker-free path.
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
      break;
  }
  epd.endFrame(true);
}

void renderSleep() {
  epd.beginFrame();
  // Always the base pose here (not the per-wake random variant drawCreature()
  // picks for the live views) so this screen reads consistently as "asleep"
  // rather than whatever alert photo pose happened to be rolled this wake.
  int cx = SCREEN_W / 2;
  int by = SCREEN_H / 2 + 40;
  int bx = cx - MARMOT_W / 2;
  int topY = by - MARMOT_GROUND_Y;
  epd.drawBitmap(bx, topY, MARMOT_BITMAP, MARMOT_W, MARMOT_H, C_BLACK, C_WHITE);

  int headX = cx + MARMOT_W / 4, headY = topY + 20;
  textAt(headX, headY, "Z", 3);
  textAt(headX + 22, headY - 24, "Z", 2);
  textAt(headX + 38, headY - 44, "Z", 1);

  epd.endFrame(true);
}

void hibernate() { epd.sleep(); }

}  // namespace display
