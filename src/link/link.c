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
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

#define PROGRAM_NAME "link"
#define PROJECT_NAME "coreutils from scratch"
#define AUTHORS "Horstaufmental"
#define VERSION "1.0"

struct help_entry {
  const char *opt;
  const char *desc;
};

static struct option long_options[] = {{"help", no_argument, 0, 1},
                                       {"version", no_argument, 0, 2},
                                       {NULL, 0, 0, 0}};

static struct help_entry help_entries[] = {
    {"     --help", "display this help and exit"},
    {"     --version", "output version information and exit"},
    {NULL, NULL}};

void print_help(const char *name) {
  printf("Usage: %s FILE1 FILE2\n"
         "  or:  %s OPTION\n", name, name);
  puts("Call the link function to create a link named FILE2 to an existing FILE1\n");
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
  int opt;
  while ((opt = getopt_long(argc, argv, "", long_options, 0)) != -1) {
    switch (opt) {
    case 1:
      print_help(argv[0]);
      return 0;
    case 2:
      print_version();
      return 0;
    case '?':
      fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
      return 1;
    }
  }
  char *file1;
  char *file2;
  // check for FILE1
  if (optind < argc && argv[optind] != NULL) {
    file1 = argv[optind];
    optind++;
  } else {
    fprintf(stderr, "link: missing operand\n"
                    "Try '%s --help' for more information.\n", argv[0]);
    return 1;
  }

  // check for FILE2
  if (optind < argc && argv[optind] != NULL) {
    file2 = argv[optind];
    optind++;
  } else {
    fprintf(stderr, "link: missing operand after '%s'\n"
                    "Try '%s --help' for more information.\n", file1, argv[0]);
    return 1;
  }

  if (link(file1, file2) != 0) {
    fprintf(stderr, "link: cannot create link '%s' to '%s': %s\n", file2, file1, strerror(errno));
    return 1;
  }
  return 0;
}
