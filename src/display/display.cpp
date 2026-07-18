#include "display.h"

#include <SPI.h>
#include <math.h>

#include <cstring>

#include "config.h"
#include "creature.h"
#include "epd_adapter.h"
#include "events.h"
#include "foraging.h"
#include "marmot_bitmap.h"
#include "marmot_hungry_bitmap.h"
#include "sprites.h"

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

static void dFillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, Shade shade) {
  if (shade == SHADE_BLACK) {
    epd.fillTriangle(x0, y0, x1, y1, x2, y2, C_BLACK);
    return;
  }
  if (shade == SHADE_WHITE) return;
  int minX = min(x0, min(x1, x2)), maxX = max(x0, max(x1, x2));
  int minY = min(y0, min(y1, y2)), maxY = max(y0, max(y1, y2));
  auto sign = [](int px, int py, int ax, int ay, int bx, int by) {
    return (px - bx) * (ay - by) - (ax - bx) * (py - by);
  };
  for (int yy = minY; yy <= maxY; yy++) {
    for (int xx = minX; xx <= maxX; xx++) {
      int d1 = sign(xx, yy, x0, y0, x1, y1);
      int d2 = sign(xx, yy, x1, y1, x2, y2);
      int d3 = sign(xx, yy, x2, y2, x0, y0);
      bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
      bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);
      if (!(hasNeg && hasPos) && ditherBlack(xx, yy, shade)) epd.drawPixel(xx, yy, C_BLACK);
    }
  }
}

static void fillDome(int cx, int baseY, int halfW, int height, Shade shade) {
  for (int i = -halfW; i <= halfW; i++) {
    int h = (int)(height * sqrtf(1.0f - (float)(i * i) / (float)(halfW * halfW)));
    dFillVLine(cx + i, baseY - h, h + 1, shade);
    epd.drawPixel(cx + i, baseY - h, C_BLACK);
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

static void drawMoon(int cx, int cy, int r, float phase, bool craters) {
  epd.fillCircle(cx, cy, r, C_WHITE);
  epd.drawCircle(cx, cy, r, C_BLACK);
  float f = cosf(2.0f * (float)M_PI * phase);
  bool waxing = phase <= 0.5f;
  for (int y = -r; y <= r; y++) {
    int xw = (int)lroundf(sqrtf((float)(r * r - y * y)));
    int xt = (int)lroundf(f * xw);
    int x0 = waxing ? -xw : -xt;
    int x1 = waxing ? xt : xw;
    if (x1 > x0) dFillHLine(cx + x0, cy + y, x1 - x0 + 1, SHADE_DARK);
  }
  if (craters) {
    const int8_t cr[][3] = {{-10, -14, 5}, {12, 6, 7}, {-4, 16, 4}, {18, -10, 3}};
    for (auto& c : cr) {
      int px = cx + (waxing ? c[0] : -c[0]), py = cy + c[1];
      dFillCircle(px, py, c[2], SHADE_LIGHT);
      epd.drawCircle(px, py, c[2], C_BLACK);
    }
  }
}

// --- sprites: native 64x64 at scale 1.0, drawn from top-left (x,y) -------
// Every sprite takes a `scale` multiplier so the Foraging view can render a
// much larger icon without a second copy of the artwork.
static void sprMushroomCap(int x, int y, float s = 1.0f) {
  auto S = [s](float v) { return (int)(v * s); };
  fillDome(x + S(32), y + S(30), S(24), S(20), SHADE_DARK);
  epd.fillRect(x + S(8), y + S(30), S(49), S(3) + 1, C_BLACK);
  dFillRect(x + S(26), y + S(33), S(12), S(24), SHADE_LIGHT);
  epd.drawRect(x + S(26), y + S(33), S(12), S(24), C_BLACK);
  dFillCircle(x + S(22), y + S(16), S(3), SHADE_LIGHT);
  dFillCircle(x + S(40), y + S(20), S(2), SHADE_LIGHT);
}

static void sprMushroomTooth(int x, int y, float s = 1.0f) {
  auto S = [s](float v) { return (int)(v * s); };
  fillDome(x + S(32), y + S(28), S(22), S(18), SHADE_DARK);
  for (int i = 0; i < 9; i++)
    epd.drawLine(x + S(14 + i * 5), y + S(28), x + S(14 + i * 5), y + S(28 + 4 + (i % 3) * 3),
                 C_BLACK);
  dFillRect(x + S(28), y + S(30), S(9), S(24), SHADE_LIGHT);
  epd.drawRect(x + S(28), y + S(30), S(9), S(24), C_BLACK);
}

static void sprMorel(int x, int y, float s = 1.0f) {
  auto S = [s](float v) { return (int)(v * s); };
  epd.drawLine(x + S(32), y + S(4), x + S(18), y + S(42), C_BLACK);
  epd.drawLine(x + S(32), y + S(4), x + S(46), y + S(42), C_BLACK);
  for (int r = 0; r < 5; r++)
    for (int cc = 0; cc <= 2 + r; cc++) {
      int bx = x + S(24 + cc * 6 - r * 3), by = y + S(10 + r * 6);
      dFillRect(bx, by, S(5), S(5), SHADE_DARK);
      epd.drawRect(bx, by, S(5), S(5), C_BLACK);
    }
  dFillRect(x + S(28), y + S(42), S(8), S(13), SHADE_LIGHT);
  epd.drawRect(x + S(28), y + S(42), S(8), S(13), C_BLACK);
}

static void sprMatsutake(int x, int y, float s = 1.0f) {
  auto S = [s](float v) { return (int)(v * s); };
  fillDome(x + S(32), y + S(24), S(20), S(14), SHADE_DARK);
  epd.fillRect(x + S(12), y + S(24), S(41), S(3) + 1, C_BLACK);
  dFillRect(x + S(25), y + S(27), S(15), S(29), SHADE_DARK);
  epd.drawRect(x + S(25), y + S(27), S(15), S(29), C_BLACK);
}

static void sprPorcini(int x, int y, float s = 1.0f) {
  auto S = [s](float v) { return (int)(v * s); };
  fillDome(x + S(32), y + S(26), S(24), S(16), SHADE_DARK);
  epd.fillRect(x + S(8), y + S(26), S(49), S(2) + 1, C_BLACK);
  dFillRect(x + S(23), y + S(28), S(18), S(28), SHADE_LIGHT);
  epd.drawRect(x + S(23), y + S(28), S(18), S(28), C_BLACK);
}

static void sprCoral(int x, int y, float s = 1.0f) {
  auto S = [s](float v) { return (int)(v * s); };
  for (int i = 0; i < 7; i++) {
    int bx = x + S(12 + i * 6);
    epd.drawLine(x + S(32), y + S(52), bx, y + S(14 + (i % 2) * 5), C_BLACK);
    dFillCircle(bx, y + S(13 + (i % 2) * 5), S(4), SHADE_LIGHT);
    epd.drawCircle(bx, y + S(13 + (i % 2) * 5), S(4), C_BLACK);
  }
}

static void sprLeafyGreen(int x, int y, float s = 1.0f) {
  auto S = [s](float v) { return (int)(v * s); };
  dFillCircle(x + S(32), y + S(26), S(18), SHADE_LIGHT);
  epd.drawCircle(x + S(32), y + S(26), S(18), C_BLACK);
  epd.drawLine(x + S(32), y + S(26), x + S(32), y + S(56), C_BLACK);
  for (int i = -1; i <= 1; i += 2) {
    epd.drawLine(x + S(32), y + S(22), x + S(32) + S(i * 11), y + S(18), C_BLACK);
    epd.drawLine(x + S(32), y + S(30), x + S(32) + S(i * 12), y + S(30), C_BLACK);
  }
}

static void sprNettle(int x, int y, float s = 1.0f) {
  auto S = [s](float v) { return (int)(v * s); };
  epd.drawLine(x + S(32), y + S(4), x + S(32), y + S(56), C_BLACK);
  for (int i = 0; i < 5; i++) {
    int yy = y + S(12 + i * 8), sp = S(17 - i * 2);
    epd.drawLine(x + S(32), yy, x + S(32) - sp, yy + S(7), C_BLACK);
    epd.drawLine(x + S(32), yy, x + S(32) + sp, yy + S(7), C_BLACK);
    epd.drawLine(x + S(32) - sp, yy + S(7), x + S(32), yy + S(9), C_BLACK);
    epd.drawLine(x + S(32) + sp, yy + S(7), x + S(32), yy + S(9), C_BLACK);
  }
}

static void sprFiddlehead(int x, int y, float s = 1.0f) {
  for (float a = 0; a < 6.28f * 2.4f; a += 0.18f) {
    float rr = (3 + a * 2.1f) * s;
    epd.drawPixel(x + (int)(30 * s + rr * cosf(a)), y + (int)(28 * s + rr * sinf(a)), C_BLACK);
    epd.drawPixel(x + (int)(31 * s + rr * cosf(a)), y + (int)(28 * s + rr * sinf(a)), C_BLACK);
  }
  auto S = [s](float v) { return (int)(v * s); };
  epd.drawLine(x + S(30), y + S(48), x + S(33), y + S(60), C_BLACK);
}

static void sprRamp(int x, int y, float s = 1.0f) {
  auto S = [s](float v) { return (int)(v * s); };
  dFillTriangle(x + S(30), y + S(50), x + S(20), y + S(8), x + S(28), y + S(12), SHADE_DARK);
  epd.drawLine(x + S(34), y + S(50), x + S(46), y + S(12), C_BLACK);
  epd.drawLine(x + S(34), y + S(50), x + S(40), y + S(14), C_BLACK);
  dFillCircle(x + S(32), y + S(53), S(6), SHADE_LIGHT);
  epd.drawCircle(x + S(32), y + S(53), S(6), C_BLACK);
}

static void sprFlowerCluster(int x, int y, float s = 1.0f) {
  auto S = [s](float v) { return (int)(v * s); };
  for (int i = 0; i < 7; i++) {
    int px = x + S(14 + (i % 4) * 11), py = y + S(10 + (i / 4) * 12);
    for (int k = 0; k < 5; k++) {
      float a = k * 1.256f;
      dFillCircle(px + (int)(S(3) * cosf(a)), py + (int)(S(3) * sinf(a)), S(2), SHADE_DARK);
    }
    dFillCircle(px, py, S(1), SHADE_LIGHT);
    epd.drawLine(px, py, x + S(32), y + S(56), C_BLACK);
  }
}

static void sprBerryCluster(int x, int y, float s = 1.0f) {
  auto S = [s](float v) { return (int)(v * s); };
  const int8_t b[][2] = {{26, 22}, {38, 24}, {32, 32}, {24, 38}, {40, 40}, {32, 46}};
  for (auto& p : b) {
    int bx = x + S(p[0]), by = y + S(p[1]);
    dFillCircle(bx, by, S(6), SHADE_DARK);
    epd.drawCircle(bx, by, S(6), C_BLACK);
    epd.fillCircle(bx - S(2), by - S(2), max(1, S(1)), C_WHITE);
  }
  epd.drawLine(x + S(32), y + S(6), x + S(32), y + S(22), C_BLACK);
}

static void drawSprite(uint8_t id, int x, int y, float scale = 1.0f) {
  switch (id) {
    case SPR_MUSHROOM_CAP:
      sprMushroomCap(x, y, scale);
      break;
    case SPR_MUSHROOM_TOOTH:
      sprMushroomTooth(x, y, scale);
      break;
    case SPR_MOREL:
      sprMorel(x, y, scale);
      break;
    case SPR_MATSUTAKE:
      sprMatsutake(x, y, scale);
      break;
    case SPR_PORCINI:
      sprPorcini(x, y, scale);
      break;
    case SPR_CORAL:
      sprCoral(x, y, scale);
      break;
    case SPR_LEAFY_GREEN:
      sprLeafyGreen(x, y, scale);
      break;
    case SPR_NETTLE:
      sprNettle(x, y, scale);
      break;
    case SPR_FIDDLEHEAD:
      sprFiddlehead(x, y, scale);
      break;
    case SPR_RAMP:
      sprRamp(x, y, scale);
      break;
    case SPR_FLOWER_CLUSTER:
      sprFlowerCluster(x, y, scale);
      break;
    case SPR_BERRY_CLUSTER:
      sprBerryCluster(x, y, scale);
      break;
    default:
      sprMushroomCap(x, y, scale);
      break;
  }
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

// A hoary marmot sitting upright on a rock ledge -- a hardcoded, dithered
// pen-and-ink-style bitmap (see include/marmot_bitmap.h, generated from
// artwork in the scratch dir) rather than live procedural shapes, for real
// fur texture and a recognizable silhouette. No procedural eye/nose/sparkle
// overlay -- just the bitmap, plus a glow ring for the Glowing mood.
static void drawCreature(int cx, int groundY, Mood mood) {
  int bx, by;
  if (mood == Mood::Hungry) {
    bx = cx - MARMOT_HUNGRY_W / 2;
    by = groundY - MARMOT_HUNGRY_GROUND_Y;
    epd.drawBitmap(bx, by, MARMOT_HUNGRY_BITMAP, MARMOT_HUNGRY_W, MARMOT_HUNGRY_H, C_BLACK,
                   C_WHITE);
  } else {
    bx = cx - MARMOT_W / 2;
    by = groundY - MARMOT_GROUND_Y;
    epd.drawBitmap(bx, by, MARMOT_BITMAP, MARMOT_W, MARMOT_H, C_BLACK, C_WHITE);
  }

  if (mood == Mood::Glowing)
    for (int ring = 34; ring <= 46; ring += 6)
      for (int a = 0; a < 360; a += 20) {
        float ra = a * (float)M_PI / 180.0f;
        epd.drawPixel(cx + (int)(ring * cosf(ra)), by + MARMOT_H / 2 + (int)(ring * sinf(ra)),
                      C_BLACK);
      }
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

// A pending wildlife sighting takes over the Main view until ENTER
// resolves it (see events::checkForEvent / onEnter() in main.cpp).
static void renderEncounter(const AppContext& ctx, const events::PendingEvent& ev) {
  bool negative = events::eventIsNegative(ev);
  textCentered(0, SCREEN_W, 6, events::eventTitle(ev.type, negative), 2);

  int stageCx = SCREEN_W / 2, stageGroundY = 235;
  drawCreature(stageCx, stageGroundY, negative ? Mood::Annoyed : Mood::Excited);

  int y = stageGroundY + 15;
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
  const float iconScale = 2.25f;
  int iconSize = (int)(64 * iconScale);
  int iconX = (SCREEN_W - iconSize) / 2, iconY = 6;
  drawSprite(f.spriteId, iconX, iconY, iconScale);

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

void hibernate() { epd.sleep(); }

}  // namespace display
