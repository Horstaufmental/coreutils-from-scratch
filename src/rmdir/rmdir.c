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
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define PROGRAM_NAME "rmdir"
#define PROJECT_NAME "coreutils from scratch"
#define AUTHORS "Horstaufmental"
#define VERSION "1.1 (Okami Era)"

struct help_entry {
  const char *opt;
  const char *desc;
};

static struct option long_options[] = {
    {"parents", no_argument, 0, 'p'},
    {"verbose", no_argument, 0, 'v'},
    {"help", no_argument, 0, 1},
    {"version", no_argument, 0, 9},
    {"ignore-fail-on-non-empty", no_argument, 0, 2},
    {0, no_argument, 0, 0}};

static struct help_entry help_entries[] = {
    {"    --ignore-fail-on-non-empty",
     "ignore each failure to remove a non-emopty directory"},
    {"-p, --parents", "remove DIRECTORY and its ancestors;\n"
                      "                                  e.g., 'rmdir -p a/b' "
                      "is similar to 'rmdir a/b a'\n"},
    {"-v, --verbose", "output a diagnostic for every directory processed"},
    {"    --help", "display this help and exit"},
    {"    --version", "output version information and exit"},
    {0, 0}};

void print_help(const char *name) {
  printf("Usage: %s [OPTION]... DIRECTORY...\n"
         "Remove the DIRECTORY(ies), if they are empty.\n\n",
         name);
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

bool ignoreNonEmpty = false;
bool verbose = false;
bool parents = false;

int isDirEmpty(const char *path) {
  struct stat file_info;

  // check if IS directory
  if (lstat(path, &file_info) == 0) {
    if (!S_ISDIR(file_info.st_mode)) {
      return -1;
    }
  }

  DIR *dir = opendir(path);

  if (dir == NULL) {
    return 1;
  }

  struct dirent *entry;
  int count = 0;

  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
      count++;
      break;
    }
  }
  closedir(dir);
  return (count == 0); // if empty, ret 1 ; if not, ret 0
}

int rmdirPath(char *path) {
  if (parents) {
    char *workPath = strdup(path);
    if (!workPath)
      return 1;

    char *paths[256];
    int count = 0;

    size_t len = strlen(workPath);
    while (len > 1 && workPath[len - 1] == '/') {
      workPath[len - 1] = '\0';
      len--;
    }

    do {
      if (strcmp(workPath, "") == 0 || strcmp(workPath, ".") == 0 ||
          strcmp(workPath, "/") == 0)
        break;
      paths[count++] = strdup(workPath);
      char *slash = strrchr(workPath, '/');
      if (!slash)
        break;
      if (slash == workPath) {
        // dont remove root
        break;
      }
      *slash = '\0';
    } while (1);

    int ret = 0;
    for (int i = 0; i < count; i++) {
      if (verbose) {
        printf("rmdir: removing directory '%s'\n", paths[i]);
      }
      if (rmdir(paths[i]) != 0) {
        if (errno == ENOTEMPTY || errno == EEXIST || errno == ENOTDIR) {
          if (!ignoreNonEmpty) {
            fprintf(stderr, "rmdir: failed to remove '%s': %s\n", paths[i],
                    strerror(errno));
            free(workPath);
            free(paths[i]);
            exit(EXIT_FAILURE);
          }
        } else {
          ret = -1;
          break;
        }
        // stop at first failure
        break;
      }
      free(paths[i]);
    }
    free(workPath);
    return ret;
  } else {
    if (verbose) {
      printf("rmdir: removing directory '%s'\n", path);
    }
    if (rmdir(path) < 0) {
      if (errno == ENOTEMPTY || errno == EEXIST || errno == ENOTDIR) {
        if (!ignoreNonEmpty) {
          return -1;
        } else {
          return 0; // ignore failure on non-empty
        }
      } else {
        return -1;
      }
    }
  }
  return 0;
}

int main(int argc, char *argv[]) {
  int opt;
  while ((opt = getopt_long(argc, argv, "pv", long_options, 0)) != -1) {
    switch (opt) {
    case 'p':
      parents = true;
      break;
    case 'v':
      verbose = true;
      break;
    case 1:
      print_help(argv[0]);
      return 0;
    case 2:
      ignoreNonEmpty = true;
      break;
    case 9:
      print_version();
      return 0;
    default:
      fprintf(stderr,
              "rm: missing operand\n"
              "Try '%s --help' for more information.\n",
              argv[0]);
      return 1;
    }
  }

  if (optind == argc) {
    fprintf(stderr,
            "rmdir: missing operand\n"
            "Try '%s --help' for more information.\n",
            argv[0]);
    return 1;
  }

  for (; optind < argc; optind++) {
    if (strcmp(argv[optind], ".") == 0) {
      fputs("rmdir: failed to remove '.': Invalid argument\n", stderr);
      return 1;
    }
    if (rmdirPath(argv[optind]) != 0) {
      if (isDirEmpty(argv[optind]) == -1) {
        if (!ignoreNonEmpty) {
          fprintf(stderr, "rmdir: failed to remove '%s': Not a directory\n",
                  argv[optind]);
          return 1;
        } else {
          continue;
        }
      }
      if (!ignoreNonEmpty) {
        fprintf(stderr, "rmdir: failed to remove '%s': %s\n", argv[optind],
                strerror(errno));
        return 1;
      } else {
        continue;
      }
    }
  }

  return 0;
}
