#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct help_entry {
  const char *opt;
  const char *desc;
};

static struct help_entry help_entries[] = {
    {"    --help", "display this help and exit"}, {NULL, NULL}};

void print_help(const char *name) {
  printf("Usage: %s [OPTION]...\n", name);
  puts("Print the user name associated with the current effective ID.\n"
       "Same as id -un.\n");

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
}

void error_handle(const char *name) {
  fprintf(stderr, "%s: cannot get information: %s\n", name, strerror(errno));
  exit(EXIT_FAILURE);
}

int main(int argc __attribute__((unused)), char *argv[]) {
  if (argv[1] != NULL) {
    if (strcmp(argv[1], "--help") == 0) {
      print_help(argv[0]);
      return 0;
    } else {
      if (argv[1][0] == '-' && argv[1][1] == '-') {
        fprintf(stderr,
                "whoami: unrecognized option '%s'\n"
                "Try '%s --help' for more information.\n",
                argv[1], argv[0]);
        return 1;
      } else {
        fprintf(stderr,
                "whoami: extra operand '%s'\n"
                "Try '%s --help' for more information.\n",
                argv[1], argv[0]);
        return 1;
      }
    }
  }

  uid_t uid = geteuid();
  struct passwd pwd;
  struct passwd *ptr;
  char *buffer;
  long buf_size = sysconf(_SC_GETPW_R_SIZE_MAX);
  if (buf_size == -1) {
    error_handle("whoami");
  }

  buffer = malloc(buf_size);
  if (buffer == NULL) {
    error_handle("whoami");
  }

  int ret = getpwuid_r(uid, &pwd, buffer, buf_size, &ptr);

  if (ret == 0) {
    if (ptr != NULL) {
      puts(ptr->pw_name);
    } else {
      error_handle("whoami");
    }
  } else {
    errno = ret;
    error_handle("whoami");
  }
  free(buffer);

  return 0;
}
