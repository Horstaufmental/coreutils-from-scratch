#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
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
  printf("Usage: %s NUMBER[SUFFIX]...\n", name);
  printf("  or:  %s OPTION\n", name);
  printf("Pause for NUMBER seconds, where NUMBER is an interger or floating-point.\n");
  printf("SUFFIX may be 's','m','h', or 'd', for seconds, minutes, hours, days.\n");
  printf("With multiple arguments, pause for the sum of their values.\n\n");

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

int parse_time(const char *str, long *out) {
    char *endptr;
    errno = 0;

    // Parse number (integer or float allowed)
    double val = strtod(str, &endptr);

    if (endptr == str) return 0;   // no number found
    if (errno == ERANGE) return 0; // overflow/underflow

    // Default multiplier = seconds
    long long multiplier = 1;

    // Check suffix
    switch (*endptr) {
        case 's': multiplier = 1; break;                 // seconds
        case 'm': multiplier = 60; break;                // minutes
        case 'h': multiplier = 60 * 60; break;           // hours
        case 'd': multiplier = 60 * 60 * 24; break;      // days
        case '\0': multiplier = 1; break;                // no suffix = seconds
        default: return 0; // invalid suffix
    }

    endptr++; // move past suffix

    if (*endptr != '\0') return 0; // extra junk at end

    // Apply multiplier with rounding
    double result = val * multiplier;

    if (result > LLONG_MAX || result < LLONG_MIN) return 0; // overflow check

    *out = (long long)(result); // round to nearest integer
    return 1;
}

int main(int argc __attribute__((unused)), char *argv[]) {
  struct timespec timeSleep;
  
  if (argv[1] != NULL) {
    if (strcasecmp(argv[1], "--help") == 0) {
      print_help(argv[0]);
      return 0;
    } else {
      for (int i = 1; argv[i] != NULL; i++) {
        if (parse_time(argv[i], &timeSleep.tv_sec) == 0) {
          printf("sleep: invalid time interval '%s'\nTry '%s --help' for more information.\n", argv[1], argv[0]);
          return 1;
        }
        timeSleep.tv_nsec = 0;
        nanosleep(&timeSleep, NULL);
      }
    }
  } else {
    printf("sleep: missing operand\nTry '%s --help' for more information.\n", argv[0]);
    return 1;
  }
  return 0;
}
