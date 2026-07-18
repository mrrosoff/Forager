#include "textentry.h"

#include <cstring>

namespace textentry {

// Space first (so a fresh entry can start with a word boundary), then
// upper/lowercase letters, digits, a handful of symbols common in WiFi
// passwords, then the two control sentinels last.
static const char CHARSET[] =
    " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*-_=+.,"
    "\x01\x02";
static const int CHARSET_LEN = sizeof(CHARSET) - 1;  // exclude the string's trailing '\0'

void init(State& s, const char* initial) {
  s.len = 0;
  s.buffer[0] = '\0';
  if (initial && initial[0]) {
    strncpy(s.buffer, initial, BUFFER_SIZE - 1);
    s.buffer[BUFFER_SIZE - 1] = '\0';
    s.len = (int)strlen(s.buffer);
  }
  s.pickerIndex = 1;  // 'A' -- index 0 is space, rarely the first pick wanted
}

void moveNext(State& s) { s.pickerIndex = (s.pickerIndex + 1) % CHARSET_LEN; }

void movePrev(State& s) { s.pickerIndex = (s.pickerIndex - 1 + CHARSET_LEN) % CHARSET_LEN; }

char current(const State& s) { return CHARSET[s.pickerIndex]; }

bool commit(State& s) {
  char c = current(s);
  if (c == DONE) return true;
  if (c == BACKSPACE) {
    if (s.len > 0) s.buffer[--s.len] = '\0';
    return false;
  }
  if (s.len < BUFFER_SIZE - 1) {
    s.buffer[s.len++] = c;
    s.buffer[s.len] = '\0';
  }
  return false;
}

}  // namespace textentry
