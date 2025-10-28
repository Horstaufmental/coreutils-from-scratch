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
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>

#define PROGRAM_NAME "printenv"
#define PROJECT_NAME "coreutils from scratch"
#define AUTHORS "Horstaufmental"
#define VERSION "1.0"

struct help_entry {
  const char *opt;
  const char *desc;
};

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


void print_help(const char *name) {
  printf("Usage: %s [OPTION]... [VARIABLE]...\n", name);
  puts("Print the values of the specified environment VARIABLE(s).\n"
       "If no VARIABLE is specified, print name and value pairs for them all.\n");

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
  puts("\nYour shell may have its own version of printenv, which usually supersedes\n"
        "the version described here.  Please refer to your shell's documentation\n"
        "for details about the options it supports.");
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

int main(int argc, char *argv[], char *envp[]) {
  bool newline = true;
  
  int opt;
  while ((opt = getopt_long(argc, argv, "0", long_options, 0)) != -1) {
    switch (opt) {
      case '0':
        newline = false;
        break;
      case 1:
        print_help(argv[0]);
        return 0;
      case 2:
        print_version();
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
