// textentry.h — an on-screen character picker driven entirely by
// LEFT/RIGHT (scroll the current character) and ENTER (commit it),
// shared by marmot naming and WiFi SSID/password entry -- anywhere this
// project needs free text with only 3 buttons and no keyboard.
#pragma once

namespace textentry {

// Not real printable characters -- sentinels appended after the printable
// charset so ENTER can trigger "delete last" or "finish editing" through
// the exact same scroll-and-commit motion as picking a letter.
static const char BACKSPACE = 1;
static const char DONE = 2;

static const int BUFFER_SIZE = 32;

struct State {
  char buffer[BUFFER_SIZE];
  int len;
  int pickerIndex;  // position in the shared charset
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

}  // namespace textentry
