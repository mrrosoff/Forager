#include "display.h"
#include "config.h"
#include "sprites.h"
#include "creature.h"
#include "foraging.h"

#include <SPI.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSerifItalic9pt7b.h>
#include <math.h>

#if EPD_PANEL_GDEY042T81
  using EpdDriver = GxEPD2_420_GDEY042T81;
#else
  using EpdDriver = GxEPD2_420;
#endif

namespace display {

static GxEPD2_BW<EpdDriver, EpdDriver::HEIGHT> epd(
    EpdDriver(PIN_EPD_CS, PIN_EPD_DC, PIN_EPD_RST, PIN_EPD_BUSY));

static const int CR_X = 30, CR_Y = 60, CR_W = 170, CR_H = 170;

// 1-bit panel; "greys" are Bayer-dithered. shade 0=white .. 16=black.
static const uint8_t SH_LIGHT = 5, SH_MID = 9, SH_DARK = 13;
static const uint8_t kBayer[4][4] = {
  { 0, 8, 2, 10}, {12, 4, 14, 6}, { 3, 11, 1, 9}, {15, 7, 13, 5}};

static inline bool ditherOn(int x, int y, uint8_t shade) {
  return kBayer[y & 3][x & 3] < shade;
}

static void shadeHLine(int x, int y, int w, uint8_t shade) {
  if (shade >= 16) { epd.drawFastHLine(x, y, w, GxEPD_BLACK); return; }
  if (shade == 0) return;
  for (int i = 0; i < w; i++)
    if (ditherOn(x + i, y, shade)) epd.drawPixel(x + i, y, GxEPD_BLACK);
}

static void shadeRect(int x, int y, int w, int h, uint8_t shade) {
  for (int j = 0; j < h; j++) shadeHLine(x, y + j, w, shade);
}

static void shadeDisc(int cx, int cy, int r, uint8_t shade) {
  for (int y = -r; y <= r; y++) {
    int xw = (int)lroundf(sqrtf((float)(r * r - y * y)));
    shadeHLine(cx - xw, cy + y, 2 * xw + 1, shade);
  }
}

// A dome: filled lower-shade cap with darker rim, for mushroom tops.
static void shadeDome(int cx, int baseY, int halfW, int height, uint8_t shade) {
  for (int i = -halfW; i <= halfW; i++) {
    int h = (int)(height * sqrtf(1.0f - (float)(i * i) / (float)(halfW * halfW)));
    shadeHLine(cx + i, baseY - h, 1, shade);
    for (int yy = baseY - h; yy <= baseY; yy++)
      if (ditherOn(cx + i, yy, shade)) epd.drawPixel(cx + i, yy, GxEPD_BLACK);
  }
  for (int i = -halfW; i <= halfW; i++) {  // crisp rim
    int h = (int)(height * sqrtf(1.0f - (float)(i * i) / (float)(halfW * halfW)));
    epd.drawPixel(cx + i, baseY - h, GxEPD_BLACK);
  }
}

static void textAt(int x, int y, const char* s, const GFXfont* f) {
  epd.setFont(f);
  epd.setTextColor(GxEPD_BLACK);
  epd.setCursor(x, y);
  epd.print(s);
}

static void textCentered(int x0, int w, int y, const char* s, const GFXfont* f) {
  epd.setFont(f);
  int16_t bx, by; uint16_t bw, bh;
  epd.getTextBounds(s, 0, y, &bx, &by, &bw, &bh);
  textAt(x0 + (w - (int)bw) / 2 - bx, y, s, f);
}

static void drawMoon(int cx, int cy, int r, float phase, bool craters) {
  epd.drawCircle(cx, cy, r, GxEPD_BLACK);
  float f = cosf(2.0f * (float)M_PI * phase);
  bool waxing = phase <= 0.5f;
  for (int y = -r; y <= r; y++) {
    int xw = (int)lroundf(sqrtf((float)(r * r - y * y)));
    int xt = (int)lroundf(f * xw);
    int x0 = waxing ? -xw : -xt;
    int x1 = waxing ? xt : xw;
    if (x1 > x0) shadeHLine(cx + x0, cy + y, x1 - x0 + 1, SH_DARK);
  }
  if (craters) {
    const int8_t cr[][3] = {{-10, -14, 5}, {12, 6, 7}, {-4, 16, 4}, {18, -10, 3}};
    for (auto& c : cr) {
      int px = cx + (waxing ? c[0] : -c[0]), py = cy + c[1];
      epd.fillCircle(px, py, c[2], GxEPD_WHITE);
      shadeDisc(px, py, c[2], SH_LIGHT);
      epd.drawCircle(px, py, c[2], GxEPD_BLACK);
    }
  }
}

// --- sprites: ~64x64, drawn from top-left (x,y) --------------------------
static void sprMushroomCap(int x, int y) {
  shadeDome(x + 32, y + 30, 24, 20, SH_MID);
  epd.fillRect(x + 8, y + 30, 49, 3, GxEPD_BLACK);
  shadeRect(x + 26, y + 33, 12, 24, SH_LIGHT);
  epd.drawRect(x + 26, y + 33, 12, 24, GxEPD_BLACK);
  epd.fillCircle(x + 22, y + 16, 3, GxEPD_WHITE);   // spots
  epd.fillCircle(x + 40, y + 20, 2, GxEPD_WHITE);
}

static void sprMushroomTooth(int x, int y) {
  shadeDome(x + 32, y + 28, 22, 18, SH_DARK);
  for (int i = 0; i < 9; i++)
    epd.drawLine(x + 14 + i * 5, y + 28, x + 14 + i * 5, y + 28 + 4 + (i % 3) * 3, GxEPD_BLACK);
  shadeRect(x + 28, y + 30, 9, 24, SH_LIGHT);
  epd.drawRect(x + 28, y + 30, 9, 24, GxEPD_BLACK);
}

static void sprMorel(int x, int y) {
  epd.drawLine(x + 32, y + 4, x + 18, y + 42, GxEPD_BLACK);
  epd.drawLine(x + 32, y + 4, x + 46, y + 42, GxEPD_BLACK);
  for (int r = 0; r < 5; r++)
    for (int cc = 0; cc <= 2 + r; cc++) {
      int bx = x + 24 + cc * 6 - r * 3, by = y + 10 + r * 6;
      shadeRect(bx, by, 5, 5, SH_MID);
      epd.drawRect(bx, by, 5, 5, GxEPD_BLACK);
    }
  shadeRect(x + 28, y + 42, 8, 13, SH_LIGHT);
  epd.drawRect(x + 28, y + 42, 8, 13, GxEPD_BLACK);
}

static void sprMatsutake(int x, int y) {
  shadeDome(x + 32, y + 24, 20, 14, SH_DARK);
  epd.fillRect(x + 12, y + 24, 41, 3, GxEPD_BLACK);
  shadeRect(x + 25, y + 27, 15, 29, SH_MID);
  epd.drawRect(x + 25, y + 27, 15, 29, GxEPD_BLACK);
}

static void sprPorcini(int x, int y) {
  shadeDome(x + 32, y + 26, 24, 16, SH_DARK);
  epd.fillRect(x + 8, y + 26, 49, 2, GxEPD_BLACK);
  shadeRect(x + 23, y + 28, 18, 28, SH_LIGHT);
  for (int i = 0; i < 18; i += 3) epd.drawFastVLine(x + 23 + i, y + 28, 28, GxEPD_BLACK);
  epd.drawRect(x + 23, y + 28, 18, 28, GxEPD_BLACK);
}

static void sprCoral(int x, int y) {
  for (int i = 0; i < 7; i++) {
    int bx = x + 12 + i * 6;
    epd.drawLine(x + 32, y + 52, bx, y + 14 + (i % 2) * 5, GxEPD_BLACK);
    shadeDisc(bx, y + 13 + (i % 2) * 5, 4, SH_LIGHT);
    epd.drawCircle(bx, y + 13 + (i % 2) * 5, 4, GxEPD_BLACK);
  }
}

static void sprLeafyGreen(int x, int y) {
  shadeDisc(x + 32, y + 26, 18, SH_LIGHT);
  epd.drawCircle(x + 32, y + 26, 18, GxEPD_BLACK);
  epd.drawLine(x + 32, y + 26, x + 32, y + 56, GxEPD_BLACK);
  for (int i = -1; i <= 1; i += 2) {
    epd.drawLine(x + 32, y + 22, x + 32 + i * 11, y + 18, GxEPD_BLACK);
    epd.drawLine(x + 32, y + 30, x + 32 + i * 12, y + 30, GxEPD_BLACK);
  }
}

static void sprNettle(int x, int y) {
  epd.drawLine(x + 32, y + 4, x + 32, y + 56, GxEPD_BLACK);
  for (int i = 0; i < 5; i++) {
    int yy = y + 12 + i * 8, sp = 17 - i * 2;
    epd.drawLine(x + 32, yy, x + 32 - sp, yy + 7, GxEPD_BLACK);
    epd.drawLine(x + 32, yy, x + 32 + sp, yy + 7, GxEPD_BLACK);
    epd.drawLine(x + 32 - sp, yy + 7, x + 32, yy + 9, GxEPD_BLACK);
    epd.drawLine(x + 32 + sp, yy + 7, x + 32, yy + 9, GxEPD_BLACK);
  }
}

static void sprFiddlehead(int x, int y) {
  for (float a = 0; a < 6.28f * 2.4f; a += 0.18f) {
    float rr = 3 + a * 2.1f;
    epd.drawPixel(x + 30 + (int)(rr * cosf(a)), y + 28 + (int)(rr * sinf(a)), GxEPD_BLACK);
    epd.drawPixel(x + 31 + (int)(rr * cosf(a)), y + 28 + (int)(rr * sinf(a)), GxEPD_BLACK);
  }
  epd.drawLine(x + 30, y + 48, x + 33, y + 60, GxEPD_BLACK);
}

static void sprRamp(int x, int y) {
  epd.fillTriangle(x + 30, y + 50, x + 20, y + 8, x + 28, y + 12, GxEPD_BLACK);
  shadeDisc(x + 36, y + 30, 6, SH_MID);
  epd.drawLine(x + 34, y + 50, x + 46, y + 12, GxEPD_BLACK);
  epd.drawLine(x + 34, y + 50, x + 40, y + 14, GxEPD_BLACK);
  shadeDisc(x + 32, y + 53, 6, SH_LIGHT);
  epd.drawCircle(x + 32, y + 53, 6, GxEPD_BLACK);
}

static void sprFlowerCluster(int x, int y) {
  for (int i = 0; i < 7; i++) {
    int px = x + 14 + (i % 4) * 11, py = y + 10 + (i / 4) * 12;
    for (int k = 0; k < 5; k++) {
      float a = k * 1.256f;
      epd.fillCircle(px + (int)(3 * cosf(a)), py + (int)(3 * sinf(a)), 2, GxEPD_BLACK);
    }
    epd.drawPixel(px, py, GxEPD_WHITE);
    epd.drawLine(px, py, x + 32, y + 56, GxEPD_BLACK);
  }
}

static void sprBerryCluster(int x, int y) {
  const int8_t b[][2] = {{26, 22}, {38, 24}, {32, 32}, {24, 38}, {40, 40}, {32, 46}};
  for (auto& p : b) {
    shadeDisc(x + p[0], y + p[1], 6, SH_DARK);
    epd.drawCircle(x + p[0], y + p[1], 6, GxEPD_BLACK);
    epd.fillCircle(x + p[0] - 2, y + p[1] - 2, 1, GxEPD_WHITE);
  }
  epd.drawLine(x + 32, y + 6, x + 32, y + 22, GxEPD_BLACK);
}

static void drawSprite(uint8_t id, int x, int y) {
  switch (id) {
    case SPR_MUSHROOM_CAP:   sprMushroomCap(x, y);   break;
    case SPR_MUSHROOM_TOOTH: sprMushroomTooth(x, y); break;
    case SPR_MOREL:          sprMorel(x, y);         break;
    case SPR_MATSUTAKE:      sprMatsutake(x, y);     break;
    case SPR_PORCINI:        sprPorcini(x, y);       break;
    case SPR_CORAL:          sprCoral(x, y);         break;
    case SPR_LEAFY_GREEN:    sprLeafyGreen(x, y);    break;
    case SPR_NETTLE:         sprNettle(x, y);        break;
    case SPR_FIDDLEHEAD:     sprFiddlehead(x, y);    break;
    case SPR_RAMP:           sprRamp(x, y);          break;
    case SPR_FLOWER_CLUSTER: sprFlowerCluster(x, y); break;
    case SPR_BERRY_CLUSTER:  sprBerryCluster(x, y);  break;
    default:                 sprMushroomCap(x, y);   break;
  }
}

static void drawCreature(Mood mood, uint8_t frame) {
  int cx = CR_X + CR_W / 2;
  int cy = CR_Y + CR_H / 2 + (int)lroundf(3.0f * sinf(frame * 0.6f));
  int bodyR = 46;

  if (mood == Mood::Glowing) {
    for (int ring = bodyR + 8; ring <= bodyR + 16; ring += 8)
      for (int a = 0; a < 360; a += 12) {
        float ra = a * (float)M_PI / 180.0f;
        epd.drawPixel(cx + (int)(ring * cosf(ra)), cy + (int)(ring * sinf(ra)), GxEPD_BLACK);
      }
  }

  shadeDisc(cx, cy, bodyR, SH_DARK);
  shadeRect(cx - bodyR, cy, bodyR * 2 + 1, bodyR, SH_DARK);
  epd.drawCircle(cx, cy, bodyR, GxEPD_BLACK);
  epd.drawFastHLine(cx - bodyR, cy + bodyR, bodyR * 2 + 1, GxEPD_BLACK);

  int fx = cx, fy = cy + 6;
  epd.fillRoundRect(fx - 32, fy - 16, 64, 32, 12, GxEPD_WHITE);
  epd.drawRoundRect(fx - 32, fy - 16, 64, 32, 12, GxEPD_BLACK);

  bool blink = (frame % 7) == 6;
  auto eye = [&](int ex, int ey) {
    switch (mood) {
      case Mood::Sleepy:
      case Mood::Dormant:
        epd.drawFastHLine(ex - 6, ey, 12, GxEPD_BLACK);
        break;
      case Mood::Hungry:
        epd.drawCircle(ex, ey, 4, GxEPD_BLACK);
        break;
      case Mood::Excited:
      case Mood::Glowing:
        if (blink) epd.drawFastHLine(ex - 6, ey, 12, GxEPD_BLACK);
        else { epd.fillCircle(ex, ey, 6, GxEPD_BLACK);
               epd.fillCircle(ex + 2, ey - 2, 2, GxEPD_WHITE); }
        break;
      default:
        if (blink) epd.drawFastHLine(ex - 5, ey, 10, GxEPD_BLACK);
        else epd.fillCircle(ex, ey, 5, GxEPD_BLACK);
    }
  };
  eye(fx - 14, fy);
  eye(fx + 14, fy);

  if (mood == Mood::Hungry) epd.drawCircle(fx, fy + 11, 3, GxEPD_BLACK);
  else if (mood == Mood::Excited || mood == Mood::Glowing)
    epd.drawLine(fx - 6, fy + 11, fx + 6, fy + 11, GxEPD_BLACK);

  if (mood != Mood::Dormant)
    for (int i = 0; i < 5; i++) {
      int sx = cx - 30 + (i * 17 + frame * 3) % 70;
      int sy = cy - bodyR - 6 - ((frame * 2 + i * 9) % 24);
      epd.fillCircle(sx, sy, 1, GxEPD_BLACK);
    }
}

static void renderMain(const AppContext& ctx) {
  char buf[48];
  strftime(buf, sizeof(buf), "%a %b %-d", &ctx.now);
  textAt(14, 26, buf, &FreeSans9pt7b);

  drawMoon(EPD_WIDTH - 50, 44, 26, ctx.moon.phase, false);
  drawCreature(ctx.creature.mood, 0);
  textCentered(CR_X - 10, CR_W + 20, CR_Y + CR_H + 6,
               creature::moodName(ctx.creature.mood), &FreeSansBold12pt7b);

  drawSprite(ctx.featured.spriteId, 16, EPD_HEIGHT - 78);
  textAt(86, EPD_HEIGHT - 46, "Forage now:", &FreeSans9pt7b);
  textAt(86, EPD_HEIGHT - 22, ctx.featured.name, &FreeSansBold12pt7b);
}

static void renderMoonView(const AppContext& ctx) {
  drawMoon(150, 150, 96, ctx.moon.phase, true);
  char buf[40];
  textAt(270, 70, "Moon", &FreeSans9pt7b);
  textAt(270, 104, ctx.moon.name, &FreeSansBold12pt7b);
  snprintf(buf, sizeof(buf), "%d%% lit", ctx.moon.illumPct);
  textAt(270, 140, buf, &FreeSans9pt7b);
  if (ctx.moon.isFull) {
    textAt(270, 184, "Full tonight", &FreeSansBold9pt7b);
  } else {
    snprintf(buf, sizeof(buf), "Full in %dd", ctx.moon.daysToFull);
    textAt(270, 184, buf, &FreeSans9pt7b);
  }
  snprintf(buf, sizeof(buf), "New in %dd", ctx.moon.daysToNew);
  textAt(270, 214, buf, &FreeSans9pt7b);
}

static void renderForaging(const AppContext& ctx) {
  int month = ctx.now.tm_mon + 1;
  textAt(16, 30, "Foraging - Seattle", &FreeSansBold12pt7b);

  drawSprite(ctx.featured.spriteId, 20, 50);
  textAt(96, 74, ctx.featured.name, &FreeSansBold12pt7b);
  textAt(96, 98, ctx.featured.note, &FreeSerifItalic9pt7b);

  drawSprite(ctx.secondary.spriteId, 20, 122);
  textAt(96, 146, ctx.secondary.name, &FreeSansBold9pt7b);
  textAt(96, 168, ctx.secondary.note, &FreeSerifItalic9pt7b);

  epd.drawFastHLine(16, 198, EPD_WIDTH - 32, GxEPD_BLACK);
  textAt(16, 222, foraging::seasonNote(month), &FreeSans9pt7b);

  char buf[64];
  if (ctx.weather.valid)
    snprintf(buf, sizeof(buf), "%s, %.0fC  %s", ctx.weather.condition,
             ctx.weather.tempC, ctx.weather.postRain ? "- good after rain!" : "- dry");
  else
    snprintf(buf, sizeof(buf), "Conditions unavailable");
  textAt(16, 250, buf, &FreeSans9pt7b);
}

static void renderStatus(const AppContext& ctx) {
  drawCreature(ctx.creature.mood, 0);
  textAt(230, 40, "Status", &FreeSansBold12pt7b);

  auto bar = [&](int y, const char* label, uint8_t pct) {
    textAt(230, y, label, &FreeSans9pt7b);
    int bx = 230, bw = 150, by = y + 8;
    epd.drawRect(bx, by, bw, 14, GxEPD_BLACK);
    epd.fillRect(bx + 2, by + 2, (bw - 4) * pct / 100, 10, GxEPD_BLACK);
  };
  bar(78, "Fullness", 100 - ctx.creature.hunger);
  bar(128, "Happiness", ctx.creature.happiness);

  char buf[48];
  if (ctx.creature.lastFed == 0) {
    snprintf(buf, sizeof(buf), "Never foraged");
  } else {
    long days = (mktime(const_cast<struct tm*>(&ctx.now)) - ctx.creature.lastFed) / 86400;
    if (days <= 0) snprintf(buf, sizeof(buf), "Foraged today");
    else snprintf(buf, sizeof(buf), "Foraged %ld day%s ago", days, days == 1 ? "" : "s");
  }
  textAt(230, 184, buf, &FreeSans9pt7b);
  textAt(230, 228, "Press ENTER when", &FreeSerifItalic9pt7b);
  textAt(230, 250, "you go foraging", &FreeSerifItalic9pt7b);
}

void begin() {
  SPI.begin(PIN_EPD_SCK, -1, PIN_EPD_MOSI, PIN_EPD_CS);
  epd.init(115200, true, 2, false);
  epd.setRotation(0);
  epd.setTextWrap(false);
}

void renderView(View v, const AppContext& ctx) {
  epd.setFullWindow();
  epd.firstPage();
  do {
    epd.fillScreen(GxEPD_WHITE);
    switch (v) {
      case View::Main:     renderMain(ctx);     break;
      case View::Moon:     renderMoonView(ctx); break;
      case View::Foraging: renderForaging(ctx); break;
      case View::Status:   renderStatus(ctx);   break;
      default: break;
    }
  } while (epd.nextPage());
}

void animateCreature(View v, const AppContext& ctx, uint8_t frame) {
  if (v != View::Main && v != View::Status) return;
  epd.setPartialWindow(CR_X, CR_Y, CR_W, CR_H);
  epd.firstPage();
  do {
    epd.fillScreen(GxEPD_WHITE);
    drawCreature(ctx.creature.mood, frame);
  } while (epd.nextPage());
}

void flashFed(const AppContext& ctx) {
  epd.setPartialWindow(CR_X, CR_Y, CR_W, CR_H);
  epd.firstPage();
  do {
    epd.fillScreen(GxEPD_WHITE);
    drawCreature(Mood::Excited, 2);
    textCentered(CR_X, CR_W, CR_Y + CR_H - 8, "fed!", &FreeSansBold12pt7b);
  } while (epd.nextPage());
}

void hibernate() {
  epd.hibernate();
}

}  // namespace display
