#include "minigames.h"

#include <Arduino.h>
#include <Preferences.h>

#include <cctype>
#include <cstring>

#include "config.h"
#include "foraging.h"
#include "journal.h"

namespace minigames {

// One NVS key per game, in the shared "forager" namespace (see CLAUDE.md's
// persistence notes) -- so Settings -> Reset Game's single clear() wipes
// high scores along with everything else, which is what "reset the game"
// should mean. Indexed by Game, so this array's order tracks that enum.
static const char* const kScoreKey[(int)Game::COUNT] = {"hsSnack", "hsSimon", "hsMemory", "hsMaze",
                                                        "hsQuiz"};
static uint16_t sBest[(int)Game::COUNT] = {0, 0, 0, 0, 0};

// The Winter Stash: one count per kind, plus the last year already
// resolved, so a December wake settles up exactly once.
static const char* const kStashKey[STASH_KINDS] = {"stashG", "stashL", "stashT", "stashF"};
static const char* const kStashYearKey = "stashYr";
static const char* const kStashDayKey = "stashDay";
static Stash sStash = {{0, 0, 0, 0}};
static uint16_t sStashYear = 0;
// Day index (epoch/86400) of the last run that actually gathered -- one
// stocking run per day, see startSnackRun().
static uint32_t sStashDay = 0;

// Which games have already had their unlock screen shown, one bit per Game.
static const char* const kAnnouncedKey = "mgSeen";
static uint8_t sAnnounced = 0;

void load() {
  Preferences p;
  p.begin("forager", /*readOnly=*/true);
  for (int i = 0; i < (int)Game::COUNT; i++) sBest[i] = p.getUShort(kScoreKey[i], 0);
  sAnnounced = p.getUChar(kAnnouncedKey, 0);
  for (int i = 0; i < STASH_KINDS; i++) sStash.count[i] = p.getUShort(kStashKey[i], 0);
  sStashYear = p.getUShort(kStashYearKey, 0);
  sStashDay = p.getULong(kStashDayKey, 0);
  p.end();
}

// -------------------------------------------------------- Winter Stash

const Stash& stash() { return sStash; }

int stashKindValue(uint8_t kind) {
  switch (kind) {
    case STASH_GRASS:
      return 1;
    case STASH_LEAVES:
      return 2;
    case STASH_TWIGS:
      return 3;
    case STASH_FLOWER:
      return 8;
    default:
      return 0;
  }
}

const char* stashKindName(uint8_t kind) {
  switch (kind) {
    case STASH_GRASS:
      return "dry grass";
    case STASH_LEAVES:
      return "leaves";
    case STASH_TWIGS:
      return "twigs";
    case STASH_FLOWER:
      return "a wildflower";
    default:
      return "";
  }
}

int stashPoints() {
  int total = 0;
  for (int i = 0; i < STASH_KINDS; i++) total += sStash.count[i] * stashKindValue((uint8_t)i);
  return total;
}

static void saveStash() {
  Preferences p;
  p.begin("forager", /*readOnly=*/false);
  for (int i = 0; i < STASH_KINDS; i++) p.putUShort(kStashKey[i], sStash.count[i]);
  p.putUShort(kStashYearKey, sStashYear);
  p.putULong(kStashDayKey, sStashDay);
  p.end();
}

bool stashResolveDue(int year, int month) { return month == 12 && sStashYear != (uint16_t)year; }

bool stashResolve(int year) {
  bool made = stashPoints() >= WINTER_STASH_GOAL;
  for (int i = 0; i < STASH_KINDS; i++) sStash.count[i] = 0;
  sStashYear = (uint16_t)year;
  saveStash();
  return made;
}

// ---------------------------------------------------------- Snack Hunt

/**
 * Rolls one bush. Most come up empty -- "sometimes you find something,
 * sometimes nothing" is the whole texture of the game -- and what's there
 * skews heavily toward bulk grass, with a wildflower as the real prize.
 *
 * These odds average about one point per pick, i.e. ~5 a day against a
 * 120-point goal: a season's work, deliberately, since the stockpile is
 * meant to be something you chip away at rather than clear in an evening.
 */
static void rollBush(SnackRound& r, int i) {
  int roll = (int)random(1000);
  if (roll < 700) {
    r.kind[i] = KIND_NONE;
    r.amount[i] = 0;
    return;
  }
  if (roll < 870) {
    r.kind[i] = STASH_GRASS;
    r.amount[i] = (uint8_t)(1 + random(4));
  } else if (roll < 940) {
    r.kind[i] = STASH_LEAVES;
    r.amount[i] = (uint8_t)(1 + random(3));
  } else if (roll < 985) {
    r.kind[i] = STASH_TWIGS;
    r.amount[i] = (uint8_t)(1 + random(2));
  } else {
    r.kind[i] = STASH_FLOWER;
    r.amount[i] = 1;
  }
}

void startSnackRound(State& s) {
  for (int i = 0; i < SnackRound::BUSHES; i++) rollBush(s.snack, i);
  s.snack.sel = 0;
  s.snack.picked = -1;
}

void startSnackRun(State& s, int64_t today) {
#if DEV_MODE_UNLOCK_MINIGAMES
  s.snack.banking = true;  // no daily limit while testing
  (void)today;
#else
  s.snack.banking = sStashDay != (uint32_t)today;
  if (s.snack.banking) {
    // Claim the day at the start of the run, not the end -- otherwise
    // quitting a bad run and restarting re-rolls the day's gathering.
    sStashDay = (uint32_t)today;
    saveStash();
  }
#endif
  s.snack.picksLeft = SnackRound::PICKS_PER_RUN;
  startSnackRound(s);
}

int snackPick(State& s) {
  int i = s.snack.sel;
  s.snack.picked = i;
  s.snack.picksLeft--;
  if (s.snack.kind[i] == KIND_NONE) return 0;
  int points = stashKindValue(s.snack.kind[i]) * s.snack.amount[i];
  if (!s.snack.banking) return points;  // practice run: scores, doesn't gather
  sStash.count[s.snack.kind[i]] += s.snack.amount[i];
  saveStash();
  return points;
}

uint8_t pendingUnlocks(Stage stage) {
  uint8_t pending = 0;
  for (int i = 0; i < (int)Game::COUNT; i++) {
    if (sAnnounced & (1 << i)) continue;
    if (isUnlocked((Game)i, stage)) pending |= (uint8_t)(1 << i);
  }
  return pending;
}

void markAnnounced(Game g) {
  sAnnounced |= (uint8_t)(1 << (int)g);
  Preferences p;
  p.begin("forager", /*readOnly=*/false);
  p.putUChar(kAnnouncedKey, sAnnounced);
  p.end();
}

int highScore(Game g) { return sBest[(int)g]; }

void finishRun(State& s) {
  int idx = (int)s.game;
  s.newBest = s.score > sBest[idx];
  if (s.newBest) {
    sBest[idx] = (uint16_t)s.score;
    Preferences p;
    p.begin("forager", /*readOnly=*/false);
    p.putUShort(kScoreKey[idx], sBest[idx]);
    p.end();
  }
  s.screen = Screen::Over;
}

const char* gameName(Game g) {
  switch (g) {
    case Game::Snack:
      return "Snack Hunt";
    case Game::Simon:
      return "Marmot Says";
    case Game::Memory:
      return "Forest Memory";
    case Game::Maze:
      return "Burrow Maze";
    default:
      return "Species Quiz";
  }
}

const char* gameBlurb(Game g) {
  switch (g) {
    case Game::Snack:
      return "Rummage for the winter stash";
    case Game::Simon:
      return "Repeat the button sequence";
    case Game::Memory:
      return "Match pairs of forest tokens";
    case Game::Maze:
      return "Dig your way to the exit";
    default:
      return "Name a species from a clue";
  }
}

bool isUnlocked(Game g, Stage stage) {
#if DEV_MODE_UNLOCK_MINIGAMES
  (void)g;
  (void)stage;
  return true;
#else
  switch (g) {
    case Game::Snack:
    case Game::Simon:
      return true;
    case Game::Memory:
      return stage != Stage::Baby;
    case Game::Maze:
      return stage == Stage::Adult;
    default:
      return journal::totalDiscovered() >= QUIZ_UNLOCK_DISCOVERED;
  }
#endif
}

std::string unlockHint(Game g, Stage stage) {
  (void)stage;
  switch (g) {
    case Game::Memory:
      return "Locked: grow to Juvenile";
    case Game::Maze:
      return "Locked: grow to Adult";
    case Game::Quiz:
      return "Locked: discover " + std::to_string(QUIZ_UNLOCK_DISCOVERED) + " species (" +
             std::to_string(journal::totalDiscovered()) + "/" +
             std::to_string(QUIZ_UNLOCK_DISCOVERED) + ")";
    default:
      return "";
  }
}

// Collects the raw species indices the player has discovered, up to `max`.
// Returns how many were written. Shared by the two species games, which
// both only ever ask about species the player has actually met.
static int collectDiscovered(int* out, int max) {
  int n = 0;
  for (int i = 0; i < foraging::speciesCount() && n < max; i++) {
    if (journal::isDiscovered(i)) out[n++] = i;
  }
  return n;
}

// -------------------------------------------------------------- Memory

void startMemoryBoard(State& s) {
  for (int i = 0; i < MemoryRound::PAIRS; i++) {
    s.memory.face[i * 2] = (uint8_t)i;
    s.memory.face[i * 2 + 1] = (uint8_t)i;
  }
  // Fisher-Yates, so every deal is equally likely -- a lazy swap-a-few-times
  // shuffle leaves pairs adjacent often enough to notice.
  for (int i = MemoryRound::CARDS - 1; i > 0; i--) {
    int j = (int)random(i + 1);
    uint8_t t = s.memory.face[i];
    s.memory.face[i] = s.memory.face[j];
    s.memory.face[j] = t;
  }
  for (int i = 0; i < MemoryRound::CARDS; i++) s.memory.matched[i] = false;
  s.memory.sel = 0;
  s.memory.firstFlip = -1;
  s.memory.secondFlip = -1;
  s.memory.pairsFound = 0;
  s.memory.missesLeft = MemoryRound::MISS_BUDGET;
}

bool memoryFlip(State& s) {
  int i = s.memory.sel;
  if (s.memory.matched[i]) return false;
  if (i == s.memory.firstFlip || i == s.memory.secondFlip) return false;
  if (s.memory.firstFlip < 0) {
    s.memory.firstFlip = i;
  } else if (s.memory.secondFlip < 0) {
    s.memory.secondFlip = i;
  } else {
    return false;  // turn already full, waiting on memoryResolveTurn()
  }
  return true;
}

bool memoryTurnPending(const State& s) {
  return s.memory.firstFlip >= 0 && s.memory.secondFlip >= 0;
}

bool memoryResolveTurn(State& s) {
  int a = s.memory.firstFlip, b = s.memory.secondFlip;
  s.memory.firstFlip = -1;
  s.memory.secondFlip = -1;
  if (a < 0 || b < 0) return false;
  if (s.memory.face[a] == s.memory.face[b]) {
    s.memory.matched[a] = true;
    s.memory.matched[b] = true;
    s.memory.pairsFound++;
    return true;
  }
  if (s.memory.missesLeft > 0) s.memory.missesLeft--;
  return false;
}

bool memoryBoardCleared(const State& s) { return s.memory.pairsFound >= MemoryRound::PAIRS; }

// --------------------------------------------------------------- Simon

void startSimonRound(State& s) {
  if (s.simon.len < SimonRound::MAX_LEN) s.simon.seq[s.simon.len++] = (uint8_t)random(3);
  s.simon.showIdx = 0;
  s.simon.inputIdx = 0;
}

bool simonPress(State& s, uint8_t button) {
  if (s.simon.inputIdx >= s.simon.len) return false;
  if (s.simon.seq[s.simon.inputIdx] != button) return false;
  s.simon.inputIdx++;
  return true;
}

bool simonRoundComplete(const State& s) { return s.simon.inputIdx >= s.simon.len; }

// ---------------------------------------------------------- Burrow Maze

// Direction index -> cell delta and the wall bit it passes through.
static const int kMazeDx[4] = {0, 1, 0, -1};
static const int kMazeDy[4] = {-1, 0, 1, 0};
static const uint8_t kMazeWall[4] = {MazeRound::WALL_N, MazeRound::WALL_E, MazeRound::WALL_S,
                                     MazeRound::WALL_W};

static bool mazeOpen(const MazeRound& m, int x, int y, int dir) {
  if (m.wall[y][x] & kMazeWall[dir]) return false;
  int nx = x + kMazeDx[dir], ny = y + kMazeDy[dir];
  return nx >= 0 && nx < MazeRound::N && ny >= 0 && ny < MazeRound::N;
}

void startMaze(State& s) {
  MazeRound& m = s.maze;
  for (int y = 0; y < MazeRound::N; y++) {
    for (int x = 0; x < MazeRound::N; x++) {
      m.wall[y][x] = 0x0F;  // every wall up
      m.seen[y][x] = false;
    }
  }

  // Depth-first backtracker with an explicit stack -- 25 cells, so recursion
  // would be fine too, but the loop version keeps stack use flat and
  // predictable on a device that also runs a WiFi stack.
  bool visited[MazeRound::N][MazeRound::N] = {};
  uint8_t stackX[MazeRound::N * MazeRound::N], stackY[MazeRound::N * MazeRound::N];
  int top = 0;
  int cx = 0, cy = 0;
  visited[0][0] = true;
  stackX[top] = 0;
  stackY[top] = 0;
  top++;
  while (top > 0) {
    cx = stackX[top - 1];
    cy = stackY[top - 1];
    int cand[4], n = 0;
    for (int d = 0; d < 4; d++) {
      int nx = cx + kMazeDx[d], ny = cy + kMazeDy[d];
      if (nx < 0 || nx >= MazeRound::N || ny < 0 || ny >= MazeRound::N) continue;
      if (visited[ny][nx]) continue;
      cand[n++] = d;
    }
    if (n == 0) {
      top--;
      continue;
    }
    int d = cand[random(n)];
    int nx = cx + kMazeDx[d], ny = cy + kMazeDy[d];
    m.wall[cy][cx] &= (uint8_t)~kMazeWall[d];
    m.wall[ny][nx] &= (uint8_t)~kMazeWall[(d + 2) % 4];  // the same wall, other side
    visited[ny][nx] = true;
    stackX[top] = (uint8_t)nx;
    stackY[top] = (uint8_t)ny;
    top++;
  }

  m.x = 0;
  m.y = 0;
  m.seen[0][0] = true;
  m.cameFrom = -1;
  m.sel = 0;
  m.movesLeft = MazeRound::MOVE_BUDGET;
}

int mazeOptions(const State& s, int* out) {
  const MazeRound& m = s.maze;
  int n = 0;
  for (int d = 0; d < 4; d++) {
    if (!mazeOpen(m, m.x, m.y, d)) continue;
    if (d == m.cameFrom) continue;
    out[n++] = d;
  }
  // A dead end: the only tunnel is the one just walked in through, so
  // offer it rather than leaving the player with nothing to press.
  if (n == 0 && m.cameFrom >= 0) out[n++] = m.cameFrom;
  return n;
}

bool mazeSolved(const State& s) {
  return s.maze.x == MazeRound::N - 1 && s.maze.y == MazeRound::N - 1;
}

void mazeAdvance(State& s) {
  MazeRound& m = s.maze;
  int opts[4];
  int n = mazeOptions(s, opts);
  if (n == 0) return;
  int dir = opts[m.sel % n];

  // Walk the chosen tunnel in a STRAIGHT LINE only. An earlier version
  // followed the corridor around corners, which meant pressing "down" could
  // leave the marmot travelling left -- the move stopped matching the
  // button that caused it, and the maze became impossible to read. Now a
  // move is always exactly the direction chosen: it runs on while the way
  // ahead stays open and there's nothing to decide, and stops the moment
  // the corridor turns, branches, or ends.
  for (int guard = 0; guard < MazeRound::N; guard++) {
    m.x += kMazeDx[dir];
    m.y += kMazeDy[dir];
    m.seen[m.y][m.x] = true;
    m.cameFrom = (dir + 2) % 4;  // the way back out of the new cell
    if (mazeSolved(s)) break;
    // Stop at anything worth looking at: a side opening (a real choice) or
    // a wall straight ahead (the corridor turns, and turning is the
    // player's call, not ours).
    bool sideOpening = false;
    for (int d = 0; d < 4; d++) {
      if (d == dir || d == m.cameFrom) continue;
      if (mazeOpen(m, m.x, m.y, d)) sideOpening = true;
    }
    if (sideOpening || !mazeOpen(m, m.x, m.y, dir)) break;
  }

  m.sel = 0;
  m.movesLeft--;
}

// -------------------------------------------------------- Species Quiz

#include "quiz_facts.h"

bool startQuizRound(State& s) {
  static int discovered[256];
  int n = collectDiscovered(discovered, (int)(sizeof(discovered) / sizeof(discovered[0])));
  if (n == 0) return false;
  int answer = discovered[random(n)];

  // Decoys come from the discovered pool too once it's big enough -- names
  // the player has never heard of would make the question answerable by
  // elimination rather than by knowing anything. The 50-species unlock
  // guarantees that in practice.
  bool decoysFromDiscovered = n >= 3;
  int chosen[3] = {answer, -1, -1};
  int filled = 1;
  while (filled < 3) {
    int candidate =
        decoysFromDiscovered ? discovered[random(n)] : (int)random(foraging::speciesCount());
    bool dup = false;
    for (int j = 0; j < filled; j++) {
      if (chosen[j] == candidate) dup = true;
    }
    if (!dup) chosen[filled++] = candidate;
  }

  s.choice.correct = (int)random(3);
  s.choice.option[s.choice.correct] = chosen[0];
  int next = 1;
  for (int i = 0; i < 3; i++) {
    if (i != s.choice.correct) s.choice.option[i] = chosen[next++];
  }
  s.choice.sel = 0;
  s.choice.factIdx = (uint8_t)random(4);  // quiz_facts.h wraps this per species
  return true;
}

const char* quizFact(const State& s) {
  const Forageable& f = choiceAnswer(s);
  const char* fact = quiz_facts::find(f.name, s.choice.factIdx);
  return fact ? fact : f.note;
}

const Forageable& choiceOption(const State& s, int i) {
  return foraging::speciesAt(s.choice.option[i]);
}

const Forageable& choiceAnswer(const State& s) { return choiceOption(s, s.choice.correct); }

bool choiceCorrect(const State& s, int sel) { return sel == s.choice.correct; }

}  // namespace minigames
