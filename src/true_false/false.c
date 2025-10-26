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
#include <string.h>
#include <stdio.h>

struct help_entry {
  const char *opt;
  const char *desc;
};

static struct help_entry help_entries[] = {
  {"    --help", "display this help and exit"},
  {NULL, NULL}
};

void print_help(const char *name) {
  printf("Usage: %s [ignored command line arguments]\n", name);
  printf("  or:  %s OPTION\n", name);
  printf("Exit with a status code indicating failure.\n\n");

  int maxlen = 0;
  for (int i = 0; help_entries[i].opt; i++) {
    int len = (int)strlen(help_entries[i].opt);
    if (len > maxlen) maxlen = len;
  }

  for (int i = 0; help_entries[i].opt; i++) {
    printf("  %-*s  %s\n", maxlen, help_entries[i].opt, help_entries[i].desc);
  }
}

int main(int argc __attribute__((unused)), char *argv[]) {
  if (argv[1] != NULL)
    if (strcasecmp(argv[1], "--help") == 0)
      print_help(argv[0]);  
  return 1;
}
