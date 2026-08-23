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
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

#include "meta.h"

static struct help_entry help_entries[] = {
  {"    --help", "display this help and exit"},
  {"    --version", "output version information and exit"},
  {NULL, NULL}
};

int parse_time(const char *str, long *out) {
    char *endptr;
    errno = 0;

    // Parse number (integer or float allowed)
    double val = strtod(str, &endptr);

    if (endptr == str) return 0;   // no number found
    if (errno == ERANGE) return 0; // overflow/underflow

    // Default multiplier = seconds
    long long multiplier = 1;

    // Check suffix
    switch (*endptr) {
        case 's': multiplier = 1; break;                 // seconds
        case 'm': multiplier = 60; break;                // minutes
        case 'h': multiplier = 60 * 60; break;           // hours
        case 'd': multiplier = 60 * 60 * 24; break;      // days
        case '\0': multiplier = 1; break;                // no suffix = seconds
        default: return 0; // invalid suffix
    }

    endptr++; // move past suffix

    if (*endptr != '\0') return 0; // extra junk at end

    // Apply multiplier with rounding
    double result = val * multiplier;

    if (result > LLONG_MAX || result < LLONG_MIN) return 0; // overflow check

    *out = (long long)(result); // round to nearest integer
    return 1;
}

int main(int argc __attribute__((unused)), char *argv[]) {
  struct timespec timeSleep;

  // TODO: use getopt_long for codebase consistency  
  if (argv[1] != NULL) {
    if (strcasecmp(argv[1], "--help") == 0) {
      char buf[512];
      snprintf(buf, 512, "Usage: %s NUMBER[SUFFIX]...\n"
                         "  or:  %s OPTION", argv[0], argv[0]);
      print_help(buf, "Pause for NUMBER seconds, where NUMBER is an integer or floating-point.\n"
                  "SUFFIX may be 's','m','h', or 'd', for seconds, minutes, hours, days.\n"
                  "With multiple arguments, pause for the sum of their values.\n",
                  help_entries, NULL);
      return 0;
    } else if (strcasecmp(argv[1], "--version") == 0) {
      print_version(PROGRAM_NAME, PROJECT_NAME, VERSION, AUTHORS);
      return 0;
    } else {
      for (int i = 1; argv[i] != NULL; i++) {
        if (parse_time(argv[i], &timeSleep.tv_sec) == 0) {
          fprintf(stderr, "%s: invalid time interval '%s'\nTry '%s --help' for more information.\n", argv[0], argv[1], argv[0]);
          return 1;
        }
        timeSleep.tv_nsec = 0;
        nanosleep(&timeSleep, NULL);
      }
    }
  } else {
    fprintf(stderr, "sleep: missing operand\nTry '%s --help' for more information.\n", argv[0]);
    return 1;
  }
  return 0;
}
