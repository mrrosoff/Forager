// Adapter: exposes Waveshare's official Epd driver as an Adafruit_GFX
// display, so existing drawing code (fillCircle, fillRoundRect, print, the
// dithering helpers) works unchanged. Adafruit_GFX implements all shape
// drawing on top of a single drawPixel() override -- we don't need the
// vendor's own Paint/font helpers at all.
#pragma once

#include <Adafruit_GFX.h>
#include "epd4in2_V2.h"
// The vendor header #defines EPD_WIDTH/EPD_HEIGHT as macros, which collide
// with config.h's own `static const int EPD_WIDTH` constants. Undefine them
// immediately so config.h (included by display.cpp after this header) isn't
// corrupted by macro substitution.
#undef EPD_WIDTH
#undef EPD_HEIGHT

// 0 = black, 1 = white -- matches the bit convention Epd::Display() expects
// (MSB-first, set bit = white), same as the panel's native buffer format.
#define EPD_GFX_BLACK 0
#define EPD_GFX_WHITE 1

class EpdGFX : public Adafruit_GFX {
 public:
  EpdGFX() : Adafruit_GFX(400, 300) {}

  int begin() {
    firstFrame_ = true;
    return epd_.Init();
  }
  void clearPanel() { epd_.Clear(); }
  void sleep() { epd_.Sleep(); }

  // Call before drawing a frame; clears the in-RAM buffer to all-white.
  void beginFrame() { memset(buffer_, 0xFF, sizeof(buffer_)); }

  // Push the buffer to the panel. `partial` requests the fast, flicker-free
  // partial-refresh path -- except unlike GxEPD2, this driver has no
  // automatic "first frame is always full" behavior, so we force a full
  // refresh ourselves on the first frame after begin() (matching Waveshare's
  // own guidance to always full-refresh on wake) and honor `partial`
  // for every frame after that.
  void endFrame(bool partial) {
    bool useFull = !partial || firstFrame_;
    if (useFull) epd_.Display(buffer_);
    else epd_.Display_Partial(buffer_, 0, 0, 400, 300);
    firstFrame_ = false;
  }

  // Adafruit_GFX's setRotation() swaps width()/height() for rotations 1/3,
  // so callers draw in the rotated (portrait) coordinate frame; this maps
  // that back to the panel's fixed native 400x300 landscape buffer.
  void drawPixel(int16_t x, int16_t y, uint16_t color) override {
    if (x < 0 || x >= width() || y < 0 || y >= height()) return;
    int16_t px = x, py = y;
    switch (getRotation()) {
      case 1: px = kPhysW - 1 - y; py = x; break;
      case 2: px = kPhysW - 1 - x; py = kPhysH - 1 - y; break;
      case 3: px = y; py = kPhysH - 1 - x; break;
      default: break;
    }
    int idx = px / 8 + py * (kPhysW / 8);
    uint8_t mask = 0x80 >> (px % 8);
    if (color == EPD_GFX_WHITE) buffer_[idx] |= mask;
    else buffer_[idx] &= ~mask;
  }

 private:
  static const int16_t kPhysW = 400, kPhysH = 300;
  Epd epd_;
  bool firstFrame_ = true;
  uint8_t buffer_[400 / 8 * 300];
};
