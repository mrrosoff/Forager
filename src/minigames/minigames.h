// minigames.h — game state + rules only; no rendering (display.cpp) and no
// button reading (main.cpp). Every game is turn-based, and that's a hard
// constraint: a panel refresh is hundreds of milliseconds, so anything scored
// on reaction time is unplayable. See CLAUDE.md for what that ruled out.
#pragma once

#include <string>

#include "model.h"

namespace minigames {

/**
 * Listed (and numbered) in unlock order, easiest first -- Snack Hunt and
 * Marmot Says are both playable from birth, so a brand-new marmot's menu
 * opens with two live rows rather than a wall of locks.
 */
enum class Game : uint8_t { Snack = 0, Simon, Memory, Maze, Quiz, COUNT };

/**
 * Unlock thresholds, on two axes so progress in either direction opens
 * something: Snack Hunt and Marmot Says need nothing (rummaging under a
 * bush and copying the marmot both work on day one), Forest Memory is gated
 * on growth, and the two word games are gated on the journal -- neither can
 * ask anything worth asking without a real collection of species behind it.
 */

static const int QUIZ_UNLOCK_DISCOVERED = 50;

bool isUnlocked(Game g, Stage stage);

// How many games are unlocked right now -- i.e. how many rows the menu
// shows. Locked games don't appear on the menu at all (see visibleGameAt()),
// so the row count and the unlocked count are the same thing.
int visibleGameCount(Stage stage);

// The game behind menu row `idx` (0-based, in Game enum/unlock order among
// just the unlocked games). Undefined if idx >= visibleGameCount(stage).
Game visibleGameAt(Stage stage, int idx);

/**
 * Which screen the minigames view is showing. Menu is the only one LEFT/
 * RIGHT still navigate the way the rest of the app does (RIGHT leaves for
 * Status); every other screen means a run is in progress and the game owns
 * all three buttons, with SETTINGS/KEY1 as "quit back to Menu" -- the same
 * back convention Settings uses.
 */
enum class Screen : uint8_t {
  Menu,      // pick a game
  Sequence,  // non-interactive frame on a timer (Simon's playback, Memory's
             // look-at-the-two-cards beat)
  Prompt,    // the player's turn
  Reveal,    // what was under the bushes; or the Quiz's full species photo
  Over,      // run finished; score vs. best
};

/**
 * Forest Memory: concentration on a 4x3 grid of forest tokens. The faces
 * are simple procedural glyphs (see display's drawForestIcon()), not species
 * photos -- a dithered photo at card size is unreadable speckle, the same
 * reason the achievement badges aren't photos either.
 *
 * Mismatches cost one of `missesLeft`; clearing the board deals a fresh one
 * and restores the budget, so a run is "how many pairs before your memory
 * runs out" rather than an untimed solitaire with no fail state.
 */
struct MemoryRound {
  static const int CARDS = 12;  // 6 pairs, 4 columns x 3 rows
  static const int PAIRS = CARDS / 2;
  static const int MISS_BUDGET = 6;
  uint8_t face[CARDS];  // icon id, two cards per id
  bool matched[CARDS];  // permanently face-up
  int sel;              // cursor
  int firstFlip;        // -1 = none face-up yet this turn
  int secondFlip;       // -1 until the second card of the turn is turned
  int pairsFound;       // this board
  int missesLeft;
};

/**
 * Snack Hunt: four bushes, rummage under one. Each bush independently
 * hides something or nothing, so a pick is genuinely a gamble rather than a
 * one-in-four shell game -- and all four are revealed afterward, so you
 * always see what you walked past.
 *
 * This is the gathering end of the Winter Stash (below): whatever a run
 * turns up is added to the persisted stockpile, which is the actual
 * long-term goal. The per-run score is just that session's haul in points.
 */
struct SnackRound {
  static const int BUSHES = 4;
  static const int PICKS_PER_RUN = 5;
  uint8_t kind[BUSHES];    // StashKind, or KIND_NONE for an empty bush
  uint8_t amount[BUSHES];  // how much, 0 if empty
  int sel;                 // cursor
  int picked;              // -1 until a bush is chosen this round
  int picksLeft;
  /**
   * False once the day's gathering is already done (see startSnackRun()).
   * The game stays fully playable -- it just stops feeding the stockpile,
   * because a stash you can grind twenty runs into in one sitting isn't a
   * long-term goal, it's a button you hold down. Later runs still score for
   * the high score, and the screen says which mode it's in.
   */
  bool banking;
};

/**
 * What a marmot drags back to the den, in rising order of value (see
 * stashPoints()) -- dry grass and leaves for bedding, twigs, and the
 * occasional wildflower. Deliberately generic marmot things rather than
 * anything from the foraging species table: the Foraging view is where
 * real species live, and having a minigame hand out "Chanterelle x3" would
 * blur two systems that mean different things.
 */
enum StashKind : uint8_t { STASH_GRASS = 0, STASH_LEAVES, STASH_TWIGS, STASH_FLOWER, STASH_KINDS };

/**
 * Sentinels for the two things under a bush that aren't stash material.
 * They live in the same `kind` field as StashKind, above the real values.
 * When either is set, `amount` holds which critter/predator it is rather
 * than a quantity.
 */
static const uint8_t KIND_NONE = 0xFF;
static const uint8_t KIND_CRITTER = 0xFE;  // harmless; flavour, no stash value
static const uint8_t KIND_SCARE = 0xFD;    // a predator: the run ends here

static const int CRITTER_COUNT = 4;
static const int PREDATOR_COUNT = 4;

// Names for the reveal line. Predator order must match the art table in
// display.cpp's snackPredatorArt().
const char* critterName(int i);
const char* predatorName(int i);

/**
 * The persisted stockpile. Unlike everything else in this module it is
 * explicitly NOT a per-run score -- it accumulates across sessions and
 * across days, and it's what Snack Hunt is really for. Reaching
 * WINTER_STASH_GOAL points before winter arrives is the goal; see
 * stashResolveDue() for what happens when it does.
 */
struct Stash {
  uint16_t count[STASH_KINDS];
};

/**
 * Roughly three and a half weeks of daily gathering (a run averages ~5
 * points; see rollBush()'s odds), so it's a season-long goal rather than
 * something a determined evening finishes.
 */
static const int WINTER_STASH_GOAL = 120;

/**
 * A marmot born too close to winter never had a runway, so it isn't
 * penalized for a thin larder -- it just gets told the winter came early.
 * Days of life before December that count as a fair chance.
 */
static const int WINTER_GRACE_DAYS = 60;

const Stash& stash();

// Points for the stockpile as a whole, weighting each kind by how hard it
// is to come by (grass 1, seed 3, root 5, flower 10).
int stashPoints();

// Points a single find is worth, for the "+5" feedback on a reveal.
int stashKindValue(uint8_t kind);

const char* stashKindName(uint8_t kind);

/**
 * True on the first wake of a winter (December) that hasn't been resolved
 * yet -- main.cpp shows the outcome screen, applies the reward or the
 * shortfall, then calls stashResolve() to bank the year and clear the pile
 * for the next one.
 */
bool stashResolveDue(int year, int month);

// True if the pile met the goal; clears it and records `year` as resolved.
bool stashResolve(int year);

/**
 * Repeating this many calls back correctly wins the run outright. Without a
 * ceiling the sequence hit MAX_LEN and then replayed the same length forever,
 * scoring each time -- an unbounded score rather than a win.
 */
static const int SIMON_WIN_ROUNDS = 25;

/**
 * Marmot Says: a growing button sequence the player repeats. Elements are
 * 0 = LEFT, 1 = RIGHT, 2 = ENTER -- i.e. the physical buttons themselves,
 * so there's nothing to map on screen beyond the arrow glyphs.
 */
struct SimonRound {
  // 25 is the win, so the sequence has to be able to reach it.
  static const int MAX_LEN = 25;
  uint8_t seq[MAX_LEN];
  int len;
  int showIdx;   // element being played back during Screen::Sequence
  int inputIdx;  // how many correct presses so far this round
  /**
   * Which of three icons stands for each button this run, indexed by button
   * (0 = LEFT, 1 = RIGHT, 2 = ENTER) and holding an icon id the display
   * maps to art. Reshuffled every run, so the calls have to be read rather
   * than memorised as "left, left, right" from a previous game.
   */
  uint8_t icon[3];
};

/**
 * Burrow Maze: a tunnel dig from the top-left corner to the bottom-right.
 *
 * Three buttons can't steer in four directions, so the maze never asks for
 * one: each junction lists the tunnels actually leaving this cell, LEFT/RIGHT
 * cycle that list, ENTER commits, and the marmot walks the corridor to the
 * next junction by itself. One press is one *decision*, not one cell. A dead
 * end offers the way you came, so backing out isn't a stuck state.
 *
 * `wall` is a per-cell N/E/S/W bitmask with both sides agreeing; `seen` is the
 * trail already dug.
 */
struct MazeRound {
  // Grid grows 5x5 -> 6x6 -> 7x7 as mazes are cleared (see mazeSizeFor()),
  // so a run gets harder rather than just longer. Storage is fixed at the
  // largest; `n` is the live size.
  static const int MAX_N = 7;
  static const uint8_t WALL_N = 1, WALL_E = 2, WALL_S = 4, WALL_W = 8;
  uint8_t wall[MAX_N][MAX_N];
  bool seen[MAX_N][MAX_N];
  int n;          // this maze's side length
  int x, y;       // marmot's cell
  int cameFrom;   // direction it entered this cell from; -1 at the start
  int sel;        // index into this cell's option list (see mazeOptions())
  int movesLeft;  // running out ends the run
};

// Side length for the given number of mazes already solved.
int mazeSizeFor(int score);

/**
 * Moves allowed for an n-wide maze -- how far ahead of the rising meltwater
 * the marmot starts. Counted in moves, not seconds, because the panel only
 * redraws on a press: a wall clock would sit frozen and then jump, drowning
 * the player to a timer they never saw move.
 */
int mazeBudgetFor(int n);

/**
 * Species Quiz: three species names, one of which owns the clue on screen.
 * Options index the raw foraging::speciesAt() table (not browse ranks) so a
 * question stays valid regardless of the per-wake browse order. `factIdx`
 * picks which clue from that species' bank is being asked (see
 * quiz_facts.h), so meeting the same species twice isn't the same question.
 */
struct ChoiceRound {
  int option[3];
  int correct;  // 0..2
  int sel;      // highlighted option
  uint8_t factIdx;
};

struct State {
  Screen screen = Screen::Menu;
  Game game = Game::Simon;
  int menuSel = 0;
  int score = 0;         // this run
  bool newBest = false;  // set by finishRun() when score beat the stored best
  bool won = false;      // run ended by beating the game, not by failing
  SnackRound snack;
  MemoryRound memory;
  SimonRound simon;
  MazeRound maze;
  ChoiceRound choice;
};

// Pulls the four persisted high scores and the announced-unlocks bitmask
// out of NVS into RAM. Call once per wake, before any highScore()/
// finishRun()/pendingUnlocks() call (mirrors journal::load()).
void load();

/**
 * Bitmask (1 << (int)Game) of games that are unlocked but whose "you can
 * play this now" screen hasn't been shown yet -- see
 * display::renderMinigameUnlock() and main.cpp's
 * showPendingMinigameUnlocks(). Tracked separately from the unlock rules
 * themselves so the reveal fires exactly once, on the wake (or the
 * Discovery resolve) that crossed the threshold, rather than every time the
 * condition happens to still be true.
 */
uint8_t pendingUnlocks(Stage stage);

// Records that g's unlock screen has been shown, persisting immediately so
// losing power right after the reveal can't replay it forever.
void markAnnounced(Game g);

int highScore(Game g);

/**
 * Ends the current run: records s.score as the new best for s.game if it
 * beats the stored one (persisting immediately -- runs are short and
 * infrequent enough that there's no reason to batch the write), sets
 * s.newBest, and moves to Screen::Over.
 */
void finishRun(State& s);

const char* gameName(Game g);

// One-line "what am I doing" blurb for the menu, under each game's name.
const char* gameBlurb(Game g);

/**
 * Fresh run: full pick budget, first round's bushes rolled. `today` is a
 * day index (epoch/86400) -- the first run of each day banks its finds into
 * the stockpile and claims the day; every run after that is a practice run
 * that scores but doesn't gather. Claiming the day up front means quitting
 * a run early can't be used to re-roll it.
 */
void startSnackRun(State& s, int64_t today);

// Rolls a new set of four bushes and clears the pick.
void startSnackRound(State& s);

/**
 * Rummages under the selected bush, banking anything found into the
 * persisted stash and spending a pick. Returns the points found (0 if the
 * bush was empty). All four bushes stay revealed until the next round.
 */
int snackPick(State& s);

// Deals a fresh shuffled board and restores the miss budget.
void startMemoryBoard(State& s);

/**
 * Turns the card under the cursor face-up. Returns false if that card is
 * already face-up or matched (the press does nothing rather than costing a
 * turn). When this fills the second slot the caller shows the pair on a
 * timed frame and then calls memoryResolveTurn().
 */
bool memoryFlip(State& s);

// True once two cards are face-up and the turn is waiting to be resolved.
bool memoryTurnPending(const State& s);

/**
 * Scores the two face-up cards: a match stays up and bumps pairsFound, a
 * mismatch flips them back and spends a miss. Returns true if it matched.
 */
bool memoryResolveTurn(State& s);

bool memoryBoardCleared(const State& s);

// Fresh run: empties the sequence, shuffles which icon each button carries,
// then deals the first call.
void startSimonRun(State& s);

// Appends one element to the sequence and rewinds playback/input. Called
// after every cleared round.
void startSimonRound(State& s);

/**
 * True if the pressed button (0/1/2, same encoding as SimonRound::seq)
 * matched the next expected element, advancing inputIdx if so. Caller
 * checks simonRoundComplete() afterward.
 */
bool simonPress(State& s, uint8_t button);

bool simonRoundComplete(const State& s);

// Carves a fresh maze and puts the marmot at the entrance.
void startMaze(State& s);

/**
 * The tunnels leading out of the current cell, as direction indices
 * (0 = N, 1 = E, 2 = S, 3 = W) written into `out` (capacity 4). Excludes
 * the way the marmot arrived, unless that's the only way out of a dead end.
 * Returns how many were written -- this is exactly the list LEFT/RIGHT
 * cycle through.
 */
int mazeOptions(const State& s, int* out);

/**
 * Commits the selected tunnel and walks it: through the chosen wall, then
 * onward through any single-exit corridor cells, stopping at the next
 * junction, dead end, or the exit. Spends one move from the budget for the
 * whole run of cells.
 */
void mazeAdvance(State& s);

bool mazeSolved(const State& s);

/**
 * Sets up the next Species Quiz question. Draws the answer from species the
 * player has actually discovered (a clue about something never seen isn't a
 * quiz, it's a coin flip); decoys come from that same pool, which the
 * 50-species unlock guarantees is big enough. Returns false if somehow
 * nothing is discovered.
 */
bool startQuizRound(State& s);

/**
 * The clue shown as the question, out of the dedicated quiz bank in
 * quiz_facts.h -- deliberately separate prose from the species table's
 * `note`/`harvestTip`, which the Foraging view already shows verbatim (a
 * quiz whose questions are quotes from the reference card you can page over
 * to is not a quiz). Falls back to the species note if a species somehow
 * has no bank entry.
 */
const char* quizFact(const State& s);

// The species behind option `i` of the current question.
const Forageable& choiceOption(const State& s, int i);

// The right answer, i.e. what the Reveal screen shows the photo of.
const Forageable& choiceAnswer(const State& s);

// True if `sel` was the right answer; the caller advances to Reveal (right)
// or Over (wrong).
bool choiceCorrect(const State& s, int sel);

}  // namespace minigames
