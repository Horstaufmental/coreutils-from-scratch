#include <unistd.h>
#include <string.h>
#include <stdio.h>

struct help_entry {
  const char *opt;
  const char *desc;
};

static struct help_entry help_entries[] = {
  {"    --help", "display this help and exit"},
  {NULL, NULL}
};

void print_help(const char *name) {
  printf("Usage: %s [ignored command line arguments]\n", name);
  printf("  or:  %s OPTION\n", name);
  printf("Exit with a status code indicating failure.\n\n");

  int maxlen = 0;
  for (int i = 0; help_entries[i].opt; i++) {
    int len = (int)strlen(help_entries[i].opt);
    if (len > maxlen) maxlen = len;
  }

  for (int i = 0; help_entries[i].opt; i++) {
    printf("  %-*s  %s\n", maxlen, help_entries[i].opt, help_entries[i].desc);
  }
}

int main(int argc __attribute__((unused)), char *argv[]) {
  if (argv[1] != NULL)
    if (strcasecmp(argv[1], "--help") == 0)
      print_help(argv[0]);  
  return 1;
}
