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
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

struct help_entry {
  const char *opt;
  const char *desc;
};

static struct help_entry help_entries[] = {
    {"    --help", "display this help and exit"}, {NULL, NULL}};

void print_help(const char *name) {
  printf("Usage: %s [STRING]...\n", name);
  printf("  or:  %s OPTION\n", name);
  printf("Repeatedly output a line with all specified STRING(s), or 'y'.\n\n");

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
}

int main(int argc __attribute__((unused)), char *argv[]) {
  bool ovr = false;

  if (argv[1] == NULL) {
    ovr = true;
  } else {
    if (strcasecmp(argv[1], "--help") == 0) {
      print_help(argv[0]);
      return 0;
    }
  }

  static char buffer[4096];
  if (!ovr)
    strncat(buffer, argv[1], sizeof(buffer) - strlen(buffer) - strlen(argv[1]));
  else
    strncat(buffer, "y", sizeof(buffer) - strlen(buffer - 1));

  strncat(buffer, "\n", sizeof(buffer) - strlen(buffer) - 1);

  while (1)
    write(1, buffer, strlen(buffer));
}
