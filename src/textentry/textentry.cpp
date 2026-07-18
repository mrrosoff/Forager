#include "textentry.h"

#include <cstring>

namespace textentry {

// Three keyboard pages, each real QWERTY-shaped rows: lowercase letters
// (default), uppercase letters (SHIFT), and symbols (SYMBOLS, a "right
// shift"). SHIFT/SYMBOLS are keys within the letters/symbols pages
// themselves so pressing one toggles to the other page; commit() (below)
// intercepts them as mode toggles rather than appending a character. Row
// lengths mirror the charset layout exactly so the grid renderer can
// center/stagger rows like a real keyboard -- moveNext()/movePrev() just
// walk the active page's flat array, so cursor motion always matches the
// row-major reading order drawn on screen.
static const char CHARSET_LOWER[] =
    "1234567890"
    "qwertyuiop"
    "asdfghjkl"
    "\x03zxcvbnm\x01"  // SHIFT z x c v b n m BACKSPACE
    "\x04 \x02";        // SYMBOLS SPACE DONE
static const int ROWLEN_LETTERS[] = {10, 10, 9, 9, 3};

static const char CHARSET_UPPER[] =
    "1234567890"
    "QWERTYUIOP"
    "ASDFGHJKL"
    "\x03ZXCVBNM\x01"  // SHIFT Z X C V B N M BACKSPACE
    "\x04 \x02";        // SYMBOLS SPACE DONE

static const char CHARSET_SYMBOLS[] =
    "1234567890"
    "!@#$%^&*()"
    "-_=+;:'\",."
    "/?\\|~`\x01"  // symbols BACKSPACE
    "\x04 \x02";    // SYMBOLS SPACE DONE
static const int ROWLEN_SYMBOLS[] = {10, 10, 10, 7, 3};

static const int CHARSET_LOWER_LEN = sizeof(CHARSET_LOWER) - 1;
static const int CHARSET_UPPER_LEN = sizeof(CHARSET_UPPER) - 1;
static const int CHARSET_SYMBOLS_LEN = sizeof(CHARSET_SYMBOLS) - 1;
static const int ROW_COUNT_LETTERS = sizeof(ROWLEN_LETTERS) / sizeof(ROWLEN_LETTERS[0]);
static const int ROW_COUNT_SYMBOLS = sizeof(ROWLEN_SYMBOLS) / sizeof(ROWLEN_SYMBOLS[0]);

static const char* activePage(const State& s, int* outLen) {
  if (s.symbols) {
    *outLen = CHARSET_SYMBOLS_LEN;
    return CHARSET_SYMBOLS;
  }
  if (s.caps) {
    *outLen = CHARSET_UPPER_LEN;
    return CHARSET_UPPER;
  }
  *outLen = CHARSET_LOWER_LEN;
  return CHARSET_LOWER;
}

void init(State& s, const char* initial) {
  s.len = 0;
  s.buffer[0] = '\0';
  if (initial && initial[0]) {
    strncpy(s.buffer, initial, BUFFER_SIZE - 1);
    s.buffer[BUFFER_SIZE - 1] = '\0';
    s.len = (int)strlen(s.buffer);
  }
  s.caps = false;
  s.symbols = false;
  s.pickerIndex = 10;  // 'q' -- index 0-9 is the digit row
}

void moveNext(State& s) {
  int len;
  activePage(s, &len);
  s.pickerIndex = (s.pickerIndex + 1) % len;
}

void movePrev(State& s) {
  int len;
  activePage(s, &len);
  s.pickerIndex = (s.pickerIndex - 1 + len) % len;
}

char current(const State& s) {
  int len;
  const char* page = activePage(s, &len);
  return page[s.pickerIndex];
}

int charsetLen(const State& s) {
  int len;
  activePage(s, &len);
  return len;
}

char charsetAt(const State& s, int index) {
  int len;
  const char* page = activePage(s, &len);
  return page[index];
}

int rowCount(const State& s) { return s.symbols ? ROW_COUNT_SYMBOLS : ROW_COUNT_LETTERS; }

int rowLen(const State& s, int row) { return s.symbols ? ROWLEN_SYMBOLS[row] : ROWLEN_LETTERS[row]; }

bool commit(State& s) {
  char c = current(s);
  if (c == DONE) return true;
  if (c == BACKSPACE) {
    if (s.len > 0) s.buffer[--s.len] = '\0';
    return false;
  }
  if (c == SHIFT) {
    s.caps = !s.caps;  // same page shape upper/lower -- pickerIndex stays valid
    return false;
  }
  if (c == SYMBOLS) {
    s.symbols = !s.symbols;
    s.pickerIndex = 0;  // different page shape -- reset to a safe index
    return false;
  }
  if (s.len < BUFFER_SIZE - 1) {
    s.buffer[s.len++] = c;
    s.buffer[s.len] = '\0';
  }
  return false;
}

}  // namespace textentry
