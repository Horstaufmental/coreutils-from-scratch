#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>

#define true 1
#define false 0

int verbose = false;
int parents = false;
int mode = false;

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
  static struct option long_options[] = {
    {"verbose", no_argument, &verbose, 'v'},
    {"parents", required_argument, &parents, 'p'},
    {"mode=", required_argument, &mode, 0},
    {0, required_argument, &mode, 'm'}
  };
  
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
    case '?':
      break;
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
