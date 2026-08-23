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
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statfs.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

#include "meta.h"
#include "args.h"
#include "longformat.h"

struct option long_options[] = {
    {"all", no_argument, 0, 'a'},
    {"almost-all", no_argument, 0, 'A'},
    {"help", no_argument, 0, 1},
    {"version", no_argument, 0, 2},
    {0, no_argument, 0, 'l'},
    {0, 0, 0, 0}
};

struct help_entry help_entries[] = {
    {"-a, --all", "show hidden and 'dot' files. Use this twice to also\n"
                  "              show the '.' and '..' directories"},
    {"-A, --almost-all", "equivalent to --all; included for compatibility with `ls -A`"},
    {"-h, --human-readable", "with -l, print sizes in human readable format (e.g., 1K 234M 2G)"},
    {"-l", "display extended file metadata as a table"},
    {"    --help", "display this help and exit"},
    {"    --version", "output version information and exit"},
    {NULL, NULL}
};

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
      {
        char buf[256];
        snprintf(buf, 256, "Usage: %s [OPTION]... [FILE]...", argv[0]);
        print_help(buf, "List information about the FILEs (the current directory by default).\n"
                    "Sort entries alphabetically if none of -cftuvSUX nor --sort is specified.\n\n"
                    "Mandatory arguments to long options are mandatory for short options too.",
                    help_entries,
                    NULL);
        return 0;
      }
    case 2:
      print_version(PROGRAM_NAME, PROJECT_NAME, VERSION, AUTHORS);
      return 0;
    default:
      
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
