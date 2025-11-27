#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>

#define P_BYTES (1 << 0)
#define P_CHARS (1 << 1)
#define P_LINES (1 << 2)
#define P_LENMX (1 << 3)

struct help_entry {
  const char *opt;
  const char *desc;
};

struct option long_options[] = {
  {"bytes", no_argument, 0, 'c'},
  {"chars", no_argument, 0, 'm'},
  {"lines", no_argument, 0, 'l'},
  {"max-line-length", no_argument, 0, 'L'},
  {"total", required_argument, 0, 3},
  {"files0-from", required_argument, 0, 4},
  {"help", no_argument, 0, 1},
  {"version", no_argument, 0, 2}
};

struct wc {
  long lines,
  words,
  chars,
  bytes,
  maxlen;
};

struct wc count_word(char *str) {
  long lines = 0, words = 0, chars = 0;
  bool in_word = true;

  for (int i = 0; i < strlen(str); i++) {
    chars++;
    if (str[i] == '\n') lines++;

    if (isspace((unsigned char)str[i])) {
      in_word = false;
    } else {
      if (!in_word) {
        words++;
        in_word = true;
      }
    }
  }

  struct wc willy; // we'll just call him that idk
  willy.lines = lines;
  willy.words = words;
  willy.chars = chars;
  willy.bytes = strlen(str);
  // TODO: maxlen
  return willy;
}

struct wc count_word_fd(int fd) {
  char buf[1024];
  ssize_t bytes;
  long lines = 0, words = 0, chars = 0;
  bool in_word = true;

  while ((bytes = read(fd, buf, sizeof(buf))) > 0) {
    for (ssize_t i = 0; i < bytes; i++) {
      chars++;
      if (buf[i] == '\n') lines++;

      if (isspace((unsigned char)buf[i])) {
        in_word = false;
      } else {
        if (!in_word) {
          words++;
          in_word = true;
        }
      }
    }
  }

  struct wc billy; // willy's twin brother
  billy.lines = lines;
  billy.words = words;
  billy.chars = chars;
  billy.bytes = bytes;
  // TODO: maxlen
  return billy;
}

