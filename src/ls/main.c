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
#define _XOPEN_SOURCE 700

#include <dirent.h>
#include <getopt.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statfs.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

#include "args.h"
#include "longformat.c"
#include "print_help.c"
#include "print_version.c"

bool includeALL = false;
bool includeALLshort = false;
bool humanReadable = false;
bool longFormat = false;

void getRealPath(char *inputPath, char *realPath) {
  if (realpath(inputPath, realPath) == NULL) {
    perror("realpath");
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char *argv[]) {
  DIR *d;
  struct dirent *dir;
  int opt;

  while ((opt = getopt_long(argc, argv, "aAhl", long_options, 0)) != -1) {
    switch (opt) {
    case 'a':
      includeALL = true;
      break;
    case 'A':
      includeALLshort = true;
      break;
    case 'h':
      humanReadable = true;
      break;
    case 'l':
      longFormat = true;
      break;
    case 1:
      print_help(argv[0]);
      return 0;
    case 2:
      print_version();
      return 0;
    default:
      print_help(argv[0]);
      return 1;
    }
  }

  char *realPath = malloc(PATH_MAX);

  if (argc - optind == 0) {
    getRealPath(".", realPath);
    d = opendir(realPath);
  } else if (argc - optind == 1) {
    getRealPath(argv[optind], realPath);
    d = opendir(realPath);
  } else {
    fprintf(stderr, "Error: please provide only 1 input.\n");
  }

  if (!d) {
    perror("opendir");
    return 1;
  }

  while ((dir = readdir(d)) != NULL) {
    // note to self: continue means to skip over the current item
    if ((strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) &&
        includeALL == false) {
      // printf("\nskipped over: '%s', bool: %d\n", dir->d_name, includeALL);
      continue;
    }
    if (dir->d_name[0] == '.' &&
        (includeALL == false && includeALLshort == false)) {
      // printf("\nskipped over: '%s', -a bool: %d, -A bool: %d\n", dir->d_name,
      // includeALL, includeALLshort);
      continue;
    }
    if (longFormat) {
      // deadass thought the issue was entirely on longformat.c, no nigga it was
      // fucking here lstat needs full path and dir->d_name is just the basename
      // but we're giving it the d_name
      char fullPath[PATH_MAX];
      snprintf(fullPath, sizeof(fullPath), "%s/%s", realPath ? realPath : ".",
               dir->d_name);
      printlongOutput(fullPath, dir->d_name);
      continue;
    }
    printf("%s  ", dir->d_name);
  }
  if (!longFormat)
    printf("\n");
  free(realPath);
  closedir(d);
  return (0);
}
