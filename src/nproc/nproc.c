#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>

typedef enum {
  True = 1,
  False = 0
} Bool;

Bool showAll = False;
int ignoreProc = 0;

struct help_entry {
  const char *opt;
  const char *desc;
};

static struct option long_options[] = {
  {"all", no_argument, 0, 2},
  {"ignore", required_argument, 0, 3},
  {"help", no_argument, 0, 1},
  {0, 0, 0, 0}
};

static struct help_entry help_entries[] = {
  {"    --all", "print the number of installed processors"},
  {"    --ignore=N", "if possible, exclude N processing units"},
  {"    --help", "display this help and exit"},
  {NULL, NULL}
};

void print_help(const char *name) {
  printf("Usage: %s [OPTION]...\n", name);
  printf("Print the number of processing units available to the current process,\n");
  printf("which may be less than the number of online processors\n\n");
  int maxlen = 0;
  for (int i = 0; help_entries[i].opt; i++) {
    int len = (int)strlen(help_entries[i].opt);
    if (len > maxlen) maxlen = len;
  }

  for (int i = 0; help_entries[i].opt; i++) {
    printf("  %-*s  %s\n", maxlen, help_entries[i].opt, help_entries[i].desc);
  }
}

int main(int argc, char *argv[]) {
  int np;
  int opt;

  while ((opt = getopt_long(argc, argv, "", long_options, 0)) != -1) {
    switch(opt) {
      case 1:
        print_help(argv[0]);
        return 0;
      case 2:
        showAll = True;
        break;
      case 3:
        ignoreProc = atoi(optarg);
        break;
      case '?':
        printf("Try '%s --help' for more information.\n", argv[0]);
        return 1;
    }
  }
  
  switch (showAll) {
    case True:
      np = sysconf(_SC_NPROCESSORS_CONF);
      break;
    case False:
      np = sysconf(_SC_NPROCESSORS_ONLN);
      break;
  }

  if (ignoreProc > 0) {
    int buffer = (ignoreProc < 1) ? 1 : ((ignoreProc > np - 1) ? np - 1 : ignoreProc);
    np -= buffer;
  }
  
  char buffer[sizeof(np)];
  sprintf(buffer, "%d\n", np);
  write(1, buffer, strlen(buffer));
}

