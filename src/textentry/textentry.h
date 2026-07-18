// textentry.h — an on-screen character picker driven entirely by
// LEFT/RIGHT (scroll the current character) and ENTER (commit it),
// shared by marmot naming and WiFi SSID/password entry -- anywhere this
// project needs free text with only 3 buttons and no keyboard.
#pragma once

namespace textentry {

// Not real printable characters -- sentinels appended after the printable
// charset so ENTER can trigger "delete last", "finish editing", or a
// keyboard-page toggle through the exact same scroll-and-commit motion as
// picking a letter.
static const char BACKSPACE = 1;
static const char DONE = 2;
static const char SHIFT = 3;    // toggles uppercase letters
static const char SYMBOLS = 4;  // toggles the symbols page (a "right shift")

static const int BUFFER_SIZE = 32;

struct State {
  char buffer[BUFFER_SIZE];
  int len;
  int pickerIndex;  // position in the active page's charset
  bool caps;         // SHIFT toggle -- uppercase letters when true
  bool symbols;       // SYMBOLS toggle -- symbols page instead of letters
};

// initial may be "" for a blank entry, or a starting value (e.g. re-editing
// an existing name). Always starts the picker on the first letter.
void init(State& s, const char* initial);

void moveNext(State& s);  // RIGHT
void movePrev(State& s);  // LEFT

// The character/action currently highlighted -- a printable char, or
// BACKSPACE/DONE.
char current(const State& s);

// Applies whatever's currently highlighted: appends a printable char,
// deletes the last character (BACKSPACE), or leaves the buffer alone and
// returns true (DONE) to signal editing is finished.
bool commit(State& s);

// Exposes the active page's charset for the on-screen keyboard grid (see
// display::renderTextEntry()) -- moveNext()/movePrev() just walk this same
// flat array, so a grid laid out in this order and a flat LEFT/RIGHT scroll
// stay in sync automatically (no separate 2D-navigation logic needed). The
// active page depends on s.caps/s.symbols, so these all take the State.
int charsetLen(const State& s);
char charsetAt(const State& s, int index);

// Row structure of the active page's charset (QWERTY-shaped letter rows
// with SHIFT/SYMBOLS/BACKSPACE folded into the row ends, then a
// SYMBOLS+SPACE+DONE row) so the renderer can lay out and center each row
// like a real keyboard instead of wrapping at a fixed column count. Rows
// are contiguous runs over the same flat charsetAt() array in order -- row
// 0 starts at index 0, row 1 starts right after row 0's keys, and so on.
int rowCount(const State& s);
int rowLen(const State& s, int row);

}  // namespace textentry
