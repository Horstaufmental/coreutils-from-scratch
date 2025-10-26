/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This file is part of coreutils from scratch.
 * Copyright (c) 2025 Horstaufmental
 *
 * coreutils from scratch is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * coreutils from scratch is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */
#include <fcntl.h>
#include <getopt.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
// oh welp js realized this exists, been manually
// defining bool in like the previous 928469 codes (sarcasm)
#include <errno.h>
#include <stdbool.h>

#define BUFSIZE 32768 // GNU Coreutils's buffer size for files

struct help_entry {
  const char *opt;
  const char *desc;
};

static struct option long_options[] = {
    {"show-all", no_argument, 0, 'A'},
    {"number-nonblank", no_argument, 0, 'b'},
    {"show-ends", no_argument, 0, 'E'},
    {"number", no_argument, 0, 'n'},
    {"squeeze-blank", no_argument, 0, 's'},
    {"show-tabs", no_argument, 0, 'T'},
    {"show-nonprinting", no_argument, 0, 'v'},
    {"help", no_argument, 0, 1},
    {0, no_argument, 0, 'e'}, // -vET
    {0, no_argument, 0, 't'}, // -vT
    {0, no_argument, 0, 'u'}, // ignored
};

static struct help_entry help_entries[] = {
    {"-A, --show-all", "equivalent to -vET"},
    {"-b, --number-nonblank", "number nonempty output lines, overrides -n"},
    {"-e", "equivalent to -vE"},
    {"-E, --show-ends", "display $ at end of each line"},
    {"-n, --number", "number all output lines"},
    {"-s, -squeeze-blank", "suppress repeated empty output lines"},
    {"-t", "equivalent to -vT"},
    {"-T, --show-tabs", "display TAB characterr as ^I"},
    {"-u", "(ignored) historically means 'unbuffered output', now obsolete"},
    {"-v, --show-nonprinting", "use ^ and M- notation, except for LFD and TAB"},
    {"    --help", "display this help and exit"}};

void print_help(const char *name) {
  printf("Usage: %s [OPTION]... [FILE]...\n", name);
  printf("Concatenate FILE(s) to standard output.\n\n");

  printf("With no FILE, or when FILE is -, read standard input.\n\n");

  // find longest option string
  int maxlen = 0;
  for (int i = 0; help_entries[i].opt; i++) {
    int len = (int)strlen(help_entries[i].opt);
    if (len > maxlen)
      maxlen = len;
  }

  // print each option aligned
  for (int i = 0; help_entries[i].opt; i++) {
    printf("  %-*s  %s\n", maxlen, help_entries[i].opt, help_entries[i].desc);
  }
  fputs("\nExamples:\n"
        "  cat f - g  Output f's contents, then standard input, then g's "
        "contents.\n"
        "  cat        Copy standard input to standard output.\n",
        stdout);
}

static unsigned long long line_number = 1;

void printLineNum(bool isBlank, bool numberAll, bool numberNonBlank) {
  if (numberNonBlank && isBlank)
    return;
  if (numberAll || (numberNonBlank && !isBlank)) {
    printf("%6llu\t", line_number++);
  }
}

void printVis(unsigned char c, bool showNonPrinting) {
  if (!showNonPrinting) {
    putchar(c);
    return;
  }

  if (c == '\n' || c == '\t') {
    putchar(c);
  } else if (c < 32) {
    printf("^%c", c + 64); // %@ through ^_
  } else if (c == 127) {
    fputs("^?", stdout);
  } else if (c >= 128) {
    fputs("M-", stdout);
    printVis(c - 128, true);
  } else {
    putchar(c);
  }
}

int main(int argc, char *argv[]) {
  int opt;

  bool showNonPrinting = false; // -v
  bool showTabs = false;        // -T
  bool squeezeBlank = false;    // -s
  bool outputNumber = false;    // -n
  bool showEnds = false;        // -E
  bool numberNoBlank = false;   // -b

  while ((opt = getopt_long(argc, argv, "AbeEnstTuv", long_options, 0)) != -1) {
    switch (opt) {
    case 'A':
      showNonPrinting = showEnds = showTabs = true;
      break;
    case 'b':
      numberNoBlank = true;
      break;
    case 'e':
      showNonPrinting = showEnds = true;
      break;
    case 'E':
      showEnds = true;
      break;
    case 'n':
      outputNumber = true;
      break;
    case 's':
      squeezeBlank = true;
      break;
    case 't':
      showNonPrinting = showTabs = true;
      break;
    case 'T':
      showTabs = true;
      break;
    case 'u':
      // :alien:
      break;
    case 'v':
      showNonPrinting = true;
      break;
    case 1:
      print_help(argv[0]);
      return 0;
    case '?':
      fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
      return 1;
    }
  }

  if (argc - optind == 0) {
    char buf[4096];
    while (true) {
      fgets(buf, sizeof(buf), stdin);
      if (feof(stdin))
        break;
      write(STDOUT_FILENO, buf, strlen(buf));
    }
  } else {
    for (; optind < argc; optind++) {
      if (strcmp(argv[optind], "-") == 0) {
        char buf[BUFSIZ];
        while (true) {
          fgets(buf, sizeof(buf), stdin);
          if (feof(stdin))
            break;
          write(STDOUT_FILENO, buf, strlen(buf));
        }
        continue;
      }
      int fd = open(argv[optind], O_RDONLY);
      if (fd == -1) {
        fprintf(stderr, "cat: cannot open '%s': %s\n", argv[optind],
                strerror(errno));
        return 1;
      }

      char buf[BUFSIZE];
      ssize_t n;
      bool atLineStart = true;
      bool prevBlank = false;

      while ((n = read(fd, buf, sizeof(buf))) > 0) {
        for (ssize_t i = 0; i < n; i++) {
          unsigned char c = buf[i];

          if (c == '\n') {
            if (prevBlank && squeezeBlank) {
              // skip
              continue;
            }

            if (atLineStart) {
              printLineNum(true, outputNumber, numberNoBlank);
            }

            if (showEnds)
              putchar('$');
            putchar('\n');

            prevBlank = true;
            atLineStart = true;
          } else {
            if (atLineStart) {
              printLineNum(false, outputNumber, numberNoBlank);
              atLineStart = false;
            }
            prevBlank = false;

            // handle -T and -v
            if (c == '\t' && showTabs) {
              fputs("^I", stdout);
            } else {
              printVis(c, showNonPrinting);
            }
          }
        }
      }
      close(fd);
    }
  }
  return 0;
}
