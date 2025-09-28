#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>

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

// yeah im redefining sys/utsname.h
struct utsname {
  char sysname[65];  /* Operating system name (e.g., "Linux") */
  char nodename[65]; /* Name within communications network
                        to which the node is attached, if any */
  char release[65];  /* Operating system release
                        (e.g., "2.6.28") */
  char version[65];  /* Operating system version */
  char machine[65];  /* Hardware type identifier */
};

int uname(struct utsname *buf) {
  if (syscall(63, buf) != 0)
    return -1;
  return 0;
}

int main(int argc __attribute__((unused)), char *argv[]) {
  struct utsname sys_info;

  if (argv[1] != NULL) {
    if (strcasecmp(argv[1], "--help") == 0) {
      print_help(argv[0]);
      return 0;
    } else {
      char buffer[257];
      sprintf(buffer, "%s: unrecognized option '%s'\nTry '%s --help' for more information", argv[0], argv[1], argv[0]);
      write(1, buffer, strlen(buffer));
      return 1;
    }
  }

  if (uname(&sys_info) == -1) {
    char buffer[257] = "rm: cannot retrieve system info: ";
    strncat(buffer, strerror(errno), sizeof(buffer) - strlen(buffer) - strlen(strerror(errno))); strncat(buffer, "\n", sizeof(buffer) - strlen(buffer) - 1);
    write(2, buffer, strlen(buffer));
    return 1;
  }

  char buffer[257];
  sprintf(buffer, "%s\n", sys_info.machine);
  write(1, buffer, strlen(buffer));
  return 0;
}
