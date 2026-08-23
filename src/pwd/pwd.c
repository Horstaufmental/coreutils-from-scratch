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
#include <fcntl.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

#include "meta.h"

// "If no option is specified, -P is assumed."
// resolve all symlinks
// otherwise use PWD from environment, even if it contains symlinks
bool physical = true;

static struct help_entry help_entries[] = {
  {"-L, --logical", "use PWD from environment, even if it contains symlinks"},
  {"-P, --physical", "resolve all symlinks"},
  {"    --help", "display this help and exit"},
  {"    --version", "output version information and exit"},
  {NULL, NULL}
};

int main(int argc __attribute__((unused)), char *argv[]) {
  char cwd[PATH_MAX];
  char *pwd_env = getenv("PWD");
  strncat(pwd_env, "\n", sizeof(pwd_env) - strlen(pwd_env) - 1);

  // TODO: use getopt_long instead for codebase consistency
  if (argv[1] != NULL) {
    if (strcasecmp(argv[1], "--logical") == 0 || strcasecmp(argv[1], "-L") == 0) {
      physical = false;
    } else if (strcasecmp(argv[1], "--physical") == 0 || strcasecmp(argv[1], "-L") == 0) {
      physical = true;
    } else if (strcasecmp(argv[1], "--help") == 0) {
      char buf[256];
      snprintf(buf, 256, "Usage: %s [OPTION]...", argv[0]);
      print_help(buf, "Print the full filename of the current working directory.\n", help_entries,
                 "If no option is specified, -P is assumed.\n\n"
                 "Your shell may have its own version of pwd, which usually supersedes\n"
                 "the version described here.  Please refer to your shell's documentation\n"
                 "for details about the options it supports.");
      return 0;
    } else if (strcasecmp(argv[1], "--version") == 0) {
      print_version(PROGRAM_NAME, PROJECT_NAME, VERSION, AUTHORS);
      return 0;
    } else {
      fprintf(stderr, "%s: unrecognized option '%s'\nTry '%s --help' for more information\n", argv[0], argv[1], argv[0]);
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
