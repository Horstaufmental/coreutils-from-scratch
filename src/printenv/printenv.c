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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>

#include "meta.h"

static struct option long_options[] = {
  {"null", no_argument, 0, '0'},
  {"help", no_argument, 0, 1},
  {"version", no_argument, 0, 2},
  {NULL, 0, 0, 0}
};

static struct help_entry help_entries[] = {
  {"-0, --null", "end each output line with NUL, not newline"},
  {"    --help", "display this help and exit"},
  {"    --version", "output version information and exit"},
  {NULL, NULL}
};

int main(int argc, char *argv[], char *envp[]) {
  bool newline = true;
  
  int opt;
  while ((opt = getopt_long(argc, argv, "0", long_options, 0)) != -1) {
    switch (opt) {
      case '0':
        newline = false;
        break;
      case 1:
        {
          char buf[256];
          snprintf(buf, 256, "Usage: %s [OPTION] [VARIABLE]...", argv[0]);
          print_help(buf, "Print the values of the specified environment VARIABLE(s).\n"
                      "If no VARIABLE is specified, print name and value pairs for them all.\n",
                      help_entries,
                      "Your shell may have its own version of printenv, which usually supersedes\n"
                      "the version described here.  Please refer to your shell's documentation\n"
                      "for details about the options it supports.");
          return 0;
        }
      case 2:
        print_version(PROGRAM_NAME, PROJECT_NAME, VERSION, AUTHORS);
        return 0;
    }
  }

  if (optind == argc) {
    int i = 0;
    while (envp[i] != NULL) {
      fputs(envp[i], stdout);
      if (newline) fputs("\n", stdout);
      i++;
    }
  } else {
    for (; optind < argc; optind++) {
      char *env = getenv(argv[optind]);
      if (env == NULL) return 1;
      
      fputs(env, stdout);
      if (newline) fputs("\n", stdout);
    }
  }
  return 0;
}
