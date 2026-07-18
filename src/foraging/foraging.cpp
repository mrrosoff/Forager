#include "foraging.h"

#include <algorithm>
#include <cstring>

#include "sprites.h"

namespace foraging {

#define MO(m) (1 << ((m) - 1))

// Master PNW / Seattle species reference. monthMask bits mark active months
// (1=Jan .. 12=Dec). caution is "" when there's no meaningful look-alike risk.
static const Forageable kSpecies[] = {
    {"Hedgehog Mushroom", "mushroom", "Toothed underside, no true gills. Lingers after frost.",
     "No toxic lookalikes -- teeth instead of gills make ID easy.",
     "Look under conifers in duff after fall rains.", MO(1) | MO(10) | MO(11) | MO(12),
     SPR_MUSHROOM_TOOTH, Biome::Forest},

    {"Miner's Lettuce", "green", "Round leaf cupping the stem. Shady wet banks.", "",
     "Snip above the root so the patch regrows.", MO(1) | MO(2) | MO(3), SPR_LEAFY_GREEN,
     Biome::Forest},

    {"Stinging Nettle", "green", "First tips now. Gloves on; cooking kills the sting.",
     "Wear gloves -- fresh leaves sting on contact.",
     "Pick only the top 4-6 leaves per plant, gloved.", MO(2) | MO(3) | MO(4), SPR_NETTLE,
     Biome::Forest},

    {"Oyster Mushroom", "mushroom", "Shelving on dead alder and maple after rain.",
     "Check for the Angel Wing lookalike on conifers -- avoid it.",
     "Cut at the base of the cluster, leave small ones.", MO(1) | MO(2) | MO(11) | MO(12),
     SPR_MUSHROOM_CAP, Biome::Forest},

    {"Morel", "mushroom", "Honeycomb cap. Watch cottonwoods as soil warms.",
     "True morels are hollow top to bottom -- cut one to confirm.",
     "Check burn sites and cottonwood river bottoms.", MO(3) | MO(4), SPR_MOREL, Biome::Forest},

    {"Chickweed", "green", "Tender mats in disturbed ground. Mild, fresh.", "",
     "Best before flowering; snip the top few inches.", MO(3) | MO(4) | MO(5), SPR_LEAFY_GREEN,
     Biome::Forest},

    {"Ramps", "green", "Wild leek. Harvest one leaf, leave the bulb.", "",
     "Take a single leaf per plant so the patch survives.", MO(4), SPR_RAMP, Biome::Forest},

    {"Fiddlehead Fern", "green", "Tight coils of lady fern. Pick before they unfurl.",
     "Must be cooked thoroughly; avoid bracken fern lookalikes.",
     "Snap coils that are still tightly curled, under 2 inches.", MO(4) | MO(5), SPR_FIDDLEHEAD,
     Biome::Forest},

    {"Elderflower", "flower", "Creamy umbels. Cordial weather. Never the leaves.",
     "Leaves and stems are toxic -- flowers and ripe berries only.",
     "Cut whole flower clusters, shake off insects first.", MO(5) | MO(6), SPR_FLOWER_CLUSTER,
     Biome::Forest},

    {"Wood Sorrel", "green", "Shamrock leaf, lemony bite. Forest floor and trailside.",
     "High in oxalic acid -- a trailside nibble, not a meal.",
     "Pinch leaves and stems; avoid roadside patches.", MO(5) | MO(6) | MO(7), SPR_LEAFY_GREEN,
     Biome::Forest},

    {"Salmonberry", "berry", "Earliest berry. Golden to red, along streams.", "",
     "Ripe berries pull free easily; unripe ones resist.", MO(6), SPR_BERRY_CLUSTER, Biome::Forest},

    {"Chanterelle", "mushroom", "Apricot scent, false gills. Conifer duff after warm rain.",
     "False ridges, not true blades -- a key ID feature vs toxic lookalikes.",
     "Look on slopes under Douglas fir a week after rain.", MO(7) | MO(8) | MO(9), SPR_MUSHROOM_CAP,
     Biome::Forest},

    {"Blackberry", "berry", "Himalayan brambles everywhere. Sun-warmed edges.", "",
     "Sun-facing berries ripen first; watch for thorns.", MO(8) | MO(9), SPR_BERRY_CLUSTER,
     Biome::Forest},

    {"Matsutake", "mushroom", "Spicy-cinnamon pine mushroom. Sandy conifer soil.", "",
     "Look for cracked duff mounds near pine roots.", MO(9), SPR_MATSUTAKE, Biome::Forest},

    {"Lobster Mushroom", "mushroom", "Orange parasitized host. Firm, seafood-sweet.",
     "The host mushroom underneath is irrelevant once parasitized.",
     "Firm all the way through means it's good to eat.", MO(9) | MO(10), SPR_MUSHROOM_CAP,
     Biome::Forest},

    {"Porcini", "mushroom", "King bolete. Fat white stem, no gills -- pores instead.",
     "Check for pores, not gills; avoid soft or wormy caps.",
     "Firm caps only; check the stem base for grubs.", MO(10), SPR_PORCINI, Biome::Forest},

    {"Yellowfoot", "mushroom", "Winter chanterelle. Hollow stem, late and cold-hardy.", "",
     "Keeps fruiting through light frost -- check familiar patches.", MO(11) | MO(12),
     SPR_MUSHROOM_CAP, Biome::Forest},

    {"Cauliflower Fungus", "mushroom", "Cream coral at conifer bases. Soak grit out.", "",
     "Cut at the base; soak well, it hides a lot of grit.", MO(11), SPR_CORAL, Biome::Forest},

    {"Velvet Shank", "mushroom", "Enokitake's wild cousin. Orange caps in cold snaps.",
     "Sticky cap and velvety stem distinguish it from toxic Galerina.",
     "Fruits on hardwood stumps during cold snaps.", MO(12) | MO(1), SPR_MUSHROOM_CAP,
     Biome::Forest},

    {"Huckleberry", "berry", "Subalpine slopes. Ripens as you gain elevation.", "",
     "Higher elevation = later ripening; chase the season uphill.", MO(7) | MO(8),
     SPR_BERRY_CLUSTER, Biome::Mountain},

    {"Cascade Blueberry", "berry", "Low mats near timberline, deep blue-black fruit.", "",
     "Fruits thin at first frost; comb low shrubs by hand.", MO(8) | MO(9), SPR_BERRY_CLUSTER,
     Biome::Mountain},

    {"Mountain Cranberry", "berry", "Lingonberry cousin. Tart red berries in dwarf mats.", "",
     "Best after light frost sweetens the fruit; pick sparingly.", MO(9) | MO(10),
     SPR_BERRY_CLUSTER, Biome::Mountain},

    {"Alpine Sorrel", "green", "Reddish, lemony leaf tucked in high rock crevices.",
     "High in oxalic acid -- a trailside nibble, not a meal.",
     "Check talus and scree edges above the treeline.", MO(6) | MO(7) | MO(8), SPR_LEAFY_GREEN,
     Biome::Mountain},

    {"Devil's Club Shoots", "green", "Spring shoots, mild and green before the spines toughen.",
     "Cook thoroughly; handle mature stalks with gloves -- spiny.",
     "Snap shoots while still tender, before leaves fully open.", MO(4) | MO(5), SPR_FIDDLEHEAD,
     Biome::Mountain},

    {"Subalpine Puffball", "mushroom", "Round white body in high meadows after summer storms.",
     "Cut in half first -- interior must be pure white, no gill shadow.",
     "Harvest young and firm; skip any with a yellowing center.", MO(8) | MO(9), SPR_MUSHROOM_CAP,
     Biome::Mountain},

    {"Swamp Onion", "green", "Wild mountain onion in wet subalpine meadows.", "",
     "Pull gently from soft mud; rinse the bulb at the creek.", MO(6) | MO(7), SPR_RAMP,
     Biome::Mountain},

    {"Douglas Fir Tips", "flower", "Bright citrus-pine new growth at branch ends.",
     "Don't confuse with Pacific yew -- yew needles are toxic.",
     "Pinch only the pale new candle growth, not old needles.", MO(4) | MO(5), SPR_FLOWER_CLUSTER,
     Biome::Mountain},

    {"Bull Kelp", "seaweed", "Long whip stipe with a bulb float; drifts up after storms.",
     "Harvest drift only, or cut blade above the holdfast to regrow.",
     "Gather fresh drift kelp on the tideline after a blow.", MO(6) | MO(7) | MO(8) | MO(9),
     SPR_LEAFY_GREEN, Biome::Coast},

    {"Sugar Kelp", "seaweed", "Wavy golden-brown blade in the low intertidal.", "",
     "Cut blades an inch above the holdfast at a minus tide.", MO(5) | MO(6) | MO(7),
     SPR_LEAFY_GREEN, Biome::Coast},

    {"Sea Lettuce", "seaweed", "Bright green, tissue-thin sheets on mid-tide rocks.",
     "Rinse well -- can harbor grit and small amphipods.",
     "Pick loose, undamaged sheets; avoid ones near outfalls.", MO(4) | MO(5) | MO(6) | MO(7),
     SPR_LEAFY_GREEN, Biome::Coast},

    {"Dulse", "seaweed", "Deep red, hand-shaped fronds in the upper splash zone.", "",
     "Peel fronds off rocks at low tide; dries well in the sun.", MO(6) | MO(7) | MO(8), SPR_CORAL,
     Biome::Coast},

    {"Sea Asparagus", "green", "Pickleweed. Crunchy, salty segmented stems in the marsh.", "",
     "Snip the top two joints only, leave the rooted base.", MO(6) | MO(7) | MO(8) | MO(9),
     SPR_LEAFY_GREEN, Biome::Coast},

    {"Goosetongue", "green", "Sea plantain. Fleshy, briny blades on the upper beach.", "",
     "Pick young center leaves; avoid grazed or muddy patches.", MO(5) | MO(6) | MO(7),
     SPR_LEAFY_GREEN, Biome::Coast},

    {"Silverweed", "green", "Silvery-leaved runner in upper tidal meadows.", "",
     "Dig slender roots in fall after the leaves die back.", MO(9) | MO(10), SPR_LEAFY_GREEN,
     Biome::Coast},

    {"Beach Strawberry", "berry", "Low dune mats, small intensely sweet fruit.", "",
     "Search sandy, sun-facing dune grass for ripe red fruit.", MO(7) | MO(8), SPR_BERRY_CLUSTER,
     Biome::Coast},

    {"Beach Pea", "flower", "Purple-pink pea flowers along driftwood lines.",
     "Raw seeds are mildly toxic in quantity -- young pods only, cooked.",
     "Pick flat young pods before the seeds swell and harden.", MO(6) | MO(7) | MO(8),
     SPR_FLOWER_CLUSTER, Biome::Coast},

    {"Wild Rose Hips", "berry", "Bright orange-red hips on beach and hedgerow roses.",
     "Scrape out the irritating seed hairs before eating.",
     "Pick after the first frost sweetens the hip.", MO(9) | MO(10), SPR_BERRY_CLUSTER,
     Biome::Coast},

    {"Razor Clam", "shellfish", "Fast-digging bivalve on open sand surf beaches.",
     "Check state biotoxin/PSP closures and hold a license before digging.",
     "Look for a 'show' dimple in the sand, then dig fast.",
     MO(10) | MO(11) | MO(12) | MO(1) | MO(2) | MO(3), SPR_MOREL, Biome::Coast},

    {"Shaggy Mane", "mushroom", "Tall shaggy cap, self-digests to ink within hours.",
     "Do not pair with alcohol -- some report Coprine-like reactions; eat fresh only.",
     "Pick young, still-white caps in lawns and disturbed soil.", MO(9) | MO(10) | MO(11),
     SPR_MUSHROOM_CAP, Biome::Forest},

    {"Death Cap", "mushroom", "Deadly amanita. White gills, skirt ring, sac at base.",
     "Deadly poisonous -- responsible for most fatal mushroom poisonings; know it to avoid it, "
     "never eat.",
     "Do not harvest -- learn to spot the white gills and basal cup under oaks and ornamentals.",
     MO(9) | MO(10) | MO(11), SPR_MUSHROOM_CAP, Biome::Forest},

    {"Destroying Angel", "mushroom", "Pure white amanita with a sac at the base.",
     "Deadly poisonous -- pure white all over is a red flag, not a green light; never eat.",
     "Do not harvest -- if you dig one up for ID, check for the volva sac at the buried base.",
     MO(9) | MO(10) | MO(11), SPR_MUSHROOM_CAP, Biome::Forest},

    {"Fly Agaric", "mushroom", "Iconic red cap with white warts, under birch and pine.",
     "Toxic and psychoactive -- causes serious GI distress and neurotoxic effects; not a food.",
     "Admire, don't pick -- easy to ID by the red cap and white flecks.", MO(9) | MO(10) | MO(11),
     SPR_MUSHROOM_CAP, Biome::Forest},

    {"Turkey Tail", "mushroom", "Thin banded fan-shelves in rainbow stripes on logs.",
     "Too tough to eat -- simmered for tea or tincture, not chewed.",
     "Snap off firm, colorful shelves from dead hardwood; dry for tea.",
     MO(9) | MO(10) | MO(11) | MO(12) | MO(1), SPR_MUSHROOM_CAP, Biome::Forest},

    {"Artist's Conk", "mushroom",
     "Woody hoof-shaped bracket, white pore surface takes a scratch drawing.",
     "Too woody to eat fresh -- used dried for tea, not as food.",
     "Look for shelf brackets on standing or fallen conifers year-round.",
     MO(1) | MO(9) | MO(10) | MO(11) | MO(12), SPR_MUSHROOM_CAP, Biome::Forest},

    {"Chicken of the Woods", "mushroom",
     "Bright orange-yellow shelving on wounds of oak, conifer stumps.",
     "Cook thoroughly; some people react with GI upset, especially conifer-grown ones -- try a "
     "small amount first.",
     "Cut only the tender outer edge of a fresh shelf, leave the tough base.",
     MO(8) | MO(9) | MO(10), SPR_MUSHROOM_CAP, Biome::Forest},

    {"Hen of the Woods", "mushroom", "Ruffled grey-brown rosette at the base of old oaks.", "",
     "Check the base of mature oaks in the same spot every fall -- it returns for years.",
     MO(9) | MO(10), SPR_MUSHROOM_CAP, Biome::Forest},

    {"Shrimp Russula", "mushroom", "Reddish-pink cap, brittle white gills, mild in conifer duff.",
     "Russulas vary widely -- taste a tiny raw bit and spit it out; peppery ones are inedible.",
     "Look for brittle, chalky-snapping stems under Douglas fir.", MO(9) | MO(10), SPR_MUSHROOM_CAP,
     Biome::Forest},

    {"Cauliflower Coral", "mushroom", "Pale branching coral clusters at conifer bases.",
     "Distinguish from thin, brittle toxic corals -- this one is thick and fleshy.",
     "Cut the whole clump at the base; check for grubs in the crevices.", MO(9) | MO(10) | MO(11),
     SPR_CORAL, Biome::Forest},

    {"Golden Chanterelle", "mushroom",
     "Deep egg-yolk color, wavy cap, distinct from the paler common chanterelle.",
     "False chanterelle (jack-o'-lantern) has true, sharp gills and glows faintly -- check for the "
     "blunt false ridges.",
     "Search mossy Douglas fir slopes a week after a warm rain.", MO(9) | MO(10), SPR_MUSHROOM_CAP,
     Biome::Forest},

    {"White Chanterelle", "mushroom",
     "Pale ivory cousin of the golden chanterelle, same false gills.", "",
     "Look in the same duff patches as goldens, slightly earlier in the season.", MO(8) | MO(9),
     SPR_MUSHROOM_CAP, Biome::Forest},

    {"Jack-o'-Lantern Mushroom", "mushroom",
     "Bright orange clusters on buried roots, true sharp gills.",
     "Toxic -- causes severe vomiting; the classic chanterelle lookalike, ID by true blade-like "
     "gills.",
     "Do not pick -- note the clustered growth on wood, not soil, as a warning sign.",
     MO(9) | MO(10), SPR_MUSHROOM_CAP, Biome::Forest},

    {"Cortinarius (Deadly Webcap)", "mushroom",
     "Rusty-brown cap with a cobwebby veil, common in conifer duff.",
     "Deadly poisonous -- causes delayed, irreversible kidney failure; never eat any brown "
     "Cortinarius.",
     "Do not harvest -- the whole genus is best left alone by beginners.", MO(9) | MO(10) | MO(11),
     SPR_MUSHROOM_CAP, Biome::Forest},

    {"Angel Wings", "mushroom",
     "Small white shelving fungus on conifer logs, resembles oyster mushroom.",
     "Linked to rare fatal poisonings in susceptible people -- avoid entirely despite resembling "
     "oysters.",
     "Do not harvest -- if it's growing on conifer wood, it isn't the true oyster.",
     MO(10) | MO(11), SPR_MUSHROOM_CAP, Biome::Forest},

    {"Puffball (Giant)", "mushroom", "Basketball-sized white puffball in fields and forest edges.",
     "Slice in half first -- must be solid white inside with no gill outline or you may have a "
     "young amanita.",
     "Pick while still firm and pure white before it yellows and turns to spore dust.",
     MO(9) | MO(10), SPR_MUSHROOM_CAP, Biome::Forest},

    {"Pig's Ear (Violet Chanterelle)", "mushroom",
     "Dark purple-brown trumpet, thin flesh, blends into duff.", "",
     "Scan closely -- the dark color camouflages it against wet soil.", MO(9) | MO(10) | MO(11),
     SPR_MUSHROOM_CAP, Biome::Forest},

    {"Black Trumpet", "mushroom",
     "Thin, funnel-shaped, nearly black -- 'trumpet of death' in name only.", "",
     "Search mossy hardwood-conifer mix slopes; they hide in shadow and leaf litter.",
     MO(9) | MO(10) | MO(11), SPR_MUSHROOM_CAP, Biome::Forest},

    {"Admirable Bolete", "mushroom", "Reddish-brown cap, yellow pores, firm flesh under conifers.",
     "Bruises blue when cut -- harmless, but confirm pore color stays yellow, not red-orange.",
     "Check firm, dry-capped boletes on well-drained forest slopes.", MO(9) | MO(10), SPR_PORCINI,
     Biome::Forest},

    {"Slippery Jack", "mushroom", "Sticky brown-capped bolete under pines, glue-like when wet.",
     "Peel the slimy cap skin -- it can upset sensitive stomachs.",
     "Look under planted pines; the slick cap is diagnostic.", MO(9) | MO(10) | MO(11), SPR_PORCINI,
     Biome::Forest},

    {"Short-Stemmed Slippery Jack", "mushroom",
     "Squat bolete with a stubby stem, common under introduced pines.",
     "Peel the cap skin before cooking.",
     "Common in planted pine groves and urban conifer plantings.", MO(10) | MO(11), SPR_PORCINI,
     Biome::Forest},

    {"Boletus edulis (King Bolete flush)", "mushroom",
     "Second porcini flush after early fall rain, fatter and firmer.",
     "Avoid any bolete with red pores or blue-staining flesh.",
     "Check the same porcini patches again after a second rain event.", MO(10) | MO(11),
     SPR_PORCINI, Biome::Forest},

    {"Angel's Wing Bolete (Suillus lakei)", "mushroom",
     "Orange-brown scaly cap, yellow pores, grows under Douglas fir only.", "",
     "Search specifically under Douglas fir needles, its only host tree.", MO(9) | MO(10),
     SPR_PORCINI, Biome::Forest},

    {"Meadow Mushroom", "mushroom",
     "Wild cousin of the grocery button mushroom, in grassy clearings.",
     "Confirm pink-to-brown gills and no basal sac -- rules out deadly Amanita.",
     "Check the base for a volva sac before eating any lookalike field mushroom.", MO(9) | MO(10),
     SPR_MUSHROOM_CAP, Biome::Forest},

    {"Horse Mushroom", "mushroom", "Large, anise-scented relative of the meadow mushroom.",
     "Confirm no basal sac and a pleasant anise smell, not a chemical/inky one.",
     "Sniff the flesh -- a sweet anise smell is the giveaway.", MO(9) | MO(10), SPR_MUSHROOM_CAP,
     Biome::Forest},

    {"Destroying Angel's cousin (Panther Cap)", "mushroom",
     "Brown-capped amanita with white warts and a basal bulb.",
     "Toxic, causes neurological symptoms -- another reason to skip all warty amanitas.",
     "Do not harvest -- note the ring and bulb as amanita warning signs.", MO(9) | MO(10) | MO(11),
     SPR_MUSHROOM_CAP, Biome::Forest},

    {"Wood Blewit", "mushroom", "Lilac-tinted cap and gills, fruits late into cold weather.",
     "Must be cooked -- causes GI upset raw, and can resemble toxic Cortinarius; confirm the lilac "
     "gill color.",
     "Check leaf litter piles and compost edges after the first frosts.", MO(10) | MO(11) | MO(12),
     SPR_MUSHROOM_CAP, Biome::Forest},

    {"Fried Chicken Mushroom (Lyophyllum decastes)", "mushroom",
     "Grey-brown clustered caps pushing up through hard-packed soil or gravel.",
     "Cook well; a few similar Lyophyllum species have caused rare poisonings.",
     "Look for tight clusters breaking through compacted trail edges.", MO(9) | MO(10),
     SPR_MUSHROOM_CAP, Biome::Forest},

    {"Man on Horseback", "mushroom", "Yellow-capped Tricholoma, sandy soil under pine.",
     "Linked to rare rhabdomyolysis with repeated heavy meals -- eat only occasionally, in "
     "moderation.",
     "Look in sandy soil under pines; cap and gills are both yellow.", MO(10) | MO(11),
     SPR_MUSHROOM_CAP, Biome::Forest},

    {"Deer Mushroom", "mushroom", "Grey-brown cap on decaying wood, common but forgettable.",
     "Must be cooked; raw specimens have caused mild GI upset.",
     "Common on rotting logs but rarely worth harvesting -- a good beginner ID practice species.",
     MO(9) | MO(10), SPR_MUSHROOM_CAP, Biome::Forest},

    {"Woolly Chanterelle (Scaly Vase)", "mushroom",
     "Shaggy grey-brown trumpet with a hollow center, under conifers.",
     "Best cooked well; some find it mildly bitter raw.",
     "Check shaded conifer duff for the scaly, funnel-shaped cap.", MO(9) | MO(10) | MO(11),
     SPR_MUSHROOM_CAP, Biome::Forest},

    {"Indigo Milk Cap", "mushroom", "Slate-blue mushroom that bleeds indigo latex when cut.", "",
     "Slice the gills -- true blue milk oozing out confirms the ID.", MO(9) | MO(10),
     SPR_MUSHROOM_CAP, Biome::Forest},

    {"Old Man of the Woods", "mushroom",
     "Shaggy black-scaled bolete, easy to spot on the forest floor.", "",
     "Look for the distinctive shaggy black scales against pale flesh underneath.", MO(9) | MO(10),
     SPR_PORCINI, Biome::Forest},

    {"Western Giant Puffball", "mushroom",
     "Softball to volleyball-sized puffball in forest clearings and edges.",
     "Cut in half before eating -- interior must be uniformly white with no embryonic mushroom "
     "shape.",
     "Harvest while firm and snow-white; skip any turning yellow or brown inside.", MO(9) | MO(10),
     SPR_MUSHROOM_CAP, Biome::Forest},

    {"Lung Lichen (Lobaria)", "lichen",
     "Broad lettuce-like green-brown lichen draping maple branches.",
     "Edible in small amounts once processed -- traditionally used more as medicine than food.",
     "Gather fallen pieces from big-leaf maple branches rather than stripping live bark.",
     MO(1) | MO(2) | MO(11) | MO(12), SPR_LEAFY_GREEN, Biome::Forest},

    {"Witch's Butter", "mushroom",
     "Bright orange gelatinous blob on dead conifer branches after rain.", "",
     "Pick plump, jelly-firm blobs right after a soaking rain; they shrivel when dry.",
     MO(11) | MO(12) | MO(1) | MO(2), SPR_CORAL, Biome::Forest},

    {"Oregon White Truffle", "mushroom",
     "Aromatic underground fungus near Douglas fir roots, found by scent or rake.", "",
     "Rake gently through duff near young Douglas fir where the soil smells earthy-ripe.",
     MO(10) | MO(11) | MO(12) | MO(1), SPR_PORCINI, Biome::Forest},

    {"Thimbleberry", "berry", "Maple-like leaf, soft red thimble-shaped fruit.", "",
     "Pick berries that crumble off the cap easily.", MO(7) | MO(8), SPR_BERRY_CLUSTER,
     Biome::Forest},

    {"Salal", "berry", "Evergreen leathery leaf, dark blue-black berry.", "",
     "Strip ripe dark berries off the flower spike.", MO(8) | MO(9), SPR_BERRY_CLUSTER,
     Biome::Forest},

    {"Red Huckleberry", "berry", "Bright red berry on angled green twigs on stumps.", "",
     "Check old nurse logs and stumps in shade.", MO(7) | MO(8), SPR_BERRY_CLUSTER, Biome::Forest},

    {"Blue Elderberry", "berry", "Powdery blue-black clusters on tall shrubs.",
     "Raw berries, seeds, leaves, and stems cause nausea -- cook and discard stems.",
     "Cut whole clusters, then cook before eating.", MO(8) | MO(9), SPR_BERRY_CLUSTER,
     Biome::Forest},

    {"Serviceberry", "berry", "Saskatoon. Sweet purple fruit in loose clusters.", "",
     "Strip ripe purple berries into a bag by hand.", MO(6) | MO(7), SPR_BERRY_CLUSTER,
     Biome::Forest},

    {"Kinnikinnick", "berry", "Bearberry. Mealy red fruit on trailing mats.",
     "Mealy and bland raw -- best cooked or dried, not a toxicity risk.",
     "Best simmered into syrup; raw fruit is dry.", MO(9) | MO(10) | MO(11), SPR_BERRY_CLUSTER,
     Biome::Forest},

    {"Trailing Blackberry", "berry", "Native bramble, low vines, smaller sweeter fruit.", "",
     "Search low along the ground, not tall canes.", MO(7) | MO(8), SPR_BERRY_CLUSTER,
     Biome::Forest},

    {"Indian Plum", "berry", "Earliest native bloom; bitter-to-sweet oval fruit.",
     "Seeds/pits contain cyanogenic compounds -- eat the flesh, not the pit.",
     "Wait for fruit to turn dark blue-black.", MO(6) | MO(7), SPR_BERRY_CLUSTER, Biome::Forest},

    {"Wild Strawberry", "berry", "Tiny intensely sweet fruit in forest openings.", "",
     "Search sunny forest-edge clearings and old trails.", MO(6) | MO(7), SPR_BERRY_CLUSTER,
     Biome::Forest},

    {"Bunchberry", "berry", "Dwarf dogwood, tight red berry cluster on the floor.",
     "Bland and seedy but not toxic -- mostly a trailside nibble.",
     "Look on mossy forest floor for the red cluster.", MO(8) | MO(9), SPR_BERRY_CLUSTER,
     Biome::Forest},

    {"Wild Ginger", "root", "Heart-shaped leaf, gingery smell when crushed.",
     "Contains aristolochic-acid relatives -- a trailside sniff, not regular eating.",
     "Crush a leaf to confirm the ginger scent first.", MO(9) | MO(10), SPR_RAMP, Biome::Forest},

    {"Sheep Sorrel", "green", "Arrow-shaped leaf, sharp lemony bite.",
     "High in oxalic acid -- a trailside nibble, not a meal.",
     "Pinch young leaves from sunny disturbed ground.", MO(4) | MO(5) | MO(6), SPR_LEAFY_GREEN,
     Biome::Forest},

    {"Curly Dock", "green", "Wavy-edged dock leaf, tangy cooked green.",
     "High in oxalic acid; cook young leaves and eat in moderation.",
     "Take young spring leaves before they toughen.", MO(3) | MO(4), SPR_LEAFY_GREEN,
     Biome::Forest},

    {"Cleavers", "green", "Sticky, hooked stems that cling to clothing.",
     "Fine hooked hairs make it rough raw -- steep as tea instead.",
     "Snip young tips for tea before flowers form.", MO(3) | MO(4) | MO(5), SPR_LEAFY_GREEN,
     Biome::Forest},

    {"Dandelion", "green", "Toothed leaf rosette, bitter green, yellow bloom.", "",
     "Pick young leaves before the flower stalk rises.", MO(2) | MO(3) | MO(4) | MO(5),
     SPR_LEAFY_GREEN, Biome::Forest},

    {"Plantain", "green", "Ribbed oval leaf in every lawn and trail edge.", "",
     "Pick tender young leaves from unsprayed ground.", MO(5) | MO(6) | MO(7), SPR_LEAFY_GREEN,
     Biome::Forest},

    {"Watercress", "green", "Peppery green mat rooted in cold running water.",
     "Harvest only from clean, fast-flowing water -- can carry liver fluke from livestock runoff.",
     "Gather from clean, fast-flowing streams only.", MO(4) | MO(5) | MO(10), SPR_LEAFY_GREEN,
     Biome::Forest},

    {"Wild Mint", "green", "Square stem, minty scent, streamside patches.", "",
     "Crush a leaf to confirm the mint scent first.", MO(6) | MO(7) | MO(8), SPR_LEAFY_GREEN,
     Biome::Forest},

    {"Self-heal", "flower", "Small purple flower spike in lawns and trailsides.", "",
     "Snip the flowering tops for tea or salad.", MO(6) | MO(7) | MO(8), SPR_FLOWER_CLUSTER,
     Biome::Forest},

    {"Cow Parsnip", "green", "Giant celery-like shoot, umbrella flower heads.",
     "Sap causes severe light-triggered burns -- confusable with deadly poison hemlock and "
     "hogweed.",
     "Peel young stalks fast; avoid skin contact with sap.", MO(4) | MO(5), SPR_FIDDLEHEAD,
     Biome::Forest},

    {"Salmonberry Shoots", "green", "Peeled spring shoots, mild cucumber-like crunch.",
     "Peel the fibrous outer skin before eating raw.",
     "Snap tender new shoots before leaves unfurl.", MO(3) | MO(4), SPR_FIDDLEHEAD, Biome::Forest},

    {"Licorice Fern", "root", "Small fern on mossy maple limbs, sweet rhizome.", "",
     "Peel the rhizome off maple bark and chew raw.", MO(10) | MO(11) | MO(12) | MO(1), SPR_RAMP,
     Biome::Forest},

    {"Camas", "root", "Blue-flowered lily bulb, a traditional root staple.",
     "Only dig bulbs you watched bloom blue -- dormant ones resemble deadly death camas.",
     "Mark plants at bloom, then dig after flowers fade.", MO(6) | MO(7), SPR_RAMP, Biome::Forest},

    {"Wapato", "root", "Arrowhead-leaf pond plant, starchy tuber.", "",
     "Tread mud with bare feet until tubers float up.", MO(9) | MO(10) | MO(11), SPR_RAMP,
     Biome::Forest},

    {"Biscuitroot", "root", "Feathery carrot-family leaf, starchy taproot.",
     "Carrot-family lookalikes include deadly poison hemlock and water hemlock -- confirm ID "
     "carefully.",
     "Dig in open rocky soil; confirm by smell and leaf.", MO(3) | MO(4) | MO(5), SPR_RAMP,
     Biome::Forest},

    {"Oregon Grape Root", "root", "Bright yellow woody root under holly-like leaves.",
     "Contains berberine -- use as a tonic in small amounts, not a food.",
     "Dig a side root, leave the main plant rooted.", MO(10) | MO(11) | MO(12), SPR_RAMP,
     Biome::Forest},

    {"Beaked Hazelnut", "nut", "Native filbert in a bristly beaked husk.",
     "Husks have irritating fine bristles -- wear gloves when hulling.",
     "Gather husked nuts once they drop to the ground.", MO(8) | MO(9), SPR_BERRY_CLUSTER,
     Biome::Forest},

    {"Garry Oak Acorn", "nut", "Oak acorn needing leaching before eating.",
     "Raw acorns are bitter with tannins -- must be leached in water before eating.",
     "Shell, chop, and soak in changed water to leach tannin.", MO(9) | MO(10), SPR_BERRY_CLUSTER,
     Biome::Forest},

    {"Cascara Bark", "bark", "Chittam bark, historic mild-laxative remedy.",
     "Potent natural laxative -- bark must be aged or dried before use, in small doses only.",
     "Strip small bark sections, then dry or age before use.", MO(4) | MO(5) | MO(6), SPR_CORAL,
     Biome::Forest},

    {"Bigleaf Maple Sap", "sap", "Late-winter sap, boils down like maple syrup.", "",
     "Tap trunks during a cold snap before buds break.", MO(1) | MO(2), SPR_LEAFY_GREEN,
     Biome::Forest},

    {"Cottonwood Buds", "bud", "Resinous balm-of-Gilead bud, sweet balsam scent.",
     "Sticky resin can irritate sensitive skin -- for salves, not raw eating.",
     "Pinch sticky buds before leaves unfurl for salve.", MO(1) | MO(2) | MO(3), SPR_FLOWER_CLUSTER,
     Biome::Forest},

    {"Red Flowering Currant Blossom", "flower", "Pink-red spring bloom, hummingbird favorite.", "",
     "Snip open clusters for a mild sweet garnish.", MO(3) | MO(4), SPR_FLOWER_CLUSTER,
     Biome::Forest},

    {"Nodding Onion", "root", "Wild onion, pink drooping flower umbel.",
     "Confirm the onion smell -- toxic death camas grows in the same meadows without it.",
     "Confirm the onion smell, then pull the small bulb.", MO(6) | MO(7), SPR_RAMP, Biome::Forest},

    {"Common Yarrow", "flower", "Feathery leaf, flat white flower clusters.",
     "Avoid large medicinal doses in pregnancy; can irritate sensitive skin.",
     "Cut flowering tops in full bloom for drying.", MO(6) | MO(7) | MO(8), SPR_FLOWER_CLUSTER,
     Biome::Forest},

    {"Oxeye Daisy", "green", "White-petaled daisy, edible bitter young leaf.", "",
     "Pick basal leaves before the flower stalk bolts.", MO(4) | MO(5), SPR_LEAFY_GREEN,
     Biome::Forest},

    {"Wax Currant", "berry", "Tart orange-red berries on dry, rocky mountain slopes.", "",
     "Pick plump translucent berries; sticky sap on the bush is normal.", MO(7) | MO(8),
     SPR_BERRY_CLUSTER, Biome::Mountain},

    {"Red-Flowering Currant", "berry", "Bland blue-black fruit follows the pink spring bloom.", "",
     "Best cooked into jam -- raw berries are mealy and mild.", MO(7) | MO(8), SPR_BERRY_CLUSTER,
     Biome::Mountain},

    {"Oregon Grape", "berry", "Holly-like leaves, sour blue berries for jelly.",
     "Very tart and seedy raw -- best cooked and strained.",
     "Strip clusters once berries turn deep dusty blue.", MO(8) | MO(9), SPR_BERRY_CLUSTER,
     Biome::Mountain},

    {"Fireweed", "green", "Tall pink spikes on burns; shoots taste like asparagus.", "",
     "Snap young shoots under 8 inches before they toughen.", MO(5) | MO(6) | MO(7), SPR_FIDDLEHEAD,
     Biome::Mountain},

    {"American Bistort", "root", "Pink bottlebrush flowers over a chestnut-like root.", "",
     "Dig roots in alpine turf after the flower spike fades; roast like chestnuts.",
     MO(8) | MO(9) | MO(10), SPR_LEAFY_GREEN, Biome::Mountain},

    {"Spring Beauty", "root", "Tiny pink-veined flowers over a marble-sized corm.", "",
     "Dig right at the snowmelt edge; corms are shallow and easy to lift.", MO(5) | MO(6) | MO(7),
     SPR_LEAFY_GREEN, Biome::Mountain},

    {"Glacier Lily", "root", "Nodding yellow bloom right at the snow's retreating edge.", "",
     "Dig sparingly -- corms take years to form; take only a few per patch.", MO(6) | MO(7),
     SPR_FLOWER_CLUSTER, Biome::Mountain},

    {"Labrador Tea", "green", "Fuzzy-backed leaves in subalpine bogs, brewed as tea.",
     "Contains toxic compounds in quantity -- steep weak, small amounts only, skip if pregnant.",
     "Pick a few leathery leaves, dry them before brewing.", MO(6) | MO(7) | MO(8), SPR_LEAFY_GREEN,
     Biome::Mountain},

    {"Yarrow", "flower", "Feathery leaves, flat white flower heads in dry meadows.",
     "Mild in tea, but can irritate skin or upset stomach in large doses.",
     "Snip flower heads in full bloom; leaves work fresh or dried.", MO(6) | MO(7) | MO(8),
     SPR_FLOWER_CLUSTER, Biome::Mountain},

    {"Indian Rhubarb", "green", "Umbrella-leaf plant along mountain streambanks.",
     "High in oxalic acid -- a trailside nibble, not a meal.",
     "Peel young stalks before the leaf fully unfurls; cook like rhubarb.", MO(5) | MO(6),
     SPR_LEAFY_GREEN, Biome::Mountain},

    {"Aspen Bolete", "mushroom", "Orange-capped bolete under eastside aspen groves.",
     "Pores, not gills, confirm it's a bolete -- cook thoroughly, flesh darkens when cut.",
     "Look in aspen stands after August rain; check the stem base for grubs.", MO(8) | MO(9),
     SPR_PORCINI, Biome::Mountain},

    {"Delicious Milk Cap", "mushroom", "Orange latex weeps when cut -- a clear, safe ID.",
     "Confirm the orange latex turns green-ish on exposure; that's the key ID trait.",
     "Look under pine at mid-elevation after fall rain; nick the gills to check the latex.",
     MO(9) | MO(10), SPR_MUSHROOM_CAP, Biome::Mountain},

    {"Hawk's Wing", "mushroom", "Scaly cap, toothed underside, in conifer duff.",
     "Teeth instead of gills make ID easy; older caps turn bitter -- pick young.",
     "Look on mossy conifer slopes; younger, paler specimens taste best.", MO(9) | MO(10),
     SPR_MUSHROOM_TOOTH, Biome::Mountain},

    {"Bear's Head Tooth", "mushroom", "Cascading white icicle spines on conifer logs.",
     "No toxic lookalikes -- shape and spines make ID easy.",
     "Check dead or dying conifer trunks and stumps at mid-elevation.", MO(9) | MO(10),
     SPR_MUSHROOM_TOOTH, Biome::Mountain},

    {"Wila (Horsehair Lichen)", "lichen", "Black hair-lichen draped on subalpine conifer limbs.",
     "Must be pit-cooked long and slow -- raw or underprepared it's inedible and some lookalikes "
     "are toxic.",
     "Gather only dark blackish-brown strands, never yellow-green ones.",
     MO(10) | MO(11) | MO(12) | MO(1), SPR_CORAL, Biome::Mountain},

    {"Rock Tripe", "lichen", "Leathery lichen patches clinging to alpine boulders.",
     "Bitter acids must be boiled or soaked out before eating.",
     "Peel dry, brittle patches off sun-exposed rock faces.", MO(7) | MO(8) | MO(9), SPR_CORAL,
     Biome::Mountain},

    {"Old Man's Beard", "lichen", "Pale green strands draping subalpine conifer branches.",
     "Confirm the stretchy white inner core -- toxic lookalike lichens lack it.",
     "Gather fallen strands off the ground after wind, no need to strip branches.",
     MO(6) | MO(7) | MO(8) | MO(9), SPR_CORAL, Biome::Mountain},

    {"Grouse Whortleberry", "berry", "Pea-sized red berries carpeting ground right at treeline.",
     "", "Comb low mats with fingers -- tedious, but the berries are intensely sweet.",
     MO(8) | MO(9), SPR_BERRY_CLUSTER, Biome::Mountain},

    {"Black Raspberry", "berry", "Blackcap. Purple-black cap pulls free of a dry cup.", "",
     "Found in mountain clearcuts and burns; ripe caps detach clean, leaving the core behind.",
     MO(7) | MO(8), SPR_BERRY_CLUSTER, Biome::Mountain},

    {"Twisted Stalk", "berry", "Watermelon berry. Red drops hang singly under the leaves.",
     "Mildly laxative in quantity -- a trailside treat, not a basketful.",
     "Check under the zigzag stem in moist subalpine forest for single hanging berries.",
     MO(8) | MO(9), SPR_BERRY_CLUSTER, Biome::Mountain},

    {"Soapberry", "berry", "Bitter red berries traditionally whipped into a froth.",
     "Sharp, bitter raw and mildly upsetting in quantity -- best whipped and sweetened.",
     "Strip clusters off dry eastside mountain shrubs after they turn translucent red.",
     MO(8) | MO(9), SPR_BERRY_CLUSTER, Biome::Mountain},

    {"Whitebark Pine Nuts", "pine nut", "Fat, buttery seed from a threatened high-elevation pine.",
     "Endangered tree -- gather only from fallen cones, never harm live trees.",
     "Look for cones already dropped below snags near ridgelines.", MO(8) | MO(9),
     SPR_BERRY_CLUSTER, Biome::Mountain},

    {"Western White Pine Nuts", "pine nut",
     "Long cones with small, sweet seeds tucked under each scale.", "",
     "Collect fresh-fallen cones and pry the scales open for seeds.", MO(9) | MO(10),
     SPR_BERRY_CLUSTER, Biome::Mountain},

    {"Lodgepole Pine Cambium", "cambium", "Sweet inner bark, peeled in a short spring window only.",
     "Strip only a small patch of bark -- girdling kills the tree.",
     "Peel a narrow strip of outer bark, scrape the white cambium beneath.", MO(5) | MO(6),
     SPR_FIDDLEHEAD, Biome::Mountain},

    {"Western Hemlock Cambium", "cambium",
     "Inner bark once dried and pounded into cakes by coastal peoples.",
     "Only take from downed trees or a narrow strip, never a full ring.",
     "Scrape cambium from a fallen trunk rather than a live one.", MO(4) | MO(5), SPR_FIDDLEHEAD,
     Biome::Mountain},

    {"Western Larch Gum", "sap", "Hardened sap chewed like gum -- a subalpine forest treat.", "",
     "Look for amber globs on trunk wounds and scrape off clean bits.",
     MO(6) | MO(7) | MO(8) | MO(9), SPR_FIDDLEHEAD, Biome::Mountain},

    {"Engelmann Spruce Tips", "tree tip",
     "Bright citrus-flavored new growth at high-elevation branch ends.",
     "Confirm true spruce -- flat single needles rule out toxic yew.",
     "Pinch only the pale, papery new tips, a few per branch.", MO(5) | MO(6), SPR_FLOWER_CLUSTER,
     Biome::Mountain},

    {"Subalpine Fir Tips", "tree tip", "Balsam-scented new growth right at the treeline.",
     "Don't confuse with Pacific yew -- yew needles are toxic.",
     "Pinch pale new candle growth from lower, reachable branches.", MO(5) | MO(6),
     SPR_FLOWER_CLUSTER, Biome::Mountain},

    {"Grand Fir Tips", "tree tip", "Glossy needles with a bright orange-citrus scent when crushed.",
     "Don't confuse with Pacific yew -- yew needles are toxic.",
     "Take a few soft new tips per branch and leave the rest.", MO(4) | MO(5), SPR_FLOWER_CLUSTER,
     Biome::Mountain},

    {"Glacier Lily Corm", "root", "Yellow snowmelt bloom hiding a once-staple corm below.",
     "Slow to mature -- dig only a few per patch, it takes years to regrow.",
     "Dig gently right at the melting snowline where blooms cluster.", MO(6) | MO(7), SPR_RAMP,
     Biome::Mountain},

    {"Spring Beauty Corm", "root", "Tiny 'mountain potato' corm beneath an early pink flower.", "",
     "Dig just after snowmelt wherever the small pink flowers carpet the ground.", MO(6) | MO(7),
     SPR_RAMP, Biome::Mountain},

    {"Bitterroot", "root", "Fleshy taproot in rocky openings; peel away the bitter skin.", "",
     "Dig before the flower opens, when the root is plumpest.", MO(5) | MO(6), SPR_RAMP,
     Biome::Mountain},

    {"Biscuitroot (Cous)", "root", "Parsnip-like root historically ground into a trail flour.",
     "Carrot-family root -- easy to confuse with toxic hemlock roots without expert ID.",
     "Dig on dry rocky slopes before the flowering stalks dry out.", MO(5) | MO(6), SPR_RAMP,
     Biome::Mountain},

    {"Yampah", "root", "Sweet carrot-family tuber found in damp mountain meadows.",
     "Grows near deadly water hemlock -- confirm ID with an expert before digging.",
     "Dig moist meadow clumps only once flowers confirm the true plant.", MO(7) | MO(8), SPR_RAMP,
     Biome::Mountain},

    {"Balsamroot", "root", "Big yellow sunflower on dry slopes; root needs long, slow cooking.", "",
     "Dig deep, fibrous taproots in fall after the seeds have set.", MO(9) | MO(10), SPR_RAMP,
     Biome::Mountain},

    {"Camas Bulb", "root", "Blue-flowered meadow bulb, historically pit-roasted for weeks.",
     "Deadly death camas looks identical once flowers fade -- dig only while in bloom.",
     "Dig only when the blue flowers are visible, never past bloom.", MO(5) | MO(6), SPR_RAMP,
     Biome::Mountain},

    {"Osha Root", "root", "Aromatic 'bear root,' a prized alpine medicinal with a dark twin.",
     "Carrot-family root that closely resembles deadly hemlock -- expert ID only.",
     "Snap the root and check for a strong celery-spice smell before harvesting.",
     MO(7) | MO(8) | MO(9), SPR_RAMP, Biome::Mountain},

    {"Angelica Root", "root",
     "Aromatic streamside root and another carrot-family hemlock look-alike.",
     "Grows beside deadly water hemlock in wet ground -- misidentification can be fatal.",
     "Dig streamside clumps only after confirming ID with a local expert.", MO(7) | MO(8), SPR_RAMP,
     Biome::Mountain},

    {"Valerian Root", "root", "Pungent subalpine meadow root brewed into a calming tea.",
     "Mild sedative -- avoid combining with alcohol or sedating medication.",
     "Dig damp meadow clumps in fall once the leaves start to yellow.", MO(9) | MO(10), SPR_RAMP,
     Biome::Mountain},

    {"Pearly Everlasting", "herb", "Papery white everlasting bloom, dried for a mild herbal tea.",
     "", "Cut flowering stalks and hang them to dry before storing.", MO(7) | MO(8) | MO(9),
     SPR_LEAFY_GREEN, Biome::Mountain},

    {"Cow Parsnip Shoots", "shoot",
     "Celery-like stalk peeled young; a genuine carrot-family risk plant.",
     "Sap causes severe sun blistering, and it resembles deadly poison hemlock.",
     "Peel young stalks fully and wear gloves, avoiding sun on bare skin.", MO(5) | MO(6),
     SPR_FIDDLEHEAD, Biome::Mountain},

    {"Fireweed Shoots", "shoot", "Asparagus-like spring shoot from the tall pink-flowered burns.",
     "", "Snap tender shoots under 8 inches before they toughen.", MO(4) | MO(5), SPR_FIDDLEHEAD,
     Biome::Mountain},

    {"Fireweed Flowers", "flower",
     "Tall pink flower spike that fills mountain burns and clearings.", "",
     "Strip petals for jelly or tea before the seed fluff appears.", MO(7) | MO(8),
     SPR_FLOWER_CLUSTER, Biome::Mountain},

    {"Kinnikinnick Berries", "berry", "Mealy red berry in low evergreen mats on dry alpine ground.",
     "", "Pick red berries late season; best simmered, they're bland raw.", MO(9) | MO(10),
     SPR_BERRY_CLUSTER, Biome::Mountain},

    {"Juniper Berries", "berry", "Blue-black cone scale used whole as a savory mountain spice.",
     "Confirm true common juniper -- don't confuse with toxic yew berries nearby.",
     "Pick fully blue-black cones and skip any still green.", MO(9) | MO(10) | MO(11),
     SPR_BERRY_CLUSTER, Biome::Mountain},

    {"Mountain Ash Berries", "berry",
     "Orange berry cluster on subalpine shrubs, bitter until cooked.",
     "Mildly toxic raw -- cook or freeze before eating any quantity.",
     "Pick after frost, then cook; raw berries upset the stomach.", MO(9) | MO(10),
     SPR_BERRY_CLUSTER, Biome::Mountain},

    {"Red Elderberry", "berry",
     "Red berry cluster by mountain streams, riskier than blue elderberry.",
     "Raw berries, seeds, and stems are toxic -- must be cooked and strained.",
     "Cook thoroughly and strain out seeds; never eat raw.", MO(6) | MO(7), SPR_BERRY_CLUSTER,
     Biome::Mountain},

    {"Pipsissewa", "herb", "Waxy evergreen leaf whorl used to flavor a traditional root tea.", "",
     "Snip a few leathery leaves per plant from the shaded forest floor.",
     MO(6) | MO(7) | MO(8) | MO(9), SPR_LEAFY_GREEN, Biome::Mountain},

    {"Nori (Laver)", "seaweed", "Purple-black sheets on rocks. Dries paper-thin in sun.", "",
     "Peel thin sheets off high rocks at the lowest winter tides.",
     MO(10) | MO(11) | MO(12) | MO(1) | MO(2) | MO(3), SPR_CORAL, Biome::Coast},

    {"Bladderwrack", "seaweed", "Forked brown fronds with paired air bladders.", "",
     "Snip fronds above the holdfast; the bladders pop when ripe.",
     MO(4) | MO(5) | MO(6) | MO(7) | MO(8), SPR_LEAFY_GREEN, Biome::Coast},

    {"Winged Kelp", "seaweed", "Long brown blade with a pale edible midrib.",
     "Strip the tough wing from the midrib before eating.",
     "Cut young blades before the midrib turns fibrous.", MO(5) | MO(6) | MO(7) | MO(8),
     SPR_LEAFY_GREEN, Biome::Coast},

    {"Sea Palm", "seaweed", "Palm-like stipe braced in the crashing surf zone.",
     "Grows only where waves are heaviest -- footing is the real risk.",
     "Cut a few fronds per holdfast so the cluster regrows.", MO(6) | MO(7) | MO(8),
     SPR_LEAFY_GREEN, Biome::Coast},

    {"Feather Boa Kelp", "seaweed", "Long feathered blade strand, chewy and briny.", "",
     "Gather fresh drift strands rather than tearing live fronds.", MO(6) | MO(7) | MO(8) | MO(9),
     SPR_LEAFY_GREEN, Biome::Coast},

    {"Turkish Towel", "seaweed", "Thick rubbery red blade, rough like a washcloth.", "",
     "Peel whole blades from low rocks at a minus tide.", MO(6) | MO(7) | MO(8) | MO(9), SPR_CORAL,
     Biome::Coast},

    {"Turkish Washcloth", "seaweed", "Dark curled red-black blades, papery when dry.", "",
     "Pinch fronds above the holdfast on mid-tide rocks.", MO(4) | MO(5) | MO(6) | MO(7) | MO(8),
     SPR_CORAL, Biome::Coast},

    {"Sea Sac", "seaweed", "Golf-ball brown blobs stuck fast to low rocks.", "",
     "Twist whole sacs free at the lowest summer tides.", MO(6) | MO(7) | MO(8), SPR_CORAL,
     Biome::Coast},

    {"Iridescent Seaweed", "seaweed", "Blue-green shimmer on wine-red blades underwater.", "",
     "Look in tidepools for the telltale iridescent sheen.", MO(5) | MO(6) | MO(7) | MO(8),
     SPR_CORAL, Biome::Coast},

    {"Sea Cabbage", "seaweed", "Ruffled golden blade, milder cousin of sugar kelp.", "",
     "Cut blades an inch above the holdfast at low tide.", MO(4) | MO(5) | MO(6) | MO(7),
     SPR_LEAFY_GREEN, Biome::Coast},

    {"Sea Hair", "seaweed", "Hollow bright-green tubes tangled on tidepool rocks.",
     "Rinse thoroughly -- traps fine sand inside the tube.",
     "Pull loose green tangles from mid-tide rocks and pools.", MO(4) | MO(5) | MO(6) | MO(7),
     SPR_LEAFY_GREEN, Biome::Coast},

    {"Giant Kelp", "seaweed", "Towering brown kelp forest off the open coast.", "",
     "Gather fresh blades adrift after a swell, not the holdfast.",
     MO(6) | MO(7) | MO(8) | MO(9) | MO(10), SPR_LEAFY_GREEN, Biome::Coast},

    {"Beach Sandwort", "green", "Succulent dune leaves in low mats above the tideline.", "",
     "Pinch tender tips before the plant flowers and toughens.", MO(4) | MO(5) | MO(6),
     SPR_LEAFY_GREEN, Biome::Coast},

    {"American Searocket", "green", "Fleshy dune plant, peppery like arugula.", "",
     "Pick young leaves and green pods before they woodify.", MO(6) | MO(7) | MO(8) | MO(9),
     SPR_LEAFY_GREEN, Biome::Coast},

    {"Orache", "green", "Silvery-green spinach relative on the upper beach.", "",
     "Snip young top leaves before the seed heads form.", MO(5) | MO(6) | MO(7) | MO(8),
     SPR_LEAFY_GREEN, Biome::Coast},

    {"Springbank Clover", "root", "Traditional root vegetable from tidal marsh meadows.",
     "Historically cultivated garden plots -- ask before digging.",
     "Dig slender roots in fall once the leaves die back.", MO(9) | MO(10) | MO(11), SPR_RAMP,
     Biome::Coast},

    {"Scurvy Grass", "green", "Small round leaves, peppery like watercress.", "",
     "Pick tender rosettes from rocky splash-zone crevices.", MO(3) | MO(4) | MO(5),
     SPR_LEAFY_GREEN, Biome::Coast},

    {"Sea Coast Angelica", "green", "Celery-scented stalks in coastal meadows and bluffs.",
     "Apiaceae family -- confirm ID carefully, hemlock lookalikes kill.",
     "Peel young spring stalks before the flower stem hardens.", MO(4) | MO(5) | MO(6),
     SPR_FLOWER_CLUSTER, Biome::Coast},

    {"Beach Lovage", "green", "Wild celery leaf, glossy and salty-sweet.",
     "Apiaceae family -- confirm ID carefully, hemlock lookalikes kill.",
     "Snip young leaves from low bluff-top patches in spring.", MO(4) | MO(5) | MO(6),
     SPR_LEAFY_GREEN, Biome::Coast},

    {"Pacific Oyster", "shellfish", "Rock-cemented bivalve on sheltered tideflat beaches.",
     "Check PSP/biotoxin closures before eating any raw shellfish.",
     "Pry from rock clusters at a minus tide, shuck fresh.",
     MO(9) | MO(10) | MO(11) | MO(12) | MO(1) | MO(2) | MO(3) | MO(4), SPR_MOREL, Biome::Coast},

    {"Bay Mussel", "shellfish", "Blue-black shells clustered on pilings and rocks.",
     "Avoid summer months -- biotoxin quarantines run roughly Jul-Oct.",
     "Twist clusters free from rocks at the lowest tides.",
     MO(11) | MO(12) | MO(1) | MO(2) | MO(3) | MO(4) | MO(5) | MO(6), SPR_MOREL, Biome::Coast},

    {"Manila Littleneck Clam", "shellfish", "Small ridged clam just under the sand surface.",
     "Check state biotoxin/PSP closures before digging.",
     "Dig 2-4 inches down in gravelly sand at low tide.",
     MO(10) | MO(11) | MO(12) | MO(1) | MO(2) | MO(3) | MO(4), SPR_MOREL, Biome::Coast},

    {"Butter Clam", "shellfish", "Deep-digging clam, thick chalky-white shell.",
     "Holds PSP toxin longest of any local clam -- check closures.",
     "Dig a foot down in firm sand-gravel beaches.",
     MO(10) | MO(11) | MO(12) | MO(1) | MO(2) | MO(3), SPR_MOREL, Biome::Coast},

    {"Geoduck", "shellfish", "Giant burrowing clam with a long siphon neck.",
     "Check state biotoxin closures and permit rules before digging.",
     "Follow the siphon 'show' straight down with a tube and shovel.",
     MO(10) | MO(11) | MO(12) | MO(1) | MO(2) | MO(3) | MO(4), SPR_MOREL, Biome::Coast},

    {"Gooseneck Barnacle", "shellfish", "Leathery stalked barnacle clumped on surf-battered rock.",
     "Harvest only where locally legal -- some areas restrict picking.",
     "Twist clusters from wave-exposed rock at the lowest tides.", MO(3) | MO(4) | MO(5) | MO(6),
     SPR_MOREL, Biome::Coast},

    {"Dungeness Crab", "shellfish", "Iconic sweet-meat crab of eelgrass and sandy flats.",
     "Check size, sex, and season limits before keeping any crab.",
     "Set rings baited with fish near eelgrass at high tide.",
     MO(7) | MO(8) | MO(9) | MO(10) | MO(11) | MO(12), SPR_MOREL, Biome::Coast},

    {"Red Rock Crab", "shellfish", "Smaller, chunky-clawed crab under low-tide rocks.",
     "Check size and daily limits before keeping any crab.",
     "Flip low-tide rocks and boulders to find them hiding.",
     MO(4) | MO(5) | MO(6) | MO(7) | MO(8) | MO(9), SPR_MOREL, Biome::Coast},

    {"Black Katy Chiton", "shellfish", "Leathery black armor-plated grazer on open rock.",
     "Tough and strong-flavored -- tenderize well before eating.",
     "Pry gently from open rock faces at the lowest tides.", MO(4) | MO(5) | MO(6) | MO(7) | MO(8),
     SPR_MOREL, Biome::Coast},

    {"Red Sea Urchin", "shellfish", "Long-spined purple-red urchin, prized for its roe.",
     "Handle spines with care; roe quality varies by season.",
     "Look in subtidal rock crevices at the lowest minus tides.", MO(5) | MO(6) | MO(7) | MO(8),
     SPR_MOREL, Biome::Coast},

    {"Limpet", "shellfish", "Cone-shelled grazer clamped tight to open rock.", "",
     "Pop them loose fast with a knife before they clamp down.",
     MO(4) | MO(5) | MO(6) | MO(7) | MO(8), SPR_MOREL, Biome::Coast},

    {"Pacific Basket Cockle", "shellfish", "Ribbed round shell resting just under sandy gravel.",
     "Check state biotoxin/PSP closures before digging.",
     "Rake shallow sand-gravel flats a few inches down.",
     MO(10) | MO(11) | MO(12) | MO(1) | MO(2) | MO(3), SPR_MOREL, Biome::Coast},

    {"Horse Clam", "shellfish", "Large gaping clam with a leathery siphon tip.",
     "Check biotoxin closures; remove the tough siphon skin.",
     "Dig deep and fast once you spot its wide neck-hole.",
     MO(10) | MO(11) | MO(12) | MO(1) | MO(2) | MO(3), SPR_MOREL, Biome::Coast},

    {"Giant Red Sea Cucumber", "shellfish", "Warty red-orange grazer on subtidal rock and sand.",
     "Only the muscular body wall is eaten -- clean thoroughly.",
     "Hand-pick from subtidal rock at the lowest minus tides.", MO(5) | MO(6) | MO(7) | MO(8),
     SPR_MOREL, Biome::Coast},

    {"Moon Snail", "shellfish", "Large round predator snail plowing through tide-flat sand.",
     "Tough meat needs long tenderizing -- an acquired harvest.",
     "Follow its smooth plow-trail across exposed sand flats.", MO(5) | MO(6) | MO(7) | MO(8),
     SPR_MOREL, Biome::Coast},

    {"Purple Varnish Clam", "shellfish", "Shiny purple-brown shell, shallow in sandy beaches.",
     "Check state biotoxin/PSP closures before digging.",
     "Rake the top few inches of sand at any minus tide.",
     MO(3) | MO(4) | MO(5) | MO(6) | MO(7) | MO(8) | MO(9) | MO(10), SPR_MOREL, Biome::Coast},

    {"Manila Clam", "shellfish", "Ridged shell, common on protected bay tideflats.",
     "Check state shellfish biotoxin closures before digging.",
     "Dig 6-8 inches down at low tide with a rake or fork.",
     MO(10) | MO(11) | MO(12) | MO(1) | MO(2) | MO(3) | MO(4), SPR_MOREL, Biome::Coast},

    {"Pacific Littleneck Clam", "shellfish", "Native littleneck, deeper-ridged than the Manila.",
     "Check state shellfish biotoxin closures before digging.",
     "Found a bit deeper than Manila clams in gravelly sand.",
     MO(10) | MO(11) | MO(12) | MO(1) | MO(2) | MO(3) | MO(4), SPR_MOREL, Biome::Coast},

    {"Pacific Gaper Clam", "shellfish", "Big clam, siphon too large to fully retract.",
     "Trim and discard the dark rubbery siphon skin before eating.",
     "Look for a squirting hole in sand at an extreme low tide.",
     MO(10) | MO(11) | MO(12) | MO(1) | MO(2) | MO(3), SPR_MOREL, Biome::Coast},

    {"Basket Cockle", "shellfish", "Ribbed round shell, sits shallow in sand and mud.", "",
     "Rake the top few inches of sand near eelgrass beds.",
     MO(9) | MO(10) | MO(11) | MO(12) | MO(1) | MO(2) | MO(3), SPR_MOREL, Biome::Coast},

    {"Softshell Clam", "shellfish", "Thin brittle shell common in estuary mudflats.", "",
     "Dig soft mud near creek mouths; the shell breaks easily.",
     MO(9) | MO(10) | MO(11) | MO(12) | MO(1) | MO(2) | MO(3), SPR_MOREL, Biome::Coast},

    {"Varnish Clam", "shellfish", "Glossy purple-brown shell, an abundant newcomer.", "",
     "Shallow digging in mixed sand often fills a limit fastest.",
     MO(9) | MO(10) | MO(11) | MO(12) | MO(1) | MO(2) | MO(3) | MO(4), SPR_MOREL, Biome::Coast},

    {"California Mussel", "shellfish", "Large ridged mussel on wave-battered outer rocks.",
     "Skip harvest in warm months when biotoxin risk peaks.",
     "Pry from exposed surf rocks only on a good minus tide.",
     MO(9) | MO(10) | MO(11) | MO(12) | MO(1) | MO(2) | MO(3) | MO(4), SPR_MOREL, Biome::Coast},

    {"Olympia Oyster", "shellfish", "Tiny native oyster, coin-sized and slow-growing.",
     "Native beds are limited -- take only where restoration allows.",
     "Look on quiet bay shell substrate below the mussel line.",
     MO(9) | MO(10) | MO(11) | MO(12) | MO(1) | MO(2) | MO(3) | MO(4), SPR_MOREL, Biome::Coast},

    {"Kelp Crab", "crab", "Olive, spider-legged crab living in kelp beds.", "",
     "Hand-pick from kelp holdfasts at a low tide.", MO(6) | MO(7) | MO(8) | MO(9), SPR_MOREL,
     Biome::Coast},

    {"Graceful Crab", "crab", "Smaller cousin of Dungeness, a common bycatch.", "",
     "Found in the same pots and pools as Dungeness crab.", MO(7) | MO(8) | MO(9), SPR_MOREL,
     Biome::Coast},

    {"Purple Sea Urchin", "urchin", "Spiny purple test; the roe is the harvested part.",
     "Handle spines with gloves; check season and daily limits.",
     "Pry loose from low rocky tidepools at a minus tide.",
     MO(10) | MO(11) | MO(12) | MO(1) | MO(2) | MO(3), SPR_CORAL, Biome::Coast},

    {"Gumboot Chiton", "chiton", "Football-sized, rust-red 'wandering meatloaf.'",
     "Take only the muscular foot; harvest sparingly, population-sensitive.",
     "Pry gently from rock faces on the lowest spring tides.", MO(4) | MO(5) | MO(6) | MO(7),
     SPR_MOREL, Biome::Coast},

    {"Blue Top Snail", "snail", "Small conical shell grazing low tidepool rocks.", "",
     "Hand-pick from tidepools; boil briefly to free the foot.",
     MO(4) | MO(5) | MO(6) | MO(7) | MO(8) | MO(9), SPR_MOREL, Biome::Coast},

    {"Spot Prawn", "shrimp", "Sweet pink shrimp with a very short spring season.",
     "WA's recreational season is brief -- confirm exact open dates.",
     "Drop baited pots into deep water off the outer coast.", MO(5), SPR_MOREL, Biome::Coast},

    {"Pinto Abalone", "shellfish", "Ear-shaped shell snail, now critically depleted.",
     "Harvest is illegal in WA waters -- look, photograph, don't take.",
     "Spot clinging to subtidal rock; leave every one in place.",
     MO(4) | MO(5) | MO(6) | MO(7) | MO(8) | MO(9), SPR_MOREL, Biome::Coast},

    {"Evergreen Huckleberry", "berry", "Glossy coastal shrub, small dark berries into winter.", "",
     "Comb dense coastal understory shrubs after other berries fade.",
     MO(9) | MO(10) | MO(11) | MO(12), SPR_BERRY_CLUSTER, Biome::Coast},

    {"American Sea Rocket", "green", "Peppery succulent leaves on the driftwood line.", "",
     "Pick young leaves and pods before the stem toughens.", MO(5) | MO(6) | MO(7), SPR_LEAFY_GREEN,
     Biome::Coast},

    {"Common Camas", "root", "Blue-flowered bulb in coastal prairie meadows.",
     "Deadly death camas looks similar -- confirm blue flowers only.",
     "Dig bulbs after bloom in old Garry oak prairie meadows.", MO(5) | MO(6), SPR_FLOWER_CLUSTER,
     Biome::Coast},

    {"Chocolate Lily", "root", "Checkered maroon flower over a rice-grain bulb cluster.", "",
     "Dig bulbs in open coastal meadows just after bloom.", MO(4) | MO(5), SPR_FLOWER_CLUSTER,
     Biome::Coast},

    {"Sea Beach Sandwort", "green", "Succulent trailing leaves on open sand foredunes.", "",
     "Pinch fleshy leaf tips from mats just above the tideline.", MO(5) | MO(6) | MO(7),
     SPR_LEAFY_GREEN, Biome::Coast},

    {"Gem-Studded Puffball", "mushroom", "Warty white ball in duff; cut in half to check.",
     "Interior must be pure white throughout -- yellow or gill shadow means don't eat.",
     "Pick young, before the skin yellows or splits.", MO(9) | MO(10) | MO(11), SPR_MUSHROOM_CAP,
     Biome::Forest},

    {"Pear-Shaped Puffball", "mushroom", "Clustered puffballs on rotting conifer logs.",
     "Cut in half first -- must be solid white inside, no developing cap shape.",
     "Pop only the firm, all-white ones off old stumps.", MO(9) | MO(10) | MO(11), SPR_MUSHROOM_CAP,
     Biome::Forest},

    {"Pigskin Poison Puffball", "mushroom",
     "Thick, scaly rind and black marbled inside -- not edible.",
     "Toxic -- dark purple-black interior distinguishes it from true puffballs, which are pure "
     "white.",
     "Learn to recognize and leave it; it's a common lookalike hazard.", MO(9) | MO(10) | MO(11),
     SPR_MUSHROOM_CAP, Biome::Forest},

    {"Zeller's Bolete", "mushroom", "Dark maroon cap, yellow pores, blue bruise. Under conifers.",
     "Confirm pores bruise blue, not red; avoid soft or slimy caps.",
     "Firm caps only, cut base to check for tunneling grubs.", MO(9) | MO(10) | MO(11), SPR_PORCINI,
     Biome::Forest},

    {"Western Painted Suillus", "mushroom", "Red-streaked cap glued to Douglas fir needles. Slimy.",
     "Peel the sticky cap skin before cooking; grows only under Douglas fir.",
     "Twist gently -- the cap skin peels back like a label.", MO(9) | MO(10) | MO(11), SPR_PORCINI,
     Biome::Forest},

    {"Red-Cracked Bolete", "mushroom",
     "Cracked reddish-brown cap over yellow pores, in mixed duff.",
     "Flesh blues faintly on cutting -- normal for this one, not a warning sign.",
     "Best in young, firm specimens; pores turn mushy with age.", MO(9) | MO(10), SPR_PORCINI,
     Biome::Forest},

    {"Butter Bolete", "mushroom", "Smooth tan cap, thick stem, mild buttery flavor.",
     "Check pore color is pale yellow, not orange-red, before eating.",
     "Look on mossy ground under hemlock after fall rain.", MO(10) | MO(11), SPR_PORCINI,
     Biome::Forest},

    {"Crown-Tipped Coral", "mushroom",
     "Tan coral with tiny crown-shaped branch tips on rotten wood.",
     "Crown-shaped branch tips separate it from thinner, unpalatable corals.",
     "Cut whole clusters from downed logs; rinse grit from the base.", MO(9) | MO(10) | MO(11),
     SPR_CORAL, Biome::Forest},

    {"Pink Coral Fungus", "mushroom", "Salmon-pink branching coral in conifer duff.",
     "Bitter, brittle corals with acrid taste should be spit out and avoided.",
     "Taste-test a tiny raw sliver; bitter ones aren't worth cooking.", MO(10) | MO(11), SPR_CORAL,
     Biome::Forest},

    {"Pig's Ears", "mushroom", "Purple-brown, vase-shaped chanterelle relative in mossy duff.",
     "False gills like a chanterelle, not true blades -- safe ID feature.",
     "Look near hemlock roots where moss stays damp.", MO(9) | MO(10), SPR_MUSHROOM_CAP,
     Biome::Forest},

    {"Woolly Chanterelle", "mushroom", "Shaggy orange funnel, showy but upsets stomachs raw.",
     "Causes GI upset for many people -- cook thoroughly and try a small amount first.",
     "Easy to spot by its ragged, scaly orange cap.", MO(9) | MO(10), SPR_MUSHROOM_CAP,
     Biome::Forest},

    {"Coral Tooth Fungus", "mushroom",
     "Branching white coral studded with tiny icicle teeth on wounds in bigleaf maple.",
     "Distinguish by branching form from unbranched Bear's Head on the same logs.",
     "Cut the whole cluster where it grows from a wound scar.", MO(9) | MO(10) | MO(11),
     SPR_MUSHROOM_TOOTH, Biome::Forest},

    {"Jelly Tooth", "mushroom",
     "Translucent gray-brown jelly cap with tiny teeth beneath, on conifer logs.",
     "No toxic lookalikes, but bland -- best added to soups for texture.",
     "Pick after rain when it's fully plumped and rubbery.", MO(10) | MO(11) | MO(12),
     SPR_MUSHROOM_TOOTH, Biome::Forest},

    {"Wood Ear", "mushroom", "Rubbery brown ear-shaped shelves on dead alder and elder branches.",
     "No toxic lookalikes in the region, but cook thoroughly before eating.",
     "Pull whole cup-shaped clusters off the bark; dries well for storage.",
     MO(1) | MO(10) | MO(11) | MO(12), SPR_MUSHROOM_CAP, Biome::Forest},

    {"Amber Jelly Roll", "mushroom", "Small amber jelly blobs on dead willow and alder twigs.",
     "Not toxic, but flavorless -- mostly a winter curiosity, not a meal.",
     "Look after freeze-thaw cycles swell the jelly back up.", MO(1) | MO(11) | MO(12),
     SPR_MUSHROOM_CAP, Biome::Forest},

    {"Black Witch's Butter", "mushroom", "Dark olive-black jelly lumps on dead hardwood branches.",
     "Edible but rubbery and bland -- best simmered into broths, not eaten raw.",
     "Rehydrates from a shriveled black crust after rain -- pick it plump.",
     MO(1) | MO(11) | MO(12), SPR_MUSHROOM_CAP, Biome::Forest},

    {"Cinnabar Polypore", "mushroom",
     "Bright orange-red bracket fungus fused flat to dead hardwood.",
     "Too tough and woody to eat -- valued for its dye, not the table.",
     "Pry brackets whole off the log; the color persists when dried.",
     MO(9) | MO(10) | MO(11) | MO(12), SPR_CORAL, Biome::Forest},

    {"False Turkey Tail", "mushroom",
     "Fuzzy gray-brown shelving crust on stumps, no pore surface underneath.",
     "Too tough to eat -- smooth underside (not pored) tells it apart from true Turkey Tail.",
     "Good for tea steeping like true Turkey Tail, just simmer longer.",
     MO(9) | MO(10) | MO(11) | MO(12), SPR_CORAL, Biome::Forest},

    {"Split Gill", "mushroom",
     "Tiny fuzzy gray fan with split, forked gill folds on dead branches.",
     "Edible in small amounts once well cooked, but tough and better left alone.",
     "Common on any downed hardwood branch year-round after rain.",
     MO(1) | MO(10) | MO(11) | MO(12), SPR_MUSHROOM_CAP, Biome::Forest},

    {"Orange Peel Fungus", "mushroom",
     "Bright orange cup fungus scattered on bare, disturbed soil.",
     "Edible raw, but confirm the bright orange upper surface and pale underside.",
     "Look along gravel paths and disturbed forest edges after rain.", MO(9) | MO(10) | MO(11),
     SPR_MUSHROOM_CAP, Biome::Forest},

    {"Fried Chicken Mushroom", "mushroom",
     "Dense gray-brown clusters at the base of hardwoods, smells savory.",
     "Cook thoroughly; some people react to it raw or undercooked.",
     "Cut whole tight clusters at the stem base near the trunk.", MO(9) | MO(10) | MO(11),
     SPR_MUSHROOM_CAP, Biome::Forest},

    {"Honey Mushroom", "mushroom",
     "Honey-gold caps in dense clusters at stump bases, black shoestrings beneath bark.",
     "Must be cooked thoroughly; confirm the black rhizomorph threads under bark to rule out "
     "deadly Galerina.",
     "Twist clusters free at the stump; check for the telltale ring on the stem.",
     MO(9) | MO(10) | MO(11), SPR_MUSHROOM_CAP, Biome::Forest},

    {"The Prince", "mushroom",
     "Large white-capped Agaricus with an almond smell, in open forest duff.",
     "Confirm yellow bruising and almond smell -- avoid any with a chemical or inky odor.",
     "Look in park-like open stands and forest clearings after rain.", MO(9) | MO(10),
     SPR_MUSHROOM_CAP, Biome::Forest},

    {"Snow Bank False Morel", "mushroom",
     "Reddish-brown brain-like cap pushing up at the edge of melting snow.",
     "Toxic raw and even parboiled for some people -- true morels are hollow, this one is "
     "chambered, not worth the risk.",
     "Best left alone; note it only to avoid confusing it with true morels.", MO(4) | MO(5),
     SPR_MOREL, Biome::Forest},

    {"Shaggy Scalycap", "mushroom",
     "Shaggy yellow-brown scaled caps clustered thick at stump bases.",
     "Edible young but causes stomach upset in some people -- cook thoroughly, skip if unsure.",
     "Pick only young caps with tight, unopened scales.", MO(9) | MO(10) | MO(11), SPR_MUSHROOM_CAP,
     Biome::Forest},

    {"Blue Chanterelle", "mushroom", "Small violet-blue funnel, false gills, in deep conifer duff.",
     "False ridges like true chanterelles; color alone confirms it's not a lookalike.",
     "Easy to miss against blue-shadowed duff -- look closely, not just for orange.",
     MO(9) | MO(10), SPR_MUSHROOM_CAP, Biome::Forest},

    {"Fairy Ring Mushroom", "mushroom",
     "Small tan caps in rings across mossy lawns and forest clearings.",
     "Confirm the tough, pliable stem and ring pattern -- some ring-forming lookalikes are toxic.",
     "Pick only the caps; the stems are too tough to eat.", MO(9) | MO(10) | MO(11),
     SPR_MUSHROOM_CAP, Biome::Forest},

    {"Rooting Shank", "mushroom", "Long buried root-like stem base, mild cap, near buried wood.",
     "No toxic lookalikes locally, but always cook thoroughly.",
     "Dig gently to keep the long tapering stem base intact.", MO(9) | MO(10) | MO(11),
     SPR_MUSHROOM_CAP, Biome::Forest},

    {"Dyer's Polypore", "mushroom",
     "Soft yellow-orange shelf bracket at conifer bases, bleeds yellow when cut young.",
     "Edible only when very young and soft -- mature brackets turn corky and inedible.",
     "Press it -- if it's spongy-soft, it's young enough to cook.", MO(9) | MO(10), SPR_CORAL,
     Biome::Forest},

    {"Lungwort Lichen", "lichen",
     "Large lettuce-like green lichen lobes on old-growth bigleaf maple trunks.",
     "Historically used medicinally, but bitter and best left growing -- it's slow to regrow and "
     "indicates old-growth health.",
     "Best photographed, not picked -- a sign of clean air and old forest.",
     MO(9) | MO(10) | MO(11) | MO(12), SPR_CORAL, Biome::Forest},

    {"Powder Cap Lichen", "lichen", "Chalky pale-green crust dusting shaded conifer bark.",
     "Not a food source -- used only as a natural dye historically.",
     "Leave in place; scrape only tiny flakes for dye testing.", MO(1) | MO(10) | MO(11) | MO(12),
     SPR_CORAL, Biome::Forest},

    {"Sword Fern", "fern", "Evergreen forest fern; rhizome was an emergency staple.",
     "Rhizome is bitter and fibrous -- historically peeled, roasted, and used mainly in famine "
     "years.",
     "Dig only a rhizome section from an abundant patch, leave the crown.",
     MO(1) | MO(2) | MO(3) | MO(10) | MO(11) | MO(12), SPR_FIDDLEHEAD, Biome::Forest},

    {"Deer Fern Fiddlehead", "fern", "Low, glossy forest fern with slender upright fronds.",
     "Eat only young coiled fiddleheads, cooked; raw fronds are tough and gritty.",
     "Snap coiled fronds where they bend easily, leave most per plant.", MO(4) | MO(5),
     SPR_FIDDLEHEAD, Biome::Forest},

    {"Bracken Fern Fiddlehead", "fern", "Common trailside fern; historic food, modern caution.",
     "Contains a suspected carcinogen -- eat sparingly, always cooked well, never raw.",
     "Snap only tightly coiled young shoots before the frond unrolls.", MO(4) | MO(5),
     SPR_FIDDLEHEAD, Biome::Forest},

    {"Lady Fern Fiddlehead", "fern", "Lacy-fronded fern of moist forest floors and streambanks.",
     "Cook thoroughly; discard the papery brown scales before eating.",
     "Pick coils under 6 inches, tightly furled, from healthy clumps.", MO(4) | MO(5),
     SPR_FIDDLEHEAD, Biome::Forest},

    {"Cattail Shoots", "green", "Tall marsh reed; peeled inner core tastes like cucumber.",
     "Confirm the flat, strap-like leaves -- avoid marsh areas with runoff contamination.",
     "Pull the young shoot and peel down to the tender white core.", MO(4) | MO(5) | MO(6),
     SPR_LEAFY_GREEN, Biome::Forest},

    {"Cattail Pollen", "flower", "Golden pollen from summer seed heads, used as flour.", "",
     "Bag the pollen spike and shake into a bag on a still morning.", MO(6) | MO(7),
     SPR_FLOWER_CLUSTER, Biome::Forest},

};

static const int kSpeciesCount = sizeof(kSpecies) / sizeof(kSpecies[0]);

static const char* kSeasonNotes[12] = {
    "Deep winter. Slim pickings, prized finds.",      // Jan
    "Late winter. The first green returns.",          // Feb
    "Early spring. Morels and tender shoots.",        // Mar
    "Spring greens. Ferns uncurl, ramps rise.",       // Apr
    "Late spring. Flowers and sour leaves.",          // May
    "Early summer. Berries lead the way.",            // Jun
    "High summer. Berries up high, first 'shrooms.",  // Jul
    "Late summer. Chanterelle peak begins.",          // Aug
    "Early fall. The great mushroom flush.",          // Sep
    "Peak fall. Baskets fill fast after rain.",       // Oct
    "Late fall. Cold-hardy fungi hold on.",           // Nov
    "Early winter. Frost mushrooms only.",            // Dec
};

static int clampMonth(int month) {
  if (month < 1) return 1;
  if (month > 12) return 12;
  return month;
}

bool inSeason(const Forageable& f, int month) {
  return (f.monthMask & (1 << (clampMonth(month) - 1))) != 0;
}

int speciesCount() { return kSpeciesCount; }

const Forageable& speciesAt(int index) {
  index = std::max(index, 0);
  if (index >= kSpeciesCount) index = kSpeciesCount - 1;
  return kSpecies[index];
}

static uint8_t browseOrder[kSpeciesCount];
static bool browseOrderBuilt = false;

void rebuildBrowseOrder(int month, bool postRain) {
  month = clampMonth(month);
  int scores[kSpeciesCount];
  for (int i = 0; i < kSpeciesCount; i++) {
    browseOrder[i] = (uint8_t)i;
    int score = 0;
    if (inSeason(kSpecies[i], month)) score += 100;
    if (postRain && strcmp(kSpecies[i].kind, "mushroom") == 0) score += 30;
    score += random(40);  // shuffles ties/near-ties so it's not the same order every wake
    scores[i] = score;
  }
  // Insertion sort by score descending -- kSpeciesCount is small (~250), and
  // this only runs once per wake.
  for (int i = 1; i < kSpeciesCount; i++) {
    uint8_t keyIdx = browseOrder[i];
    int keyScore = scores[keyIdx];
    int j = i - 1;
    while (j >= 0 && scores[browseOrder[j]] < keyScore) {
      browseOrder[j + 1] = browseOrder[j];
      j--;
    }
    browseOrder[j + 1] = keyIdx;
  }
  browseOrderBuilt = true;
}

const Forageable& speciesAtRank(int rank) {
  if (!browseOrderBuilt) rebuildBrowseOrder(1, false);
  rank = std::max(rank, 0);
  if (rank >= kSpeciesCount) rank = kSpeciesCount - 1;
  return kSpecies[browseOrder[rank]];
}

Forageable featured(int month) {
  month = clampMonth(month);
  for (int i = 0; i < kSpeciesCount; i++)
    if (inSeason(kSpecies[i], month)) return kSpecies[i];
  return kSpecies[0];
}

Forageable secondary(int month) {
  month = clampMonth(month);
  bool foundFirst = false;
  for (int i = 0; i < kSpeciesCount; i++) {
    if (inSeason(kSpecies[i], month)) {
      if (foundFirst) return kSpecies[i];
      foundFirst = true;
    }
  }
  return kSpecies[(kSpeciesCount > 1) ? 1 : 0];
}

const char* seasonNote(int month) { return kSeasonNotes[clampMonth(month) - 1]; }

const char* biomeName(Biome b) {
  switch (b) {
    case Biome::Mountain:
      return "Mountain";
    case Biome::Coast:
      return "Coast";
    case Biome::Forest:
    default:
      return "Forest";
  }
}

}  // namespace foraging
