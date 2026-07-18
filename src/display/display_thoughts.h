// display_thoughts.h — flavor-text pools for the Main view's occasional
// thought bubble (see maybeDrawThoughtBubble() in display.cpp), one pool
// per growth stage. Pure data, split out of display.cpp; only included
// from there.
#pragma once

static const char* const kThoughtsBaby[] = {
    "Mrrp?", "Blorp!", "Squeak squeak", "Gnaa?", "Brrrble", "Hii?", "Wobble wobble", "Mlem",
};
static const int kThoughtsBabyCount = sizeof(kThoughtsBaby) / sizeof(kThoughtsBaby[0]);

static const char* const kThoughtsJuvenile[] = {
    "Rock. Good.", "Big sky", "Run fast?", "Where mom", "Shiny thing", "More food", "Dig?",
    "Tail",
};
static const int kThoughtsJuvenileCount =
    sizeof(kThoughtsJuvenile) / sizeof(kThoughtsJuvenile[0]);

static const char* const kThoughtsAdult[] = {
    "Berries...",
    "Nap time?",
    "So fluffy",
    "*yawn*",
    "Rocks are nice",
    "Snack o'clock",
    "Where's the sun?",
    "I love this rock",
    "Whistling practice",
    "Just five more minutes",
    "Sunbeam spot",
    "Best rock ever",
    "Mmm, huckleberries",
    "Cloud watching",
    "Stretchy time",
    "Who dug this?",
    "Fresh dirt smell",
    "Warm rock, best rock",
    "Snacking intensifies",
    "Just resting my eyes",
    "Mountain air",
    "That was a good nap",
    "One more berry",
    "Wind feels nice",
    "Chubby and happy",
    "Watching the clouds",
    "Time for a snooze",
    "Rustling leaves",
    "Is that a hawk?",
    "Cozy burrow vibes",
    "Fluffed my fur",
    "Tail check",
    "Good digging day",
    "Sunny side up",
    "Practicing my whistle",
    "Mossy and cozy",
    "Snow patch nearby",
    "Where'd everyone go?",
    "Big yawn incoming",
    "Rock's still warm",
    "Grass is extra tasty",
    "Just vibing",
    "Paws are cold",
    "Belly's full",
    "Nice breeze today",
    "Another lazy day",
    "Snuggled in",
    "Watching for eagles",
    "Funny shaped cloud",
    "Time to forage more",
    "Napping is a skill",
    "So many rocks to nap on",
    "Chewing thoughtfully",
    "Meadow smells great",
    "Just one more stretch",
    "Basically a rockstar",
    "Contemplating snacks",
    "This spot is mine now",
    "Perfectly toasty",
    "Living the burrow life",
};
static const int kThoughtsAdultCount = sizeof(kThoughtsAdult) / sizeof(kThoughtsAdult[0]);

// Stat-reactive adult pools -- picked instead of kThoughtsAdult whenever
// hunger/happiness/energy is in a bad place (see pickThoughtPool() in
// display.cpp), so the thought bubble reflects what's actually wrong rather
// than always sounding content.
static const char* const kThoughtsAdultHungry[] = {
    "So hungry...",
    "Need food now",
    "Stomach's growling",
    "When's dinner?",
    "Starving over here",
    "Just one berry, please",
    "Can't stop thinking about snacks",
    "Empty belly blues",
    "Foraging sounds great right now",
    "Anything to eat?",
};
static const int kThoughtsAdultHungryCount =
    sizeof(kThoughtsAdultHungry) / sizeof(kThoughtsAdultHungry[0]);

static const char* const kThoughtsAdultSad[] = {
    "So lonely",
    "Nobody visits anymore",
    "Feeling forgotten",
    "Wish someone would play",
    "Everything feels grey",
    "Missing the attention",
    "Just want to be noticed",
    "Heavy heart today",
    "Days blur together",
    "Nobody checks on me",
};
static const int kThoughtsAdultSadCount = sizeof(kThoughtsAdultSad) / sizeof(kThoughtsAdultSad[0]);

static const char* const kThoughtsAdultTired[] = {
    "So exhausted",
    "Can barely keep my eyes open",
    "Need real rest",
    "Running on empty",
    "Every step feels heavy",
    "Just want to collapse",
    "Worn all the way out",
    "Too tired to move",
    "Dragging myself along",
    "Desperate for a nap",
};
static const int kThoughtsAdultTiredCount =
    sizeof(kThoughtsAdultTired) / sizeof(kThoughtsAdultTired[0]);
