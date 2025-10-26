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
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

// "If no option is specified, -P is assumed."
// resolve all symlinks
// otherwise use PWD from environment, even if it contains symlinks
bool physical = true;

struct help_entry {
  const char *opt;
  const char *desc;
};

static struct help_entry help_entries[] = {
  {"-L, --logical", "use PWD from environment, even if it contains symlinks"},
  {"-P, --physical", "resolve all symlinks"},
  {"    --help", "display this help and exit"},
  {NULL, NULL}
};

void print_help(const char *name) {
  printf("Usage: %s [OPTION]...\n", name);
  printf("Print the full filename of the current working directory.\n\n");

  // find longest option string
  int maxlen = 0;
  for (int i = 0; help_entries[i].opt; i++) {
    int len = (int)strlen(help_entries[i].opt);
    if (len > maxlen) maxlen = len;
  }

  // print each option aligned
  for (int i = 0; help_entries[i].opt; i++) {
    printf("  %-*s  %s\n", maxlen, help_entries[i].opt, help_entries[i].desc);
  }
}

int main(int argc __attribute__((unused)), char *argv[]) {
  char cwd[PATH_MAX];
  char *pwd_env = getenv("PWD");
  strncat(pwd_env, "\n", sizeof(pwd_env) - strlen(pwd_env) - 1);

  if (argv[1] != NULL) {
    if (strcasecmp(argv[1], "--logical") == 0 || strcasecmp(argv[1], "-L") == 0) {
      physical = false;
    } else if (strcasecmp(argv[1], "--physical") == 0 || strcasecmp(argv[1], "-L") == 0) {
      physical = true;
    } else if (strcasecmp(argv[1], "--help") == 0) {
      print_help(argv[0]);
      return 0;
    } else {
      fprintf(stderr, "pwd: unrecognized option '%s'\nTry '%s --help' for more information\n", argv[1], argv[0]);
      return 1;
    }
  }

  if (!physical) {
    puts(pwd_env);
    return 0;
  }
  
  if (!getcwd(cwd, sizeof(cwd))) {
    fprintf(stderr, "pwd: cannot get current working directory: %s\n", strerror(errno));
  }
  
  puts(cwd);
  return 0;
}
