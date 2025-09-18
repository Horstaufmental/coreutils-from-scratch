#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>
#include <argp.h>

#define true 1
#define false 0

int verbose = false;
int parents = false;
int mode = false;

struct help_entry {
  const char *opt;
  const char *desc;
};

static struct option long_options[] = {
  {"verbose", no_argument, 0, 'v'},
  {"parents", no_argument, 0, 'p'},
  {"mode", required_argument, 0, 'm'},
  {"help", no_argument, 0, 1},
  {0, 0, 0, 0}
};

static struct help_entry help_entries[] = {
  {"-m, --mode=MODE", "set file mode (as in chmod), not a=rwx - umask"},
  {"-p, --parents", "no error if existing, make parent directories as needed,\n"
  "                  with their file modes unaffected by any -m option"},
  {"-v, --verbose", "print a message for each created directory"},
  {"--help", "display this help and exit"}
};

void print_help(const char *name) {
  printf("Usage: %s [OPTION]... DIRECTORY...\n", name);
  printf("Create the DIRECTORY(ies), if they do not already exist.\n\n");
  printf("Mandatory arguments to long options are mandatory for short options too.\n");

  // find longest option string
  int maxlen = 0;
  for (int i = 0; help_entries[i].opt; i++) {
    int len = (int)strlen(help_entries[i].opt);
    if (len > maxlen) maxlen = len;
  }
  
  // print each option aligned
  for (int i = 0; help_entries[i].opt; i++) {
    printf("  %-*s  %s\n", maxlen, help_entries[i].opt, help_entries[i].desc);
  }
}

void createDir(char *dirName, mode_t modeV) {
  if (parents == true) {
    char *tmp = strdup(dirName);
    char *p = tmp;
    int status = 0;

    if (tmp == NULL) {
      exit(EXIT_FAILURE);
    }

    if (*p == '/')
      p++;

    for (; *p; p++) {
      if (*p == '/') {
        *p = '\0'; // temporaily terminate string

        if (mkdir(tmp, modeV) != 0) {
          if (verbose == true) {
            printf("mkdir: created directory '%s'\n", tmp);
          }
          if (errno != EEXIST) {
            status = -1;
            break;
          }
        } else {
          if (verbose == true) {
            printf("mkdir: created directory '%s'\n", tmp);
          }
        }
        *p = '/';
      }
    }

    if (status == 0) {
      if (mkdir(tmp, modeV) != 0) {
        if (errno != EEXIST) {
          status = -1;
        }
      } else {
        if (verbose == true) {
          printf("mkdir: created directory '%s'\n", tmp);
        }
      }
    }
    
    free(tmp);
    if (status == -1) {
      printf("mkdir: cannot create directory '%s'\n", dirName);
      exit(EXIT_FAILURE);
    }
    return;
  }
  int check = mkdir(dirName, modeV);

  if (check == 0) {
    if (verbose == true)
      printf("mkdir: created directory '%s'\n", dirName);
  } else {
    printf("mkdir: cannot create directory '%s': %s\n", dirName,
           strerror(errno));
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("mkdir: missing operand\n");
    return 1;
  }

  int opt;
  mode_t modeV;
  while ((opt = getopt_long(argc, argv, "vpm:", long_options, 0)) != -1) {
    switch (opt) {
    case 'v':
      verbose = true;
      break;
    case 'p':
      parents = true;
      break;
    case 'm':
      mode = true;
      modeV = (mode_t) strtol(optarg, NULL, 8);
      break;
    case 1:
      print_help(argv[0]);
      return 0;
    case '?':
      return 1;
    default:
      break;
    }
  }
  if (mode == false) {
    modeV = 0755;
  }
  for (; optind < argc; optind++) {
    createDir(argv[optind], modeV);
  }

  return 0;
}
