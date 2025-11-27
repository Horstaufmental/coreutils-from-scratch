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

// this is hell
// but if it works
// it works

#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>

#define PROGRAM_NAME "rm"
#define PROJECT_NAME "coreutils from scratch"
#define AUTHORS "Horstaufmental"
#define VERSION "1.1 (Okami Era)"

bool verbose = false;
bool recursive = false;
bool rmEmpty = false;
bool force = false;
/*
0 = false
1 = prompt ONCE (-I) (when files > 3 or recursively)
2 = prompt all  (-i)
*/
int prompt = 0;
bool shouldPrompt = false;
bool preserveRoot = true;

int argCount; // for -I

struct help_entry {
  const char *opt;
  const char *desc;
};

static struct option long_options[] = {{"verbose", no_argument, 0, 'v'},
                                       {"help", no_argument, 0, 1},
                                       {"interactive", optional_argument, 0, 2},
                                       {"no-preserve-root", no_argument, 0, 3},
                                       {"recursive", no_argument, 0, 'r'},
                                       {"dir", no_argument, 0, 'd'},
                                       {"version", no_argument, 0, 9},
                                       {0, no_argument, 0, 'R'},
                                       {0, no_argument, 0, 'i'}, // prompt all
                                       {0, no_argument, 0, 'I'}, // prompt once
                                       {0, 0, 0, 0}};

static struct help_entry help_entries[] = {
    {"-f, --force", "ignore nonexistent files and arguments, never prompt"},
    {"-i", "prompt before every removal"},
    {"-I", "prompt once before removing more than thre files, or\n"
           "       when removing recursively; less intrusive than -i,\n"
           "       while still giving protection against most mistakes"},
    {"    --interactive[=WHEN]",
     "prompt according to WHEN: never, once (-I) or\n"
     "                             always (-i); without WHEN, prompt always"},
    {"    --no-preserve-root", "do not treat '/' specially"},
    {"-r, -R, --recursive", "remove directories and their contents recurively"},
    {"-d, --dir", "remove empty directories"},
    {"-v, --verbose", "explain what is being done"},
    {"    --help", "display this help and exit"},
    {"    --version", "output version information and exit"},
    {NULL, NULL}
};

void print_help(const char *name) {
  printf("Usage: %s [OPTION]... [FILE]...\n", name);
  printf("Remove (unlink) the FILE(s).\n\n");

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

int isYes(const char *input) {
  return strcasecmp(input, "y") == 0 || strcasecmp(input, "yes") == 0 ||
         strcasecmp(input, "ye") == 0;
}

int prompt_user(const char *message, const char *fileName) {
  char buffer[16];
  if (fileName == NULL) {
    printf("%s? ", message);
  } else
    printf("%s '%s'? ", message, fileName);

  fgets(buffer, sizeof(buffer), stdin);
  buffer[strcspn(buffer, "\n")] = '\0';
  return isYes(buffer);
}

int isDirEmpty(const char *path) {
  struct stat file_info;

  // check if IS directory
  if (lstat(path, &file_info) == 0) {
    if (!S_ISDIR(file_info.st_mode)) {
      return 1;
    }
  }

  DIR *dir = opendir(path);

  if (dir == NULL) {
    fprintf(stderr, "rm: cannot remove '%s': %s", path, strerror(errno));
    exit(EXIT_FAILURE);
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

void writeProtectCheck(const char *fileName, bool *isEmpty,
                       const char *fileType) {
  FILE *fp;
  char buffer[16];
  fp = fopen(fileName, "a");
  if (fp == NULL) {
    if ((errno == EACCES || errno == EPERM) && !force) {
      // still uses the old prompt logic, cba to replace it
      if (isEmpty)
        printf("rm: remove write-protected %s empty file '%s'?: ", fileType,
               fileName);
      else
        printf("rm: remove write-protected %s file '%s'?: ", fileType,
               fileName);
      if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
        // printf("%s - %d", buffer, isYes(buffer));
        if (!isYes(buffer) || strcasecmp(buffer, "") == 0) {
          exit(EXIT_FAILURE);
        }
      }
    }
  } else
    fclose(fp);
}

int recurseDir(const char *fileName) {
  struct stat file_info;

  if (lstat(fileName, &file_info) != 0) {
    fprintf(stderr, "rm: cannot remove '%s': %s\n", fileName, strerror(errno));
    return -1;
  }

  if (!S_ISDIR(file_info.st_mode)) {
    if (prompt == 2) {
      if (!prompt_user("rm: remove", fileName)) {
        return 0;
      }
      if (verbose)
        printf("rm: removed '%s'\n", fileName);
    }
    if (unlink(fileName) != 0) {
      fprintf(stderr, "rm: cannot remove '%s': %s\n", fileName,
              strerror(errno));
      return -1;
    }
    return 0;
  }

  DIR *dir = opendir(fileName);
  if (!dir) {
    if (!force)
      fprintf(stderr, "rm: cannot remove '%s': %s\n", fileName,
              strerror(errno));
    return -1;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;

    char *child_path;
    if (asprintf(&child_path, "%s/%s", fileName, entry->d_name) < 0) {
      if (!force)
        fprintf(stderr, "rm: cannot remove '%s': %s\n", fileName,
                strerror(errno));
      closedir(dir);
      return -1;
    }

    if (recurseDir(child_path) != 0) {
      free(child_path);
      closedir(dir);
      exit(EXIT_FAILURE);
    }
    free(child_path);
  }
  closedir(dir);
  if (prompt == 2)
    if (!prompt_user("rm: remove", fileName))
      exit(EXIT_SUCCESS);
  if (rmdir(fileName) < 0) {
    if (!force)
      fprintf(stderr, "rm: cannot remove '%s': %s\n", fileName,
              strerror(errno));
    exit(EXIT_FAILURE);
  }
  return 0;
}

/*
TODO:
- [x] convert all high-level remove()
to POSIX syscalls unlink() and rmdir()
- [x] force
- [x] interactive
- [x] preserveRoot
- [x] recursive
- [x] emptyDir
- [x] verbose
*/
void removeFile(const char *fileName) {
  bool isEmpty = false;
  struct stat file_info;

  if ((recursive && preserveRoot) && strcasecmp(fileName, "/") == 0) {
    printf("rm: it is dangerous to operate recursively on `/'\n");
    printf("rm: use --no-preserve-root to override this failsafe\n");
    exit(EXIT_FAILURE);
  }

  if (lstat(fileName, &file_info) == 0) {
    // directory empty check
    if ((file_info.st_size == 0) || isDirEmpty(fileName)) {
      isEmpty = true;
    }

    // if is directory and not recursive and remove empty
    if (S_ISDIR(file_info.st_mode) && !recursive && !rmEmpty) {
      fprintf(stderr, "rm: cannot remove '%s': Is a directory\n", fileName);
      exit(EXIT_FAILURE);
    } // if is directory (second check), recursive and rm empty dir is on, but
      // not empty
    else if (S_ISDIR(file_info.st_mode) && rmEmpty && !isEmpty && !recursive) {
      fprintf(stderr, "rm: cannot remove '%s': Directory not empty\n",
              fileName);
      exit(EXIT_FAILURE);
    }
  }
  // To: Horst, THIS IS PART OF THE STAT CHECK BTW
  else { // if it doesnt exist (fails `stat(fileName, &file_info) == 0` check)
    fprintf(stderr, "rm: cannot remove '%s': No such file or directory\n",
            fileName);
    exit(EXIT_FAILURE);
  }

  // type check
  char fileType[255];
  if (S_ISREG(file_info.st_mode)) {
    strncpy(fileType, "regular", sizeof(fileType));
  } else if (S_ISDIR(file_info.st_mode)) {
    strncpy(fileType, "directory", sizeof(fileType));
  } else if (S_ISBLK(file_info.st_mode)) {
    strncpy(fileType, "block", sizeof(fileType));
  } else if (S_ISFIFO(file_info.st_mode)) {
    strncpy(fileType, "fifo", sizeof(fileType));
  } else if (S_ISSOCK(file_info.st_mode)) {
    strncpy(fileType, "socket", sizeof(fileType));
  } else if (S_ISCHR(file_info.st_mode)) {
    strncpy(fileType, "character special", sizeof(fileType));
  }

  writeProtectCheck(fileName, &isEmpty, fileType);
  if (S_ISDIR(file_info.st_mode)) {
    // directory

    // non recursive on directory
    // and rm empty dir but not empty
    // check is already performed
    switch (prompt) {
    case 0: // no prompt
      if (recursive)
        recurseDir(fileName);
      else if (rmdir(fileName) < 0) {
        if (!force)
          fprintf(stderr, "rm: cannot remove '%s': %s\n", fileName,
                  strerror(errno));
        exit(EXIT_FAILURE);
      }
      break;
    case 1: // prompt once (files > 3)
      if (shouldPrompt) {
        char message[255];
        if (argCount > 3) {
          sprintf(message, "rm: remove %d arguments", argCount);
          if (!prompt_user(message, NULL)) {
            exit(EXIT_SUCCESS);
          }
        } else {
          sprintf(message, "rm: remove %d argument", argCount);
          if (!prompt_user(message, NULL)) {
            exit(EXIT_SUCCESS);
          }
        }
        shouldPrompt = false;
        break;
      }
      if (!isEmpty && recursive)
        recurseDir(fileName);
      else if (rmdir(fileName) != 0) {
        if (!force)
          fprintf(stderr, "rm: cannot remove '%s': %s\n", fileName,
                  strerror(errno));
        exit(EXIT_FAILURE);
      };
      break;
    case 2: // prompt always
      if (!isEmpty && recursive) {
        if (!prompt_user("rm: descend into directory", fileName))
          exit(EXIT_SUCCESS);
        recurseDir(fileName);
      } else {
        if (!prompt_user("rm: remove directory", fileName))
          exit(EXIT_SUCCESS);
        if (rmdir(fileName) != 0) {
          if (!force)
            fprintf(stderr, "rm: cannot remove '%s': %s\n", fileName,
                    strerror(errno));
          exit(EXIT_FAILURE);
        }
      }
      break;
    }
  }

  if (verbose) {
    if (S_ISDIR(file_info.st_mode))
      printf("rm: removed directory '%s'\n", fileName);
    else
      printf("rm: removed '%s'\n", fileName);
  }
}

int main(int argc, char *argv[]) {
  int opt;
  while ((opt = getopt_long(argc, argv, "fiIrRdv", long_options, 0)) != -1) {
    switch (opt) {
    case 'v':
      verbose = true;
      break;
    case 'f':
      force = true;
      break;
    case 'i':
      prompt = 2;
      break;
    case 'I':
      prompt = 1;
      break;
    case 2:
      if (optarg == NULL) { // had to be the first cuz the rest will segfault
        prompt = 2;         // defaults to alaways if just --interactive is used
      } else if (strcasecmp(optarg, "never") == 0 ||
                 strcasecmp(optarg, "no") == 0 ||
                 strcasecmp(optarg, "none") == 0) {
        prompt = 0;
      } else if (strcasecmp(optarg, "once") == 0) {
        prompt = 1;
      } else if (strcasecmp(optarg, "always") == 0 ||
                 strcasecmp(optarg, "yes") == 0) {
        prompt = 2;
      } else {
        printf("rm: ambiguous argument ‘%s’ for ‘--interactive’\n", optarg);
        printf("Valid arguments are:\n");
        printf("  - ‘never’, ‘no’, ‘none’\n");
        printf("  - ‘once’\n");
        printf("  - ‘always’, ‘yes’\n");
        printf("Try '%s --help' for more information.\n", argv[0]);
        return 1;
      }
      break;
    case 3:
      preserveRoot = false;
      break;
    case 'r':
      recursive = true;
      break;
    case 'R':
      recursive = true;
      break;
    case 'd':
      rmEmpty = true;
      break;
    case 1:
      print_help(argv[0]);
      return 0;
    case 9:
      print_version();
      return 0;
    default:
      if (!force)
        fprintf(stderr,
                "rm: missing operand\nTry '%s --help' for more information.\n",
                argv[0]);
      return 1;
    }
  }
  // only performed one time, checks if theres no non-option argument (optind
  // will equal to argc)
  if (optind == argc) {
    if (!force)
      fprintf(stderr,
              "rm: missing operand\nTry 'rm --help' for more information.\n");
    return 1;
  }

  argCount = argc - optind;

  if ((argc - optind) > 3 || recursive)
    shouldPrompt = true;

  for (; optind < argc; optind++) {
    removeFile(argv[optind]);
  }

  return 0;
}
