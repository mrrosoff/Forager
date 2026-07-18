// display_marmot_art.h — the MarmotArt pose-pool tables (per-mood adult
// pools, small-canvas Status-view counterparts, Baby/Juvenile pools).
// Pure data, split out of display.cpp so that file is mostly rendering
// logic. Only included from display.cpp, after all the marmot bitmap
// headers it references.
#pragma once

struct MarmotArt {
  const uint8_t* bitmap;
  int w, h, groundY;
};


// Adult poses are split by mood instead of one shared pool -- each real
// hoary-marmot photo (see include/bitmaps/marmot/) was picked for a pose/
// expression that reads as a specific mood (upright and alert vs. lounging
// vs. mid-yell, etc.), so drawCreature() below picks from the pool matching
// the creature's actual mood rather than a random pose regardless of mood.
// Dormant (deep-winter hibernation) has no distinct photos of its own --
// hibernating marmots aren't something this project's photo sourcing can
// realistically depict, so it reuses the Sleepy pool. Picked once per wake,
// not per render, so the pose doesn't change mid-session.
static const MarmotArt kMarmotExcited[] = {
    {MARMOT_VARIANT6_BITMAP, MARMOT_VARIANT6_W, MARMOT_VARIANT6_H, MARMOT_VARIANT6_H},
    {MARMOT_VARIANT9_BITMAP, MARMOT_VARIANT9_W, MARMOT_VARIANT9_H, MARMOT_VARIANT9_H},
    {MARMOT_VARIANT18_BITMAP, MARMOT_VARIANT18_W, MARMOT_VARIANT18_H, MARMOT_VARIANT18_H},
};
static const int kMarmotExcitedCount = sizeof(kMarmotExcited) / sizeof(kMarmotExcited[0]);

// Startled poses for Mood::Scared -- only shown on a bad wake-time encounter.
static const MarmotArt kMarmotScared[] = {
    {MARMOT_VARIANT17_BITMAP, MARMOT_VARIANT17_W, MARMOT_VARIANT17_H, MARMOT_VARIANT17_H},
    {MARMOT_VARIANT26_BITMAP, MARMOT_VARIANT26_W, MARMOT_VARIANT26_H, MARMOT_VARIANT26_H},
};
static const int kMarmotScaredCount = sizeof(kMarmotScared) / sizeof(kMarmotScared[0]);

static const MarmotArt kMarmotContent[] = {
    {MARMOT_VARIANT5_BITMAP, MARMOT_VARIANT5_W, MARMOT_VARIANT5_H, MARMOT_VARIANT5_H},
    {MARMOT_VARIANT7_BITMAP, MARMOT_VARIANT7_W, MARMOT_VARIANT7_H, MARMOT_VARIANT7_H},
    {MARMOT_VARIANT8_BITMAP, MARMOT_VARIANT8_W, MARMOT_VARIANT8_H, MARMOT_VARIANT8_H},
    {MARMOT_VARIANT10_BITMAP, MARMOT_VARIANT10_W, MARMOT_VARIANT10_H, MARMOT_VARIANT10_H},
    {MARMOT_VARIANT11_BITMAP, MARMOT_VARIANT11_W, MARMOT_VARIANT11_H, MARMOT_VARIANT11_H},
    {MARMOT_VARIANT20_BITMAP, MARMOT_VARIANT20_W, MARMOT_VARIANT20_H, MARMOT_VARIANT20_H},
    {MARMOT_VARIANT27_BITMAP, MARMOT_VARIANT27_W, MARMOT_VARIANT27_H, MARMOT_VARIANT27_H},
    {MARMOT_VARIANT28_BITMAP, MARMOT_VARIANT28_W, MARMOT_VARIANT28_H, MARMOT_VARIANT28_H},
};
static const int kMarmotContentCount = sizeof(kMarmotContent) / sizeof(kMarmotContent[0]);

// Eating poses, split out from Content specifically for Hungry mood.
static const MarmotArt kMarmotHungry[] = {
    {MARMOT_VARIANT13_BITMAP, MARMOT_VARIANT13_W, MARMOT_VARIANT13_H, MARMOT_VARIANT13_H},
    {MARMOT_VARIANT21_BITMAP, MARMOT_VARIANT21_W, MARMOT_VARIANT21_H, MARMOT_VARIANT21_H},
    {MARMOT_VARIANT24_BITMAP, MARMOT_VARIANT24_W, MARMOT_VARIANT24_H, MARMOT_VARIANT24_H},
    {MARMOT_VARIANT25_BITMAP, MARMOT_VARIANT25_W, MARMOT_VARIANT25_H, MARMOT_VARIANT25_H},
};
static const int kMarmotHungryCount = sizeof(kMarmotHungry) / sizeof(kMarmotHungry[0]);

static const MarmotArt kMarmotSleepy[] = {
    {MARMOT_VARIANT15_BITMAP, MARMOT_VARIANT15_W, MARMOT_VARIANT15_H, MARMOT_VARIANT15_H},
    {MARMOT_VARIANT22_BITMAP, MARMOT_VARIANT22_W, MARMOT_VARIANT22_H, MARMOT_VARIANT22_H},
};
static const int kMarmotSleepyCount = sizeof(kMarmotSleepy) / sizeof(kMarmotSleepy[0]);

static const MarmotArt kMarmotAnnoyed[] = {
    {MARMOT_VARIANT16_BITMAP, MARMOT_VARIANT16_W, MARMOT_VARIANT16_H, MARMOT_VARIANT16_H},
    {MARMOT_VARIANT26_BITMAP, MARMOT_VARIANT26_W, MARMOT_VARIANT26_H, MARMOT_VARIANT26_H},
};
static const int kMarmotAnnoyedCount = sizeof(kMarmotAnnoyed) / sizeof(kMarmotAnnoyed[0]);

// Small-canvas counterparts, genuinely re-dithered at 112x136 (not scaled
// from the full-size bitmaps at draw time -- nearest-neighbor downscaling
// an already-dithered 1-bit image aliases the dither pattern into noise).
// Used by the Status view; index-for-index parallel to the arrays above so
// pickVariant()'s cached index picks the same photo in both.
static const MarmotArt kMarmotExcitedSmall[] = {
    {MARMOT_VARIANT6_SMALL_BITMAP, MARMOT_VARIANT6_SMALL_W, MARMOT_VARIANT6_SMALL_H,
     MARMOT_VARIANT6_SMALL_H},
    {MARMOT_VARIANT9_SMALL_BITMAP, MARMOT_VARIANT9_SMALL_W, MARMOT_VARIANT9_SMALL_H,
     MARMOT_VARIANT9_SMALL_H},
    {MARMOT_VARIANT18_SMALL_BITMAP, MARMOT_VARIANT18_SMALL_W, MARMOT_VARIANT18_SMALL_H,
     MARMOT_VARIANT18_SMALL_H},
};

static const MarmotArt kMarmotScaredSmall[] = {
    {MARMOT_VARIANT17_SMALL_BITMAP, MARMOT_VARIANT17_SMALL_W, MARMOT_VARIANT17_SMALL_H,
     MARMOT_VARIANT17_SMALL_H},
    {MARMOT_VARIANT26_SMALL_BITMAP, MARMOT_VARIANT26_SMALL_W, MARMOT_VARIANT26_SMALL_H,
     MARMOT_VARIANT26_SMALL_H},
};

static const MarmotArt kMarmotContentSmall[] = {
    {MARMOT_VARIANT5_SMALL_BITMAP, MARMOT_VARIANT5_SMALL_W, MARMOT_VARIANT5_SMALL_H,
     MARMOT_VARIANT5_SMALL_H},
    {MARMOT_VARIANT7_SMALL_BITMAP, MARMOT_VARIANT7_SMALL_W, MARMOT_VARIANT7_SMALL_H,
     MARMOT_VARIANT7_SMALL_H},
    {MARMOT_VARIANT8_SMALL_BITMAP, MARMOT_VARIANT8_SMALL_W, MARMOT_VARIANT8_SMALL_H,
     MARMOT_VARIANT8_SMALL_H},
    {MARMOT_VARIANT10_SMALL_BITMAP, MARMOT_VARIANT10_SMALL_W, MARMOT_VARIANT10_SMALL_H,
     MARMOT_VARIANT10_SMALL_H},
    {MARMOT_VARIANT11_SMALL_BITMAP, MARMOT_VARIANT11_SMALL_W, MARMOT_VARIANT11_SMALL_H,
     MARMOT_VARIANT11_SMALL_H},
    {MARMOT_VARIANT20_SMALL_BITMAP, MARMOT_VARIANT20_SMALL_W, MARMOT_VARIANT20_SMALL_H,
     MARMOT_VARIANT20_SMALL_H},
    {MARMOT_VARIANT27_SMALL_BITMAP, MARMOT_VARIANT27_SMALL_W, MARMOT_VARIANT27_SMALL_H,
     MARMOT_VARIANT27_SMALL_H},
    {MARMOT_VARIANT28_SMALL_BITMAP, MARMOT_VARIANT28_SMALL_W, MARMOT_VARIANT28_SMALL_H,
     MARMOT_VARIANT28_SMALL_H},
};

static const MarmotArt kMarmotHungrySmall[] = {
    {MARMOT_VARIANT13_SMALL_BITMAP, MARMOT_VARIANT13_SMALL_W, MARMOT_VARIANT13_SMALL_H,
     MARMOT_VARIANT13_SMALL_H},
    {MARMOT_VARIANT21_SMALL_BITMAP, MARMOT_VARIANT21_SMALL_W, MARMOT_VARIANT21_SMALL_H,
     MARMOT_VARIANT21_SMALL_H},
    {MARMOT_VARIANT24_SMALL_BITMAP, MARMOT_VARIANT24_SMALL_W, MARMOT_VARIANT24_SMALL_H,
     MARMOT_VARIANT24_SMALL_H},
    {MARMOT_VARIANT25_SMALL_BITMAP, MARMOT_VARIANT25_SMALL_W, MARMOT_VARIANT25_SMALL_H,
     MARMOT_VARIANT25_SMALL_H},
};

static const MarmotArt kMarmotSleepySmall[] = {
    {MARMOT_VARIANT15_SMALL_BITMAP, MARMOT_VARIANT15_SMALL_W, MARMOT_VARIANT15_SMALL_H,
     MARMOT_VARIANT15_SMALL_H},
    {MARMOT_VARIANT22_SMALL_BITMAP, MARMOT_VARIANT22_SMALL_W, MARMOT_VARIANT22_SMALL_H,
     MARMOT_VARIANT22_SMALL_H},
};

static const MarmotArt kMarmotAnnoyedSmall[] = {
    {MARMOT_VARIANT16_SMALL_BITMAP, MARMOT_VARIANT16_SMALL_W, MARMOT_VARIANT16_SMALL_H,
     MARMOT_VARIANT16_SMALL_H},
    {MARMOT_VARIANT26_SMALL_BITMAP, MARMOT_VARIANT26_SMALL_W, MARMOT_VARIANT26_SMALL_H,
     MARMOT_VARIANT26_SMALL_H},
};

// Dedicated Baby and Juvenile pose pools -- real hoary/yellow-bellied marmot
// pup photos found on a wider Commons sweep (zoo/park and family shots turned
// out to have much cleaner backgrounds than the wild adult shots this
// project mostly draws from), each rotated randomly per wake exactly like
// the adult pool below.
static const MarmotArt kBabyVariants[] = {
    {MARMOT_BABY_BITMAP, MARMOT_BABY_W, MARMOT_BABY_H, MARMOT_BABY_H},
    {MARMOT_BABY2_BITMAP, MARMOT_BABY2_W, MARMOT_BABY2_H, MARMOT_BABY2_H},
    {MARMOT_BABY4_BITMAP, MARMOT_BABY4_W, MARMOT_BABY4_H, MARMOT_BABY4_H},
    {MARMOT_BABY6_BITMAP, MARMOT_BABY6_W, MARMOT_BABY6_H, MARMOT_BABY6_H},
};
static const int kBabyVariantCount = sizeof(kBabyVariants) / sizeof(kBabyVariants[0]);

static const MarmotArt kJuvenileVariants[] = {
    {MARMOT_JUVENILE2_BITMAP, MARMOT_JUVENILE2_W, MARMOT_JUVENILE2_H, MARMOT_JUVENILE2_H},
    {MARMOT_JUVENILE3_BITMAP, MARMOT_JUVENILE3_W, MARMOT_JUVENILE3_H, MARMOT_JUVENILE3_H},
    {MARMOT_BABY7_BITMAP, MARMOT_BABY7_W, MARMOT_BABY7_H, MARMOT_BABY7_H},
};
static const int kJuvenileVariantCount = sizeof(kJuvenileVariants) / sizeof(kJuvenileVariants[0]);

static const MarmotArt kBabyVariantsSmall[] = {
    {MARMOT_BABY_SMALL_BITMAP, MARMOT_BABY_SMALL_W, MARMOT_BABY_SMALL_H, MARMOT_BABY_SMALL_H},
    {MARMOT_BABY2_SMALL_BITMAP, MARMOT_BABY2_SMALL_W, MARMOT_BABY2_SMALL_H, MARMOT_BABY2_SMALL_H},
    {MARMOT_BABY4_SMALL_BITMAP, MARMOT_BABY4_SMALL_W, MARMOT_BABY4_SMALL_H, MARMOT_BABY4_SMALL_H},
    {MARMOT_BABY6_SMALL_BITMAP, MARMOT_BABY6_SMALL_W, MARMOT_BABY6_SMALL_H, MARMOT_BABY6_SMALL_H},
};

static const MarmotArt kJuvenileVariantsSmall[] = {
    {MARMOT_JUVENILE2_SMALL_BITMAP, MARMOT_JUVENILE2_SMALL_W, MARMOT_JUVENILE2_SMALL_H,
     MARMOT_JUVENILE2_SMALL_H},
    {MARMOT_JUVENILE3_SMALL_BITMAP, MARMOT_JUVENILE3_SMALL_W, MARMOT_JUVENILE3_SMALL_H,
     MARMOT_JUVENILE3_SMALL_H},
    {MARMOT_BABY7_SMALL_BITMAP, MARMOT_BABY7_SMALL_W, MARMOT_BABY7_SMALL_H,
     MARMOT_BABY7_SMALL_H},
};
