// Generated index tying together every per-species bitmap header in this
// directory -- see scratchpad species/batch_species.py + pack_species.py.
// Species without a photo bitmap here fall back to the procedural
// drawSprite() icon in display.cpp. Include ONLY this file, not the
// individual per-species headers, to pull in species art.
#pragma once

#include <cstring>

#include "admirable_bolete_bitmap.h"
#include "alpine_blueberry_bitmap.h"
#include "alpine_sorrel_bitmap.h"
#include "alpine_strawberry_bitmap.h"
#include "amber_jelly_roll_bitmap.h"
#include "american_sea_rocket_bitmap.h"
#include "american_searocket_bitmap.h"
#include "angel_s_wing_bolete__suillus_lakei_bitmap.h"
#include "angel_wings_bitmap.h"
#include "artist_s_conk_bitmap.h"
#include "aspen_bolete_bitmap.h"
#include "basket_cockle_bitmap.h"
#include "bay_mussel_bitmap.h"
#include "beach_asparagus_bitmap.h"
#include "beach_lovage_bitmap.h"
#include "beach_pea_bitmap.h"
#include "beach_sandwort_bitmap.h"
#include "beach_strawberry_bitmap.h"
#include "beaked_hazelnut_bitmap.h"
#include "bear_s_head_tooth_bitmap.h"
#include "bigleaf_maple_sap_bitmap.h"
#include "birch_polypore_bitmap.h"
#include "black_gooseberry_bitmap.h"
#include "black_huckleberry_bitmap.h"
#include "black_katy_chiton_bitmap.h"
#include "black_raspberry_bitmap.h"
#include "black_trumpet_bitmap.h"
#include "black_witch_s_butter_bitmap.h"
#include "blackberry_bitmap.h"
#include "bladderwrack_bitmap.h"
#include "blue_chanterelle_bitmap.h"
#include "blue_elderberry_bitmap.h"
#include "blue_elderberry_cluster_bitmap.h"
#include "blue_top_snail_bitmap.h"
#include "boletus_edulis__king_bolete_flush_bitmap.h"
#include "bracken_fern_fiddlehead_bitmap.h"
#include "bull_kelp_bitmap.h"
#include "bunchberry_bitmap.h"
#include "butter_bolete_bitmap.h"
#include "butter_clam_bitmap.h"
#include "california_mussel_bitmap.h"
#include "cascade_blueberry_bitmap.h"
#include "cattail_pollen_bitmap.h"
#include "cattail_shoots_bitmap.h"
#include "cauliflower_coral_bitmap.h"
#include "cauliflower_fungus_bitmap.h"
#include "chanterelle_bitmap.h"
#include "chicken_fat_suillus_bitmap.h"
#include "chicken_of_the_woods_bitmap.h"
#include "chickweed_bitmap.h"
#include "cinnabar_polypore_bitmap.h"
#include "cleavers_bitmap.h"
#include "common_yarrow_bitmap.h"
#include "coral_tooth_fungus_bitmap.h"
#include "cortinarius__deadly_webcap_bitmap.h"
#include "cottonwood_buds_bitmap.h"
#include "cow_parsnip_bitmap.h"
#include "cow_parsnip_shoots_bitmap.h"
#include "crown_tipped_coral_bitmap.h"
#include "curly_dock_bitmap.h"
#include "dandelion_bitmap.h"
#include "death_cap_bitmap.h"
#include "deer_fern_fiddlehead_bitmap.h"
#include "deer_mushroom_bitmap.h"
#include "delicious_milk_cap_bitmap.h"
#include "destroying_angel_bitmap.h"
#include "destroying_angel_s_cousin__panther_cap_bitmap.h"
#include "devil_s_club_shoots_bitmap.h"
#include "douglas_aster_bitmap.h"
#include "douglas_fir_tips_bitmap.h"
#include "dryads_saddle_bitmap.h"
#include "dulse_bitmap.h"
#include "dungeness_crab_bitmap.h"
#include "dyer_s_polypore_bitmap.h"
#include "elderflower_bitmap.h"
#include "elm_oyster_bitmap.h"
#include "engelmann_spruce_tips_bitmap.h"
#include "evergreen_huckleberry_bitmap.h"
#include "fairy_ring_mushroom_bitmap.h"
#include "false_turkey_tail_bitmap.h"
#include "feather_boa_kelp_bitmap.h"
#include "fiddlehead_fern_bitmap.h"
#include "fireweed_bitmap.h"
#include "fireweed_flowers_bitmap.h"
#include "fireweed_shoots_bitmap.h"
#include "fly_agaric_bitmap.h"
#include "fried_chicken_mushroom_bitmap.h"
#include "fried_chicken_mushroom__lyophyllum_decastes_bitmap.h"
#include "garry_oak_acorn_bitmap.h"
#include "gem_studded_puffball_bitmap.h"
#include "geoduck_bitmap.h"
#include "giant_kelp_bitmap.h"
#include "giant_red_sea_cucumber_bitmap.h"
#include "golden_chanterelle_bitmap.h"
#include "gooseneck_barnacle_bitmap.h"
#include "goosetongue_bitmap.h"
#include "graceful_crab_bitmap.h"
#include "grand_fir_tips_bitmap.h"
#include "grouse_whortleberry_bitmap.h"
#include "gumboot_chiton_bitmap.h"
#include "hawk_s_wing_bitmap.h"
#include "hedgehog_mushroom_bitmap.h"
#include "hen_of_the_woods_bitmap.h"
#include "honey_mushroom_bitmap.h"
#include "horse_clam_bitmap.h"
#include "horse_mushroom_bitmap.h"
#include "huckleberry_bitmap.h"
#include "indian_plum_bitmap.h"
#include "indian_rhubarb_bitmap.h"
#include "indigo_milk_cap_bitmap.h"
#include "jack_o__lantern_mushroom_bitmap.h"
#include "jelly_tooth_bitmap.h"
#include "juniper_berries_bitmap.h"
#include "kelp_crab_bitmap.h"
#include "king_bolete_button_bitmap.h"
#include "kinnikinnick_bitmap.h"
#include "kinnikinnick_berries_bitmap.h"
#include "labrador_tea_bitmap.h"
#include "lady_fern_fiddlehead_bitmap.h"
#include "limpet_bitmap.h"
#include "lobster_mushroom_bitmap.h"
#include "lodgepole_pine_cambium_bitmap.h"
#include "man_on_horseback_bitmap.h"
#include "manila_clam_bitmap.h"
#include "manila_littleneck_clam_bitmap.h"
#include "matsutake_bitmap.h"
#include "meadow_mushroom_bitmap.h"
#include "miner_s_lettuce_bitmap.h"
#include "moon_snail_bitmap.h"
#include "morel_bitmap.h"
#include "mountain_ash_berries_bitmap.h"
#include "mountain_cranberry_bitmap.h"
#include "mountain_sorrel_leaves_bitmap.h"
#include "mountain_strawberry_bitmap.h"
#include "nettle_tips_bitmap.h"
#include "nori__laver_bitmap.h"
#include "oarweed_bitmap.h"
#include "old_man_of_the_woods_bitmap.h"
#include "olympia_oyster_bitmap.h"
#include "orache_bitmap.h"
#include "orange_peel_fungus_bitmap.h"
#include "oregon_grape_bitmap.h"
#include "oregon_white_truffle_bitmap.h"
#include "oxeye_daisy_bitmap.h"
#include "oyster_mushroom_bitmap.h"
#include "pacific_basket_cockle_bitmap.h"
#include "pacific_gaper_clam_bitmap.h"
#include "pacific_littleneck_clam_bitmap.h"
#include "pacific_oyster_bitmap.h"
#include "partridgefoot_greens_bitmap.h"
#include "pear_shaped_puffball_bitmap.h"
#include "pearly_everlasting_bitmap.h"
#include "pig_s_ear__violet_chanterelle_bitmap.h"
#include "pig_s_ears_bitmap.h"
#include "pigskin_poison_puffball_bitmap.h"
#include "pink_coral_fungus_bitmap.h"
#include "pinto_abalone_bitmap.h"
#include "pipsissewa_bitmap.h"
#include "plantain_bitmap.h"
#include "porcini_bitmap.h"
#include "puffball__giant_bitmap.h"
#include "purple_sea_urchin_bitmap.h"
#include "purple_varnish_clam_bitmap.h"
#include "ramps_bitmap.h"
#include "raspberry_bitmap.h"
#include "razor_clam_bitmap.h"
#include "red_cracked_bolete_bitmap.h"
#include "red_currant_bitmap.h"
#include "red_elderberry_bitmap.h"
#include "red_flowering_currant_bitmap.h"
#include "red_flowering_currant_blossom_bitmap.h"
#include "red_huckleberry_bitmap.h"
#include "red_rock_crab_bitmap.h"
#include "red_sea_urchin_bitmap.h"
#include "reishi_hemlock_varnish_shelf_bitmap.h"
#include "rockweed_bitmap.h"
#include "rooting_shank_bitmap.h"
#include "saffron_milk_cap_bitmap.h"
#include "salal_bitmap.h"
#include "salmonberry_bitmap.h"
#include "salmonberry_shoots_bitmap.h"
#include "scurvy_grass_bitmap.h"
#include "sea_asparagus_bitmap.h"
#include "sea_beach_sandwort_bitmap.h"
#include "sea_cabbage_bitmap.h"
#include "sea_coast_angelica_bitmap.h"
#include "sea_hair_bitmap.h"
#include "sea_lettuce_bitmap.h"
#include "sea_palm_bitmap.h"
#include "sea_sac_bitmap.h"
#include "self_heal_bitmap.h"
#include "serviceberry_bitmap.h"
#include "shaggy_mane_bitmap.h"
#include "shaggy_parasol_bitmap.h"
#include "shaggy_scalycap_bitmap.h"
#include "sheep_sorrel_bitmap.h"
#include "short_stemmed_slippery_jack_bitmap.h"
#include "shrimp_russula_bitmap.h"
#include "silverweed_bitmap.h"
#include "sitka_mountain_ash_berries_bitmap.h"
#include "slippery_jack_bitmap.h"
#include "snow_bank_false_morel_bitmap.h"
#include "soapberry_bitmap.h"
#include "softshell_clam_bitmap.h"
#include "split_gill_bitmap.h"
#include "spot_prawn_bitmap.h"
#include "stinging_nettle_bitmap.h"
#include "subalpine_daisy_bitmap.h"
#include "subalpine_fir_tips_bitmap.h"
#include "subalpine_fireweed_bitmap.h"
#include "subalpine_puffball_bitmap.h"
#include "sugar_kelp_bitmap.h"
#include "swamp_onion_bitmap.h"
#include "sword_fern_bitmap.h"
#include "the_prince_bitmap.h"
#include "thimbleberry_bitmap.h"
#include "trailing_blackberry_bitmap.h"
#include "turkey_tail_bitmap.h"
#include "turkish_towel_bitmap.h"
#include "turkish_washcloth_bitmap.h"
#include "twisted_stalk_bitmap.h"
#include "varnish_clam_bitmap.h"
#include "velvet_shank_bitmap.h"
#include "watercress_bitmap.h"
#include "wax_currant_bitmap.h"
#include "western_giant_puffball_bitmap.h"
#include "western_hemlock_cambium_bitmap.h"
#include "western_larch_gum_bitmap.h"
#include "western_painted_suillus_bitmap.h"
#include "western_white_pine_nuts_bitmap.h"
#include "white_chanterelle_bitmap.h"
#include "whitebark_pine_nuts_bitmap.h"
#include "wild_blueberry_bitmap.h"
#include "wild_gooseberry_bitmap.h"
#include "wild_mint_bitmap.h"
#include "wild_rose_hips_bitmap.h"
#include "wild_strawberry_bitmap.h"
#include "wine_cap_mushroom_bitmap.h"
#include "winged_kelp_bitmap.h"
#include "witch_s_butter_bitmap.h"
#include "wood_blewit_bitmap.h"
#include "wood_ear_bitmap.h"
#include "wood_sorrel_bitmap.h"
#include "woodland_strawberry_bitmap.h"
#include "woolly_chanterelle_bitmap.h"
#include "woolly_chanterelle__scaly_vase_bitmap.h"
#include "yarrow_bitmap.h"
#include "yellow_salsify_bitmap.h"
#include "yellowfoot_bitmap.h"
#include "zeller_s_bolete_bitmap.h"

namespace species_bitmaps {

struct SpeciesBitmap {
  const char* name;
  const uint8_t* bitmap;
  uint8_t w, h;
};

static const SpeciesBitmap kSpeciesBitmaps[] = {
    {"Hedgehog Mushroom", SPECIES_HEDGEHOG_MUSHROOM_BITMAP, SPECIES_HEDGEHOG_MUSHROOM_W, SPECIES_HEDGEHOG_MUSHROOM_H},
    {"Miner's Lettuce", SPECIES_MINER_S_LETTUCE_BITMAP, SPECIES_MINER_S_LETTUCE_W, SPECIES_MINER_S_LETTUCE_H},
    {"Stinging Nettle", SPECIES_STINGING_NETTLE_BITMAP, SPECIES_STINGING_NETTLE_W, SPECIES_STINGING_NETTLE_H},
    {"Oyster Mushroom", SPECIES_OYSTER_MUSHROOM_BITMAP, SPECIES_OYSTER_MUSHROOM_W, SPECIES_OYSTER_MUSHROOM_H},
    {"Morel", SPECIES_MOREL_BITMAP, SPECIES_MOREL_W, SPECIES_MOREL_H},
    {"Chickweed", SPECIES_CHICKWEED_BITMAP, SPECIES_CHICKWEED_W, SPECIES_CHICKWEED_H},
    {"Ramps", SPECIES_RAMPS_BITMAP, SPECIES_RAMPS_W, SPECIES_RAMPS_H},
    {"Fiddlehead Fern", SPECIES_FIDDLEHEAD_FERN_BITMAP, SPECIES_FIDDLEHEAD_FERN_W, SPECIES_FIDDLEHEAD_FERN_H},
    {"Elderflower", SPECIES_ELDERFLOWER_BITMAP, SPECIES_ELDERFLOWER_W, SPECIES_ELDERFLOWER_H},
    {"Wood Sorrel", SPECIES_WOOD_SORREL_BITMAP, SPECIES_WOOD_SORREL_W, SPECIES_WOOD_SORREL_H},
    {"Salmonberry", SPECIES_SALMONBERRY_BITMAP, SPECIES_SALMONBERRY_W, SPECIES_SALMONBERRY_H},
    {"Chanterelle", SPECIES_CHANTERELLE_BITMAP, SPECIES_CHANTERELLE_W, SPECIES_CHANTERELLE_H},
    {"Blackberry", SPECIES_BLACKBERRY_BITMAP, SPECIES_BLACKBERRY_W, SPECIES_BLACKBERRY_H},
    {"Matsutake", SPECIES_MATSUTAKE_BITMAP, SPECIES_MATSUTAKE_W, SPECIES_MATSUTAKE_H},
    {"Lobster Mushroom", SPECIES_LOBSTER_MUSHROOM_BITMAP, SPECIES_LOBSTER_MUSHROOM_W, SPECIES_LOBSTER_MUSHROOM_H},
    {"Porcini", SPECIES_PORCINI_BITMAP, SPECIES_PORCINI_W, SPECIES_PORCINI_H},
    {"Yellowfoot", SPECIES_YELLOWFOOT_BITMAP, SPECIES_YELLOWFOOT_W, SPECIES_YELLOWFOOT_H},
    {"Cauliflower Fungus", SPECIES_CAULIFLOWER_FUNGUS_BITMAP, SPECIES_CAULIFLOWER_FUNGUS_W, SPECIES_CAULIFLOWER_FUNGUS_H},
    {"Velvet Shank", SPECIES_VELVET_SHANK_BITMAP, SPECIES_VELVET_SHANK_W, SPECIES_VELVET_SHANK_H},
    {"Huckleberry", SPECIES_HUCKLEBERRY_BITMAP, SPECIES_HUCKLEBERRY_W, SPECIES_HUCKLEBERRY_H},
    {"Raspberry", SPECIES_RASPBERRY_BITMAP, SPECIES_RASPBERRY_W, SPECIES_RASPBERRY_H},
    {"Mountain Cranberry", SPECIES_MOUNTAIN_CRANBERRY_BITMAP, SPECIES_MOUNTAIN_CRANBERRY_W, SPECIES_MOUNTAIN_CRANBERRY_H},
    {"Alpine Sorrel", SPECIES_ALPINE_SORREL_BITMAP, SPECIES_ALPINE_SORREL_W, SPECIES_ALPINE_SORREL_H},
    {"Devil's Club Shoots", SPECIES_DEVIL_S_CLUB_SHOOTS_BITMAP, SPECIES_DEVIL_S_CLUB_SHOOTS_W, SPECIES_DEVIL_S_CLUB_SHOOTS_H},
    {"Subalpine Puffball", SPECIES_SUBALPINE_PUFFBALL_BITMAP, SPECIES_SUBALPINE_PUFFBALL_W, SPECIES_SUBALPINE_PUFFBALL_H},
    {"Swamp Onion", SPECIES_SWAMP_ONION_BITMAP, SPECIES_SWAMP_ONION_W, SPECIES_SWAMP_ONION_H},
    {"Douglas Fir Tips", SPECIES_DOUGLAS_FIR_TIPS_BITMAP, SPECIES_DOUGLAS_FIR_TIPS_W, SPECIES_DOUGLAS_FIR_TIPS_H},
    {"Bull Kelp", SPECIES_BULL_KELP_BITMAP, SPECIES_BULL_KELP_W, SPECIES_BULL_KELP_H},
    {"Sugar Kelp", SPECIES_SUGAR_KELP_BITMAP, SPECIES_SUGAR_KELP_W, SPECIES_SUGAR_KELP_H},
    {"Sea Lettuce", SPECIES_SEA_LETTUCE_BITMAP, SPECIES_SEA_LETTUCE_W, SPECIES_SEA_LETTUCE_H},
    {"Dulse", SPECIES_DULSE_BITMAP, SPECIES_DULSE_W, SPECIES_DULSE_H},
    {"Sea Asparagus", SPECIES_SEA_ASPARAGUS_BITMAP, SPECIES_SEA_ASPARAGUS_W, SPECIES_SEA_ASPARAGUS_H},
    {"Goosetongue", SPECIES_GOOSETONGUE_BITMAP, SPECIES_GOOSETONGUE_W, SPECIES_GOOSETONGUE_H},
    {"Silverweed", SPECIES_SILVERWEED_BITMAP, SPECIES_SILVERWEED_W, SPECIES_SILVERWEED_H},
    {"Beach Strawberry", SPECIES_BEACH_STRAWBERRY_BITMAP, SPECIES_BEACH_STRAWBERRY_W, SPECIES_BEACH_STRAWBERRY_H},
    {"Beach Pea", SPECIES_BEACH_PEA_BITMAP, SPECIES_BEACH_PEA_W, SPECIES_BEACH_PEA_H},
    {"Wild Rose Hips", SPECIES_WILD_ROSE_HIPS_BITMAP, SPECIES_WILD_ROSE_HIPS_W, SPECIES_WILD_ROSE_HIPS_H},
    {"Razor Clam", SPECIES_RAZOR_CLAM_BITMAP, SPECIES_RAZOR_CLAM_W, SPECIES_RAZOR_CLAM_H},
    {"Shaggy Mane", SPECIES_SHAGGY_MANE_BITMAP, SPECIES_SHAGGY_MANE_W, SPECIES_SHAGGY_MANE_H},
    {"Death Cap", SPECIES_DEATH_CAP_BITMAP, SPECIES_DEATH_CAP_W, SPECIES_DEATH_CAP_H},
    {"Destroying Angel", SPECIES_DESTROYING_ANGEL_BITMAP, SPECIES_DESTROYING_ANGEL_W, SPECIES_DESTROYING_ANGEL_H},
    {"Fly Agaric", SPECIES_FLY_AGARIC_BITMAP, SPECIES_FLY_AGARIC_W, SPECIES_FLY_AGARIC_H},
    {"Turkey Tail", SPECIES_TURKEY_TAIL_BITMAP, SPECIES_TURKEY_TAIL_W, SPECIES_TURKEY_TAIL_H},
    {"Artist's Conk", SPECIES_ARTIST_S_CONK_BITMAP, SPECIES_ARTIST_S_CONK_W, SPECIES_ARTIST_S_CONK_H},
    {"Chicken of the Woods", SPECIES_CHICKEN_OF_THE_WOODS_BITMAP, SPECIES_CHICKEN_OF_THE_WOODS_W, SPECIES_CHICKEN_OF_THE_WOODS_H},
    {"Hen of the Woods", SPECIES_HEN_OF_THE_WOODS_BITMAP, SPECIES_HEN_OF_THE_WOODS_W, SPECIES_HEN_OF_THE_WOODS_H},
    {"Shrimp Russula", SPECIES_SHRIMP_RUSSULA_BITMAP, SPECIES_SHRIMP_RUSSULA_W, SPECIES_SHRIMP_RUSSULA_H},
    {"Cauliflower Coral", SPECIES_CAULIFLOWER_CORAL_BITMAP, SPECIES_CAULIFLOWER_CORAL_W, SPECIES_CAULIFLOWER_CORAL_H},
    {"Golden Chanterelle", SPECIES_GOLDEN_CHANTERELLE_BITMAP, SPECIES_GOLDEN_CHANTERELLE_W, SPECIES_GOLDEN_CHANTERELLE_H},
    {"White Chanterelle", SPECIES_WHITE_CHANTERELLE_BITMAP, SPECIES_WHITE_CHANTERELLE_W, SPECIES_WHITE_CHANTERELLE_H},
    {"Jack-o'-Lantern Mushroom", SPECIES_JACK_O__LANTERN_MUSHROOM_BITMAP, SPECIES_JACK_O__LANTERN_MUSHROOM_W, SPECIES_JACK_O__LANTERN_MUSHROOM_H},
    {"Cortinarius (Deadly Webcap)", SPECIES_CORTINARIUS__DEADLY_WEBCAP_BITMAP, SPECIES_CORTINARIUS__DEADLY_WEBCAP_W, SPECIES_CORTINARIUS__DEADLY_WEBCAP_H},
    {"Angel Wings", SPECIES_ANGEL_WINGS_BITMAP, SPECIES_ANGEL_WINGS_W, SPECIES_ANGEL_WINGS_H},
    {"Puffball (Giant)", SPECIES_PUFFBALL__GIANT_BITMAP, SPECIES_PUFFBALL__GIANT_W, SPECIES_PUFFBALL__GIANT_H},
    {"Pig's Ear (Violet Chanterelle)", SPECIES_PIG_S_EAR__VIOLET_CHANTERELLE_BITMAP, SPECIES_PIG_S_EAR__VIOLET_CHANTERELLE_W, SPECIES_PIG_S_EAR__VIOLET_CHANTERELLE_H},
    {"Black Trumpet", SPECIES_BLACK_TRUMPET_BITMAP, SPECIES_BLACK_TRUMPET_W, SPECIES_BLACK_TRUMPET_H},
    {"Admirable Bolete", SPECIES_ADMIRABLE_BOLETE_BITMAP, SPECIES_ADMIRABLE_BOLETE_W, SPECIES_ADMIRABLE_BOLETE_H},
    {"Slippery Jack", SPECIES_SLIPPERY_JACK_BITMAP, SPECIES_SLIPPERY_JACK_W, SPECIES_SLIPPERY_JACK_H},
    {"Short-Stemmed Slippery Jack", SPECIES_SHORT_STEMMED_SLIPPERY_JACK_BITMAP, SPECIES_SHORT_STEMMED_SLIPPERY_JACK_W, SPECIES_SHORT_STEMMED_SLIPPERY_JACK_H},
    {"Boletus edulis (King Bolete flush)", SPECIES_BOLETUS_EDULIS__KING_BOLETE_FLUSH_BITMAP, SPECIES_BOLETUS_EDULIS__KING_BOLETE_FLUSH_W, SPECIES_BOLETUS_EDULIS__KING_BOLETE_FLUSH_H},
    {"Angel's Wing Bolete (Suillus lakei)", SPECIES_ANGEL_S_WING_BOLETE__SUILLUS_LAKEI_BITMAP, SPECIES_ANGEL_S_WING_BOLETE__SUILLUS_LAKEI_W, SPECIES_ANGEL_S_WING_BOLETE__SUILLUS_LAKEI_H},
    {"Meadow Mushroom", SPECIES_MEADOW_MUSHROOM_BITMAP, SPECIES_MEADOW_MUSHROOM_W, SPECIES_MEADOW_MUSHROOM_H},
    {"Horse Mushroom", SPECIES_HORSE_MUSHROOM_BITMAP, SPECIES_HORSE_MUSHROOM_W, SPECIES_HORSE_MUSHROOM_H},
    {"Destroying Angel's cousin (Panther Cap)", SPECIES_DESTROYING_ANGEL_S_COUSIN__PANTHER_CAP_BITMAP, SPECIES_DESTROYING_ANGEL_S_COUSIN__PANTHER_CAP_W, SPECIES_DESTROYING_ANGEL_S_COUSIN__PANTHER_CAP_H},
    {"Wood Blewit", SPECIES_WOOD_BLEWIT_BITMAP, SPECIES_WOOD_BLEWIT_W, SPECIES_WOOD_BLEWIT_H},
    {"Fried Chicken Mushroom (Lyophyllum decastes)", SPECIES_FRIED_CHICKEN_MUSHROOM__LYOPHYLLUM_DECASTES_BITMAP, SPECIES_FRIED_CHICKEN_MUSHROOM__LYOPHYLLUM_DECASTES_W, SPECIES_FRIED_CHICKEN_MUSHROOM__LYOPHYLLUM_DECASTES_H},
    {"Man on Horseback", SPECIES_MAN_ON_HORSEBACK_BITMAP, SPECIES_MAN_ON_HORSEBACK_W, SPECIES_MAN_ON_HORSEBACK_H},
    {"Deer Mushroom", SPECIES_DEER_MUSHROOM_BITMAP, SPECIES_DEER_MUSHROOM_W, SPECIES_DEER_MUSHROOM_H},
    {"Woolly Chanterelle (Scaly Vase)", SPECIES_WOOLLY_CHANTERELLE__SCALY_VASE_BITMAP, SPECIES_WOOLLY_CHANTERELLE__SCALY_VASE_W, SPECIES_WOOLLY_CHANTERELLE__SCALY_VASE_H},
    {"Indigo Milk Cap", SPECIES_INDIGO_MILK_CAP_BITMAP, SPECIES_INDIGO_MILK_CAP_W, SPECIES_INDIGO_MILK_CAP_H},
    {"Old Man of the Woods", SPECIES_OLD_MAN_OF_THE_WOODS_BITMAP, SPECIES_OLD_MAN_OF_THE_WOODS_W, SPECIES_OLD_MAN_OF_THE_WOODS_H},
    {"Western Giant Puffball", SPECIES_WESTERN_GIANT_PUFFBALL_BITMAP, SPECIES_WESTERN_GIANT_PUFFBALL_W, SPECIES_WESTERN_GIANT_PUFFBALL_H},
    {"Larch Bolete", SPECIES_LARCH_BOLETE_BITMAP, SPECIES_LARCH_BOLETE_W, SPECIES_LARCH_BOLETE_H},
    {"Witch's Butter", SPECIES_WITCH_S_BUTTER_BITMAP, SPECIES_WITCH_S_BUTTER_W, SPECIES_WITCH_S_BUTTER_H},
    {"Oregon White Truffle", SPECIES_OREGON_WHITE_TRUFFLE_BITMAP, SPECIES_OREGON_WHITE_TRUFFLE_W, SPECIES_OREGON_WHITE_TRUFFLE_H},
    {"Thimbleberry", SPECIES_THIMBLEBERRY_BITMAP, SPECIES_THIMBLEBERRY_W, SPECIES_THIMBLEBERRY_H},
    {"Salal", SPECIES_SALAL_BITMAP, SPECIES_SALAL_W, SPECIES_SALAL_H},
    {"Red Huckleberry", SPECIES_RED_HUCKLEBERRY_BITMAP, SPECIES_RED_HUCKLEBERRY_W, SPECIES_RED_HUCKLEBERRY_H},
    {"Blue Elderberry", SPECIES_BLUE_ELDERBERRY_BITMAP, SPECIES_BLUE_ELDERBERRY_W, SPECIES_BLUE_ELDERBERRY_H},
    {"Serviceberry", SPECIES_SERVICEBERRY_BITMAP, SPECIES_SERVICEBERRY_W, SPECIES_SERVICEBERRY_H},
    {"Kinnikinnick", SPECIES_KINNIKINNICK_BITMAP, SPECIES_KINNIKINNICK_W, SPECIES_KINNIKINNICK_H},
    {"Trailing Blackberry", SPECIES_TRAILING_BLACKBERRY_BITMAP, SPECIES_TRAILING_BLACKBERRY_W, SPECIES_TRAILING_BLACKBERRY_H},
    {"Indian Plum", SPECIES_INDIAN_PLUM_BITMAP, SPECIES_INDIAN_PLUM_W, SPECIES_INDIAN_PLUM_H},
    {"Wild Strawberry", SPECIES_WILD_STRAWBERRY_BITMAP, SPECIES_WILD_STRAWBERRY_W, SPECIES_WILD_STRAWBERRY_H},
    {"Bunchberry", SPECIES_BUNCHBERRY_BITMAP, SPECIES_BUNCHBERRY_W, SPECIES_BUNCHBERRY_H},
    {"Wine Cap Mushroom", SPECIES_WINE_CAP_MUSHROOM_BITMAP, SPECIES_WINE_CAP_MUSHROOM_W, SPECIES_WINE_CAP_MUSHROOM_H},
    {"Sheep Sorrel", SPECIES_SHEEP_SORREL_BITMAP, SPECIES_SHEEP_SORREL_W, SPECIES_SHEEP_SORREL_H},
    {"Curly Dock", SPECIES_CURLY_DOCK_BITMAP, SPECIES_CURLY_DOCK_W, SPECIES_CURLY_DOCK_H},
    {"Cleavers", SPECIES_CLEAVERS_BITMAP, SPECIES_CLEAVERS_W, SPECIES_CLEAVERS_H},
    {"Dandelion", SPECIES_DANDELION_BITMAP, SPECIES_DANDELION_W, SPECIES_DANDELION_H},
    {"Plantain", SPECIES_PLANTAIN_BITMAP, SPECIES_PLANTAIN_W, SPECIES_PLANTAIN_H},
    {"Watercress", SPECIES_WATERCRESS_BITMAP, SPECIES_WATERCRESS_W, SPECIES_WATERCRESS_H},
    {"Wild Mint", SPECIES_WILD_MINT_BITMAP, SPECIES_WILD_MINT_W, SPECIES_WILD_MINT_H},
    {"Self-heal", SPECIES_SELF_HEAL_BITMAP, SPECIES_SELF_HEAL_W, SPECIES_SELF_HEAL_H},
    {"Cow Parsnip", SPECIES_COW_PARSNIP_BITMAP, SPECIES_COW_PARSNIP_W, SPECIES_COW_PARSNIP_H},
    {"Salmonberry Shoots", SPECIES_SALMONBERRY_SHOOTS_BITMAP, SPECIES_SALMONBERRY_SHOOTS_W, SPECIES_SALMONBERRY_SHOOTS_H},
    {"Reishi (Hemlock Varnish Shelf)", SPECIES_REISHI_HEMLOCK_VARNISH_SHELF_BITMAP, SPECIES_REISHI_HEMLOCK_VARNISH_SHELF_W, SPECIES_REISHI_HEMLOCK_VARNISH_SHELF_H},
    {"Beech Mushroom", SPECIES_BEECH_MUSHROOM_BITMAP, SPECIES_BEECH_MUSHROOM_W, SPECIES_BEECH_MUSHROOM_H},
    {"Saffron Milk Cap", SPECIES_SAFFRON_MILK_CAP_BITMAP, SPECIES_SAFFRON_MILK_CAP_W, SPECIES_SAFFRON_MILK_CAP_H},
    {"Shaggy Parasol", SPECIES_SHAGGY_PARASOL_BITMAP, SPECIES_SHAGGY_PARASOL_W, SPECIES_SHAGGY_PARASOL_H},
    {"Douglas Aster", SPECIES_DOUGLAS_ASTER_BITMAP, SPECIES_DOUGLAS_ASTER_W, SPECIES_DOUGLAS_ASTER_H},
    {"Beaked Hazelnut", SPECIES_BEAKED_HAZELNUT_BITMAP, SPECIES_BEAKED_HAZELNUT_W, SPECIES_BEAKED_HAZELNUT_H},
    {"Garry Oak Acorn", SPECIES_GARRY_OAK_ACORN_BITMAP, SPECIES_GARRY_OAK_ACORN_W, SPECIES_GARRY_OAK_ACORN_H},
    {"Yellow Salsify", SPECIES_YELLOW_SALSIFY_BITMAP, SPECIES_YELLOW_SALSIFY_W, SPECIES_YELLOW_SALSIFY_H},
    {"Bigleaf Maple Sap", SPECIES_BIGLEAF_MAPLE_SAP_BITMAP, SPECIES_BIGLEAF_MAPLE_SAP_W, SPECIES_BIGLEAF_MAPLE_SAP_H},
    {"Cottonwood Buds", SPECIES_COTTONWOOD_BUDS_BITMAP, SPECIES_COTTONWOOD_BUDS_W, SPECIES_COTTONWOOD_BUDS_H},
    {"Red Flowering Currant Blossom", SPECIES_RED_FLOWERING_CURRANT_BLOSSOM_BITMAP, SPECIES_RED_FLOWERING_CURRANT_BLOSSOM_W, SPECIES_RED_FLOWERING_CURRANT_BLOSSOM_H},
    {"Nettle Tips", SPECIES_NETTLE_TIPS_BITMAP, SPECIES_NETTLE_TIPS_W, SPECIES_NETTLE_TIPS_H},
    {"Common Yarrow", SPECIES_COMMON_YARROW_BITMAP, SPECIES_COMMON_YARROW_W, SPECIES_COMMON_YARROW_H},
    {"Oxeye Daisy", SPECIES_OXEYE_DAISY_BITMAP, SPECIES_OXEYE_DAISY_W, SPECIES_OXEYE_DAISY_H},
    {"Wax Currant", SPECIES_WAX_CURRANT_BITMAP, SPECIES_WAX_CURRANT_W, SPECIES_WAX_CURRANT_H},
    {"Red-Flowering Currant", SPECIES_RED_FLOWERING_CURRANT_BITMAP, SPECIES_RED_FLOWERING_CURRANT_W, SPECIES_RED_FLOWERING_CURRANT_H},
    {"Oregon Grape", SPECIES_OREGON_GRAPE_BITMAP, SPECIES_OREGON_GRAPE_W, SPECIES_OREGON_GRAPE_H},
    {"Fireweed", SPECIES_FIREWEED_BITMAP, SPECIES_FIREWEED_W, SPECIES_FIREWEED_H},
    {"King Bolete Button", SPECIES_KING_BOLETE_BUTTON_BITMAP, SPECIES_KING_BOLETE_BUTTON_W, SPECIES_KING_BOLETE_BUTTON_H},
    {"Alpine Blueberry", SPECIES_ALPINE_BLUEBERRY_BITMAP, SPECIES_ALPINE_BLUEBERRY_W, SPECIES_ALPINE_BLUEBERRY_H},
    {"Subalpine Daisy", SPECIES_SUBALPINE_DAISY_BITMAP, SPECIES_SUBALPINE_DAISY_W, SPECIES_SUBALPINE_DAISY_H},
    {"Labrador Tea", SPECIES_LABRADOR_TEA_BITMAP, SPECIES_LABRADOR_TEA_W, SPECIES_LABRADOR_TEA_H},
    {"Yarrow", SPECIES_YARROW_BITMAP, SPECIES_YARROW_W, SPECIES_YARROW_H},
    {"Indian Rhubarb", SPECIES_INDIAN_RHUBARB_BITMAP, SPECIES_INDIAN_RHUBARB_W, SPECIES_INDIAN_RHUBARB_H},
    {"Aspen Bolete", SPECIES_ASPEN_BOLETE_BITMAP, SPECIES_ASPEN_BOLETE_W, SPECIES_ASPEN_BOLETE_H},
    {"Delicious Milk Cap", SPECIES_DELICIOUS_MILK_CAP_BITMAP, SPECIES_DELICIOUS_MILK_CAP_W, SPECIES_DELICIOUS_MILK_CAP_H},
    {"Hawk's Wing", SPECIES_HAWK_S_WING_BITMAP, SPECIES_HAWK_S_WING_W, SPECIES_HAWK_S_WING_H},
    {"Bear's Head Tooth", SPECIES_BEAR_S_HEAD_TOOTH_BITMAP, SPECIES_BEAR_S_HEAD_TOOTH_W, SPECIES_BEAR_S_HEAD_TOOTH_H},
    {"Alpine Strawberry", SPECIES_ALPINE_STRAWBERRY_BITMAP, SPECIES_ALPINE_STRAWBERRY_W, SPECIES_ALPINE_STRAWBERRY_H},
    {"Woodland Strawberry", SPECIES_WOODLAND_STRAWBERRY_BITMAP, SPECIES_WOODLAND_STRAWBERRY_W, SPECIES_WOODLAND_STRAWBERRY_H},
    {"Red Currant", SPECIES_RED_CURRANT_BITMAP, SPECIES_RED_CURRANT_W, SPECIES_RED_CURRANT_H},
    {"Grouse Whortleberry", SPECIES_GROUSE_WHORTLEBERRY_BITMAP, SPECIES_GROUSE_WHORTLEBERRY_W, SPECIES_GROUSE_WHORTLEBERRY_H},
    {"Black Raspberry", SPECIES_BLACK_RASPBERRY_BITMAP, SPECIES_BLACK_RASPBERRY_W, SPECIES_BLACK_RASPBERRY_H},
    {"Twisted Stalk", SPECIES_TWISTED_STALK_BITMAP, SPECIES_TWISTED_STALK_W, SPECIES_TWISTED_STALK_H},
    {"Soapberry", SPECIES_SOAPBERRY_BITMAP, SPECIES_SOAPBERRY_W, SPECIES_SOAPBERRY_H},
    {"Whitebark Pine Nuts", SPECIES_WHITEBARK_PINE_NUTS_BITMAP, SPECIES_WHITEBARK_PINE_NUTS_W, SPECIES_WHITEBARK_PINE_NUTS_H},
    {"Western White Pine Nuts", SPECIES_WESTERN_WHITE_PINE_NUTS_BITMAP, SPECIES_WESTERN_WHITE_PINE_NUTS_W, SPECIES_WESTERN_WHITE_PINE_NUTS_H},
    {"Lodgepole Pine Cambium", SPECIES_LODGEPOLE_PINE_CAMBIUM_BITMAP, SPECIES_LODGEPOLE_PINE_CAMBIUM_W, SPECIES_LODGEPOLE_PINE_CAMBIUM_H},
    {"Western Hemlock Cambium", SPECIES_WESTERN_HEMLOCK_CAMBIUM_BITMAP, SPECIES_WESTERN_HEMLOCK_CAMBIUM_W, SPECIES_WESTERN_HEMLOCK_CAMBIUM_H},
    {"Western Larch Gum", SPECIES_WESTERN_LARCH_GUM_BITMAP, SPECIES_WESTERN_LARCH_GUM_W, SPECIES_WESTERN_LARCH_GUM_H},
    {"Engelmann Spruce Tips", SPECIES_ENGELMANN_SPRUCE_TIPS_BITMAP, SPECIES_ENGELMANN_SPRUCE_TIPS_W, SPECIES_ENGELMANN_SPRUCE_TIPS_H},
    {"Subalpine Fir Tips", SPECIES_SUBALPINE_FIR_TIPS_BITMAP, SPECIES_SUBALPINE_FIR_TIPS_W, SPECIES_SUBALPINE_FIR_TIPS_H},
    {"Grand Fir Tips", SPECIES_GRAND_FIR_TIPS_BITMAP, SPECIES_GRAND_FIR_TIPS_W, SPECIES_GRAND_FIR_TIPS_H},
    {"Wild Gooseberry", SPECIES_WILD_GOOSEBERRY_BITMAP, SPECIES_WILD_GOOSEBERRY_W, SPECIES_WILD_GOOSEBERRY_H},
    {"Sitka Mountain Ash Berries", SPECIES_SITKA_MOUNTAIN_ASH_BERRIES_BITMAP, SPECIES_SITKA_MOUNTAIN_ASH_BERRIES_W, SPECIES_SITKA_MOUNTAIN_ASH_BERRIES_H},
    {"Cascade Blueberry", SPECIES_CASCADE_BLUEBERRY_BITMAP, SPECIES_CASCADE_BLUEBERRY_W, SPECIES_CASCADE_BLUEBERRY_H},
    {"Black Huckleberry", SPECIES_BLACK_HUCKLEBERRY_BITMAP, SPECIES_BLACK_HUCKLEBERRY_W, SPECIES_BLACK_HUCKLEBERRY_H},
    {"Mountain Sorrel Leaves", SPECIES_MOUNTAIN_SORREL_LEAVES_BITMAP, SPECIES_MOUNTAIN_SORREL_LEAVES_W, SPECIES_MOUNTAIN_SORREL_LEAVES_H},
    {"Subalpine Fireweed", SPECIES_SUBALPINE_FIREWEED_BITMAP, SPECIES_SUBALPINE_FIREWEED_W, SPECIES_SUBALPINE_FIREWEED_H},
    {"Wild Blueberry", SPECIES_WILD_BLUEBERRY_BITMAP, SPECIES_WILD_BLUEBERRY_W, SPECIES_WILD_BLUEBERRY_H},
    {"Black Gooseberry", SPECIES_BLACK_GOOSEBERRY_BITMAP, SPECIES_BLACK_GOOSEBERRY_W, SPECIES_BLACK_GOOSEBERRY_H},
    {"Blue Elderberry Cluster", SPECIES_BLUE_ELDERBERRY_CLUSTER_BITMAP, SPECIES_BLUE_ELDERBERRY_CLUSTER_W, SPECIES_BLUE_ELDERBERRY_CLUSTER_H},
    {"Mountain Strawberry", SPECIES_MOUNTAIN_STRAWBERRY_BITMAP, SPECIES_MOUNTAIN_STRAWBERRY_W, SPECIES_MOUNTAIN_STRAWBERRY_H},
    {"Pearly Everlasting", SPECIES_PEARLY_EVERLASTING_BITMAP, SPECIES_PEARLY_EVERLASTING_W, SPECIES_PEARLY_EVERLASTING_H},
    {"Cow Parsnip Shoots", SPECIES_COW_PARSNIP_SHOOTS_BITMAP, SPECIES_COW_PARSNIP_SHOOTS_W, SPECIES_COW_PARSNIP_SHOOTS_H},
    {"Fireweed Shoots", SPECIES_FIREWEED_SHOOTS_BITMAP, SPECIES_FIREWEED_SHOOTS_W, SPECIES_FIREWEED_SHOOTS_H},
    {"Fireweed Flowers", SPECIES_FIREWEED_FLOWERS_BITMAP, SPECIES_FIREWEED_FLOWERS_W, SPECIES_FIREWEED_FLOWERS_H},
    {"Kinnikinnick Berries", SPECIES_KINNIKINNICK_BERRIES_BITMAP, SPECIES_KINNIKINNICK_BERRIES_W, SPECIES_KINNIKINNICK_BERRIES_H},
    {"Juniper Berries", SPECIES_JUNIPER_BERRIES_BITMAP, SPECIES_JUNIPER_BERRIES_W, SPECIES_JUNIPER_BERRIES_H},
    {"Mountain Ash Berries", SPECIES_MOUNTAIN_ASH_BERRIES_BITMAP, SPECIES_MOUNTAIN_ASH_BERRIES_W, SPECIES_MOUNTAIN_ASH_BERRIES_H},
    {"Red Elderberry", SPECIES_RED_ELDERBERRY_BITMAP, SPECIES_RED_ELDERBERRY_W, SPECIES_RED_ELDERBERRY_H},
    {"Pipsissewa", SPECIES_PIPSISSEWA_BITMAP, SPECIES_PIPSISSEWA_W, SPECIES_PIPSISSEWA_H},
    {"Nori (Laver)", SPECIES_NORI__LAVER_BITMAP, SPECIES_NORI__LAVER_W, SPECIES_NORI__LAVER_H},
    {"Bladderwrack", SPECIES_BLADDERWRACK_BITMAP, SPECIES_BLADDERWRACK_W, SPECIES_BLADDERWRACK_H},
    {"Winged Kelp", SPECIES_WINGED_KELP_BITMAP, SPECIES_WINGED_KELP_W, SPECIES_WINGED_KELP_H},
    {"Sea Palm", SPECIES_SEA_PALM_BITMAP, SPECIES_SEA_PALM_W, SPECIES_SEA_PALM_H},
    {"Feather Boa Kelp", SPECIES_FEATHER_BOA_KELP_BITMAP, SPECIES_FEATHER_BOA_KELP_W, SPECIES_FEATHER_BOA_KELP_H},
    {"Turkish Towel", SPECIES_TURKISH_TOWEL_BITMAP, SPECIES_TURKISH_TOWEL_W, SPECIES_TURKISH_TOWEL_H},
    {"Turkish Washcloth", SPECIES_TURKISH_WASHCLOTH_BITMAP, SPECIES_TURKISH_WASHCLOTH_W, SPECIES_TURKISH_WASHCLOTH_H},
    {"Sea Sac", SPECIES_SEA_SAC_BITMAP, SPECIES_SEA_SAC_W, SPECIES_SEA_SAC_H},
    {"Oarweed", SPECIES_OARWEED_BITMAP, SPECIES_OARWEED_W, SPECIES_OARWEED_H},
    {"Sea Cabbage", SPECIES_SEA_CABBAGE_BITMAP, SPECIES_SEA_CABBAGE_W, SPECIES_SEA_CABBAGE_H},
    {"Sea Hair", SPECIES_SEA_HAIR_BITMAP, SPECIES_SEA_HAIR_W, SPECIES_SEA_HAIR_H},
    {"Giant Kelp", SPECIES_GIANT_KELP_BITMAP, SPECIES_GIANT_KELP_W, SPECIES_GIANT_KELP_H},
    {"Beach Sandwort", SPECIES_BEACH_SANDWORT_BITMAP, SPECIES_BEACH_SANDWORT_W, SPECIES_BEACH_SANDWORT_H},
    {"American Searocket", SPECIES_AMERICAN_SEAROCKET_BITMAP, SPECIES_AMERICAN_SEAROCKET_W, SPECIES_AMERICAN_SEAROCKET_H},
    {"Orache", SPECIES_ORACHE_BITMAP, SPECIES_ORACHE_W, SPECIES_ORACHE_H},
    {"Beach Asparagus", SPECIES_BEACH_ASPARAGUS_BITMAP, SPECIES_BEACH_ASPARAGUS_W, SPECIES_BEACH_ASPARAGUS_H},
    {"Scurvy Grass", SPECIES_SCURVY_GRASS_BITMAP, SPECIES_SCURVY_GRASS_W, SPECIES_SCURVY_GRASS_H},
    {"Sea Coast Angelica", SPECIES_SEA_COAST_ANGELICA_BITMAP, SPECIES_SEA_COAST_ANGELICA_W, SPECIES_SEA_COAST_ANGELICA_H},
    {"Beach Lovage", SPECIES_BEACH_LOVAGE_BITMAP, SPECIES_BEACH_LOVAGE_W, SPECIES_BEACH_LOVAGE_H},
    {"Pacific Oyster", SPECIES_PACIFIC_OYSTER_BITMAP, SPECIES_PACIFIC_OYSTER_W, SPECIES_PACIFIC_OYSTER_H},
    {"Bay Mussel", SPECIES_BAY_MUSSEL_BITMAP, SPECIES_BAY_MUSSEL_W, SPECIES_BAY_MUSSEL_H},
    {"Manila Littleneck Clam", SPECIES_MANILA_LITTLENECK_CLAM_BITMAP, SPECIES_MANILA_LITTLENECK_CLAM_W, SPECIES_MANILA_LITTLENECK_CLAM_H},
    {"Butter Clam", SPECIES_BUTTER_CLAM_BITMAP, SPECIES_BUTTER_CLAM_W, SPECIES_BUTTER_CLAM_H},
    {"Geoduck", SPECIES_GEODUCK_BITMAP, SPECIES_GEODUCK_W, SPECIES_GEODUCK_H},
    {"Gooseneck Barnacle", SPECIES_GOOSENECK_BARNACLE_BITMAP, SPECIES_GOOSENECK_BARNACLE_W, SPECIES_GOOSENECK_BARNACLE_H},
    {"Dungeness Crab", SPECIES_DUNGENESS_CRAB_BITMAP, SPECIES_DUNGENESS_CRAB_W, SPECIES_DUNGENESS_CRAB_H},
    {"Red Rock Crab", SPECIES_RED_ROCK_CRAB_BITMAP, SPECIES_RED_ROCK_CRAB_W, SPECIES_RED_ROCK_CRAB_H},
    {"Black Katy Chiton", SPECIES_BLACK_KATY_CHITON_BITMAP, SPECIES_BLACK_KATY_CHITON_W, SPECIES_BLACK_KATY_CHITON_H},
    {"Red Sea Urchin", SPECIES_RED_SEA_URCHIN_BITMAP, SPECIES_RED_SEA_URCHIN_W, SPECIES_RED_SEA_URCHIN_H},
    {"Limpet", SPECIES_LIMPET_BITMAP, SPECIES_LIMPET_W, SPECIES_LIMPET_H},
    {"Pacific Basket Cockle", SPECIES_PACIFIC_BASKET_COCKLE_BITMAP, SPECIES_PACIFIC_BASKET_COCKLE_W, SPECIES_PACIFIC_BASKET_COCKLE_H},
    {"Horse Clam", SPECIES_HORSE_CLAM_BITMAP, SPECIES_HORSE_CLAM_W, SPECIES_HORSE_CLAM_H},
    {"Giant Red Sea Cucumber", SPECIES_GIANT_RED_SEA_CUCUMBER_BITMAP, SPECIES_GIANT_RED_SEA_CUCUMBER_W, SPECIES_GIANT_RED_SEA_CUCUMBER_H},
    {"Moon Snail", SPECIES_MOON_SNAIL_BITMAP, SPECIES_MOON_SNAIL_W, SPECIES_MOON_SNAIL_H},
    {"Purple Varnish Clam", SPECIES_PURPLE_VARNISH_CLAM_BITMAP, SPECIES_PURPLE_VARNISH_CLAM_W, SPECIES_PURPLE_VARNISH_CLAM_H},
    {"Manila Clam", SPECIES_MANILA_CLAM_BITMAP, SPECIES_MANILA_CLAM_W, SPECIES_MANILA_CLAM_H},
    {"Pacific Littleneck Clam", SPECIES_PACIFIC_LITTLENECK_CLAM_BITMAP, SPECIES_PACIFIC_LITTLENECK_CLAM_W, SPECIES_PACIFIC_LITTLENECK_CLAM_H},
    {"Pacific Gaper Clam", SPECIES_PACIFIC_GAPER_CLAM_BITMAP, SPECIES_PACIFIC_GAPER_CLAM_W, SPECIES_PACIFIC_GAPER_CLAM_H},
    {"Basket Cockle", SPECIES_BASKET_COCKLE_BITMAP, SPECIES_BASKET_COCKLE_W, SPECIES_BASKET_COCKLE_H},
    {"Softshell Clam", SPECIES_SOFTSHELL_CLAM_BITMAP, SPECIES_SOFTSHELL_CLAM_W, SPECIES_SOFTSHELL_CLAM_H},
    {"Varnish Clam", SPECIES_VARNISH_CLAM_BITMAP, SPECIES_VARNISH_CLAM_W, SPECIES_VARNISH_CLAM_H},
    {"California Mussel", SPECIES_CALIFORNIA_MUSSEL_BITMAP, SPECIES_CALIFORNIA_MUSSEL_W, SPECIES_CALIFORNIA_MUSSEL_H},
    {"Olympia Oyster", SPECIES_OLYMPIA_OYSTER_BITMAP, SPECIES_OLYMPIA_OYSTER_W, SPECIES_OLYMPIA_OYSTER_H},
    {"Kelp Crab", SPECIES_KELP_CRAB_BITMAP, SPECIES_KELP_CRAB_W, SPECIES_KELP_CRAB_H},
    {"Graceful Crab", SPECIES_GRACEFUL_CRAB_BITMAP, SPECIES_GRACEFUL_CRAB_W, SPECIES_GRACEFUL_CRAB_H},
    {"Purple Sea Urchin", SPECIES_PURPLE_SEA_URCHIN_BITMAP, SPECIES_PURPLE_SEA_URCHIN_W, SPECIES_PURPLE_SEA_URCHIN_H},
    {"Gumboot Chiton", SPECIES_GUMBOOT_CHITON_BITMAP, SPECIES_GUMBOOT_CHITON_W, SPECIES_GUMBOOT_CHITON_H},
    {"Blue Top Snail", SPECIES_BLUE_TOP_SNAIL_BITMAP, SPECIES_BLUE_TOP_SNAIL_W, SPECIES_BLUE_TOP_SNAIL_H},
    {"Spot Prawn", SPECIES_SPOT_PRAWN_BITMAP, SPECIES_SPOT_PRAWN_W, SPECIES_SPOT_PRAWN_H},
    {"Pinto Abalone", SPECIES_PINTO_ABALONE_BITMAP, SPECIES_PINTO_ABALONE_W, SPECIES_PINTO_ABALONE_H},
    {"Evergreen Huckleberry", SPECIES_EVERGREEN_HUCKLEBERRY_BITMAP, SPECIES_EVERGREEN_HUCKLEBERRY_W, SPECIES_EVERGREEN_HUCKLEBERRY_H},
    {"American Sea Rocket", SPECIES_AMERICAN_SEA_ROCKET_BITMAP, SPECIES_AMERICAN_SEA_ROCKET_W, SPECIES_AMERICAN_SEA_ROCKET_H},
    {"Rockweed", SPECIES_ROCKWEED_BITMAP, SPECIES_ROCKWEED_W, SPECIES_ROCKWEED_H},
    {"Partridgefoot Greens", SPECIES_PARTRIDGEFOOT_GREENS_BITMAP, SPECIES_PARTRIDGEFOOT_GREENS_W, SPECIES_PARTRIDGEFOOT_GREENS_H},
    {"Sea Beach Sandwort", SPECIES_SEA_BEACH_SANDWORT_BITMAP, SPECIES_SEA_BEACH_SANDWORT_W, SPECIES_SEA_BEACH_SANDWORT_H},
    {"Gem-Studded Puffball", SPECIES_GEM_STUDDED_PUFFBALL_BITMAP, SPECIES_GEM_STUDDED_PUFFBALL_W, SPECIES_GEM_STUDDED_PUFFBALL_H},
    {"Pear-Shaped Puffball", SPECIES_PEAR_SHAPED_PUFFBALL_BITMAP, SPECIES_PEAR_SHAPED_PUFFBALL_W, SPECIES_PEAR_SHAPED_PUFFBALL_H},
    {"Pigskin Poison Puffball", SPECIES_PIGSKIN_POISON_PUFFBALL_BITMAP, SPECIES_PIGSKIN_POISON_PUFFBALL_W, SPECIES_PIGSKIN_POISON_PUFFBALL_H},
    {"Zeller's Bolete", SPECIES_ZELLER_S_BOLETE_BITMAP, SPECIES_ZELLER_S_BOLETE_W, SPECIES_ZELLER_S_BOLETE_H},
    {"Western Painted Suillus", SPECIES_WESTERN_PAINTED_SUILLUS_BITMAP, SPECIES_WESTERN_PAINTED_SUILLUS_W, SPECIES_WESTERN_PAINTED_SUILLUS_H},
    {"Red-Cracked Bolete", SPECIES_RED_CRACKED_BOLETE_BITMAP, SPECIES_RED_CRACKED_BOLETE_W, SPECIES_RED_CRACKED_BOLETE_H},
    {"Butter Bolete", SPECIES_BUTTER_BOLETE_BITMAP, SPECIES_BUTTER_BOLETE_W, SPECIES_BUTTER_BOLETE_H},
    {"Crown-Tipped Coral", SPECIES_CROWN_TIPPED_CORAL_BITMAP, SPECIES_CROWN_TIPPED_CORAL_W, SPECIES_CROWN_TIPPED_CORAL_H},
    {"Pink Coral Fungus", SPECIES_PINK_CORAL_FUNGUS_BITMAP, SPECIES_PINK_CORAL_FUNGUS_W, SPECIES_PINK_CORAL_FUNGUS_H},
    {"Pig's Ears", SPECIES_PIG_S_EARS_BITMAP, SPECIES_PIG_S_EARS_W, SPECIES_PIG_S_EARS_H},
    {"Woolly Chanterelle", SPECIES_WOOLLY_CHANTERELLE_BITMAP, SPECIES_WOOLLY_CHANTERELLE_W, SPECIES_WOOLLY_CHANTERELLE_H},
    {"Coral Tooth Fungus", SPECIES_CORAL_TOOTH_FUNGUS_BITMAP, SPECIES_CORAL_TOOTH_FUNGUS_W, SPECIES_CORAL_TOOTH_FUNGUS_H},
    {"Jelly Tooth", SPECIES_JELLY_TOOTH_BITMAP, SPECIES_JELLY_TOOTH_W, SPECIES_JELLY_TOOTH_H},
    {"Wood Ear", SPECIES_WOOD_EAR_BITMAP, SPECIES_WOOD_EAR_W, SPECIES_WOOD_EAR_H},
    {"Amber Jelly Roll", SPECIES_AMBER_JELLY_ROLL_BITMAP, SPECIES_AMBER_JELLY_ROLL_W, SPECIES_AMBER_JELLY_ROLL_H},
    {"Black Witch's Butter", SPECIES_BLACK_WITCH_S_BUTTER_BITMAP, SPECIES_BLACK_WITCH_S_BUTTER_W, SPECIES_BLACK_WITCH_S_BUTTER_H},
    {"Cinnabar Polypore", SPECIES_CINNABAR_POLYPORE_BITMAP, SPECIES_CINNABAR_POLYPORE_W, SPECIES_CINNABAR_POLYPORE_H},
    {"False Turkey Tail", SPECIES_FALSE_TURKEY_TAIL_BITMAP, SPECIES_FALSE_TURKEY_TAIL_W, SPECIES_FALSE_TURKEY_TAIL_H},
    {"Split Gill", SPECIES_SPLIT_GILL_BITMAP, SPECIES_SPLIT_GILL_W, SPECIES_SPLIT_GILL_H},
    {"Orange Peel Fungus", SPECIES_ORANGE_PEEL_FUNGUS_BITMAP, SPECIES_ORANGE_PEEL_FUNGUS_W, SPECIES_ORANGE_PEEL_FUNGUS_H},
    {"Fried Chicken Mushroom", SPECIES_FRIED_CHICKEN_MUSHROOM_BITMAP, SPECIES_FRIED_CHICKEN_MUSHROOM_W, SPECIES_FRIED_CHICKEN_MUSHROOM_H},
    {"Honey Mushroom", SPECIES_HONEY_MUSHROOM_BITMAP, SPECIES_HONEY_MUSHROOM_W, SPECIES_HONEY_MUSHROOM_H},
    {"The Prince", SPECIES_THE_PRINCE_BITMAP, SPECIES_THE_PRINCE_W, SPECIES_THE_PRINCE_H},
    {"Snow Bank False Morel", SPECIES_SNOW_BANK_FALSE_MOREL_BITMAP, SPECIES_SNOW_BANK_FALSE_MOREL_W, SPECIES_SNOW_BANK_FALSE_MOREL_H},
    {"Shaggy Scalycap", SPECIES_SHAGGY_SCALYCAP_BITMAP, SPECIES_SHAGGY_SCALYCAP_W, SPECIES_SHAGGY_SCALYCAP_H},
    {"Blue Chanterelle", SPECIES_BLUE_CHANTERELLE_BITMAP, SPECIES_BLUE_CHANTERELLE_W, SPECIES_BLUE_CHANTERELLE_H},
    {"Fairy Ring Mushroom", SPECIES_FAIRY_RING_MUSHROOM_BITMAP, SPECIES_FAIRY_RING_MUSHROOM_W, SPECIES_FAIRY_RING_MUSHROOM_H},
    {"Rooting Shank", SPECIES_ROOTING_SHANK_BITMAP, SPECIES_ROOTING_SHANK_W, SPECIES_ROOTING_SHANK_H},
    {"Dyer's Polypore", SPECIES_DYER_S_POLYPORE_BITMAP, SPECIES_DYER_S_POLYPORE_W, SPECIES_DYER_S_POLYPORE_H},
    {"Birch Polypore", SPECIES_BIRCH_POLYPORE_BITMAP, SPECIES_BIRCH_POLYPORE_W, SPECIES_BIRCH_POLYPORE_H},
    {"Dryad's Saddle", SPECIES_DRYADS_SADDLE_BITMAP, SPECIES_DRYADS_SADDLE_W, SPECIES_DRYADS_SADDLE_H},
    {"Sword Fern", SPECIES_SWORD_FERN_BITMAP, SPECIES_SWORD_FERN_W, SPECIES_SWORD_FERN_H},
    {"Deer Fern Fiddlehead", SPECIES_DEER_FERN_FIDDLEHEAD_BITMAP, SPECIES_DEER_FERN_FIDDLEHEAD_W, SPECIES_DEER_FERN_FIDDLEHEAD_H},
    {"Bracken Fern Fiddlehead", SPECIES_BRACKEN_FERN_FIDDLEHEAD_BITMAP, SPECIES_BRACKEN_FERN_FIDDLEHEAD_W, SPECIES_BRACKEN_FERN_FIDDLEHEAD_H},
    {"Lady Fern Fiddlehead", SPECIES_LADY_FERN_FIDDLEHEAD_BITMAP, SPECIES_LADY_FERN_FIDDLEHEAD_W, SPECIES_LADY_FERN_FIDDLEHEAD_H},
    {"Cattail Shoots", SPECIES_CATTAIL_SHOOTS_BITMAP, SPECIES_CATTAIL_SHOOTS_W, SPECIES_CATTAIL_SHOOTS_H},
    {"Cattail Pollen", SPECIES_CATTAIL_POLLEN_BITMAP, SPECIES_CATTAIL_POLLEN_W, SPECIES_CATTAIL_POLLEN_H},
};
static const int kSpeciesBitmapCount = sizeof(kSpeciesBitmaps) / sizeof(kSpeciesBitmaps[0]);

inline const SpeciesBitmap* find(const char* name) {
  for (int i = 0; i < kSpeciesBitmapCount; i++) {
    if (strcmp(kSpeciesBitmaps[i].name, name) == 0) return &kSpeciesBitmaps[i];
  }
  return nullptr;
}

}  // namespace species_bitmaps
