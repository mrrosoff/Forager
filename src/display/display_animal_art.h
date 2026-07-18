// display_animal_art.h — real dithered animal-photo bitmaps for the
// AnimalSighting encounter screen. Index order must match kAnimals[] in
// events.cpp (see events.h's animalIndex() doc comment). Pure data, split
// out of display.cpp; only included from there, after the animal bitmap
// headers it references.
#pragma once

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
