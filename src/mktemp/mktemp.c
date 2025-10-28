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
#include <asm-generic/errno-base.h>
#include <complex.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define PROGRAM_NAME "mktemp"
#define PROJECT_NAME "coreutils from scratch"
#define AUTHORS "Horstaufmental"
#define VERSION "1.0"

bool dir = false;
bool dry_run = false;
bool quiet = false;

struct help_entry {
  const char *opt;
  const char *desc;
};

static struct option long_options[] = {{"directory", no_argument, 0, 'd'},
                                       {"dry-run", no_argument, 0, 'u'},
                                       {"quiet", no_argument, 0, 'q'},
                                       {"suffix", required_argument, 0, 2},
                                       {"tmpdir", optional_argument, 0, 3},
                                       {"help", no_argument, 0, 1},
                                       {"version", no_argument, 0, 9},
                                       {NULL, required_argument, 0, 'p'},
                                       {NULL, 0, 0, 0}};

static struct help_entry help_entries[] = {
    {"-d, --directory", "create a directory, not a file"},
    {"-u, --dry-run", "do not create anything; merely print a name (unsafe)"},
    {"-q, --quiet", "suppress diagnostics about file/dir-creation failure"},
    {"    --suffix=SUFF",
     "append SUFF to template; SUFF must not contain a slash.\n"
     "                     This option is implied if TEMPLATE does not end in "
     "X"},
    {"-p, --tmpdir[=DIR]",
     "interpret TEMPLATE relative to DIR; if DIR is not\n"
     "                      specified, use $TMPDIR if set, else /tmp. With\n"
     "                      this option, TEMPLATE may contain slashes, but\n"
     "                      mktemp creates only the final component"},
    {"    --help", "display this help and exit"},
    {"    --version", "output version information and exit"},
    {NULL, NULL}};

void print_help(const char *name) {
  printf("Usage: %s [OPTION]... [TEMPLATE]\n", name);
  puts("Create a temporary file or directory, safely, and print its name.\n"
       "TEMPLATE must contain at least consecutive 'X's in last component.\n"
       "If TEMPLATE is not specified, use tmp.XXXXXXXXXX, and --tmpdir is "
       "implied.\n"
       "Files are created u+rw, and directories u+rwx, minus umask "
       "restrictions.\n");

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

/*
by default, mkstemp/mkdtemp only modifies the last 6 Xs
and leave the remaining Xs before that untouched

so we'll need to take care of that ourselves if we wanted
template longer than just 6 Xs
*/

size_t count_consecutive_x(char *tmpl) {
  size_t len = strlen(tmpl);
  size_t x_count = 0;

  for (ssize_t i = len - 1; i >= 0; i--) {
    if (tmpl[i] == 'X') {
      x_count++;
    }
  }
  return x_count;
}

static void randomize_prefix_x(char *tmpl) {
  static const char letters[] =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

  size_t len = strlen(tmpl);
  if (len < 6)
    return;

  ssize_t end = len - 1;
  while (end >= 0 && tmpl[end] != 'X')
    end--;
  if (end < 0)
    return;

  ssize_t start = end;
  while (start >= 0 && tmpl[start] == 'X')
    start--;

  ssize_t x_run_len = (size_t)(end - start);
  if (x_run_len <= 6)
    return;

  ssize_t prefix_len = x_run_len - 6;
  for (ssize_t i = 0; i < prefix_len; i++) {
    tmpl[start + 1 + i] = letters[rand() % (sizeof(letters) - 1)];
  }
}

int main(int argc, char *argv[]) {
  srand(time(NULL));

  int opt;
  char *tmpdir = NULL;
  char template[1024] = "";
  char *suffix = NULL;
  while ((opt = getopt_long(argc, argv, "duqp:", long_options, NULL)) != -1) {
    switch (opt) {
    case 'd':
      dir = true;
      break;
    case 'u':
      dry_run = true;
      break;
    case 'q':
      quiet = true;
      break;
    case 3:
    check_tmpdir:
      if (optarg == NULL) {
        tmpdir = getenv("TMPDIR");
        if (tmpdir == NULL) {
          struct stat stats;
          if (stat("/tmp", &stats) == 0 && S_ISDIR(stats.st_mode)) {
            tmpdir = "/tmp";
          } else {
            fprintf(stderr, "mktemp: cannot find tmpdir\n");
            return 1;
          }
        }
      } else {
        tmpdir = optarg;
        if (tmpdir[strlen(tmpdir)] == '/') {
          tmpdir[strlen(tmpdir)] = '\0';
        }
      }
      break;
    case 'p':
      goto check_tmpdir;
    case 2:
      for (size_t i = 0; i < strlen(optarg) - 1;
           i++) { // checks until the second last character
        if (optarg[i] == '/') {
          fprintf(stderr,
                  "mktemp: invalid suffix '%s', contains directory seperator\n",
                  optarg);
          return 1;
        }
      }

      if (optarg[strlen(optarg)] == '/') {
        optarg[strlen(optarg)] = '\0';
      }
      suffix = optarg;
      break;
    case 1:
      print_help(argv[0]);
      return 0;
    case 9:
      print_version();
      return 0;
    case '?':
      fprintf(stderr, "Try '%s --help' for more information\n", argv[0]);
      return 1;
    }
  }

  // if X < 6, then mkstemp/mkdtemp are not going to work
  bool manual_create = false;

  int retry_count = 0;

retry_create:
  // implying TEMPLATE isnt manually set
  if (optind == argc) {
    snprintf(template, sizeof(template), "tmp.XXXXXXXXXX");
  } else { // otherwise
    snprintf(template, sizeof(template), "%s", argv[optind]);
    if (tmpdir == NULL)
      tmpdir = getenv("PWD");
    optind++;
  }

  if (optind < argc) {
    fprintf(stderr,
            "mktemp: too many templates\n"
            "Try '%s --help' for more information.\n",
            argv[0]);
  }

  if (suffix != NULL) {
    strncat(template, suffix, sizeof(template) - sizeof(suffix) - 1);
    manual_create = true;
  }

  if (count_consecutive_x(template) < 3) {
    fprintf(stderr, "mktemp: too few X's in template '%s'\n", template);
    return 1;
  } else if (count_consecutive_x(template) < 6) {
    manual_create = true;
  }

  if (tmpdir == NULL) {
    tmpdir = getenv("TMPDIR");
    if (tmpdir == NULL) {
      struct stat stats;
      if (stat("/tmp", &stats) == 0 && S_ISDIR(stats.st_mode)) {
        tmpdir = "/tmp";
      } else {
        fprintf(stderr, "mktemp: cannot find tmpdir\n");
        return 1;
      }
    }
  }

  randomize_prefix_x(template);

  char path[4096];
  snprintf(path, sizeof(path), "%s/%s", tmpdir, template);

  if (manual_create)
    goto manual_creation;

  if (dir) {
    if (mkdtemp(path) == NULL) {
      if (!quiet)
        goto exit_failure_dir;
    } else {
      puts(path);
      if (dry_run)
        rmdir(path);
    }
  } else {
    if (mkstemp(path) == -1) {
      if (!quiet)
        goto exit_failure_file;
    } else {
      puts(path);
      if (dry_run)
        unlink(path);
    }
  }
  goto exit;

// if Xs < 6, regular mkstemp/mkdtemp aint gonna work
manual_creation:;
  // manually modifying the remaining Xs (we're just writing the same previous 2
  // functions but idc)
  const char *letters =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  size_t len = strlen(path);

  for (ssize_t i = len - 1; i >= 0; i--) {
    if (path[i] == 'X') {
      path[i] = letters[rand() % 62];
    }
  }

  if (dir) {
    if (mkdir(path, 0700) != 0) {
      if (!quiet)
        goto exit_failure_dir;
    } else {
      if (tmpdir != getenv("PWD"))
        puts(path);
      else
        puts(template);
      if (dry_run)
        rmdir(path);
    }
  } else {
    if (open(path, O_RDWR | O_CREAT | O_EXCL, 0700) == -1) {
      if (errno ==
          EEXIST) { // would be suprised if it actually errored out this way
        if (retry_count >= 3) {
          if (!quiet) {
            fprintf(stderr, "mktemp: cannot generate unique template\n");
            return 1;
          }
        } else {
          retry_count++;
          goto retry_create;
        }
      } else {
        if (!quiet)
          goto exit_failure_file;
      }
    } else {
      if (tmpdir != getenv("PWD"))
        puts(path);
      else
        puts(template);
      if (dry_run)
        unlink(path);
    }
  }
  goto exit;

exit_failure_file:
  fprintf(stderr, "mktemp: failed to create file via template '%s': %s\n", path,
          strerror(errno));
  return 1;
exit_failure_dir:
  fprintf(stderr, "mktemp: failed to create directory via template '%s': %s\n",
          path, strerror(errno));
exit:
  return 0;
}
