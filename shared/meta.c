#include "meta.h"

#include <stdio.h>
#include <string.h>

#define BOLD_ON \033[1m
#define BOLD_OFF \033[0m

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

void print_help(const char *usage, const char *desc,
                const struct help_entry *help_entries, const char *footer) {
  puts(usage);
  puts(desc);

  // find longest option string
  int maxlen = 0;
  for (int i = 0; help_entries[i].opt; i++) {
    int len = (int)strlen(help_entries[i].opt);
    if (len > maxlen)
      maxlen = len;
  }

  // print each option aligned
  for (int i = 0; help_entries[i].opt; i++) {
    if (help_entries[i].placement == Inline)
      printf("  " TOSTRING(BOLD_ON) "%-*s" TOSTRING(BOLD_OFF) "  %s\n", maxlen, help_entries[i].opt, help_entries[i].desc);
    else
      printf("  " TOSTRING(BOLD_ON) "%-*s" TOSTRING(BOLD_OFF) "\n         %s\n", maxlen, help_entries[i].opt, help_entries[i].desc);
  }

  if (footer != NULL) {
    putchar('\n');
    puts(footer);
  }
}

void print_version(const char *prog_name, const char *proj_name,
                   const char *ver, const char *authors) {
  printf("%s (%s) %s\n", prog_name, proj_name, ver);
  printf("Copyright (C) 2025 %s\n", authors);
  puts("License GPLv3+: GNU GPL version 3 or later "
       "<https://gnu.org/licenses/gpl.html>.\n"
       "This is free software: you are free to change and redistribute it.\n"
       "There is NO WARRANTY, to the extent permitted by law.\n");
  printf("Written by %s\n", authors);
}
