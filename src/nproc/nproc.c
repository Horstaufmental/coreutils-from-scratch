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
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdbool.h>

#define PROGRAM_NAME "nproc"
#define PROJECT_NAME "coreutils from scratch"
#define AUTHORS "Horstaufmental"
#define VERSION "1.1 (Okami Era)"

bool showAll = false;
int ignoreProc = 0;

struct help_entry {
  const char *opt;
  const char *desc;
};

static struct option long_options[] = {
  {"all", no_argument, 0, 2},
  {"ignore", required_argument, 0, 3},
  {"help", no_argument, 0, 1},
  {"version", no_argument, 0, 9},
  {0, 0, 0, 0}
};

static struct help_entry help_entries[] = {
  {"    --all", "print the number of installed processors"},
  {"    --ignore=N", "if possible, exclude N processing units"},
  {"    --help", "display this help and exit"},
  {"    --version", "output version information and exit"},
  {NULL, NULL}
};

void print_help(const char *name) {
  printf("Usage: %s [OPTION]...\n", name);
  printf("Print the number of processing units available to the current process,\n");
  printf("which may be less than the number of online processors\n\n");
  int maxlen = 0;
  for (int i = 0; help_entries[i].opt; i++) {
    int len = (int)strlen(help_entries[i].opt);
    if (len > maxlen) maxlen = len;
  }

  for (int i = 0; help_entries[i].opt; i++) {
    printf("  %-*s  %s\n", maxlen, help_entries[i].opt, help_entries[i].desc);
  }
}

void print_version() {
  printf("%s (%s) %s\n", PROGRAM_NAME, PROJECT_NAME, VERSION);
  printf("Copyright (C) 2025 %s\n", AUTHORS);
  puts("License GPLv3+: GNU GPL version 3 or later "
  "<https://gnu.org/licenses/gpl.html>.\n"
  "This is free software: you are free to change and redistribute it.\n"
  "There is NO WARRANTY, to the extent permitted by law.\n");
  printf("Written by %s\n", AUTHORS);
}

int main(int argc, char *argv[]) {
  int np;
  int opt;

  while ((opt = getopt_long(argc, argv, "", long_options, 0)) != -1) {
    switch(opt) {
      case 1:
        print_help(argv[0]);
        return 0;
      case 2:
        showAll = true;
        break;
      case 3:
        ignoreProc = atoi(optarg);
        break;
      case 9:
        print_version();
        return 0;
      case '?':
        printf("Try '%s --help' for more information.\n", argv[0]);
        return 1;
    }
  }
  
  np = (showAll) ? sysconf(_SC_NPROCESSORS_CONF) : sysconf(_SC_NPROCESSORS_ONLN);

  if (ignoreProc > 0) {
    int buffer = (ignoreProc < 1) ? 1 : ((ignoreProc > np - 1) ? np - 1 : ignoreProc);
    np -= buffer;
  }
  
  printf("%d", np);
  return 0;
}

