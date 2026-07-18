// sprites.h — shared sprite IDs so the foraging DB and the renderer agree
// on which botanical illustration to draw for a species.
#pragma once

#include <stdint.h>

enum SpriteId : uint8_t {
  SPR_MUSHROOM_CAP = 0,  // generic gilled mushroom (chanterelle, oyster…)
  SPR_MUSHROOM_TOOTH,    // hedgehog / toothed
  SPR_MOREL,             // honeycomb morel
  SPR_MATSUTAKE,         // stout pine mushroom
  SPR_PORCINI,           // fat-stemmed bolete
  SPR_CORAL,             // cauliflower / coral mushroom
  SPR_LEAFY_GREEN,       // miner's lettuce / chickweed / sorrel
  SPR_NETTLE,            // serrated nettle leaf
  SPR_FIDDLEHEAD,        // coiled fern
  SPR_RAMP,              // ramps / wild leek
  SPR_FLOWER_CLUSTER,    // elderflower umbel
  SPR_BERRY_CLUSTER,     // salmonberry / huckleberry / blackberry
  SPR_COUNT
};
