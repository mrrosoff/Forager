#include "display.h"

#include <SPI.h>
#include <math.h>

#include <algorithm>
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
#include "bitmaps/marmot/marmot_baby4_bitmap.h"
#include "bitmaps/marmot/marmot_baby6_bitmap.h"
#include "bitmaps/marmot/marmot_baby7_bitmap.h"
#include "bitmaps/marmot/marmot_baby_bitmap.h"
#include "bitmaps/marmot/marmot_bitmap.h"
#include "bitmaps/marmot/marmot_death_bitmap.h"
#include "bitmaps/marmot/marmot_juvenile2_bitmap.h"
#include "bitmaps/marmot/marmot_juvenile3_bitmap.h"
#include "bitmaps/marmot/marmot_variant10_bitmap.h"
#include "bitmaps/marmot/marmot_variant11_bitmap.h"
#include "bitmaps/marmot/marmot_variant13_bitmap.h"
#include "bitmaps/marmot/marmot_variant15_bitmap.h"
#include "bitmaps/marmot/marmot_variant16_bitmap.h"
#include "bitmaps/marmot/marmot_variant17_bitmap.h"
#include "bitmaps/marmot/marmot_variant18_bitmap.h"
#include "bitmaps/marmot/marmot_variant20_bitmap.h"
#include "bitmaps/marmot/marmot_variant21_bitmap.h"
#include "bitmaps/marmot/marmot_variant22_bitmap.h"
#include "bitmaps/marmot/marmot_variant24_bitmap.h"
#include "bitmaps/marmot/marmot_variant25_bitmap.h"
#include "bitmaps/marmot/marmot_variant26_bitmap.h"
#include "bitmaps/marmot/marmot_variant27_bitmap.h"
#include "bitmaps/marmot/marmot_variant28_bitmap.h"
#include "bitmaps/marmot/marmot_variant5_bitmap.h"
#include "bitmaps/marmot/marmot_variant6_bitmap.h"
#include "bitmaps/marmot/marmot_variant7_bitmap.h"
#include "bitmaps/marmot/marmot_variant8_bitmap.h"
#include "bitmaps/marmot/marmot_variant9_bitmap.h"
#include "bitmaps/marmot/marmot_baby2_small_bitmap.h"
#include "bitmaps/marmot/marmot_baby4_small_bitmap.h"
#include "bitmaps/marmot/marmot_baby6_small_bitmap.h"
#include "bitmaps/marmot/marmot_baby7_small_bitmap.h"
#include "bitmaps/marmot/marmot_baby_small_bitmap.h"
#include "bitmaps/marmot/marmot_juvenile2_small_bitmap.h"
#include "bitmaps/marmot/marmot_juvenile3_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant10_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant11_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant13_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant15_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant16_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant17_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant18_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant20_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant21_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant22_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant24_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant25_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant26_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant27_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant28_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant5_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant6_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant7_small_bitmap.h"
#include "bitmaps/marmot/marmot_variant8_small_bitmap.h"
#include "bitmaps/badges/berries_bitmap.h"
#include "bitmaps/badges/complete_bitmap.h"
#include "bitmaps/badges/halfway_bitmap.h"
#include "bitmaps/badges/herbalist_bitmap.h"
#include "bitmaps/badges/mushrooms_bitmap.h"
#include "bitmaps/badges/naturalist_bitmap.h"
#include "bitmaps/badges/storms_bitmap.h"
#include "bitmaps/badges/wanderer_bitmap.h"
#include "bitmaps/badges/wildlife_bitmap.h"
#include "bitmaps/marmot/marmot_variant9_small_bitmap.h"
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
static int drawCreature(int cx, int groundY, Mood mood, Stage stage, bool small = false) {
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
  return by;
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
  // ForagingFind isn't resolved by ENTER here (see main.cpp's onEnter()) --
  // its effect only happens on Foraging, so no preview for it.
  if (ev.type != events::EventType::ForagingFind) {
    textAt(16, y, events::eventEffectPreview(ev), 1);
  }

  drawNavBar("Status", "Acknowledge", "Foraging");
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
static void maybeDrawThoughtBubble(int headX, int headY, Stage stage) {
  static int8_t roll = -1;
  static int8_t thoughtIdx = -1;
  if (roll < 0) {
    roll = (int8_t)random(3);
    int count = stage == Stage::Baby       ? kThoughtsBabyCount
                : stage == Stage::Juvenile ? kThoughtsJuvenileCount
                                            : kThoughtsAdultCount;
    thoughtIdx = (int8_t)random(count);
  }
  if (roll == 0) return;

  const char* thought = stage == Stage::Baby       ? kThoughtsBaby[thoughtIdx]
                         : stage == Stage::Juvenile ? kThoughtsJuvenile[thoughtIdx]
                                                     : kThoughtsAdult[thoughtIdx];
  int16_t bx, by;
  uint16_t bw, bh;
  epd.setFont(nullptr);
  epd.setTextSize(1);
  epd.getTextBounds(thought, 0, 0, &bx, &by, &bw, &bh);

  int padX = 8, padY = 6;
  int cx = headX, cy = headY - 34 - bh / 2;
  int rw = (int)bw / 2 + padX, rh = (int)bh / 2 + padY;
  epd.fillRoundRect(cx - rw, cy - rh, rw * 2, rh * 2, 8, C_WHITE);
  epd.drawRoundRect(cx - rw, cy - rh, rw * 2, rh * 2, 8, C_BLACK);
  textAt(cx - (int)bw / 2, cy - (int)bh / 2, thought, 1);

  epd.fillCircle(headX - 10, headY - 10, 4, C_WHITE);
  epd.drawCircle(headX - 10, headY - 10, 4, C_BLACK);
  epd.fillCircle(headX - 4, headY - 20, 3, C_WHITE);
  epd.drawCircle(headX - 4, headY - 20, 3, C_BLACK);
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
  drawWeatherGlyph(SCREEN_W - 40, 8, ctx.weather);

  char buf[24];
  strftime(buf, sizeof(buf), "%a %b %d", &ctx.now);
  textAt(8, 30, buf, 1);

  // No separate ground/habitat drawing -- the marmot bitmap already has its
  // own rock ledge baked in, and a second ground line under it just clashed.
  int stageCx = STAGE_X + STAGE_W / 2 - 20;
  int stageGroundY = STAGE_Y + STAGE_H - 24;
  int topY = drawCreature(stageCx, stageGroundY, ctx.creature.mood, (Stage)ctx.stage);
  // Anchored to the actual pose's top edge (with a floor so it never rises
  // above where the name/date text sits) rather than a fixed offset -- a
  // tall pose keeps the bubble up near the top of the stage area like
  // before, but a shorter pose (more headroom above it) pulls the bubble
  // down closer to the creature instead of leaving a big empty gap.
  int bubbleY = std::max(topY + 30, STAGE_Y + 70);
  maybeDrawThoughtBubble(STAGE_X + STAGE_W - 55, bubbleY, (Stage)ctx.stage);

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

  char posBuf[16];
  bool eaten = journal::hasEaten(foraging::indexAtRank(speciesIdx));
  snprintf(posBuf, sizeof(posBuf), "%d/%d%s", speciesIdx + 1, foraging::browsableCount(),
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
  textAt(8, 10, titleBuf, 2);

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
  bar(304, "Curiosity", computeCuriosity(ctx.weather));

  if (ctx.weather.valid) {
    char buf[48];
    snprintf(buf, sizeof(buf), "%s, %.0fC", ctx.weather.condition, ctx.weather.tempC);
    textAt(20, 344, buf, 1);
  }

  char progressBuf[48];
  snprintf(progressBuf, sizeof(progressBuf), "Streak: %ud | Eaten: %d/%d",
           ctx.creature.feedStreakDays, journal::totalEaten(), foraging::speciesCount());
  textAt(20, 364, progressBuf, 1);

  bool achievementsReachable = (Stage)ctx.stage == Stage::Adult;
  drawNavBar(achievementsReachable ? "Achievements" : "", "", "Main");
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
      epd.fillTriangle(cx, cy - r * 2 / 3, cx - r * 2 / 3, cy + r / 2, cx + r * 2 / 3,
                        cy + r / 2, C_BLACK);
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
      epd.drawBitmap(cx - b.bitmapW / 2, cy - b.bitmapH / 2, b.bitmap, b.bitmapW, b.bitmapH, C_BLACK);
    } else {
      drawBadgeIcon(b.icon, cx, cy, r);
    }
  } else {
    textCentered(cx - r, r * 2, cy - 4, "?", 1);
  }
  textCentered(cx - 45, 90, cy + r + 4, b.name, 1);
  char countBuf[12];
  int shown = b.count < b.threshold ? b.count : b.threshold;
  snprintf(countBuf, sizeof(countBuf), "%d/%d", shown, b.threshold);
  textCentered(cx - 45, 90, cy + r + 16, countBuf, 1);
}

static void renderAchievements(const AppContext& ctx) {
  textAt(8, 10, "Achievements", 2);

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

  int total = journal::totalEaten();
  char buf[48];
  snprintf(buf, sizeof(buf), "Species: %d/%d   Streak: %ud", total, foraging::speciesCount(),
           ctx.creature.feedStreakDays);
  textCentered(0, SCREEN_W, 46, buf, 1);

  int herbalist = foraging::countEatenOfKind("green") + foraging::countEatenOfKind("herb") +
                  foraging::countEatenOfKind("flower") + foraging::countEatenOfKind("fern") +
                  foraging::countEatenOfKind("root") + foraging::countEatenOfKind("shoot");

  // 3 foraging (food types), 2 animals, 2 other events, 2 completionist.
  const BadgeDef badges[9] = {
      {"Mushrooms", BadgeIcon::Mushroom, foraging::countEatenOfKind("mushroom"), 15, BADGE_MUSHROOMS_BITMAP,
       BADGE_MUSHROOMS_W, BADGE_MUSHROOMS_H},
      {"Berries", BadgeIcon::Berries, foraging::countEatenOfKind("berry"), 10, BADGE_BERRIES_BITMAP,
       BADGE_BERRIES_W, BADGE_BERRIES_H},
      {"Herbalist", BadgeIcon::Leaf, herbalist, 15, BADGE_HERBALIST_BITMAP, BADGE_HERBALIST_W,
       BADGE_HERBALIST_H},
      {"Wildlife", BadgeIcon::Paw, journal::totalAnimalSightings(), 10, BADGE_WILDLIFE_BITMAP,
       BADGE_WILDLIFE_W, BADGE_WILDLIFE_H},
      {"Naturalist", BadgeIcon::Binoculars, journal::totalAnimalSightings(), 25, BADGE_NATURALIST_BITMAP,
       BADGE_NATURALIST_W, BADGE_NATURALIST_H},
      {"Storms", BadgeIcon::Cloud, journal::totalWeatherEvents(), 10, BADGE_STORMS_BITMAP, BADGE_STORMS_W,
       BADGE_STORMS_H},
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

  drawNavBar("", "", "Status");
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
    case View::Achievements:
      renderAchievements(ctx);
      break;
    default:
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
  char headline[40];
  snprintf(headline, sizeof(headline), "%s grew up!", name);
  textCentered(0, SCREEN_W, 20, headline, 1);

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
  textCentered(0, SCREEN_W, 40, "Your marmot has died.", 2);
  int bx = (SCREEN_W - MARMOT_DEATH_W) / 2, by = 76;
  epd.drawBitmap(bx, by, MARMOT_DEATH_BITMAP, MARMOT_DEATH_W, MARMOT_DEATH_H, C_BLACK, C_WHITE);
  int textY = by + MARMOT_DEATH_H + 20;
  textCentered(0, SCREEN_W, textY, deathReasonLine(cause), 1);
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
  textAt(8, 10, "Settings", 2);

  if (confirmPending) {
    if (selected == 1) {
      textCentered(0, SCREEN_W, 160, "Really reset the game?", 1);
      textCentered(0, SCREEN_W, 180, "This erases all progress.", 1);
    } else {
      textCentered(0, SCREEN_W, 160, "Power off?", 1);
      textCentered(0, SCREEN_W, 180, "Only the switch wakes it.", 1);
    }
    drawNavBar("", "", "");
    textCentered(0, SCREEN_W, SCREEN_H - 34, "ENTER = Yes, BACK = No", 1);
  } else {
    const char* options[] = {"WiFi Networks", "Reset Game", "Power Off"};
    for (int i = 0; i < 3; i++) {
      char line[24];
      snprintf(line, sizeof(line), "%s %s", i == selected ? ">" : " ", options[i]);
      textAt(20, 90 + i * 30, line, 1);
    }
    drawNavBar("Prev", "Select", "Next");
    textCentered(0, SCREEN_W, SCREEN_H - 34, "BACK = previous menu", 1);
  }

  epd.endFrame(true);
}

// Short label for a grid cell -- printable characters show as themselves,
// the control/toggle sentinels get a word so an empty-looking or sentinel
// cell isn't mistaken for a rendering glitch.
static void keyLabel(char c, char* out, size_t outSize) {
  if (c == textentry::BACKSPACE) {
    snprintf(out, outSize, "DEL");
  } else if (c == textentry::DONE) {
    snprintf(out, outSize, "OK");
  } else if (c == textentry::SHIFT) {
    snprintf(out, outSize, "SHIFT");
  } else if (c == textentry::SYMBOLS) {
    snprintf(out, outSize, "SYM");
  } else if (c == ' ') {
    snprintf(out, outSize, "SPACE");
  } else {
    snprintf(out, outSize, "%c", c);
  }
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
      char label[8];
      keyLabel(textentry::charsetAt(s, idx + col), label, sizeof(label));
      int16_t bx, by;
      uint16_t bw, bh;
      epd.getTextBounds(label, 0, 0, &bx, &by, &bw, &bh);
      widths[col] = std::max((int)bw + keyPad, minKeyW);
      rowW += widths[col];
    }
    int cx = (SCREEN_W - rowW) / 2;
    int cy = gridY + row * cellH;
    for (int col = 0; col < rowLen; col++, idx++) {
      char label[8];
      char c = textentry::charsetAt(s, idx);
      keyLabel(c, label, sizeof(label));
      int w = widths[col];
      int16_t bx, by;
      uint16_t bw, bh;
      epd.getTextBounds(label, 0, 0, &bx, &by, &bw, &bh);
      bool toggledOn = (c == textentry::SHIFT && s.caps) || (c == textentry::SYMBOLS && s.symbols);
      if (idx == s.pickerIndex) {
        epd.fillRect(cx + 1, cy + 1, w - 2, cellH - 2, C_BLACK);
        epd.setTextColor(C_WHITE);
        epd.setCursor(cx + (w - (int)bw) / 2, cy + (cellH - (int)bh) / 2);
        epd.print(label);
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
    char msg[48];
    snprintf(msg, sizeof(msg), "Remove '%s'?", wifistore::at(selected).ssid);
    textCentered(0, SCREEN_W, 160, msg, 1);
    drawNavBar("", "", "");
    textCentered(0, SCREEN_W, SCREEN_H - 34, "ENTER = Yes, BACK = No", 1);
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

  drawNavBar("Prev", "Select", "Next");
  textCentered(0, SCREEN_W, SCREEN_H - 34, "BACK = previous menu", 1);
  epd.endFrame(true);
}

/**
 * Brief goodbye screen shown right before a Power Off deep-sleep with no
 * wake source armed -- only the physical power switch brings the device
 * back after this.
 */
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
  const MarmotArt* art = stage == Stage::Baby     ? &kBabyVariants[0]
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
  int headX = std::min(bx + art->w - 40, SCREEN_W - 50);
  int headY = stage == Stage::Adult ? std::max(by - 100, 44) : std::max(topY + 20, 44);
  textAt(headX, headY, "Z", 3);
  textAt(headX + 22, headY - 24, "Z", 2);
  textAt(headX + 38, headY - 44, "Z", 1);

  epd.endFrame(true);
}


void hibernate() { epd.sleep(); }

}  // namespace display
