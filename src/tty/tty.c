#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

bool silentOut = false;

struct help_entry {
  const char *opt;
  const char *desc;
};

static struct help_entry help_entries[] = {
  {"-s, --silent, --quiet", "print nothing, only return an exit status"},
  {"    --help", "display this help and exit"},
  {NULL, NULL}
};

void print_help(const char *name) {
  printf("Usage: %s [OPTION]...\n", name);
  printf("  or:  %s OPTION\n", name);
  printf("Print the file name of the terminal connected to standard input.\n\n");

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
  if (argv[1] != NULL) {
    if (strcmp(argv[1], "--silent") == 0 || strcmp(argv[1], "--quiet") == 0 || strcmp(argv[1], "-s") == 0) {
      silentOut = true;
    } else if (strcmp(argv[1], "--help") == 0) {
      print_help(argv[0]);
      return 0;
    } else {
      fprintf(stderr, "tty: unrecognized option '%s'\nTry '%s --help' for more information.\n", argv[1], argv[0]);
      return 1;
    }
  }

  char *ref = ttyname(STDIN_FILENO);
  if (ref == NULL) {
    if (!silentOut) fprintf(stderr, "tty: failed to retrieve the current terminal: %s\n", strerror(errno));
    return 1;
  }

  if (!silentOut)
    printf("%s\n", ref);

  return 0;
}
