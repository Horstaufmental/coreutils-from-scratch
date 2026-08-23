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
#include <getopt.h>
#include <stdlib.h>
#include <stdbool.h>

#include "meta.h"

bool showAll = false;
int ignoreProc = 0;

static struct option long_options[] = {
  {"all", no_argument, 0, 2},
  {"ignore", required_argument, 0, 3},
  {"help", no_argument, 0, 1},
  {"version", no_argument, 0, 9},
  {0, 0, 0, 0}
};

static struct help_entry help_entries[] = {
  {"    --all",
          "print the number of installed processors\n"
   "       disregarding any OpenMP environment variables, or CPU quotas."},
  {"    --ignore=N",
          "if possible, exclude N processing units\n"
   "       The result is guaranteed to be at least 1."},
  {"    --help", "display this help and exit"},
  {"    --version", "output version information and exit"},
  {NULL, NULL}
};

int main(int argc, char *argv[]) {
  int np;
  int opt;

  while ((opt = getopt_long(argc, argv, "", long_options, 0)) != -1) {
    switch(opt) {
      case 1:
        {
          char buf[256];
          snprintf(buf, 256, "Usage: %s [OPTION]...", argv[0]);
          print_help(buf, "Print the number of processing units available to the current process,\n"
                      "which may be less than the number of online processors.\n"
                      "If the 'OMP_NUM_THREADS' or 'OMP_THREAD_LIMIT' environment variables are set,\n"
                      "then they will determine the minimum and maximum returned value respectively\n",
                    help_entries, NULL);
          return 0;
        }
      case 2:
        showAll = true;
        break;
      case 3:
        ignoreProc = atoi(optarg);
        break;
      case 9:
        print_version(PROGRAM_NAME, PROJECT_NAME, VERSION, AUTHORS);
        return 0;
      case '?':
        printf("Try '%s --help' for more information.\n", argv[0]);
        return 1;
    }
  }
  
  np = (showAll) ? sysconf(_SC_NPROCESSORS_CONF) : sysconf(_SC_NPROCESSORS_ONLN);

  // TODO: put env variables 'OMP_NUM_THREADS' and 'OMP_THREAD_LIMIT' into account
  // on determining the min and max returned value
  if (ignoreProc > 0) {
    int buffer = (ignoreProc < 1) ? 1 : ((ignoreProc > np - 1) ? np - 1 : ignoreProc);
    np -= buffer;
  }
  
  printf("%d\n", np);
  return 0;
}
