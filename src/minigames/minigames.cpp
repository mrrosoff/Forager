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
      return "wood";
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
#if DEV_MODE_SHOW_ALL_CONTENT
  // Walk a fixed list covering every possible bush content, four per round
  // (all four open on the reveal), so two or three rounds shows the lot.
  struct DevBush {
    uint8_t kind, amount;
  };
  static const DevBush kAll[] = {
      {KIND_NONE, 0},    {STASH_GRASS, 1},  {STASH_GRASS, 4},  {STASH_LEAVES, 2}, {STASH_TWIGS, 1},
      {STASH_TWIGS, 2},  {STASH_FLOWER, 1}, {KIND_CRITTER, 0}, {KIND_CRITTER, 1}, {KIND_CRITTER, 2},
      {KIND_CRITTER, 3}, {KIND_SCARE, 0},   {KIND_SCARE, 1},   {KIND_SCARE, 2},   {KIND_SCARE, 3},
  };
  static int devIdx = 0;
  const DevBush& b = kAll[devIdx % (int)(sizeof(kAll) / sizeof(kAll[0]))];
  devIdx++;
  r.kind[i] = b.kind;
  r.amount[i] = b.amount;
  return;
#else
  int roll = (int)random(1000);
  if (roll < 620) {
    r.kind[i] = KIND_NONE;
    r.amount[i] = 0;
  } else if (roll < 780) {
    r.kind[i] = STASH_GRASS;
    r.amount[i] = (uint8_t)(1 + random(4));
  } else if (roll < 845) {
    r.kind[i] = STASH_LEAVES;
    r.amount[i] = (uint8_t)(1 + random(3));
  } else if (roll < 885) {
    r.kind[i] = STASH_TWIGS;
    r.amount[i] = (uint8_t)(1 + random(2));
  } else if (roll < 898) {
    r.kind[i] = STASH_FLOWER;
    r.amount[i] = 1;
  } else if (roll < 965) {
    // Something living, but harmless -- no stash value, just the small
    // surprise of turning over a rock.
    r.kind[i] = KIND_CRITTER;
    r.amount[i] = (uint8_t)random(CRITTER_COUNT);
  } else {
    // A predator. Not fatal: the marmot bolts, which costs the rest of the
    // run's picks but never what's already banked (see snackPick()).
    r.kind[i] = KIND_SCARE;
    r.amount[i] = (uint8_t)random(PREDATOR_COUNT);
  }
#endif
}

const char* critterName(int i) {
  switch (i) {
    case 0:
      return "a ground beetle";
    case 1:
      return "an alligator lizard";
    case 2:
      return "a spider";
    default:
      return "a cricket";
  }
}

const char* predatorName(int i) {
  switch (i) {
    case 0:
      return "A coyote";
    case 1:
      return "A cougar";
    case 2:
      return "A red fox";
    default:
      return "A barred owl";
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
  uint8_t kind = s.snack.kind[i];
  if (kind == KIND_SCARE) {
    // Bolting ends the run, so spend the remaining picks -- but everything
    // already found stays banked. The cost is the rest of today's foraging,
    // not the day's haul.
    s.snack.picksLeft = 0;
    return 0;
  }
  if (kind == KIND_NONE || kind == KIND_CRITTER) return 0;
  int points = stashKindValue(kind) * s.snack.amount[i];
  if (!s.snack.banking) return points;  // practice run: scores, doesn't gather
  sStash.count[kind] += s.snack.amount[i];
  saveStash();
  return points;
}

// Games available from birth aren't announced: their "unlock" is just the
// game existing, and a brand-new marmot already walks through a birth reveal
// and a naming screen before reaching Main -- two more ceremony screens for
// things that were never locked is all ritual and no news.
static bool unlockedAtBirth(Game g) { return g == Game::Snack || g == Game::Simon; }

uint8_t pendingUnlocks(Stage stage) {
  uint8_t pending = 0;
  for (int i = 0; i < (int)Game::COUNT; i++) {
    if (sAnnounced & (1 << i)) continue;
    if (unlockedAtBirth((Game)i)) continue;
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
      return "Outrun the meltwater to the exit";
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

void startSimonRun(State& s) {
  s.simon.len = 0;
  s.won = false;
  // One icon per button, shuffled -- so which button means which call has to
  // be read off the screen each run rather than remembered from the last.
  for (int i = 0; i < 3; i++) s.simon.icon[i] = (uint8_t)i;
  for (int i = 2; i > 0; i--) {
    int j = (int)random(i + 1);
    uint8_t t = s.simon.icon[i];
    s.simon.icon[i] = s.simon.icon[j];
    s.simon.icon[j] = t;
  }
  startSimonRound(s);
}

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
  return nx >= 0 && nx < m.n && ny >= 0 && ny < m.n;
}

int mazeSizeFor(int score) {
  if (score < 2) return 5;
  if (score < 4) return 6;
  return MazeRound::MAX_N;
}

int mazeBudgetFor(int n) {
  // Four moves per row of maze. Braiding (see startMaze()) means junctions
  // are frequent, so a straight run stops more often and each move covers
  // less ground than it did in a single-corridor maze -- three per row was
  // tuned before that and is now unfairly tight.
  return n * 4;
}

void startMaze(State& s) {
  MazeRound& m = s.maze;
  m.n = mazeSizeFor(s.score);
  for (int y = 0; y < m.n; y++) {
    for (int x = 0; x < m.n; x++) {
      m.wall[y][x] = 0x0F;  // every wall up
      m.seen[y][x] = false;
    }
  }

  // Depth-first backtracker with an explicit stack -- at most 49 cells, so
  // recursion would be fine too, but the loop version keeps stack use flat
  // and predictable on a device that also runs a WiFi stack.
  bool visited[MazeRound::MAX_N][MazeRound::MAX_N] = {};
  uint8_t stackX[MazeRound::MAX_N * MazeRound::MAX_N];
  uint8_t stackY[MazeRound::MAX_N * MazeRound::MAX_N];
  int top = 0;
  int cx = 0, cy = 0;
  visited[0][0] = true;
  stackX[top] = 0;
  stackY[top] = 0;
  top++;
  while (top > 0) {
    cx = stackX[top - 1];
    cy = stackY[top - 1];
    int cand[4], nc = 0;
    for (int d = 0; d < 4; d++) {
      int nx = cx + kMazeDx[d], ny = cy + kMazeDy[d];
      if (nx < 0 || nx >= m.n || ny < 0 || ny >= m.n) continue;
      if (visited[ny][nx]) continue;
      cand[nc++] = d;
    }
    if (nc == 0) {
      top--;
      continue;
    }
    int d = cand[random(nc)];
    int nx = cx + kMazeDx[d], ny = cy + kMazeDy[d];
    m.wall[cy][cx] &= (uint8_t)~kMazeWall[d];
    m.wall[ny][nx] &= (uint8_t)~kMazeWall[(d + 2) % 4];  // the same wall, other side
    visited[ny][nx] = true;
    stackX[top] = (uint8_t)nx;
    stackY[top] = (uint8_t)ny;
    top++;
  }

  // Braid the maze. A plain depth-first maze is a *perfect* maze: exactly
  // one route between any two cells, which means most cells have a single
  // way onward and "which tunnel?" is usually no choice at all -- a long
  // corridor with occasional dead ends. Knocking extra walls out creates
  // loops, so cells routinely offer two or three real options and there's
  // more than one way through.
  int extra = m.n * m.n / 2;
  for (int k = 0; k < extra; k++) {
    int x = (int)random(m.n), y = (int)random(m.n);
    int d = (int)random(4);
    int nx = x + kMazeDx[d], ny = y + kMazeDy[d];
    if (nx < 0 || nx >= m.n || ny < 0 || ny >= m.n) continue;
    m.wall[y][x] &= (uint8_t)~kMazeWall[d];
    m.wall[ny][nx] &= (uint8_t)~kMazeWall[(d + 2) % 4];
  }

  // Then open out most dead ends, which is what "braided" usually means:
  // a dead end is a move spent on nothing, and with the meltwater rising
  // behind you a maze full of them is just punishing rather than tricky.
  for (int y = 0; y < m.n; y++) {
    for (int x = 0; x < m.n; x++) {
      int open = 0;
      for (int d = 0; d < 4; d++) {
        if (mazeOpen(m, x, y, d)) open++;
      }
      if (open != 1 || random(100) < 25) continue;  // leave a quarter of them
      for (int t = 0; t < 8; t++) {
        int d = (int)random(4);
        int nx = x + kMazeDx[d], ny = y + kMazeDy[d];
        if (nx < 0 || nx >= m.n || ny < 0 || ny >= m.n) continue;
        if (!(m.wall[y][x] & kMazeWall[d])) continue;  // already open
        m.wall[y][x] &= (uint8_t)~kMazeWall[d];
        m.wall[ny][nx] &= (uint8_t)~kMazeWall[(d + 2) % 4];
        break;
      }
    }
  }

  m.x = 0;
  m.y = 0;
  m.seen[0][0] = true;
  m.cameFrom = -1;
  m.sel = 0;
  m.movesLeft = mazeBudgetFor(m.n);
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

bool mazeSolved(const State& s) { return s.maze.x == s.maze.n - 1 && s.maze.y == s.maze.n - 1; }

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
  for (int guard = 0; guard < m.n; guard++) {
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
