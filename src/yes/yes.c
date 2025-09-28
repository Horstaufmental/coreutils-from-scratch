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
  printf("Usage: %s [STRING]...\n", name);
  printf("  or:  %s OPTION\n", name);
  printf("Repeatedly output a line with all specified STRING(s), or 'y'.\n\n");

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


int main(int argc __attribute__((unused)), char *argv[]) {
  static int ovr;
  
  if (argv[1] == NULL) {
    ovr = 1;
  } else {
  if (strcasecmp(argv[1], "--help") == 0) {
      print_help(argv[0]);
      return 0;
    }
  }

  static char buffer[4096];
  if (!ovr)
    strncat(buffer, argv[1], sizeof(buffer) - strlen(buffer) - strlen(argv[1]));
  else
    strncat(buffer, "y", sizeof(buffer) - strlen(buffer - 1));

  strncat(buffer, "\n", sizeof(buffer) - strlen(buffer) - 1);

  while (1)
    write(1, buffer, strlen(buffer));
}
