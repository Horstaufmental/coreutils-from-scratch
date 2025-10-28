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
#include <errno.h>
#include <getopt.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PROGRAM_NAME "groups"
#define PROJECT_NAME "coreutils from scratch"
#define AUTHORS "Horstaufmental"
#define VERSION "1.1 (Okami Era)"

#define BUF_SIZE 1025

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
  printf("Usage: %s [OPTION]... [USERNAME]...\n", name);
  puts("Print group memberships for each USERNAME or, if no USERNAME is "
       "specified, for\n"
       "the current process (which may differ if the groups database has "
       "changed).\n");

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

void print_to_var(char *buf, char *str, bool comma) {
  char buffer[BUF_SIZE];
  buffer[0] = '\0';
  // printf("buf (1) : |%s\n", buf);
  // printf("str     : |%s\n", str);
  snprintf(buffer, sizeof(buffer), "%s", buf ? buf : "");
  // printf("buffer  : |%s\n", buffer);
  if (strlen(buffer) == 0) {
    snprintf(buf, BUF_SIZE, "%s", str);
  } else {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf(buf, BUF_SIZE, "%s%s%s", buffer, comma ? "," : " ", str);
#pragma GCC diagnostic pop
    // printf("buf (2) : |%s\n\n", buf);
  }

  if (strlen(buf) >= BUF_SIZE - 1) {
    fprintf(stderr, "warning: print_to_var() output truncated.\n");
  }
}

int print_groups(char *user, struct passwd *pw, bool userset_yes) {
  int ngroups = 10;

  gid_t *gids = (gid_t *)malloc(10 * sizeof(gid_t));
  if (gids == NULL)
    return 1;

  int ret = getgrouplist(user, pw->pw_gid, gids, &ngroups);

  if (ret == -1) {
    free(gids);
    gids = (gid_t *)malloc((ngroups * 2) * sizeof(gid_t));
    if (gids == NULL) {
      return 1;
    }
    ret = getgrouplist(user, pw->pw_gid, gids, &ngroups);
  }

  if (ret == -1) {
    free(gids);
    return 1;
  }

  if (userset_yes)
    printf("%s : ", user);

  char *buffer = malloc(BUF_SIZE);
  if (buffer == NULL) {
    return 1;
  }
  buffer[0] = '\0';

  for (int i = 0; i < ngroups; i++) {
    struct group *gr = getgrgid(gids[i]);
    if (gr != NULL) {
      print_to_var(buffer, gr->gr_name, false);
    }
  }

  puts(buffer);

  free(buffer);
  free(gids);

  return 0;
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

  bool userset_yes = false;

  if (optind == argc) {
    struct passwd *pw;
    uid_t uid = getuid();
    struct passwd pwd;
    struct passwd *ptr;
    char *buffer;
    long buf_size = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (buf_size == -1)
      return 1;

    buffer = malloc(buf_size);
    if (buffer == NULL)
      return 1;

    int ret = getpwuid_r(uid, &pwd, buffer, buf_size, &ptr);

    if (ret == 0) {
      if (ptr != NULL) {
        pw = getpwnam(ptr->pw_name);
        if (pw == NULL) {
          if (errno == 0 || errno == ENOENT || errno == ESRCH ||
              errno == EBADF || errno == EPERM) {
            fprintf(stderr, "id: '%s': no such user\n", ptr->pw_name);
            exit(EXIT_FAILURE);
          } else
            goto exit_fail;
        }
      }
    }
    if (print_groups(pw->pw_name, pw, userset_yes) != 0)
      goto exit_fail;
  } else {
    userset_yes = true;
    for (; optind < argc; optind++) {
      char *username = argv[optind];

      struct passwd *user_info = NULL;
      if (username != NULL) {
        for (int i = 0; username[i] != '\0'; i++) {
          if (username[i] == '\n') {
            username[i] = '\0';
            break;
          }
        }
      }
      user_info = getpwnam(username);

      errno = 0;
      if (user_info == NULL) {
        /*
        "The formulation given above under "RETURN VALUE" is from
         POSIX.1-2001. It does not call "not found" an error, and hence
         does not specify what value errno might have in this situation."

         "Experiments on various UNIX-like systems show that
         lots of different values occur in this situation: 0, ENOENT,
         EBADF, ESRCH, EWOULDBLOCK, EPERM, and probably others."
        */
        if (errno == 0 || errno == ENOENT || errno == ESRCH || errno == EBADF ||
            errno == EPERM) {
          fprintf(stderr, "id: '%s': no such user\n", username);
          return 1;
        } else
          goto exit_fail;
      }
      if (print_groups(user_info->pw_name, user_info, userset_yes) != 0)
        goto exit_fail;
    }
  }
  return 0;

exit_fail:
  fprintf(stderr, "groups: cannot get groups: %s\n", strerror(errno));
  return 1;
}
