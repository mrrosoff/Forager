// Official Waveshare epd4in2_V2 demo, adapted only for our custom SPI pins.
// Purpose: isolate whether display artifacts are a GxEPD2/Forager code issue
// or inherent to the panel/wiring, using the vendor's own reference driver.
//
// Safety, per Waveshare's own datasheet precautions:
// - Don't refresh in partial mode indefinitely -- a full refresh must follow
//   a burst of partial refreshes, or the display degrades irreversibly.
// - Don't leave the panel powered/refreshing continuously -- sleep it when
//   not actively refreshing, or the high-voltage state can damage it.
// - Recommended refresh interval is >=180s; this demo runs a bounded number
//   of cycles (not an unattended infinite loop) with sleeps in between.
#include <Arduino.h>
#include <SPI.h>
#include "epd4in2_V2.h"
#include "epdpaint.h"
#include "imagedata.h"

#define COLORED     0
#define UNCOLORED   1

static const int MAX_CYCLES = 3;
static int cycleCount = 0;
static uint32_t lastCycleMs = 0;
static const uint32_t CYCLE_INTERVAL_MS = 30UL * 1000UL;  // well under 180s, but bounded

static void runPartialDemo(Epd& epd) {
  unsigned char image[1500];
  Paint paint(image, 400, 28);  // width must be a multiple of 8

  paint.Clear(UNCOLORED);
  paint.DrawStringAt(20, 5, "e-Paper Demo", &Font16, COLORED);
  Serial.println("Display_Partial: text 1");
  epd.Display_Partial_Not_refresh(paint.GetImage(), 0, 32, 0 + paint.GetWidth(), 32 + paint.GetHeight());

  paint.Clear(COLORED);
  paint.DrawStringAt(20, 5, "Hello world!", &Font16, UNCOLORED);
  Serial.println("Display_Partial: text 2");
  epd.Display_Partial_Not_refresh(paint.GetImage(), 0, 64, 0 + paint.GetWidth(), 64 + paint.GetHeight());

  paint.SetWidth(64);
  paint.SetHeight(64);

  paint.Clear(UNCOLORED);
  paint.DrawRectangle(0, 0, 40, 50, COLORED);
  paint.DrawLine(0, 0, 40, 50, COLORED);
  paint.DrawLine(40, 0, 0, 50, COLORED);
  Serial.println("Display_Partial: rectangle+X");
  epd.Display_Partial_Not_refresh(paint.GetImage(), 10, 130, 10 + paint.GetWidth(), 130 + paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawCircle(32, 32, 30, COLORED);
  Serial.println("Display_Partial: circle");
  epd.Display_Partial_Not_refresh(paint.GetImage(), 90, 120, 90 + paint.GetWidth(), 120 + paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawFilledRectangle(0, 0, 40, 50, COLORED);
  Serial.println("Display_Partial: filled rectangle");
  epd.Display_Partial_Not_refresh(paint.GetImage(), 10, 200, 10 + paint.GetWidth(), 200 + paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawFilledCircle(32, 32, 30, COLORED);
  Serial.println("Display_Partial: filled circle (refreshes now)");
  epd.Display_Partial(paint.GetImage(), 90, 190, 90 + paint.GetWidth(), 190 + paint.GetHeight());
}

static void runCycle() {
  Serial.printf("--- cycle %d/%d ---\n", cycleCount + 1, MAX_CYCLES);
  Epd epd;
  if (epd.Init() != 0) {
    Serial.println("e-Paper init failed");
    return;
  }

  // Always clear (full refresh) on wake before drawing anything new, per
  // Waveshare's guidance to avoid afterimage.
  Serial.println("Clear() on wake");
  epd.Clear();

  Serial.println("Display(IMAGE_BUTTERFLY) full refresh");
  epd.Display(IMAGE_BUTTERFLY);
  delay(1000);

  runPartialDemo(epd);

  // Intentionally skipping the closing full refresh here, against
  // Waveshare's own guidance, to directly test its effect.
  Serial.println("Sleep()");
  epd.Sleep();
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("epd4in2_V2 partial-refresh + sleep/wake demo starting");
  runCycle();
  cycleCount++;
  lastCycleMs = millis();
}

void loop() {
  if (cycleCount >= MAX_CYCLES) {
    delay(1000);
    return;  // done -- panel stays asleep, no more refreshes
  }
  if (millis() - lastCycleMs < CYCLE_INTERVAL_MS) {
    delay(200);
    return;
  }
  runCycle();
  cycleCount++;
  lastCycleMs = millis();
}
