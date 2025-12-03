#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define P_BYTES (1 << 0)
#define P_CHARS (1 << 1)
#define P_LINES (1 << 2)
#define P_LENMX (1 << 3)
#define P_WORDS (1 << 4)

#define T_AUTO (1 << 5)
#define T_ALWY (1 << 6)
#define T_ONLY (1 << 7)
#define T_NEVR (1 << 8)

struct help_entry {
  const char *opt;
  const char *desc;
};

static struct option long_options[] = {{"bytes", no_argument, 0, 'c'},
                                       {"chars", no_argument, 0, 'm'},
                                       {"lines", no_argument, 0, 'l'},
                                       {"max-line-length", no_argument, 0, 'L'},
                                       {"words", no_argument, 0, 'w'},
                                       {"total", required_argument, 0, 3},
                                       {"files0-from", required_argument, 0, 4},
                                       {"help", no_argument, 0, 1},
                                       {"version", no_argument, 0, 2}};

struct wc {
  long lines, words, chars, bytes, maxlen;
};

struct wc count_word(char *str) {
  long lines = 0, words = 0, chars = 0;
  bool in_word = true;

  for (size_t i = 0; i < strlen(str); i++) {
    chars++;
    if (str[i] == '\n')
      lines++;

    if (isspace((unsigned char)str[i])) {
      in_word = false;
    } else {
      if (!in_word) {
        words++;
        in_word = true;
      }
    }
  }

  struct wc willy; // we'll just call him that idk
  willy.lines = lines;
  willy.words = words;
  willy.chars = chars;
  willy.bytes = strlen(str);
  // TODO: maxlen
  willy.maxlen = 0;
  return willy;
}

struct wc count_word_fd(int fd) {
  char buf[1024];
  ssize_t bytes;
  int bytesread;
  long lines = 0, words = 0, chars = 0;
  bool in_word = true;

  while ((bytes = read(fd, buf, sizeof(buf))) > 0) {
    for (ssize_t i = 0; i < bytes; i++) {
      bytesread += bytes;
      chars++;
      if (buf[i] == '\n')
        lines++;

      if (isspace((unsigned char)buf[i])) {
        in_word = false;
      } else {
        if (!in_word) {
          words++;
          in_word = true;
        }
      }
    }
  }

  struct wc billy; // willy's twin brother
  billy.lines = lines;
  billy.words = words;
  billy.chars = chars;
  billy.bytes = bytesread;
  // TODO: maxlen
  billy.maxlen = 0;
  return billy;
}

void print_results(int flags, char *name, struct wc willer) {
  char buf[256];
  if (flags & P_LINES)
    // printf("%6ld", willer.bytes);
    
  if (flags & P_WORDS)
    printf("%7ld", willer.words);
  if (flags & P_CHARS)
    printf("%7ld", willer.chars);
  if (flags & P_BYTES)
    printf("%7ld", willer.bytes);
  if (flags & P_LENMX)
    printf("%7ld", willer.maxlen);

  if (name == NULL)
    fputs("\n", stdout);
  else
    printf(" %s\n", name);
}

void process_the_fucking_struct(char *buf, int fd, struct wc williams[],
                                int *count) {
  struct wc willer;
  if (buf != NULL)
    willer = count_word(buf);
  else
    willer = count_word_fd(fd);

  if ((long unsigned int)*count < sizeof(*williams)) {
    williams[*count] = willer;
    (*count)++;
  }
}

void print_to_var(char *buf, char *str, bool comma) {
  char buffer[4096];
  buffer[0] = '\0';
  // printf("buf (1) : |%s\n", buf);
  // printf("str     : |%s\n", str);
  snprintf(buffer, sizeof(buffer), "%s", buf ? buf : "");
  // printf("buffer  : |%s\n", buffer);
  if (strlen(buffer) == 0) {
    snprintf(buf, 4096, "%s", str);
  } else {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf(buf, 4096, "%s%s%s", buffer, comma ? "," : " ", str);
#pragma GCC diagnostic pop
    // printf("buf (2) : |%s\n\n", buf);
  }

  if (strlen(buf) >= 4096 - 1) {
    fprintf(stderr, "warning: print_to_var() output truncated.\n");
  }
}

int main(int argc, char *argv[]) {
  int flags = 0;
  int when = 0;
  int opt;
  while ((opt = getopt_long(argc, argv, "cmlL", long_options, NULL)) != -1) {
    switch (opt) {
    case 'c':
      flags |= P_BYTES;
      break;
    case 'm':
      flags |= P_CHARS;
      break;
    case 'l':
      flags |= P_LINES;
      break;
    case 'L':
      flags |= P_LENMX;
      break;
    case 'w':
      flags |= P_WORDS;
      break;
    case 3:
      if (strncasecmp(optarg, "auto", strlen(optarg)) == 0) {
        when |= T_AUTO;
      } else if (strncasecmp(optarg, "always", strlen(optarg)) == 0) {
        when |= T_ALWY;
      } else if (strncasecmp(optarg, "only", strlen(optarg)) == 0) {
        when |= T_ONLY;
      } else if (strncasecmp(optarg, "never", strlen(optarg)) == 0) {
        when |= T_NEVR;
      } else {
        fprintf(stderr,
                "%s: invalid argument '%s' for '--total'\n"
                "Valid arguments are:\n"
                "  - 'auto'\n"
                "  - 'always'\n"
                "  - 'only'\n"
                "  - 'never'\n",
                argv[0], optarg);
        fprintf(stderr, "Try '%s --help' for more information\n", argv[0]);
        return 1;
      }
      break;
    case 4:
      // later
      break;
    case 1:
      return 0;
    case 2:
      return 0;
    case '?':
      fprintf(stderr, "Try '%s --help' for more information\n", argv[0]);
      return 1;
    }
  }

  if (flags == 0) {
    flags = P_WORDS | P_LINES | P_BYTES;
  }
  if (when == 0) {
    when = T_AUTO;
  }

  struct wc williams[50];
  char names[50][1024];
  int count = 0;
  if (argc == optind) {
  do_stdin:;
    char buf[4096];
    while (true) {
      fgets(buf, sizeof(buf), stdin);
      if (feof(stdin))
        break;
    }
    strncpy(names[count], buf, sizeof(names[count]));
    process_the_fucking_struct(buf, 0, williams, &count);
  } else {
    if (strncmp(argv[optind], "-", strlen(argv[optind])) == 0)
      goto do_stdin;
    for (; optind < argc; optind++) {
      int fd = open(argv[optind], O_RDONLY);
      if (fd == -1) {
        fprintf(stderr, "%s: %s: %s\n", argv[0], argv[optind], strerror(errno));
        return 1;
      }
      strncpy(names[count], argv[optind], sizeof(names[count]));
      process_the_fucking_struct(NULL, fd, williams, &count);
      close(fd);
    }
  }

  if (count == 1) {
    print_results(flags, NULL, williams[0]);
    if (flags & T_ALWY)
      print_results(flags, "total", williams[0]); // :troll:
  } else if (count > 1) {
    struct wc final;
    for (int i = 0; i < count; i++) {
      final.bytes += williams[i].bytes;
      final.chars += williams[i].chars;
      final.lines += williams[i].lines;
      final.maxlen += williams[i].maxlen;
      final.words += williams[i].words;
    }
    char to_print[256];
    char int2string[256];
    if (flags & P_BYTES) {
      snprintf(int2string, sizeof(int2string), "%ld", final.bytes);
      print_to_var(to_print, int2string, false);
    }
    if (flags & P_CHARS) {
      snprintf(int2string, sizeof(int2string), "%ld", final.chars);
      print_to_var(to_print, int2string, false);
    }
    if (flags & P_LINES) {
      snprintf(int2string, sizeof(int2string), "%ld", final.lines);
      print_to_var(to_print, int2string, false);
    }
    if (flags & P_LENMX) {
      snprintf(int2string, sizeof(int2string), "%ld", final.maxlen);
      print_to_var(to_print, int2string, false);
    }
    if (flags & P_WORDS) {
      snprintf(int2string, sizeof(int2string), "%ld", final.words);
      print_to_var(to_print, int2string, false);
    }
    
    if (flags & T_ONLY) {
      puts(to_print);
      return 0;
    }

    for (int i = 0; i < count; i++) {
      print_results(flags, names[i], williams[i]);
    }
    if ((flags & T_AUTO || flags & T_ALWY) && !(flags & T_NEVR)) {
      printf("%s total\n", to_print);
    }
  }
}
