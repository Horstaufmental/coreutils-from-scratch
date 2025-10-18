#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/utsname.h>

struct help_entry {
  const char *opt;
  const char *desc;
};

static struct help_entry help_entries[] = {
  {"    --help", "display this help and exit"}
};

void print_help(const char *name) {
  printf("Usage: %s [OPTION]...\n", name);
  printf("Print machine hardware name (same as uname -m)\n\n");

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
  struct utsname sys_info;

  if (argv[1] != NULL) {
    if (strcasecmp(argv[1], "--help") == 0) {
      print_help(argv[0]);
      return 0;
    } else {
      fprintf(stderr, "%s: unrecognized option '%s'\nTry '%s --help' for more information", argv[0], argv[1], argv[0]);
      return 1;
    }
  }

  if (uname(&sys_info) == -1) {
    fprintf(stderr, "rm: cannot retrieve system info: %s\n", strerror(errno));
    return 1;
  }

  puts(sys_info.machine);
  return 0;
}
