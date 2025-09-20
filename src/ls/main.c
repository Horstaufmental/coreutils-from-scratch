#define _XOPEN_SOURCE 700

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/statfs.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>

#include "bytetohr.h"
#include "print_help.c"
#include "longformat.c"

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
    if ((strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) && includeALL == false) {
      // printf("\nskipped over: '%s', bool: %d\n", dir->d_name, includeALL);
      continue;
    }
    if (dir->d_name[0] == '.' && (includeALL == false && includeALLshort == false)) {
      // printf("\nskipped over: '%s', -a bool: %d, -A bool: %d\n", dir->d_name, includeALL, includeALLshort);
      continue;
    }
    if (longFormat) {
      // deadass thought the issue was entirely on longformat.c, no nigga it was fucking here
      // lstat needs full path and dir->d_name is just the basename but we're giving it the d_name
      char fullPath[PATH_MAX];
      snprintf(fullPath, sizeof(fullPath), "%s/%s", realPath ? realPath : ".", dir->d_name);
      printlongOutput(fullPath, dir->d_name);
      continue;
    }
    printf("%s  ", dir->d_name);
  }
  free(realPath);
  closedir(d);
  return (0);
}
