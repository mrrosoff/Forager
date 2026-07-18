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
#include "bitmaps/marmot/marmot_baby2_bitmap.h"
#include "bitmaps/marmot/marmot_baby3_bitmap.h"
#include "bitmaps/marmot/marmot_baby_bitmap.h"
#include "bitmaps/marmot/marmot_bitmap.h"
#include "bitmaps/marmot/marmot_juvenile2_bitmap.h"
#include "bitmaps/marmot/marmot_juvenile_bitmap.h"
#include "bitmaps/marmot/marmot_variant10_bitmap.h"
#include "bitmaps/marmot/marmot_variant11_bitmap.h"
#include "bitmaps/marmot/marmot_variant12_bitmap.h"
#include "bitmaps/marmot/marmot_variant1_bitmap.h"
#include "bitmaps/marmot/marmot_variant2_bitmap.h"
#include "bitmaps/marmot/marmot_variant3_bitmap.h"
#include "bitmaps/marmot/marmot_variant4_bitmap.h"
#include "bitmaps/marmot/marmot_variant5_bitmap.h"
#include "bitmaps/marmot/marmot_variant6_bitmap.h"
#include "bitmaps/marmot/marmot_variant7_bitmap.h"
#include "bitmaps/marmot/marmot_variant8_bitmap.h"
#include "bitmaps/marmot/marmot_variant9_bitmap.h"
#include "bitmaps/species/species_index.h"
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
    {MARMOT_VARIANT6_BITMAP, MARMOT_VARIANT6_W, MARMOT_VARIANT6_H, MARMOT_VARIANT6_H},
    {MARMOT_VARIANT7_BITMAP, MARMOT_VARIANT7_W, MARMOT_VARIANT7_H, MARMOT_VARIANT7_H},
    {MARMOT_VARIANT8_BITMAP, MARMOT_VARIANT8_W, MARMOT_VARIANT8_H, MARMOT_VARIANT8_H},
    {MARMOT_VARIANT9_BITMAP, MARMOT_VARIANT9_W, MARMOT_VARIANT9_H, MARMOT_VARIANT9_H},
    {MARMOT_VARIANT10_BITMAP, MARMOT_VARIANT10_W, MARMOT_VARIANT10_H, MARMOT_VARIANT10_H},
    {MARMOT_VARIANT11_BITMAP, MARMOT_VARIANT11_W, MARMOT_VARIANT11_H, MARMOT_VARIANT11_H},
    {MARMOT_VARIANT12_BITMAP, MARMOT_VARIANT12_W, MARMOT_VARIANT12_H, MARMOT_VARIANT12_H},
};
static const int kMarmotVariantCount = sizeof(kMarmotVariants) / sizeof(kMarmotVariants[0]);

// Dedicated Baby and Juvenile pose pools -- real hoary/yellow-bellied marmot
// pup photos found on a wider Commons sweep (zoo/park and family shots turned
// out to have much cleaner backgrounds than the wild adult shots this
// project mostly draws from), each rotated randomly per wake exactly like
// the adult pool below.
static const MarmotArt kBabyVariants[] = {
    {MARMOT_BABY_BITMAP, MARMOT_BABY_W, MARMOT_BABY_H, MARMOT_BABY_H},
    {MARMOT_BABY2_BITMAP, MARMOT_BABY2_W, MARMOT_BABY2_H, MARMOT_BABY2_H},
    {MARMOT_BABY3_BITMAP, MARMOT_BABY3_W, MARMOT_BABY3_H, MARMOT_BABY3_H},
};
static const int kBabyVariantCount = sizeof(kBabyVariants) / sizeof(kBabyVariants[0]);

static const MarmotArt kJuvenileVariants[] = {
    {MARMOT_JUVENILE_BITMAP, MARMOT_JUVENILE_W, MARMOT_JUVENILE_H, MARMOT_JUVENILE_H},
    {MARMOT_JUVENILE2_BITMAP, MARMOT_JUVENILE2_W, MARMOT_JUVENILE2_H, MARMOT_JUVENILE2_H},
};
static const int kJuvenileVariantCount = sizeof(kJuvenileVariants) / sizeof(kJuvenileVariants[0]);

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

static void drawCreature(int cx, int groundY, Mood mood, Stage stage) {
  if (stage == Stage::Baby || stage == Stage::Juvenile) {
    static int8_t babyVariant = -1;
    static int8_t juvenileVariant = -1;
    const MarmotArt& art =
        (stage == Stage::Baby)
            ? kBabyVariants[pickVariant(babyVariant, kBabyVariantCount)]
            : kJuvenileVariants[pickVariant(juvenileVariant, kJuvenileVariantCount)];
    int bx = cx - art.w / 2;
    int by = groundY - art.groundY;
    epd.drawBitmap(bx, by, art.bitmap, art.w, art.h, C_BLACK, C_WHITE);
    return;
  }

  // No dedicated hungry-mood art -- reuses the same adult pose pool as every
  // other non-baby/juvenile mood. Hunger is communicated through the Status
  // view's meter, not a distinct sprite.
  static int8_t variant = -1;
  const MarmotArt& art = kMarmotVariants[pickVariant(variant, kMarmotVariantCount)];
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
    drawCreature(stageCx, stageGroundY, negative ? Mood::Annoyed : Mood::Excited, (Stage)ctx.stage);
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

  textAt(8, 6, ctx.creature.name, 2);
  drawWeatherGlyph(SCREEN_W - 40, 4, ctx.weather);

  char buf[24];
  strftime(buf, sizeof(buf), "%a %b %d", &ctx.now);
  textAt(8, 30, buf, 1);

  // No separate ground/habitat drawing -- the marmot bitmap already has its
  // own rock ledge baked in, and a second ground line under it just clashed.
  int stageCx = STAGE_X + STAGE_W / 2 - 20;
  int stageGroundY = STAGE_Y + STAGE_H - 24;
  drawCreature(stageCx, stageGroundY, ctx.creature.mood, (Stage)ctx.stage);

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
  ev.exact = ctx.eventExact != 0;
  bool isMatch = events::eventMatchesSpecies(ev, f);

  char posBuf[16];
  bool eaten = journal::hasEaten(foraging::indexAtRank(speciesIdx));
  snprintf(posBuf, sizeof(posBuf), "%d/%d%s", speciesIdx + 1, foraging::speciesCount(),
           eaten ? " (eaten)" : "");
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

  drawNavBar("Main", "Eat", "Scroll");
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
  char titleBuf[40];
  snprintf(titleBuf, sizeof(titleBuf), "%s - %s", ctx.creature.name, stageName((Stage)ctx.stage));
  textAt(8, 6, titleBuf, 2);

  // No mood-name text or "Foraged X days ago" line anymore -- the mascot
  // moves down to fill the space those left behind.
  int stageCx = SCREEN_W / 2, stageGroundY = 230;
  drawCreature(stageCx, stageGroundY, ctx.creature.mood, (Stage)ctx.stage);

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

  char progressBuf[48];
  snprintf(progressBuf, sizeof(progressBuf), "Streak: %ud | Eaten: %d/%d",
           ctx.creature.feedStreakDays, journal::totalEaten(), foraging::speciesCount());
  textAt(20, 376, progressBuf, 1);

  drawNavBar("Achievements", "", "Main");
}

// Badge progress line: "[X] Name (n/threshold)" -- a filled box once count
// reaches threshold. No per-badge icons; matches the existing blocky-font,
// text-only aesthetic used everywhere else.
static int drawBadge(int y, const char* name, int count, int threshold) {
  bool done = count >= threshold;
  int shown = count < threshold ? count : threshold;
  char line[48];
  snprintf(line, sizeof(line), "[%c] %s (%d/%d)", done ? 'X' : ' ', name, shown, threshold);
  textAt(8, y, line, 1);
  return y + 16;
}

static void renderAchievements(const AppContext& ctx) {
  textAt(8, 6, "Achievements", 2);

  // Locked until full-grown -- badges/streak tracking still runs underneath
  // the whole time (see journal.cpp/creature.cpp), this just gates the
  // reveal so there's something to look forward to at Adult.
  if ((Stage)ctx.stage != Stage::Adult) {
    textCentered(0, SCREEN_W, 160, "Locked", 2);
    textCentered(0, SCREEN_W, 190, "Come back once your marmot", 1);
    textCentered(0, SCREEN_W, 204, "is fully grown.", 1);
    drawNavBar("", "", "Status");
    return;
  }

  int y = 40;

  int total = journal::totalEaten();
  char buf[48];
  snprintf(buf, sizeof(buf), "Species eaten: %d/%d", total, foraging::speciesCount());
  textAt(8, y, buf, 1);
  y += 14;
  snprintf(buf, sizeof(buf), "Feed streak: %u days", ctx.creature.feedStreakDays);
  textAt(8, y, buf, 1);
  y += 22;

  y = drawBadge(y, "Mushroom Hunter", foraging::countEatenOfKind("mushroom"), 15);
  y = drawBadge(y, "Berry Picker", foraging::countEatenOfKind("berry"), 10);

  int herbalist = foraging::countEatenOfKind("green") + foraging::countEatenOfKind("herb") +
                  foraging::countEatenOfKind("flower") + foraging::countEatenOfKind("fern") +
                  foraging::countEatenOfKind("root") + foraging::countEatenOfKind("shoot");
  y = drawBadge(y, "Herbalist", herbalist, 15);

  y = drawBadge(y, "Coastal Forager", foraging::countEatenOfBiome(Biome::Coast), 15);
  y = drawBadge(y, "Mountain Forager", foraging::countEatenOfBiome(Biome::Mountain), 10);

  int tidepool = foraging::countEatenOfKind("shellfish") + foraging::countEatenOfKind("crab") +
                 foraging::countEatenOfKind("urchin") + foraging::countEatenOfKind("snail") +
                 foraging::countEatenOfKind("chiton") + foraging::countEatenOfKind("shrimp");
  y = drawBadge(y, "Tidepool Forager", tidepool, 8);

  y = drawBadge(y, "Completionist", total, foraging::speciesCount());

  drawNavBar("", "", "Status");
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
    case View::Achievements:
      renderAchievements(ctx);
      break;
    default:
      break;
  }
  epd.endFrame(true);
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
  epd.endFrame(true);
}

void renderTransition(Stage newStage) {
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
  const MarmotArt& art = (newStage == Stage::Juvenile) ? kJuvenileVariants[0] : kMarmotVariants[0];
  int cx = SCREEN_W / 2;
  int by = SCREEN_H / 2 + 60;
  int bx = cx - art.w / 2;
  int topY = by - art.groundY;
  epd.drawBitmap(bx, topY, art.bitmap, art.w, art.h, C_BLACK, C_WHITE);
  const char* label = (newStage == Stage::Juvenile) ? "Now a Juvenile!" : "Now an Adult!";
  textCentered(0, SCREEN_W, topY - 42, "Your marmot grew up!", 1);
  textCentered(0, SCREEN_W, topY - 26, label, 2);
  textCentered(0, SCREEN_W, SCREEN_H - 30, "Press ENTER", 1);
  epd.endFrame(true);
}

void renderRanAway() {
  epd.beginFrame();
  const MarmotArt& art = kMarmotVariants[0];
  int cx = SCREEN_W / 2;
  int by = SCREEN_H / 2 + 40;
  int bx = cx - art.w / 2;
  int topY = by - art.groundY;
  epd.drawBitmap(bx, topY, art.bitmap, art.w, art.h, C_BLACK, C_WHITE);
  textCentered(0, SCREEN_W, topY - 56, "Your marmot wandered off...", 1);
  textCentered(0, SCREEN_W, topY - 40, "Neglected for too long.", 1);
  textCentered(0, SCREEN_W, SCREEN_H - 44, "Press ENTER to start over", 1);
  textCentered(0, SCREEN_W, SCREEN_H - 30, "with a new baby marmot.", 1);
  epd.endFrame(true);
}

/**
 * Settings overlay -- triggered by the dedicated SETTINGS button (see
 * main.cpp), not part of the normal View cycle. selected: 0 = Power Off,
 * 1 = Reset Game. confirmPending shows a yes/no sub-screen for Reset Game,
 * since it's destructive.
 */
void renderSettings(int selected, bool confirmPending) {
  epd.beginFrame();
  textAt(8, 6, "Settings", 2);

  if (confirmPending) {
    textCentered(0, SCREEN_W, 160, "Really reset the game?", 1);
    textCentered(0, SCREEN_W, 180, "This erases all progress.", 1);
    drawNavBar("No", "", "");
    textCentered(0, SCREEN_W, SCREEN_H - 34, "ENTER = Yes", 1);
  } else {
    const char* options[] = {"Power Off", "Reset Game", "WiFi Networks"};
    for (int i = 0; i < 3; i++) {
      char line[24];
      snprintf(line, sizeof(line), "%s %s", i == selected ? ">" : " ", options[i]);
      textAt(20, 60 + i * 20, line, 1);
    }
    drawNavBar("Exit", "Select", "Next");
  }

  epd.endFrame(true);
}

void renderTextEntry(const char* prompt, const char* buffer, char currentPick) {
  epd.beginFrame();
  textAt(8, 6, prompt, 1);
  textAt(8, 40, buffer, 2);

  char label[8];
  if (currentPick == textentry::BACKSPACE) {
    snprintf(label, sizeof(label), "DEL");
  } else if (currentPick == textentry::DONE) {
    snprintf(label, sizeof(label), "OK");
  } else if (currentPick == ' ') {
    snprintf(label, sizeof(label), "_");
  } else {
    snprintf(label, sizeof(label), "%c", currentPick);
  }
  textCentered(0, SCREEN_W, SCREEN_H / 2, label, 4);

  drawNavBar("Prev", "Pick", "Next");
  epd.endFrame(true);
}

void renderWifiMenu(int selected, bool confirmRemove) {
  epd.beginFrame();
  textAt(8, 6, "WiFi Networks", 2);

  int count = wifistore::count();
  if (confirmRemove && selected >= 0 && selected < count) {
    char msg[48];
    snprintf(msg, sizeof(msg), "Remove '%s'?", wifistore::at(selected).ssid);
    textCentered(0, SCREEN_W, 160, msg, 1);
    drawNavBar("No", "", "");
    textCentered(0, SCREEN_W, SCREEN_H - 34, "ENTER = Yes", 1);
    epd.endFrame(true);
    return;
  }

  int y = 40;
  if (count == 0) {
    textAt(20, y, "(no saved networks)", 1);
    y += 20;
  } else {
    for (int i = 0; i < count; i++) {
      char line[40];
      snprintf(line, sizeof(line), "%s %s", i == selected ? ">" : " ", wifistore::at(i).ssid);
      textAt(20, y, line, 1);
      y += 18;
    }
  }
  char addLine[20];
  snprintf(addLine, sizeof(addLine), "%s Add Network", selected == count ? ">" : " ");
  textAt(20, y, addLine, 1);

  drawNavBar("Exit", "Select", "Next");
  epd.endFrame(true);
}

/**
 * Brief goodbye screen shown right before a Power Off deep-sleep with no
 * wake source armed -- only the physical power switch brings the device
 * back after this.
 */
void renderPowerOff() {
  epd.beginFrame();
  textCentered(0, SCREEN_W, SCREEN_H / 2 - 10, "Powered off", 2);
  textCentered(0, SCREEN_W, SCREEN_H / 2 + 14, "Flip the switch to wake", 1);
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

// --- TEMPORARY review helpers (see DEV_MODE_SCREEN_CYCLE in config.h) -----

// Encounter/Main/Status/Foraging/Achievements variety shown by the misc
// section below, past the bitmap galleries. Not meant to be exhaustive over
// every event content entry -- just one example per event type/outcome so
// every distinct screen layout gets reviewed once.
static const int kDebugMiscCount = 18;

static void renderDebugMisc(int i) {
  AppContext dummy{};
  time_t now = 1700000000;  // arbitrary fixed epoch, deterministic for review
  localtime_r(&now, &dummy.now);
  dummy.weather.valid = true;
  dummy.weather.tempC = 15.0f;
  dummy.weather.postRain = true;
  snprintf(dummy.weather.condition, sizeof(dummy.weather.condition), "Partly Cloudy");
  dummy.creature.mood = Mood::Content;
  dummy.creature.hunger = 30;
  dummy.creature.happiness = 70;
  dummy.creature.feedStreakDays = 5;
  strncpy(dummy.creature.name, "Marmot", sizeof(dummy.creature.name) - 1);
  dummy.featured = foraging::featured(6);

  auto ev = [](events::EventType t, uint8_t dataId, bool exact = false) {
    events::PendingEvent e;
    e.type = t;
    e.dataId = dataId;
    e.exact = exact;
    return e;
  };

  switch (i) {
    case 0:
      dummy.stage = (uint8_t)Stage::Baby;
      epd.beginFrame();
      renderMain(dummy);
      epd.endFrame(true);
      return;
    case 1:
      dummy.stage = (uint8_t)Stage::Juvenile;
      epd.beginFrame();
      renderMain(dummy);
      epd.endFrame(true);
      return;
    case 2:
      dummy.stage = (uint8_t)Stage::Adult;
      epd.beginFrame();
      renderMain(dummy);
      epd.endFrame(true);
      return;
    case 3:
      dummy.stage = (uint8_t)Stage::Adult;
      epd.beginFrame();
      renderStatus(dummy);
      epd.endFrame(true);
      return;
    case 4:
      dummy.stage = (uint8_t)Stage::Juvenile;
      epd.beginFrame();
      renderForaging(dummy, 0);
      epd.endFrame(true);
      return;
    case 5:
      dummy.stage = (uint8_t)Stage::Baby;
      epd.beginFrame();
      renderAchievements(dummy);  // locked
      epd.endFrame(true);
      return;
    case 6:
      dummy.stage = (uint8_t)Stage::Adult;
      epd.beginFrame();
      renderAchievements(dummy);  // unlocked
      epd.endFrame(true);
      return;
    case 7:
      epd.beginFrame();
      renderEncounter(dummy, ev(events::EventType::AnimalSighting, 0));
      epd.endFrame(true);
      return;
    case 8:
      epd.beginFrame();
      renderEncounter(dummy, ev(events::EventType::AnimalSighting, 6));  // Black Bear: predator
      epd.endFrame(true);
      return;
    case 9:
      epd.beginFrame();
      renderEncounter(dummy, ev(events::EventType::ForagingFind, 0, false));
      epd.endFrame(true);
      return;
    case 10:
      epd.beginFrame();
      renderEncounter(dummy, ev(events::EventType::ForagingFind, 0, true));
      epd.endFrame(true);
      return;
    case 11:
      epd.beginFrame();
      renderEncounter(dummy, ev(events::EventType::TrailMishap, 0));
      epd.endFrame(true);
      return;
    case 12:
      epd.beginFrame();
      renderEncounter(dummy, ev(events::EventType::WeatherEvent, 0));  // Rainbow: positive
      epd.endFrame(true);
      return;
    case 13:
      epd.beginFrame();
      renderEncounter(dummy, ev(events::EventType::WeatherEvent, 3));  // Sudden Downpour: negative
      epd.endFrame(true);
      return;
    case 14:
      epd.beginFrame();
      renderEncounter(dummy, ev(events::EventType::BabyCare, 0));
      epd.endFrame(true);
      return;
    case 15:
      epd.beginFrame();
      renderEncounter(dummy, ev(events::EventType::TrailTreasure, 0));
      epd.endFrame(true);
      return;
    case 16:
      epd.beginFrame();
      renderEncounter(dummy, ev(events::EventType::MarmotEncounter, 0));  // friendly
      epd.endFrame(true);
      return;
    case 17:
      epd.beginFrame();
      renderEncounter(dummy, ev(events::EventType::MarmotEncounter, 10));  // rival
      epd.endFrame(true);
      return;
    default:
      return;
  }
}

int debugScreenCount() {
  return kMarmotVariantCount + kBabyVariantCount + kJuvenileVariantCount + kAnimalArtCount +
         species_bitmaps::kSpeciesBitmapCount + kDebugMiscCount +
         9 /* birth, transitions, ran away, power off, sleep, settings x3 */;
}

void renderDebugScreen(int index) {
  int i = index;

  if (i < kMarmotVariantCount) {
    const MarmotArt& art = kMarmotVariants[i];
    epd.beginFrame();
    int cx = SCREEN_W / 2, by = SCREEN_H / 2 + 40;
    epd.drawBitmap(cx - art.w / 2, by - art.groundY, art.bitmap, art.w, art.h, C_BLACK, C_WHITE);
    char buf[32];
    snprintf(buf, sizeof(buf), "Adult pose %d/%d", i + 1, kMarmotVariantCount);
    textCentered(0, SCREEN_W, 16, buf, 1);
    epd.endFrame(true);
    return;
  }
  i -= kMarmotVariantCount;

  if (i < kBabyVariantCount) {
    const MarmotArt& art = kBabyVariants[i];
    epd.beginFrame();
    int cx = SCREEN_W / 2, by = SCREEN_H / 2 + 40;
    epd.drawBitmap(cx - art.w / 2, by - art.groundY, art.bitmap, art.w, art.h, C_BLACK, C_WHITE);
    char buf[32];
    snprintf(buf, sizeof(buf), "Baby pose %d/%d", i + 1, kBabyVariantCount);
    textCentered(0, SCREEN_W, 16, buf, 1);
    epd.endFrame(true);
    return;
  }
  i -= kBabyVariantCount;

  if (i < kJuvenileVariantCount) {
    const MarmotArt& art = kJuvenileVariants[i];
    epd.beginFrame();
    int cx = SCREEN_W / 2, by = SCREEN_H / 2 + 40;
    epd.drawBitmap(cx - art.w / 2, by - art.groundY, art.bitmap, art.w, art.h, C_BLACK, C_WHITE);
    char buf[32];
    snprintf(buf, sizeof(buf), "Juvenile pose %d/%d", i + 1, kJuvenileVariantCount);
    textCentered(0, SCREEN_W, 16, buf, 1);
    epd.endFrame(true);
    return;
  }
  i -= kJuvenileVariantCount;

  if (i < kAnimalArtCount) {
    events::PendingEvent ev;
    ev.type = events::EventType::AnimalSighting;
    ev.dataId = (uint8_t)i;
    const AnimalArt& art = kAnimalArt[i];
    epd.beginFrame();
    char buf[48];
    snprintf(buf, sizeof(buf), "Animal %d/%d: %s", i + 1, kAnimalArtCount, events::eventName(ev));
    textCentered(0, SCREEN_W, 16, buf, 1);
    if (art.bitmap) {
      epd.drawBitmap((SCREEN_W - art.w) / 2, 60, art.bitmap, art.w, art.h, C_BLACK, C_WHITE);
    } else {
      textCentered(0, SCREEN_W, 160, "(no sourced photo)", 1);
    }
    epd.endFrame(true);
    return;
  }
  i -= kAnimalArtCount;

  if (i < species_bitmaps::kSpeciesBitmapCount) {
    // Deterministic shuffle (not a real per-boot random -- just a fixed
    // coprime-multiplier permutation) so this review cycle doesn't walk the
    // list in the same alphabetical order every time. 131 is coprime with
    // every kSpeciesBitmapCount this project is realistically going to have
    // (no shared factor of 2 or 5), so every index gets hit exactly once.
    int shuffled = (i * 131) % species_bitmaps::kSpeciesBitmapCount;
    const species_bitmaps::SpeciesBitmap& sb = species_bitmaps::kSpeciesBitmaps[shuffled];
    epd.beginFrame();
    char buf[48];
    snprintf(buf, sizeof(buf), "Species %d/%d", i + 1, species_bitmaps::kSpeciesBitmapCount);
    textCentered(0, SCREEN_W, 16, buf, 1);
    textCentered(0, SCREEN_W, 36, sb.name, 1);
    epd.drawBitmap((SCREEN_W - sb.w) / 2, 66, sb.bitmap, sb.w, sb.h, C_BLACK, C_WHITE);
    epd.endFrame(true);
    return;
  }
  i -= species_bitmaps::kSpeciesBitmapCount;

  if (i < kDebugMiscCount) {
    renderDebugMisc(i);
    return;
  }
  i -= kDebugMiscCount;

  // These all manage their own beginFrame()/endFrame() already.
  switch (i) {
    case 0:
      renderBirth();
      return;
    case 1:
      renderTransition(Stage::Juvenile);
      return;
    case 2:
      renderTransition(Stage::Adult);
      return;
    case 3:
      renderRanAway();
      return;
    case 4:
      renderPowerOff();
      return;
    case 5:
      renderSleep();
      return;
    case 6:
      renderSettings(0, false);
      return;
    case 7:
      renderSettings(1, false);
      return;
    case 8:
      renderSettings(1, true);
      return;
    default:
      return;
  }
}

void hibernate() { epd.sleep(); }

}  // namespace display
