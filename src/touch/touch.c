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
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

struct help_entry {
  char *opt;
  char *desc;
};

static struct option long_options[] = {{"no-create", no_argument, 0, 'c'},
                                       {"no-dereference", no_argument, 0, 'h'},
                                       {"reference", required_argument, 0, 'r'},
                                       {"time", required_argument, 0, 2},
                                       {"help", no_argument, 0, 1},
                                       {0, no_argument, 0, 'a'},
                                       {0, no_argument, 0, 'f'},
                                       {0, no_argument, 0, 'm'},
                                       {0, required_argument, 0, 't'},
                                       {0, 0, 0, 0}};

static struct help_entry help_entries[] = {
    {"-a", "change only the access"},
    {"-c, --no-create", "do not create any files"},
    {"-f", "(ignored)"},
    {"-h, --no-dereference",
     "affect each symbolic link instead of any referenced"
     "                        file (useful only on system that change the"
     "                        timestamps of a symlink)"},
    {"-m", "change only the modification time"},
    {"-r, --reference=FILE", "use this file's times instead of current time"},
    {"-t STAMP", "use [[CC]YY]MMDDhhmm[.ss] instead of current time"},
    {"    --time=WORD",
     "change the specified time:"
     "                   WORD is access, atime, or use: equivalent to -a"
     "                   WORD is modify or mtime: equivalent to -m"},
    {"    --help", "display this help and exit"},
    {0, 0}};

void print_help(const char *name) {
  printf("Usage: %s [OPTION]... FILE...\n"
         "Update the access and modification times of each FILE to the current "
         "time.\n\n",
         name);

  puts("A FILE argument that does not exist is created empty, unless -c or -h\n"
       "is supplied.\n");

  puts("A FILE argument string of - is handled specially and causes touch to\n"
       "change the times of the file associated with standard output.\n");

  puts("Mandatory arguments to long options are mandatory for short options "
       "too.");

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

#define CHANGE_ATIME (1 << 0)
#define CHANGE_MTIME (1 << 1)

bool onlyAccess = false;
bool noCreate = false;
bool noDereference = false;
bool onlyModTime = false;

char *reference = NULL;
time_t t;
bool isT = false;

int changeTime(char *name, unsigned int flags, int fd) {
  struct stat fs2;
  if (!noDereference) {
    if (reference != NULL) {
      if (stat(reference, &fs2) == -1) {
        return 1;
      }
    }
  } else {
    if (reference != NULL) {
      if (lstat(reference, &fs2) == -1) {
        return 1;
      }
    }
  }

  struct timespec times[2];
  if (!isT) {
    if (reference == NULL) {
      if (flags & CHANGE_ATIME)
        times[0].tv_nsec = UTIME_NOW;
      else
        times[0].tv_nsec = UTIME_OMIT;

      if (flags & CHANGE_MTIME)
        times[1].tv_nsec = UTIME_NOW;
      else
        times[1].tv_nsec = UTIME_OMIT;
    } else {
      if (flags & CHANGE_ATIME)
        times[0] = fs2.st_atim;
      else
        times[0].tv_nsec = UTIME_OMIT;

      if (flags & CHANGE_MTIME)
        times[1] = fs2.st_mtim;
      else
        times[1].tv_nsec = UTIME_OMIT;
    }
  } else {
    if (flags & CHANGE_ATIME) {
      times[0].tv_sec = t;
      times[0].tv_nsec = 0;
    } else
      times[0].tv_nsec = UTIME_OMIT;

    if (flags & CHANGE_MTIME) {
      times[1].tv_sec = t;
      times[1].tv_nsec = 0;
    } else
      times[1].tv_nsec = UTIME_OMIT;
  }

  if (fd != STDOUT_FILENO) {
    if (utimensat(AT_FDCWD, name, times, 0) == -1) {
      return 1;
    }
  } else {
    if (futimens(fd, times) == -1) {
      return 1;
    }
  }

  return 0;
}

int touchFile(char *name, unsigned int flags) {
  int fd;
  if (strcmp(name, "-") == 0)
    fd = STDOUT_FILENO;
  else if (!noCreate) {
    fd = open(name, O_CREAT | O_WRONLY, 0666);
    if (fd == -1)
      return 1;
    close(fd);
  } else {
    fd = open(name, O_WRONLY);
    if (fd == -1)
      if (errno == ENOENT)
        return 0;
    close(fd);
  }

  if (fd != STDOUT_FILENO)
    fd = 0; // clearing
  if (changeTime(name, flags, fd) != 0) {
    return 1;
  }
  return 0;
}

time_t parse_timestamp(const char *stamp) {
  struct tm tm = {0};
  time_t now = time(NULL);
  struct tm *now_tm = localtime(&now);
  char buf[3] = {0};

  size_t len = strlen(stamp);
  if (len < 8) {
    fprintf(stderr, "touch: invalid date format '%s'\n", stamp);
    exit(EXIT_FAILURE);
  }

  int year_full = 0, year_short = 0;

  const char *p = stamp;
  if (len >= 12 && len <= 15) {
    strncpy(buf, p, 2);
    p += 2;
    year_full = atoi(buf) * 100; // century
  }

  strncpy(buf, p, 2);
  p += 2;
  year_short = atoi(buf);
  if (year_full == 0) {
    // infer century
    int century = (now_tm->tm_year + 1900) / 100 * 100;
    year_full = century;
  }
  tm.tm_year = (year_full + year_short) - 1900;

  strncpy(buf, p, 2);
  p += 2;
  tm.tm_mon = atoi(buf) - 1; // month
  strncpy(buf, p, 2);
  p += 2;
  tm.tm_mday = atoi(buf); // day
  strncpy(buf, p, 2);
  p += 2;
  tm.tm_hour = atoi(buf); // hour
  strncpy(buf, p, 2);
  p += 2;
  tm.tm_min = atoi(buf); // minute

  tm.tm_sec = 0;
  if (*p == '.') {
    p++;
    strncpy(buf, p, 2);
    tm.tm_sec = atoi(buf);
  }

  tm.tm_isdst = -1;
  return mktime(&tm);
}

/*
TODO:
[x] -a
[x] -c, --no-create
 - im not doing --date, no im not even going to try
[x] -h, --no-dereference
[x] -m
[x] -t [[CC]YY]MMDDhhmm[.ss]
[x] --time
*/
int main(int argc, char *argv[]) {
  unsigned int flags = CHANGE_ATIME | CHANGE_MTIME;
  int opt;
  while ((opt = getopt_long(argc, argv, "chr:afmt:", long_options, 0)) != -1) {
    switch (opt) {
    case 'a':
      onlyAccess = true;
      break;
    case 'c':
      noCreate = true;
      break;
    case 'f':
      break;
    case 'h':
      noDereference = true;
      noCreate = true;
      break;
    case 'm':
      onlyModTime = true;
      break;
    case 'r':
      if (optarg != NULL)
        reference = optarg;
      else {
        fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
        return 1;
      }
      break;
    case 't':
      if (optarg == NULL) {
        fprintf(stderr,
                "touch: option requires an argument -- 't'\n"
                "Try '%s --help' for more information.\n",
                argv[0]);
        return 1;
      }
      t = parse_timestamp(optarg);
      isT = true;
      break;
    case 2:
      if (optarg == NULL) {
        fprintf(stderr,
                "touch: option '--time', requires an argument\n"
                "Try '%s --help' for more information.\n",
                argv[0]);
        return 1;
      } else if (strcmp(optarg, "access") == 0 ||
                 strcmp(optarg, "atime") == 0 || strcmp(optarg, "use") == 0) {
        onlyAccess = true;
        break;
      } else if (strcmp(optarg, "modify") == 0 ||
                 strcmp(optarg, "mtime") == 0) {
        onlyModTime = true;
        ;
        break;
      } else {
        fprintf(stderr,
                "touch: invalid argument `%s` for `--time`\n"
                "  - 'atime', 'access', 'use'\n"
                "  - 'mtime', 'modify'\n"
                "Try '%s --help' for more information.\n",
                optarg, argv[0]);
        return 1;
      }
    case 1:
      print_help(argv[0]);
      return 0;
    case '?':
      fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
      return 1;
    default:
      fprintf(stderr,
              "touch: unrecognized option '%s'\n"
              "Try '%s --help' for more information.\n",
              argv[optind], argv[0]);
      return 1;
    }
  }

  if (optind == argc) {
    fprintf(stderr,
            "touch: missing file operand\n"
            "Try '%s --help' for more information.\n",
            argv[0]);
    return 1;
  }

  // flags check
  if (onlyAccess)
    flags &= ~CHANGE_MTIME;
  if (onlyModTime)
    flags &= ~CHANGE_ATIME;

  for (; optind < argc; optind++) {
    if (touchFile(argv[optind], flags) != 0) {
      fprintf(stderr, "touch: cannot touch '%s': %s\n", argv[optind],
              strerror(errno));
      return 1;
    }
  }
  return 0;
}
