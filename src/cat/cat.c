/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This file is part of coreutils from scratch.
 * Copyright (c) 2025 Horstaufmental
 *
 * coreutils from scratch is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * coreutils from scratch is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */
#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
// oh welp js realized this exists, been manually
// defining bool in like the previous 928469 codes (sarcasm)
#include <errno.h>
#include <stdbool.h>

#define PROGRAM_NAME "cat"
#define PROJECT_NAME "coreutils from scratch"
#define AUTHORS "Horstaufmental"
#define VERSION "1.3"

#define BUFSIZE 32768 // GNU Coreutils's buffer size for files

struct help_entry
{
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
    {"version", no_argument, 0, 2},
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
    {"    --help", "display this help and exit"},
    {"    --version", "output version information and exit"},
    {NULL, NULL}};

void print_help(const char *name)
{
  printf("Usage: %s [OPTION]... [FILE]...\n", name);
  printf("Concatenate FILE(s) to standard output.\n\n");

  printf("With no FILE, or when FILE is -, read standard input.\n\n");

  // find longest option string
  int maxlen = 0;
  for (int i = 0; help_entries[i].opt; i++)
  {
    int len = (int)strlen(help_entries[i].opt);
    if (len > maxlen)
      maxlen = len;
  }

  // print each option aligned
  for (int i = 0; help_entries[i].opt; i++)
  {
    printf("  %-*s  %s\n", maxlen, help_entries[i].opt, help_entries[i].desc);
  }
  fputs("\nExamples:\n"
        "  cat f - g  Output f's contents, then standard input, then g's "
        "contents.\n"
        "  cat        Copy standard input to standard output.\n",
        stdout);
}

void print_version()
{
  printf("%s (%s) %s\n", PROGRAM_NAME, PROJECT_NAME, VERSION);
  printf("Copyright (C) 2025 %s\n", AUTHORS);
  puts("License GPLv3+: GNU GPL version 3 or later "
       "<https://gnu.org/licenses/gpl.html>.\n"
       "This is free software: you are free to change and redistribute it.\n"
       "There is NO WARRANTY, to the extent permitted by law.\n");
  printf("Written by %s\n", AUTHORS);
}

static unsigned long long line_number = 1;

void printLineNum(bool isBlank, bool numberAll, bool numberNonBlank)
{
  if (numberNonBlank && isBlank)
    return;
  if (numberAll || (numberNonBlank && !isBlank))
  {
    printf("%6llu\t", line_number++);
  }
}

void printVis(unsigned char c, bool showNonPrinting)
{
  if (!showNonPrinting)
  {
    putchar(c);
    return;
  }

  if (c == '\n' || c == '\t')
  {
    putchar(c);
  }
  else if (c < 32)
  {
    printf("^%c", c + 64); // %@ through ^_
  }
  else if (c == 127)
  {
    fputs("^?", stdout);
  }
  else if (c >= 128)
  {
    fputs("M-", stdout);
    printVis(c - 128, true);
  }
  else
  {
    putchar(c);
  }
}

int read_fd(int fd, bool showNonPrinting, bool showTabs,
            bool squeezeBlank, bool outputNumber, bool showEnds,
            bool numberNoBlank)
{
  char buf[BUFSIZE];
  ssize_t n;
  bool atLineStart = true;
  bool prevBlank = false;

  while ((n = read(fd, buf, sizeof(buf))) > 0)
  {
    for (ssize_t i = 0; i < n; i++)
    {
      unsigned char c = buf[i];

      if (c == '\n')
      {
        bool isBlank = atLineStart;
        if (isBlank && prevBlank && squeezeBlank)
        {
          // skip
          continue;
        }

        if (atLineStart)
        {
          printLineNum(true, outputNumber, numberNoBlank);
        }

        if (showEnds)
          putchar('$');
        putchar('\n');

        prevBlank = isBlank;
        atLineStart = true;
      }
      else
      {
        if (atLineStart)
        {
          printLineNum(false, outputNumber, numberNoBlank);
          atLineStart = false;
        }
        prevBlank = false;

        // handle -T and -v
        if (c == '\t' && showTabs)
        {
          fputs("^I", stdout);
        }
        else
        {
          printVis(c, showNonPrinting);
        }
      }
    }
  }

  if (n == -1)
  {
    fprintf(stderr, "cat: %s\n", strerror(errno));
    return 1;
  }
  return 0;
}

int read_fd_mmap(int fd, size_t file_size, bool showNonPrinting, bool showTabs,
                 bool squeezeBlank, bool outputNumber, bool showEnds,
                 bool numberNoBlank)
{
  void *data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data == MAP_FAILED)
    return read_fd(fd, showNonPrinting, showTabs,
                   squeezeBlank, outputNumber, showEnds,
                   numberNoBlank);

  madvise(data, file_size, MADV_SEQUENTIAL);
  unsigned char *buf = (unsigned char *)data;
  bool atLineStart = true;
  bool prevBlank = false;

  for (size_t i = 0; i < file_size; i++)
  {
    unsigned char c = buf[i];

    if (c == '\n')
    {
      if (prevBlank && squeezeBlank)
      {
        // skip
        continue;
      }

      if (atLineStart)
      {
        printLineNum(true, outputNumber, numberNoBlank);
      }

      if (showEnds)
        putchar('$');
      putchar('\n');

      prevBlank = true;
      atLineStart = true;
    }
    else
    {
      if (atLineStart)
      {
        printLineNum(false, outputNumber, numberNoBlank);
        atLineStart = false;
      }
      prevBlank = false;

      // handle -T and -v
      if (c == '\t' && showTabs)
      {
        fputs("^I", stdout);
      }
      else
      {
        printVis(c, showNonPrinting);
      }
    }
  }
  munmap(data, file_size);
  return 0;
}

int read_wrapper(int fd, bool showNonPrinting, bool showTabs,
                 bool squeezeBlank, bool outputNumber, bool showEnds,
                 bool numberNoBlank)
{
  struct stat st;
  if (fstat(fd, &st) == -1)
  {
    fprintf(stderr, "cat: %s\n", strerror(errno));
    return read_fd(fd, showNonPrinting, showTabs,
                   squeezeBlank, outputNumber, showEnds,
                   numberNoBlank);
  }

  if (S_ISREG(st.st_mode) && st.st_size > 65536)
  {
    return read_fd_mmap(fd, st.st_size, showNonPrinting, showTabs,
                        squeezeBlank, outputNumber, showEnds,
                        numberNoBlank);
  }
  else
  {
    return read_fd(fd, showNonPrinting, showTabs,
                   squeezeBlank, outputNumber, showEnds,
                   numberNoBlank);
  }
}

int main(int argc, char *argv[])
{
  int opt;

  bool showNonPrinting = false; // -v
  bool showTabs = false;        // -T
  bool squeezeBlank = false;    // -s
  bool outputNumber = false;    // -n
  bool showEnds = false;        // -E
  bool numberNoBlank = false;   // -b

  while ((opt = getopt_long(argc, argv, "AbeEnstTuv", long_options, 0)) != -1)
  {
    switch (opt)
    {
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

  if (argc - optind == 0)
  {
    char buf[BUFSIZ];
    while (true)
    {
      char *r;
      r = fgets(buf, sizeof(buf), stdin);
      if (r == NULL && feof(stdin))
        break;
      else if (r == NULL && ferror(stdin)) {
        fprintf(stderr, "cat: %s\n", strerror(errno));
        return 1;
      }
      if (write(STDOUT_FILENO, buf, strlen(buf)) == -1) {
        fprintf(stderr, "cat: %s\n", strerror(errno));
        return 1;
      }
    }
  }
  else
  {
    for (; optind < argc; optind++)
    {
      if (strcmp(argv[optind], "-") == 0)
      {
        char buf[BUFSIZ];
        while (true)
        {
          char *r;
          r = fgets(buf, sizeof(buf), stdin);
          if (r == NULL && feof(stdin))
            break;
          else if (r == NULL && ferror(stdin)) {
            fprintf(stderr, "cat: %s\n", strerror(errno));
            return 1;
          }
          if (write(STDOUT_FILENO, buf, strlen(buf)) == -1) {
            fprintf(stderr, "cat: %s\n", strerror(errno));
            return 1;
          }
        }
        continue;
      }
      int fd = open(argv[optind], O_RDONLY);
      if (fd == -1)
      {
        fprintf(stderr, "cat: cannot open '%s': %s\n", argv[optind],
                strerror(errno));
        return 1;
      }

      if (read_wrapper(fd, showNonPrinting, showTabs,
                       squeezeBlank, outputNumber, showEnds,
                       numberNoBlank) != 0)
      {
        fprintf(stderr, "cat: '%s': %s\n", argv[optind],
                strerror(errno));
        close(fd);
        return 1;
      }

      close(fd);
    }
  }
  return 0;
}
