// quiz_facts.h — the Species Quiz question bank.
//
// Pure data, included only by minigames.cpp (same convention as
// foraging_species.h and events_data.h -- see CLAUDE.md's layout notes).
//
// **These are deliberately NOT the species table's `note`/`harvestTip`
// strings.** The Foraging view already prints those verbatim on the card
// for every discovered species, so quizzing on them would just be asking
// the player to recall a screen they can page over to. These are separate
// clues in a riddle voice, written to be answerable from what a forager
// actually knows about the organism.
//
// Rules for adding entries:
//   - The clue must never contain the species' own name (or an obvious
//     inflection of it) -- that hands over the answer.
//   - Two clues per species is the target, keyed by name; `find()` wraps
//     whatever index it's given across however many exist, so 1 is fine and
//     3+ works too.
//   - Keep each under ~110 characters: the quiz wraps at 44 columns and the
//     clue box is three lines tall.
#pragma once

#include <cstring>

namespace quiz_facts {

struct Entry {
  const char* species;  // must match foraging_species.h's `name` exactly
  const char* clue;
};

static const Entry kFacts[] = {
    // --- fungi: the notorious ones ---
    {"Dead Man's Fingers",
     "Charcoal-black clubs shove up out of buried hardwood, and the shape "
     "is exactly what the grim common name says."},
    {"Dead Man's Fingers",
     "A wood-rotting fungus, not a partner to any tree -- it works stumps "
     "and buried roots, and it lasts all year."},
    {"Death Cap",
     "Responsible for more fatal mushroom poisonings worldwide than any other "
     "species, and it arrived here with imported trees."},
    {"Death Cap",
     "Greenish cap, white gills, a ring, and a cup at the buried base -- a whole "
     "meal's worth is enough to destroy a liver."},
    {"Destroying Angel",
     "All white, top to bottom, with a sac at the buried base -- the sac is "
     "the tell that separates it from a field mushroom."},
    {"Destroying Angel",
     "Its toxin gives you a day of feeling fine before the liver damage "
     "shows, which is what makes it so lethal."},
    {"Fly Agaric",
     "The storybook toadstool: scarlet with white flecks, which are leftovers of "
     "the veil that wrapped the whole button."},
    {"Fly Agaric",
     "Named for the old practice of crumbling it into milk as an insect trap; it "
     "partners with birch and pine roots."},
    {"Panther Cap",
     "A brown-capped relative of the storybook toadstool, with the same white "
     "warts, a ring, and a bulb at the base."},
    {"Deadly Webcap",
     "Named for the cobwebby veil across its young gills; kidney failure from "
     "it can take weeks to appear."},
    {"Jack-o'-Lantern",
     "Orange, gilled, clustered on wood -- and its gills genuinely glow faintly "
     "in the dark. Mistaken for a prized funnel-shaped edible."},
    {"Angel Wings",
     "Pure white shelves on conifer wood, eaten for years until a run of deaths in "
     "Japan proved otherwise."},
    {"Pigskin Poison Puffball",
     "Thick warty rind and a hard black interior even when young -- cut "
     "any ball you find, a pure white inside is the safe sign."},
    {"Snow Bank False Morel",
     "It fruits at the retreating edge of the spring drifts, with a "
     "brainy, lobed cap rather than a pitted honeycomb one."},

    // --- fungi: the prizes ---
    {"Morel",
     "Cut one lengthwise: a genuine one is hollow from cap to stem base, with the pits "
     "on the outside."},
    {"Morel", "Famous for erupting by the thousand in the first spring after a forest fire."},
    {"Chanterelle",
     "It has blunt false ridges running down the stem instead of true blades, and "
     "it smells faintly of apricots."},
    {"Chanterelle",
     "Washington's official state mushroom, and it cannot be farmed -- it lives on "
     "conifer roots."},
    {"Golden Chanterelle",
     "The classic egg-yolk-coloured find of a wet autumn slope, with forking "
     "blunt ridges rather than knife-edged gills."},
    {"White Chanterelle",
     "A pale ivory version of the region's most sought-after autumn find, "
     "with the same forking blunt ridges."},
    {"Matsutake",
     "Graded and shipped to Japan by the pound; its smell is usually described as "
     "cinnamon crossed with dirty socks."},
    {"Matsutake",
     "Hunters look for a cracked, heaved mound in sandy duff and dig, because the "
     "best ones never break the surface."},
    {"Porcini",
     "No gills at all -- a sponge of pale pores under a fat brown cap, on a stem thick "
     "enough to look swollen."},
    {"Porcini",
     "The dried mushroom in Italian cooking; the young buttons are the ones worth "
     "carrying home."},
    {"King Bolete Button",
     "The prize bolete before its cap ever opens: a firm brown marble on a "
     "fat white stem, pores still tight and pale."},
    {"Black Trumpet",
     "Thin, hollow, funnel-shaped and nearly black, which is why foragers stare "
     "at leaf litter for minutes before spotting the first one."},
    {"Lobster Mushroom",
     "Not one organism but two: a parasitic fungus has coated and reshaped its "
     "host into a bright orange lump."},
    {"Lobster Mushroom",
     "Bright orange and pimpled outside, firm white inside, with a smell "
     "closer to shellfish than to mushroom."},
    {"Hen of the Woods",
     "A ruffled grey rosette of overlapping fronds that returns to the same "
     "old oak base for years."},
    {"Chicken of the Woods",
     "A shelf of brilliant sulphur-orange on a wounded trunk, with a "
     "texture that gave it a poultry nickname."},
    {"Lion's Mane",
     "No cap and no gills -- just a white ball of dangling icicle spines on a "
     "hardwood wound."},
    {"Bear's Head Tooth",
     "A cascade of white spines hanging in tiers from a dead conifer, like an "
     "icicle-covered branch."},
    {"Coral Tooth Fungus",
     "White branching arms studded with tiny hanging teeth, almost always "
     "from a wound scar on bigleaf maple."},
    {"Oyster Mushroom",
     "Shelves of soft, pale caps on dead alder and maple, with gills running "
     "right down onto a stubby off-centre stem."},
    {"Oyster Mushroom",
     "One of the few fungi known to hunt: its threads trap and digest tiny "
     "roundworms for nitrogen."},
    {"Yellowfoot",
     "A slender funnel with a hollow yellow stem that keeps fruiting through frost, "
     "long after the golden ones have gone."},
    {"Velvet Shank",
     "Orange caps over a dark suede-textured stem, fruiting in cold snaps; a pale "
     "cultivated form is sold in bundles as enoki."},
    {"Shaggy Mane",
     "Pick it in the morning or lose it -- within hours the gills dissolve "
     "themselves into black ink."},
    {"Shaggy Mane",
     "A tall white column of upturned scales that shoulders up through lawns, "
     "gravel and even asphalt."},
    {"Shaggy Parasol",
     "A big scaly cap with a movable ring on the stem; the flesh reddens where "
     "you handle it."},
    {"Meadow Mushroom",
     "The wild ancestor of the button mushroom in every supermarket, found in "
     "grazed pasture and lawns."},
    {"Turkey Tail",
     "Thin overlapping fans banded in concentric stripes -- one of the most studied "
     "fungi in cancer research."},
    {"Artist's Conk",
     "Scratch its white pore surface and the line turns brown and stays -- people "
     "have drawn on them for centuries."},
    {"Chaga",
     "A black cracked mass like a burnt clinker on a living birch, orange-brown and corky "
     "on the inside."},
    {"Indigo Milk Cap",
     "Cut it and it bleeds bright blue, then slowly stains green where it's "
     "handled."},
    {"Saffron Milk Cap",
     "Cut it and carrot-orange latex wells up; the whole mushroom then goes "
     "green with age or bruising."},
    {"Shrimp Russula",
     "A brittle mushroom whose stem snaps clean like chalk instead of tearing "
     "into fibres."},
    {"Cauliflower Coral",
     "A cream-coloured mass of flat wavy ribbons at a conifer base, like a "
     "loofah or a bath sponge."},
    {"Puffball (Giant)",
     "A white ball in a meadow that can reach football size and releases "
     "trillions of spores as a brown smoke."},
    {"Gem-Studded Puffball",
     "A small warty white ball -- cut it top to bottom, and pure white "
     "inside is the only acceptable answer."},
    {"Wood Blewit",
     "Lilac-purple all over when young, fading tan, and it fruits in leaf piles "
     "late into the cold."},
    {"Old Man of the Woods",
     "A bolete covered in shaggy black-grey scales, looking more like a "
     "scorched pine cone than a mushroom."},
    {"Slippery Jack",
     "A bolete with a cap so slimy when wet that most people peel it before "
     "cooking; it lives under pines."},
    {"Admirable Bolete",
     "A reddish-brown bolete of conifer slopes, sometimes said to taste "
     "faintly of lemon."},
    {"Zeller's Bolete",
     "A dark maroon, almost black-red cap over yellow pores that bruise blue "
     "within seconds."},
    {"Butter Bolete",
     "A pale tan bolete of mossy hemlock ground with a fat stem and a mild, rich "
     "flavour that earns the dairy nickname."},
    {"Angel's Wing Bolete",
     "A scaly orange-brown bolete that grows only under one tree: the "
     "Douglas fir."},
    {"Hawk's Wing",
     "Its cap is covered in big upturned dark scales, and the underside is a bed of "
     "soft teeth rather than gills."},
    {"Wine Cap Mushroom",
     "A burgundy-capped mushroom of wood-chip beds, so easy to cultivate that "
     "gardeners deliberately sow it in mulch."},
    {"Witch's Butter",
     "A bright orange jelly blob that shrivels to a crust in dry weather and "
     "swells back after rain."},
    {"Black Witch's Butter",
     "Olive-black jelly lumps on dead hardwood that dry to a hard crust "
     "and rehydrate whole after a soaking."},
    {"Amber Jelly Roll",
     "Small translucent honey-coloured discs on dead willow and alder twigs, "
     "rubbery between the fingers."},
    {"Wood Ear",
     "Brown, rubbery, and shaped exactly like its namesake body part; it goes into hot "
     "and sour soup."},
    {"Cinnabar Polypore",
     "A bracket fungus in a shocking orange-red that keeps its colour even "
     "after drying on the log."},
    {"Orange Peel Fungus",
     "A brilliant curled cup scattered on bare disturbed soil, looking "
     "exactly like something dropped after lunch."},
    {"Fried Chicken Mushroom",
     "Dense grey-brown clusters at hardwood bases, named for the smell "
     "and taste rather than the look."},
    {"Honey Mushroom",
     "A tree-killing parasite whose black bootlace cords run under bark; one "
     "individual in Oregon is among the largest organisms alive."},
    {"Birch Polypore",
     "A smooth pale kidney-shaped bracket found on one tree only; strips of it "
     "were once used to hone razors."},
    {"Dryad's Saddle",
     "A big scaly bracket on spring hardwood wounds that smells distinctly of "
     "watermelon rind or cucumber."},
    {"Pig's Ears",
     "A vase-shaped purple-brown relative of the golden funnel mushrooms, tucked in "
     "mossy hemlock duff."},
    {"Woolly Chanterelle",
     "A ragged, shaggy orange funnel, showy enough to grab -- and a reliable "
     "stomach-ache if it isn't cooked hard."},
    {"Blue Chanterelle",
     "A small violet funnel with blunt false ridges, nearly invisible against "
     "shadowed conifer duff."},
    {"Oregon White Truffle",
     "It fruits entirely underground and relies on smell -- and on animals "
     "digging it up -- to spread its spores."},

    // --- greens, herbs and flowers ---
    {"Miner's Lettuce",
     "The stem appears to pierce a single round leaf; gold rush prospectors ate "
     "it to stave off scurvy."},
    {"Miner's Lettuce",
     "A succulent shade plant of wet banks with a small white flower rising "
     "from the middle of its disc-shaped leaf."},
    {"Stinging Nettle",
     "Hollow silica hairs inject the sting, and heat or a few minutes in water "
     "destroys it completely."},
    {"Stinging Nettle",
     "Its long fibres were spun into cloth and fishing line for centuries "
     "before flax took over."},
    {"Chickweed",
     "A sprawling mat weed with one line of hairs running along the stem, switching "
     "sides at each leaf pair."},
    {"Ramps",
     "A wild onion relative so heavily dug in eastern North America that several states "
     "now restrict harvesting it."},
    {"Fiddlehead Fern",
     "The edible stage lasts days: a tightly wound crozier that unrolls into a "
     "frond and is then no longer food."},
    {"Wood Sorrel",
     "Shamrock-shaped leaflets that fold down at night, with a sharp lemony bite "
     "from oxalic acid."},
    {"Sheep Sorrel",
     "Arrowhead-shaped leaves with two backward lobes and the same sour lemon "
     "tang, in poor sunny ground."},
    {"Curly Dock",
     "Wavy-edged leaves on a tall stalk that ripens into a rusty-brown seed spike "
     "left standing all winter."},
    {"Dandelion",
     "Every seed head is a clone: it sets seed without pollination, which is why "
     "lawns fill so uniformly."},
    {"Dandelion",
     "A hollow, leafless, milky-sapped stalk holding a single yellow head over a "
     "toothed rosette."},
    {"Plantain",
     "A flat rosette of ribbed oval leaves with tough parallel veins, chewed and "
     "pressed onto stings as a field remedy."},
    {"Watercress",
     "It grows rooted in cold running water, which is exactly why it must be checked "
     "for what is upstream of it."},
    {"Wild Mint",
     "Roll the stem in your fingers: a square cross-section plus the smell is the "
     "whole identification."},
    {"Self-heal",
     "A short purple flower spike with a squarish stem, so common in mown lawns that "
     "most people step over it daily."},
    {"Yarrow",
     "Feathery, almost fern-like leaves and a flat white flower head; battlefield "
     "medicine used it to slow bleeding."},
    {"Oxeye Daisy",
     "The classic white-petalled, yellow-centred roadside bloom of children's "
     "flower chains -- an escaped European import here."},
    {"Douglas Aster",
     "Lavender rays around a yellow eye, blooming along forest edges when almost "
     "everything else has finished."},
    {"Lupine",
     "A tall spike of pea flowers over palm-shaped leaflets; its roots host bacteria "
     "that fertilise the soil around it."},
    {"Bleeding Heart",
     "Rows of dangling pink lockets on an arching stem; ants carry its seeds "
     "away for the oily packet attached."},
    {"Glacier Lily",
     "A nodding yellow bloom with swept-back petals that pushes up through the "
     "last of the melting alpine snow."},
    {"Subalpine Daisy",
     "Lavender rays and a yellow centre, carpeting high meadows in the few "
     "weeks between snowmelt and frost."},
    {"Skunk Cabbage",
     "It generates its own heat to melt through swamp ice, and advertises with a "
     "smell that draws flies instead of bees."},
    {"Skunk Cabbage",
     "A glowing yellow hood wrapped around a knobbly spike, standing in spring "
     "seep mud before the leaves unfurl."},
    {"Devil's Club Shoots",
     "The plant is armoured in brittle spines on stem and leaf vein alike; "
     "only the earliest tips are soft enough to eat."},
    {"Swamp Onion",
     "A wet-meadow bulb that announces itself by smell alone -- if the crushed leaf "
     "doesn't smell right, it isn't this."},
    {"Douglas Fir Tips",
     "The pale new growth at each branch end tastes bright and citrusy, "
     "nothing like the resinous older needles."},
    {"Cottonwood Buds",
     "Sticky, intensely fragrant resin coats the spring buds, collected for the "
     "salve called balm of Gilead."},
    {"Licorice Fern",
     "A fern that grows out of the moss on maple trunks rather than the ground; "
     "its rhizome tastes sweet and anise-like."},
    {"Sword Fern",
     "The region's dominant evergreen forest fern, each leaflet carrying a small "
     "thumb-like lobe at its base."},
    {"Deer Fern Fiddlehead",
     "This fern makes two kinds of frond: flat evergreen ones that sprawl, "
     "and narrow upright ones that carry spores."},
    {"Lady Fern Fiddlehead",
     "The lacy, delicate fern of damp streambanks, dying back completely "
     "each winter unlike its evergreen neighbours."},
    {"Vanilla Leaf",
     "Three broad leaflets fanned on a bare stalk, carpeting shady forest; the "
     "dried leaves smell of the pod they're named for."},
    {"Labrador Tea",
     "Leathery bog leaves with rusty fuzz curled under the edges, brewed by "
     "northern travellers for centuries."},
    {"Pipsissewa",
     "A tiny evergreen with whorled toothed leaves; its flavour once went into "
     "traditional root beer."},
    {"Pearly Everlasting",
     "Papery white bracts that hold their shape and colour for months after "
     "drying, which is where the name comes from."},
    {"Indian Rhubarb",
     "Huge umbrella leaves on mountain streambanks, with a peeled stalk that is "
     "cooked exactly like the garden vegetable it's named for."},
    {"Cattail Shoots",
     "Pull the young marsh shoot apart and the tender white core inside tastes "
     "startlingly like cucumber."},
    {"Fireweed Shoots",
     "The first plant to blanket a burn in pink, and its spring shoots come up "
     "like thin asparagus."},
    {"Subalpine Fireweed",
     "Tall pink spikes that colonise high slopes after fire or snowmelt, "
     "opening flowers from the bottom of the spike upward."},
    {"Yellow Salsify",
     "Grass-like leaves and a yellow flower that closes by midday, ripening into "
     "a seed globe like a dandelion's on steroids."},
    {"Mountain Sorrel Leaves",
     "Kidney-shaped leaves in rocky alpine seeps with a sharp lemon bite "
     "from the same acid as garden rhubarb."},
    {"Partridgefoot Greens",
     "A dense mossy alpine mat of tiny fan-cut leaves under short white "
     "flower spikes, on exposed ridges and bluffs."},
    {"Sea Asparagus",
     "Leafless jointed green stems standing in salt marsh mud, crunchy and "
     "already seasoned by the tide."},
    {"Goosetongue",
     "Fleshy, narrow, faintly salty blades on the upper beach -- a shoreline "
     "cousin of the lawn weed with ribbed leaves."},
    {"Silverweed",
     "Toothed leaflets with a bright silvery underside, spreading by runners across "
     "upper tidal meadows; the roots were a staple starch."},
    {"Beach Sandwort",
     "A succulent dune mat of paired glossy leaves that traps blowing sand and "
     "helps build the foredune."},
    {"American Searocket",
     "A fleshy dune plant with a peppery mustard bite and seed pods that "
     "float off to colonise other beaches."},
    {"Orache",
     "A silvery-dusted beach relative of spinach and quinoa, with triangular "
     "arrowhead-shaped leaves."},
    {"Scurvy Grass",
     "Small round peppery leaves in splash-zone crevices, carried on ships "
     "specifically to prevent the deficiency it's named for."},
    {"Sea Coast Angelica",
     "Stout hollow celery-scented stalks on coastal bluffs, in a family "
     "where the deadly look-alikes demand real certainty."},
    {"Beach Lovage",
     "Glossy toothed leaves on the bluff edge that taste like salted celery; a "
     "wild relative of the garden herb."},
    {"Beach Pea",
     "A sprawling vine of purple-pink pea flowers along the driftwood line, holding "
     "sand together with deep runners."},
    {"Wild Rose Hips",
     "The fruit left after the petals fall carries more vitamin C by weight than "
     "an orange; the seed hairs must be strained out."},
    {"Nootka Rose",
     "A thorny native rose with large single pink five-petalled flowers and a pair "
     "of straight prickles below each leaf."},
};

// Second block, so no single array literal gets unmanageably long -- find()
// searches both in order.
static const Entry kFacts2[] = {
    // --- berries and nuts ---
    {"Salmonberry",
     "The first berry of the year here, ripening golden to red while the trailing "
     "brambles are still in flower."},
    {"Salmonberry",
     "Its magenta flowers open so early they're a key nectar source for returning "
     "rufous hummingbirds."},
    {"Blackberry",
     "An aggressive Eurasian import with thick arching canes and five-angled stems, "
     "now the default hedge of the region."},
    {"Trailing Blackberry",
     "The native bramble: thin wiry vines running along the ground, with "
     "smaller and far sweeter fruit than the invader."},
    {"Thimbleberry",
     "Big soft maple-shaped leaves and a fruit so fragile it collapses in your "
     "hand, which is why it's never sold."},
    {"Raspberry",
     "Pick it and the fruit slides off leaving a white core behind, which is what "
     "separates this whole group from blackberries."},
    {"Black Raspberry",
     "Purple-black fruit that pulls free of its core like its red cousin, on "
     "canes with a chalky white bloom."},
    {"Huckleberry",
     "The higher you climb the later it ripens, so the season can be chased uphill "
     "for weeks."},
    {"Huckleberry",
     "Repeated attempts to farm it have failed -- it needs the soil fungi of "
     "mountain forest, so every jar is wild-picked."},
    {"Black Huckleberry",
     "The classic large purple-black mountain berry of the region, best where "
     "an old burn has opened the canopy."},
    {"Red Huckleberry",
     "Bright translucent red berries on twigs so sharply green and angled they "
     "identify the shrub in winter."},
    {"Evergreen Huckleberry",
     "A glossy-leaved coastal shrub, leaves toothed and leathery, holding "
     "small dark berries well into winter."},
    {"Alpine Blueberry",
     "Powder-blue fruit on a shrub barely ankle-high, growing right where the "
     "trees give out."},
    {"Wild Blueberry",
     "Ankle-high mats near treeline carrying small intensely tart blue fruit "
     "with a dusty bloom."},
    {"Grouse Whortleberry",
     "Pea-sized red berries on twiggy green mats at treeline -- so small "
     "that picking a cupful takes real patience."},
    {"Mountain Cranberry",
     "A tiny evergreen mat berry, tart and firm, that sweetens after the "
     "first hard frost; Scandinavians call it lingonberry."},
    {"Highbush Cranberry",
     "Not a true bog berry at all but a viburnum, hanging translucent red "
     "clusters beside mountain streams."},
    {"Salal",
     "Thick leathery evergreen leaves, prized by the florist trade, over dark blue "
     "berries on a sticky-haired stalk."},
    {"Oregon Grape",
     "Holly-like spiny leaflets and sour blue fruit; the bright yellow inner bark "
     "was used as a dye."},
    {"Oregon Grape",
     "The state south of here made it their official flower; despite the fruit, "
     "it is no relation to any vine."},
    {"Blue Elderberry",
     "Flat-topped clusters of tiny blue-black fruit under a powdery bloom, on a "
     "shrub with opposite compound leaves."},
    {"Red Elderberry",
     "Bright red clusters by mountain streams; the seeds and stems make it a "
     "cook-it-or-leave-it proposition."},
    {"Elderflower",
     "Creamy flat-topped umbels with a heavy muscat scent, turned into cordial "
     "across northern Europe."},
    {"Serviceberry",
     "Also called saskatoon: purple pome fruit, more apple than berry botanically, "
     "on a small tree that flowers before it leafs out."},
    {"Kinnikinnick",
     "A trailing evergreen mat with small paddle leaves and mealy red fruit; its "
     "dried leaves went into smoking mixtures."},
    {"Bunchberry",
     "A dogwood shrunk to ankle height: four white bracts over a whorl of leaves, "
     "then a tight cluster of red fruit."},
    {"Bunchberry",
     "Its flower fires pollen in under half a millisecond, one of the fastest "
     "movements known in plants."},
    {"Wild Strawberry",
     "Runners, three toothed leaflets, and a fruit with the seeds sitting on "
     "the outside rather than inside."},
    {"Alpine Strawberry",
     "A tiny wild strawberry of rocky high turf, intensely aromatic and "
     "rarely bigger than a fingernail."},
    {"Beach Strawberry",
     "A coastal dune strawberry with thick glossy leaves -- one of the two "
     "wild parents of every garden strawberry."},
    {"Indian Plum",
     "Often the very first native shrub to leaf out here, with cucumber-scented "
     "leaves and dangling white flowers in late winter."},
    {"Twisted Stalk",
     "Berries hang singly under a zigzag stem, which is the whole trick to "
     "spotting them; the taste is faintly of watermelon."},
    {"Soapberry",
     "Whipped with water it foams up like beaten egg white, a traditional dessert "
     "whose bitterness is the point."},
    {"Snowberry",
     "Clusters of waxy white berries that persist on bare stems all winter -- "
     "ornamental, and not for eating."},
    {"Wax Currant",
     "Dry rocky slopes, sticky-hairy leaves, and pale orange-red fruit with the "
     "dried flower still stuck to the tip."},
    {"Red-Flowering Currant",
     "Hot pink flower clusters in early spring make it a garden favourite; "
     "the blue-black fruit that follows is a letdown."},
    {"Red Currant",
     "Translucent red fruit hanging in strings from an arching thornless shrub, "
     "each berry showing its seeds through the skin."},
    {"Wild Gooseberry",
     "Spines on the stem and often on the fruit itself, with pale veins running "
     "down each hanging berry."},
    {"Juniper Berries",
     "Not a berry but a fleshy cone, and the defining botanical flavour of gin."},
    {"Mountain Ash Berries",
     "Orange clusters on a subalpine tree with feather-compound leaves, "
     "bitter until frost and cooking take the edge off."},
    {"Beaked Hazelnut",
     "The husk drawn out into a long bristly beak is the giveaway; squirrels "
     "usually clear a bush before people do."},
    {"Garry Oak Acorn",
     "The nut of the only native oak this far north, and it must be leached in "
     "changed water before it's edible."},
    {"Whitebark Pine Nuts",
     "A high-ridge pine in steep decline whose seeds are cached by a single "
     "bird species that also plants its forests."},
    {"Western White Pine Nuts",
     "Long banana-curved cones drop whole, each scale hiding a small "
     "sweet winged seed."},
    {"Subalpine Fir Tips",
     "The narrow spire-shaped high-country fir, whose pale new tips carry a "
     "sharp balsam scent."},
    {"Grand Fir Tips", "Crush the flat glossy needles and the smell is unmistakably tangerine."},

    // --- seaweeds ---
    {"Bull Kelp",
     "A single hollow whip-like stipe ends in a gas-filled float; it grows from spore "
     "to ten metres in one season, then dies."},
    {"Bull Kelp",
     "The dried stipe was worked into fishing line and storage bottles along this "
     "coast."},
    {"Sugar Kelp",
     "A single ruffled golden blade with no stem branches, drying to a white bloom of "
     "sweet mannitol on the surface."},
    {"Sea Lettuce",
     "Bright green sheets only two cells thick, translucent enough to read through "
     "when held to the light."},
    {"Nori (Laver)",
     "Thin purple-black sheets peeled off high rocks -- the same genus dried into "
     "the sheets wrapped around sushi."},
    {"Bladderwrack",
     "Forked olive-brown fronds with paired air bladders either side of the "
     "midrib, popping underfoot on mid-tide rocks."},
    {"Winged Kelp",
     "A long brown blade with frilly wings either side of a pale midrib; Japan "
     "knows the same genus as wakame."},
    {"Sea Palm",
     "A stubby upright stipe topped with drooping straps, standing rigid on rock "
     "where the surf is at its worst."},
    {"Feather Boa Kelp",
     "A long strap hung with small blades and floats along both edges, exactly "
     "like the costume accessory it's named for."},
    {"Turkish Towel",
     "A thick red blade covered in stiff bumps, rough as terrycloth -- it's "
     "harvested industrially for carrageenan."},
    {"Turkish Washcloth",
     "Dark red-black curled blades that dry to stiff paper and soften again "
     "the moment the tide returns."},
    {"Sea Sac",
     "Brown hollow sacs the size of golf balls, gripping low rock and squirting water "
     "when squeezed."},
    {"Oarweed",
     "A broad brown blade split into finger-like straps above a thick rubbery stem, "
     "anchored by a claw-like holdfast."},
    {"Sea Cabbage",
     "A single broad ruffled golden blade with no splits, milder than its sweeter "
     "relative on the same rocks."},
    {"Sea Hair",
     "Fine hollow bright-green threads tangled in soft wet masses across tidepool "
     "rock."},
    {"Giant Kelp",
     "The fastest-growing organism in the sea -- up to half a metre a day -- forming "
     "the canopy of an underwater forest."},

    // --- shellfish and shore animals ---
    {"Razor Clam",
     "It can dig itself down faster than a person can shovel, which is why the dimple "
     "it leaves means dig now."},
    {"Razor Clam",
     "A long narrow shell with a glossy varnished coat, on open surf beaches rather "
     "than sheltered bays."},
    {"Geoduck",
     "The largest burrowing clam in the world, with a neck it cannot retract and a "
     "lifespan well over a century."},
    {"Pacific Oyster",
     "Introduced from Japan and now the backbone of the region's shellfish "
     "farms, cementing itself permanently to rock."},
    {"Olympia Oyster",
     "The only oyster native to this coast, barely coin-sized, nearly wiped out "
     "by a century of over-harvest and pollution."},
    {"Bay Mussel",
     "Blue-black shells anchored to pilings by tough golden threads the animal spins "
     "from a gland in its foot."},
    {"California Mussel",
     "The large ridged mussel of wave-hammered outer coast rock, thick-shelled "
     "where its sheltered cousin is smooth."},
    {"Butter Clam",
     "A thick chalky-white clam that digs deep and lives for decades; it holds "
     "biotoxins longer than most."},
    {"Manila Clam",
     "An accidental import that arrived with oyster seed and now dominates the "
     "region's gravel beaches."},
    {"Pacific Littleneck Clam",
     "The native counterpart to the imported bay clam, rounder and more "
     "deeply ribbed, buried a bit deeper."},
    {"Purple Varnish Clam",
     "A recent arrival, probably in ballast water, with a glossy brown "
     "coating and purple inside the shell."},
    {"Softshell Clam",
     "A brittle thin-shelled estuary clam of soft mud, introduced from the "
     "Atlantic coast over a century ago."},
    {"Pacific Gaper Clam",
     "A big clam whose siphon is far too large to pull back inside, so the "
     "shell never fully closes."},
    {"Pacific Basket Cockle",
     "A round strongly ribbed shell that lies just under the surface, and "
     "can flip itself away with a muscular foot."},
    {"Gooseneck Barnacle",
     "A stalked barnacle of surf-blasted rock; medieval Europe believed a "
     "species of goose hatched from it."},
    {"Dungeness Crab",
     "Named for a small town on this coast, and the region's most valuable "
     "commercial shellfish."},
    {"Red Rock Crab",
     "A chunky black-tipped-claw crab found under low-tide boulders, smaller than "
     "the famous one but with heavier pincers."},
    {"Kelp Crab",
     "An olive spider-legged crab that clings in kelp beds and decorates its own "
     "shell with bits of algae."},
    {"Graceful Crab",
     "A slender, long-legged relative of the region's prize crab, often pulled up "
     "in the same pots by mistake."},
    {"Spot Prawn",
     "The largest shrimp on this coast, marked with white flecks on the tail, and "
     "it changes from male to female as it ages."},
    {"Red Sea Urchin",
     "Long-spined and harvested for its roe, which is sold as uni; individuals "
     "can live over a century."},
    {"Purple Sea Urchin",
     "Its five-toothed jaw apparatus is called Aristotle's lantern; booms in "
     "its numbers can strip a kelp forest bare."},
    {"Gumboot Chiton",
     "The largest chiton in the world, a rust-red slab whose eight plates are "
     "buried under a leathery girdle."},
    {"Black Katy Chiton",
     "A grazer wearing eight white plates set into glossy black leather, "
     "clamped to open rock."},
    {"Limpet",
     "A cone-shelled grazer that returns to the exact same scar on the rock after every "
     "feeding trip."},
    {"Blue Top Snail",
     "A small conical tidepool snail with a pearly interior showing through worn "
     "patches of shell."},
    {"Moon Snail",
     "It drills a neat bevelled hole through other shells to eat them, and lays eggs "
     "in a stiff collar of sand and mucus."},
    {"Nudibranch",
     "A shell-less sea slug that can steal stinging cells from the prey it eats and "
     "reuse them in its own frilly appendages."},
    {"Pinto Abalone",
     "An ear-shaped shell with a line of breathing holes, now so depleted that "
     "harvest is closed entirely."},
    {"Ochre Sea Star",
     "Orange or purple, it was the original textbook keystone predator -- and it "
     "was hit hardest by the wasting disease of 2013."},
};

// Third block: second clues for species that had only one, so a repeat
// encounter with a familiar species isn't a repeat question.
static const Entry kFacts3[] = {
    {"Chickweed",
     "A mild-tasting weed of disturbed ground whose small white flowers have five "
     "petals so deeply split they look like ten."},
    {"Ramps",
     "Broad smooth leaves that come up before the trees leaf out, and are gone by the "
     "time the canopy closes."},
    {"Fiddlehead Fern",
     "Rub the coil: the papery brown husk flakes off, and a groove runs down "
     "the inside of the stem like celery."},
    {"Wood Sorrel",
     "Grazing animals avoid too much of it for the same reason spinach is limited "
     "in some diets: it's loaded with oxalates."},
    {"Elderflower",
     "The tree's pith is so soft it was hollowed into whistles and pipes, which is "
     "where the shrub's old name comes from."},
    {"Salal",
     "Its thick evergreen leaves are cut by the million for florists' greenery, making it "
     "one of the region's biggest wild harvests."},
    {"Thimbleberry",
     "Pull the fruit and it comes away as a hollow cap, because the core stays "
     "behind on the plant."},
    {"Serviceberry",
     "The bloom was a calendar: when it flowered, the ground had thawed enough to "
     "dig a grave, and the shad were running."},
    {"Kinnikinnick",
     "A tough evergreen mat that holds bare sandy soil together, staying green "
     "under snow all winter."},
    {"Blackberry",
     "One cane can root where its tip touches ground, which is how a single plant "
     "becomes an impassable thicket in a few years."},
    {"Salmonberry",
     "The fruit ranges from gold to deep red on the same bush, and the colour says "
     "nothing about ripeness."},
    {"Raspberry",
     "The canes live two years: leaves the first, fruit the second, then they die "
     "back and are replaced."},
    {"Wild Strawberry",
     "Spreads by runners rather than seed, so a patch is often one plant that "
     "has been walking downhill for years."},
    {"Red Huckleberry",
     "It usually roots on a rotting stump or log rather than in soil, perched "
     "above the forest floor."},
    {"Evergreen Huckleberry",
     "New spring growth comes in bronze-red before it hardens to dark "
     "glossy green."},
    {"Oregon Grape",
     "The inner bark and root run bright yellow, from an alkaloid that was used "
     "both as a dye and as a medicine."},
    {"Blue Elderberry",
     "The blue is a natural wax bloom -- rub it off and the fruit underneath is "
     "nearly black."},
    {"Red Elderberry",
     "Birds strip it within days of ripening, so a bush loaded one morning can "
     "be bare the next."},
    {"Indian Plum",
     "Male and female flowers grow on separate shrubs, so only some of them ever "
     "set fruit at all."},
    {"Snowberry",
     "The white fruit is full of saponins, and it persists through winter as one of "
     "the last things left for birds."},
    {"Soapberry",
     "Its foaming comes from the same class of compounds as soap, which is why it "
     "must be whipped in a scrupulously grease-free bowl."},
    {"Mountain Ash Berries",
     "Frost breaks down the bitter compounds, so the same fruit that is "
     "inedible in September is usable in November."},
    {"Highbush Cranberry",
     "Its flower head has a ring of large showy sterile blooms around the "
     "small fertile ones -- billboard and product."},
    {"Juniper Berries",
     "Each takes two or three years to ripen, so green and blue-black ones sit "
     "on the same branch at once."},
    {"Twisted Stalk",
     "The stem kinks sharply at every leaf, which is the fastest way to tell it "
     "from the poisonous plants it grows beside."},
    {"Bunchberry",
     "It needs a cold winter to flower, which is why it thrives at elevation and "
     "sulks in a lowland garden."},
    {"Wild Gooseberry",
     "Some varieties were once banned in the US because the shrub hosts a rust "
     "that kills white pines."},
    {"Red Currant",
     "Unlike its bristly relatives, this one has no spines at all, which makes "
     "stripping a bush painless."},
    {"Wax Currant",
     "Its leaves and stems are sticky to the touch from resin glands, which is "
     "unmistakable once you've felt it."},
    {"Red-Flowering Currant",
     "David Douglas sent it to Britain in the 1820s, and it became a "
     "garden staple there long before anyone here planted it."},
    {"Black Raspberry",
     "The canes arch over until the tip touches soil and roots, so a patch "
     "advances in loops rather than spreading by runners."},
    {"Alpine Blueberry",
     "Its leaves turn scarlet in autumn, so a whole slope changes colour weeks "
     "before the lowland trees do."},
    {"Grouse Whortleberry",
     "Its twigs stay bright green through winter, which makes the ankle-high "
     "mats easy to find under snow."},
    {"Black Huckleberry",
     "Fire is its friend: burned slopes fruit heavily for a decade or two, "
     "then taper off as the forest closes back in."},
    {"Beach Strawberry",
     "Its thick waxy leaves resist salt spray, which is what lets it live on "
     "open dunes at all."},
    {"Alpine Strawberry",
     "Unlike garden strawberries it sets fruit all season rather than in one "
     "flush, a few berries at a time."},
    {"Beaked Hazelnut",
     "The husk's bristles work into skin like fibreglass, so the nuts are "
     "usually gathered with gloves."},
    {"Garry Oak Acorn",
     "The oak savanna it grows in was maintained for centuries by deliberate "
     "burning, and has largely vanished without it."},
    {"Whitebark Pine Nuts",
     "Its cones don't open on their own -- a bird has to tear them apart, "
     "which is the only way the seeds get out."},
    {"Western White Pine Nuts",
     "A blister rust introduced around 1900 killed most of the big ones, "
     "and the survivors are being bred for resistance."},
    {"Stinging Nettle",
     "Cooked, dried or even thoroughly crushed, the sting is gone -- the hairs "
     "are tiny syringes, and broken ones can't inject."},
    {"Sheep Sorrel",
     "It spreads by creeping roots as much as by seed, so pulling the tops just "
     "prunes it."},
    {"Curly Dock",
     "Its taproot goes down feet rather than inches, which is why it outlives every "
     "attempt to weed it out."},
    {"Plantain",
     "Europeans brought it everywhere they settled, and it followed them so reliably "
     "it was called the white man's footprint."},
    {"Watercress",
     "It's in the mustard family, which is where the peppery bite comes from -- "
     "the same compounds as horseradish."},
    {"Wild Mint",
     "The only native member of its aromatic square-stemmed family here -- crush a "
     "leaf and the scent settles the identification instantly."},
    {"Self-heal",
     "Its old names all promise medicine -- it was a standard wound herb in European "
     "folk practice for centuries."},
    {"Yarrow",
     "Its Latin name honours Achilles, who is said to have carried it to treat his "
     "soldiers' wounds."},
    {"Dandelion",
     "Every part has been used: greens, root roasted as a coffee substitute, and "
     "flowers fermented into wine."},
    {"Oxeye Daisy",
     "It's a serious pasture weed -- cattle won't eat it, so it takes over the "
     "grass they will."},
    {"Douglas Aster",
     "One of the last nectar sources before frost, so it's usually the busiest "
     "plant on a September trail."},
    {"Lupine",
     "Its seeds contain bitter alkaloids, and telling the edible species from the toxic "
     "ones is a genuinely hard problem."},
    {"Bleeding Heart",
     "The whole plant is toxic, and it goes fully dormant by midsummer -- above "
     "ground in spring, gone by August."},
    {"Glacier Lily",
     "Bears dig the bulbs by the hillside after snowmelt, leaving whole meadows "
     "turned over."},
    {"Subalpine Daisy",
     "The high meadow it carpets is snow-covered nine months a year, so its "
     "whole season is about six weeks long."},
    {"Skunk Cabbage",
     "Bears eat it early in spring after hibernation, and its huge leaves were "
     "used as natural wax paper."},
    {"Devil's Club Shoots",
     "Its spines break off in the skin and fester, which is why it's "
     "respected far more than it's harvested."},
    {"Swamp Onion",
     "Anything in this group that lacks the smell should be treated as a deadly "
     "look-alike, not a variant."},
    {"Cattail Shoots",
     "Nearly every part is usable at some point in the year -- shoots, pollen, "
     "and starchy rhizomes."},
    {"Fireweed Shoots",
     "It blanketed bombed-out London so thoroughly after the Blitz that "
     "Britons still call it bombweed."},
    {"Subalpine Fireweed",
     "The flowers open from the bottom of the spike upward, so how far the "
     "bloom has climbed tells you how far into summer you are."},
    {"Labrador Tea",
     "It grows in acidic peat bogs where few shrubs can, and its leaf edges roll "
     "under to conserve water."},
    {"Pipsissewa",
     "It's partly parasitic on soil fungi, which is why transplanting it almost "
     "always fails."},
    {"Pearly Everlasting",
     "It's the larval food plant for painted lady butterflies, so a stand of "
     "it is usually crawling with caterpillars."},
    {"Vanilla Leaf",
     "Bundles of the dried leaves were hung indoors as an insect repellent and air "
     "freshener."},
    {"Licorice Fern",
     "It dries to a crisp in summer drought and revives with the autumn rains, "
     "seemingly from nothing."},
    {"Sword Fern",
     "A single plant can carry a hundred fronds and live for decades, which is why "
     "they form the understory's permanent furniture."},
    {"Lady Fern Fiddlehead",
     "Its fronds taper at both ends rather than staying broad to the base, "
     "the quickest way to tell it apart."},
    {"Deer Fern Fiddlehead",
     "The spore-bearing fronds stand stiffly upright in the middle while "
     "the sterile ones sprawl flat around them."},
    {"Indian Rhubarb",
     "Its flower stalk comes up bare, before the leaves, so early in the season "
     "there's nothing but a pink spike in the creek."},
    {"Cottonwood Buds",
     "The scent carries on warm spring air well before you reach the tree, "
     "which is usually how you find one."},
    {"Douglas Fir Tips",
     "The tree isn't a true fir at all, and its cones carry little three-pointed "
     "bracts said to be the hind legs of hiding mice."},
    {"Grand Fir Tips",
     "Its needles lie in two flat rows either side of the twig, so a sprig looks "
     "pressed rather than bushy."},
    {"Subalpine Fir Tips",
     "Its narrow spire shape sheds snow that would snap a broader tree, which "
     "is how it survives at treeline."},
    {"Mountain Sorrel Leaves",
     "A staple against scurvy for arctic and alpine travellers, since it "
     "grows where almost nothing green does."},
    {"Sea Asparagus",
     "It survives being submerged in salt water twice a day by storing the salt "
     "in its own tissue."},
    {"Goosetongue",
     "It grows in the salt marsh alongside a genuinely dangerous look-alike, so "
     "this is one to be certain about."},
    {"Silverweed",
     "Its roots were a managed crop on this coast, with family-owned plots weeded "
     "and harvested for generations."},
    {"Beach Sandwort",
     "It's a dune builder: sand piles against it, it grows up through the pile, "
     "and the dune gets taller."},
    {"Scurvy Grass",
     "Its peppery bite comes from mustard oils, the same family of compounds as "
     "the cress in a sandwich."},
    {"Sea Coast Angelica",
     "Hollow stalks and umbrella-shaped flower heads put it in a family that "
     "includes both celery and hemlock."},
    {"Beach Lovage",
     "A coastal bluff plant whose leaves smell strongly of celery when crushed, "
     "used the same way as a herb."},
    {"Orache",
     "It tolerates salty soil that would kill spinach, which is exactly why it grows "
     "where it does."},
    {"American Searocket",
     "Its seed pods are corky and float, carrying the plant to new beaches on "
     "the tide."},
    {"Bull Kelp",
     "It anchors to rock with a claw-like holdfast, and winter storms tear whole "
     "plants loose to wash up in windrows."},
    {"Sugar Kelp",
     "Farmed in ropes along this coast, it's one of the fastest-growing crops there "
     "is, and needs no soil, feed or fresh water."},
    {"Sea Lettuce",
     "It blooms thickly where nutrients run off the land, so a heavy mat of it is "
     "often a sign of pollution."},
    {"Nori (Laver)",
     "Its life cycle was only worked out in the 1940s, and that discovery is what "
     "made farming it possible at all."},
    {"Bladderwrack",
     "It was the original source of iodine, discovered in the ash when it was "
     "burned for soda."},
    {"Giant Kelp",
     "Its fronds are held up by gas-filled floats, so the whole forest hangs from "
     "the surface rather than standing from the seabed."},
    {"Pacific Oyster",
     "It changes sex as it grows, and a single female can release tens of "
     "millions of eggs in a season."},
    {"Bay Mussel",
     "It filters several litres an hour, which is why shellfish closures track "
     "algal blooms so closely."},
    {"Butter Clam",
     "It stores paralytic toxins in its siphon for a year or more after a bloom, "
     "long after other shellfish have cleared."},
    {"Manila Clam",
     "It sits shallower than the native clams it grows beside, which is why a rake "
     "finds it first."},
    {"Dungeness Crab",
     "Only males above a size limit may be kept, and they must be measured "
     "across the shell just ahead of the points."},
    {"Moon Snail",
     "Its foot inflates with seawater to many times the shell's volume, then has to "
     "be squeezed out to withdraw."},
    {"Limpet",
     "It grinds a shallow scar in the rock so its shell seats exactly, sealing tight "
     "against waves and drying air."},
    {"Gooseneck Barnacle",
     "It's a crustacean, not a mollusc -- inside the plates are feathery legs "
     "that comb the water for food."},
    {"Purple Sea Urchin",
     "It can grind a pit into solid rock with its teeth and spines, and then "
     "sits in that pit for years."},
    {"Ochre Sea Star",
     "Remove it from a shore and mussels take over completely -- the experiment "
     "that gave ecology the phrase keystone species."},
    {"Spot Prawn",
     "Its season is measured in days rather than weeks, and the catch is usually "
     "sold live off the boat."},
};

/**
 * Returns clue `idx` for `species` (wrapping across however many that
 * species has), or nullptr if it has no entries yet -- the caller falls
 * back to the species table's note in that case.
 */
inline const char* find(const char* species, unsigned idx) {
  const Entry* tables[3] = {kFacts, kFacts2, kFacts3};
  const int counts[3] = {(int)(sizeof(kFacts) / sizeof(kFacts[0])),
                         (int)(sizeof(kFacts2) / sizeof(kFacts2[0])),
                         (int)(sizeof(kFacts3) / sizeof(kFacts3[0]))};
  int matches = 0;
  for (int t = 0; t < 3; t++) {
    for (int i = 0; i < counts[t]; i++) {
      if (strcmp(tables[t][i].species, species) == 0) matches++;
    }
  }
  if (matches == 0) return nullptr;
  unsigned want = idx % (unsigned)matches;
  unsigned seen = 0;
  for (int t = 0; t < 3; t++) {
    for (int i = 0; i < counts[t]; i++) {
      if (strcmp(tables[t][i].species, species) != 0) continue;
      if (seen == want) return tables[t][i].clue;
      seen++;
    }
  }
  return nullptr;
}

}  // namespace quiz_facts
