#include <asm-generic/errno-base.h>
#include <errno.h>
#include <getopt.h>
#include <grp.h>
#include <pwd.h>
#include <selinux/selinux.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 1025

#define P_CONTEXT (1 << 0)
#define P_GROUP (1 << 1)
#define P_GROUPS (1 << 2)
#define P_NAME (1 << 3)
#define P_REAL (1 << 4)
#define P_USER (1 << 5)
#define P_ZERO (1 << 6)
#define P_MODE_MASK (P_CONTEXT | P_GROUP | P_GROUPS | P_USER)

#define P_DEFAULT (1 << 7)
#define P_GROUPS_DEF (1 << 8) // for calling P_GROUPS with default format
#define P_DEF_MASKZ (P_DEFAULT | P_ZERO)
#define P_DEF_MASKN (P_DEFAULT | P_NAME)
#define P_DEF_MASKR (P_DEFAULT | P_REAL)

struct help_entry {
  const char *opt;
  const char *desc;
};

static struct option long_options[] = {
    {"context", no_argument, 0, 'Z'}, {"group", no_argument, 0, 'g'},
    {"groups", no_argument, 0, 'G'},  {"name", no_argument, 0, 'n'},
    {"real", no_argument, 0, 'r'},    {"user", no_argument, 0, 'u'},
    {"zero", no_argument, 0, 'z'},    {"help", no_argument, 0, 1},
    {0, no_argument, 0, 'a'},         {0, 0, 0, 0}};

static struct help_entry help_entries[] = {
    {"-a", "ignore, for compatibility with other versions"},
    {"-Z, --context", "print only the security context of the process"},
    {"-g, --group", "print only the effective group ID"},
    {"-G, --groups", "print all group IDs"},
    {"-n, --name", "print a name instead of a number, for -ugG"},
    {"-r, --real", "print the real ID instead of the effective ID, with -ugG"},
    {"-u, --user", "print only the effective user ID"},
    {"-z, --zero", "delimit entries with NUL characters, not whitespace;\n"
                   "                  not permitted in default format"},
    {"    --help", "display this help and exit"},
    {NULL, NULL}};

void print_help(const char *name) {
  printf("Usage: %s [OPTION]... [USER]...\n", name);
  puts("Print user and group information for each specified USER,\n"
       "or (when USER omitted) for the current process.\n");

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
  puts(
      "\nWithout any OPTION, print some useful set of identified information.");
}

int check_mutex(unsigned int flags, unsigned int mask) {
  unsigned int m = flags & mask;
  return (m && (m & (m - 1))); // more than one bit set = nuh uh
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

int print_id(unsigned int flags, char *user) {
  if (flags & P_CONTEXT && (is_selinux_enabled() != 1)) {
    fputs("id: --context (-Z) works only on an SELinux-enabled kernel\n",
          stderr);
    exit(EXIT_FAILURE);
  }
  if (check_mutex(flags, P_DEF_MASKZ) == 1 ||
      check_mutex(flags, P_DEF_MASKN) == 1 ||
      check_mutex(flags, P_DEF_MASKR) == 1) {
    if (flags & P_ZERO)
      fputs("id: option --zero not permitted in default format\n", stderr);
    else
      fputs("id: cannot print only names or real IDs in default format\n",
            stderr);
    exit(EXIT_FAILURE);
  }
  if (check_mutex(flags, P_MODE_MASK)) {
    fputs("id: cannot print \"only\" of more than one choice\n", stderr);
    exit(EXIT_FAILURE);
  }

  struct passwd *user_info = NULL;

  if (user != NULL) {
    for (int i = 0; user[i] != '\0'; i++) {
      if (user[i] == '\n') {
        user[i] = '\0';
        break;
      }
    }
    user_info = getpwnam(user);

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
        fprintf(stderr, "id: '%s': no such user\n", user);
        exit(EXIT_FAILURE);
      } else
        return 1;
    }
  }

  if (flags & P_CONTEXT) {
    if (user_info != NULL) {
      fprintf(stderr,
              "id: cannot print security context when user specified\n");
      return 1;
    }
    pid_t pid = getpid();
    char *context = NULL;

    if (getpidcon(pid, &context) < 0) {
      fprintf(stderr, "id: cannot get security context: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
    if (flags & P_ZERO)
      fputs(context, stdout);
    else
      puts(context);
    freecon(context);
  }
  if (flags & P_GROUP) {
    gid_t gid;
    if (user_info != NULL) {
      gid = user_info->pw_gid;
    } else {
      if (flags & P_REAL) {
        gid = getgid();
      } else {
        gid = getegid();
      }
    }
    struct group grp;
    struct group *ptr;
    char *buffer;
    long buf_size = sysconf(_SC_GETGR_R_SIZE_MAX);
    if (buf_size == -1) {
      return 1;
    }

    buffer = malloc(buf_size);
    if (buffer == NULL) {
      return 1;
    }

    int ret = getgrgid_r(gid, &grp, buffer, buf_size, &ptr);

    if (ret == 0) {
      if (ptr != NULL) {
        if (flags & P_NAME) {
          if (flags & P_ZERO)
            fputs(ptr->gr_name, stdout);
          else
            puts(ptr->gr_name);
        } else {
          if (flags & P_ZERO)
            printf("%d", ptr->gr_gid);
          else
            printf("%d\n", ptr->gr_gid);
        }
      } else
        return 1;
    } else {
      errno = ret;
      return 1;
    }

    free(buffer);
  }
  if (flags & P_GROUPS) {
    char *username;
    struct passwd *pw;
    gid_t *gids;
    int ngroups = 10;

    if (user_info != NULL) {
      pw = user_info;
    } else {
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
              fprintf(stderr, "id: '%s': no such user\n", user);
              exit(EXIT_FAILURE);
            } else
              return 1;
          }
        }
      }
    }

    username = pw->pw_name;

    gids = (gid_t *)malloc(10 * sizeof(gid_t));
    if (gids == NULL)
      return 1;

    int ret = getgrouplist(username, pw->pw_gid, gids, &ngroups);

    if (ret == -1) {
      free(gids);
      gids = (gid_t *)malloc(ngroups * sizeof(gid_t));
      if (gids == NULL) {
        return 1;
      }
      ret = getgrouplist(username, pw->pw_gid, gids, &ngroups);
    }

    if (ret == -1) {
      free(gids);
      return 1;
    }

    if (flags & P_GROUPS_DEF)
      fputs("groups=", stdout);

    char *buffer = malloc(BUF_SIZE);
    if (buffer == NULL) {
      return 1;
    }
    buffer[0] = '\0';

    for (int i = 0; i < ngroups; i++) {
      struct group *gr = getgrgid(gids[i]);
      if (gr != NULL) {
        char buf[256];

        if (flags & P_GROUPS_DEF) {
          // printf("%d(%s)", gids[i], gr->gr_name);
          sprintf(buf, "%d(%s)", gids[i], gr->gr_name);
          print_to_var(buffer, buf, true);
        } else if (flags & P_NAME) {
          // printf("%s ", gr->gr_name);
          print_to_var(buffer, gr->gr_name, false);
        } else {
          // printf("%d ", gids[i]);
          sprintf(buf, "%d", gids[i]);
          print_to_var(buffer, buf, false);
        }
      }
    }

    fputs(buffer, stdout);

    if ((~flags & P_ZERO) != 0)
      fputs("\n", stdout);
    
    free(buffer);
    free(gids);
  }
  if (flags & P_USER) {
    if (user_info != NULL) {
      if (flags & P_NAME) {
        if (flags & P_ZERO)
          fputs(user_info->pw_name, stdout);
        else
          puts(user_info->pw_name);
      } else {
        if (flags & P_ZERO)
          printf("%d", user_info->pw_uid);
        else
          printf("%d\n", user_info->pw_uid);
      }
      return 0;
    }

    uid_t uid;
    if (flags & P_REAL) {
      uid = getuid();
    } else {
      uid = geteuid();
    }
    struct passwd pwd;
    struct passwd *ptr;
    char *buffer;
    long buf_size = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (buf_size == -1) {
      return 1;
    }

    buffer = malloc(buf_size);
    if (buffer == NULL) {
      return 1;
    }

    int ret = getpwuid_r(uid, &pwd, buffer, buf_size, &ptr);

    if (ret == 0) {
      if (ptr != NULL) {
        if (flags & P_NAME) {
          if (flags & P_ZERO)
            fputs(ptr->pw_name, stdout);
          else
            puts(ptr->pw_name);
        } else {
          if (flags & P_ZERO)
            printf("%d", ptr->pw_uid);
          else
            printf("%d\n", ptr->pw_uid);
        }
      } else
        return 1;
    } else {
      errno = ret;
      return 1;
    }
    free(buffer);
  }
  if (flags & P_DEFAULT) {
    if (user_info != NULL) {
      struct group *gr = getgrgid(user_info->pw_gid);
      printf("uid=%d(%s) gid=%d(%s) ", user_info->pw_uid, user_info->pw_name,
             user_info->pw_gid, gr->gr_name);
      if (print_id(P_GROUPS | P_GROUPS_DEF, user_info->pw_name) != 0) {
        return 1;
      }
      return 0;
    }
    uid_t uid = getuid();
    gid_t gid = getgid();

    struct passwd *usr = getpwuid(uid);
    struct group *grp = getgrgid(gid);

    printf("uid=%d(%s) gid=%d(%s) ", uid, usr->pw_name, gid, grp->gr_name);
    if (print_id(P_GROUPS | P_GROUPS_DEF, usr->pw_name) != 0) {
      return 1;
    }
  }
  return 0;
}

int main(int argc, char *argv[]) {
  int opt;
  unsigned int flags = 0;
  while ((opt = getopt_long(argc, argv, "aZgGnruz", long_options, 0)) != -1) {
    switch (opt) {
    case 'a':
      break;
    case 'Z':
      flags |= P_CONTEXT;
      break;
    case 'g':
      flags |= P_GROUP;
      break;
    case 'G':
      flags |= P_GROUPS;
      break;
    case 'n':
      flags |= P_NAME;
      break;
    case 'r':
      flags |= P_REAL;
      break;
    case 'u':
      flags |= P_USER;
      break;
    case 'z':
      flags |= P_ZERO;
      break;
    case 1:
      print_help(argv[0]);
      return 0;
    case '?':
      fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
      return 1;
    }
  }
  if ((flags & P_MODE_MASK) == 0) // if no "only" options, print default format
    flags |= P_DEFAULT;

  if (optind == argc) {
    if (print_id(flags, NULL) != 0) {
      fprintf(stderr, "id: cannot get information: %s\n", strerror(errno));
      return 1;
    }
  }

  for (; optind < argc; optind++) {
    if (print_id(flags, argv[optind]) != 0) {
      fprintf(stderr, "id: cannot get information: %s\n", strerror(errno));
      return 1;
    }
  }

  return 0;
}
