/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This file is part of coreutils from scratch.
 * Copyright (c) 2025 Horstaufmental
 *
 * coreutils from scratch is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * coreutils from scratch is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */
#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <locale.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>

#define P_BYTES (1 << 0)
#define P_CHARS (1 << 1)
#define P_LINES (1 << 2)
#define P_LENMX (1 << 3)
#define P_WORDS (1 << 4)
#define P_DEFAULT (1 << 5)

#define T_AUTO (1 << 0)
#define T_ALWY (1 << 1)
#define T_ONLY (1 << 2)
#define T_NEVR (1 << 3)

#define PROGRAM_NAME "wc"
#define PROJECT_NAME "coreutils from scratch"
#define AUTHORS "Horstaufmental"
#define VERSION "1.0"

struct help_entry {
  const char *opt;
  const char *desc;
};

static struct help_entry help_entries[] = {
  {"-c, --bytes", "print the byte counts"},
  {"-m, --chars", "print the character counts"},
  {"-l, --lines", "print the newline counts"},
  {"    --files0-from=F", "read input from the files specified by\n"
   "                      NUL-terminated names in file F;\n"
   "                      if F is - then read names from standard input"},
  {"-L, --max-line-length", "print the maximum display width"},
  {"-w, --words", "print the word counts"},
  {"    --total=WHEN", "when to print a line with total counts;\n"
   "                      WHEN can be: auto, always, only, never"},
  {"    --help", "display this help and exit"},
  {"    --version", "output version information and exit"},
  {0, 0}
};

void print_help(const char *name)
{
  printf("Usage: %s [OPTION]... [FILE]...\n"
         "  or:  %s [OPTION]... --files0-from=F\n",
         name, name);
  puts("Print newline, word, and byte counts for each FILE, and a total line if.\n"
       "more than one FILE is specified.  A word is a nonempty sequence of non white\n"
       "space delimited by white space characters or by start or end of input.\n\n"

       "With no FILE, or when FILE is -, read standard input.\n\n"

       "The options below may be used to select which counts are printed, always in\n"
       "the following order: newline, word, character, byte, maximum line length.\n");

  // find longest option string
  int maxlen = 0;
  for (int i = 0; help_entries[i].opt; i++)
  {
    int len = (int)strlen(help_entries[i].opt);
    if (len > maxlen)
      maxlen = len;
  }

  // print each option aligned
  for (int i = 0; help_entries[i].opt; i++)
  {
    printf("  %-*s  %s\n", maxlen, help_entries[i].opt, help_entries[i].desc);
  }
}

void print_version()
{
  printf("%s (%s) %s\n", PROGRAM_NAME, PROJECT_NAME, VERSION);
  printf("Copyright (C) 2025 %s\n", AUTHORS);
  puts("License GPLv3+: GNU GPL version 3 or later "
       "<https://gnu.org/licenses/gpl.html>.\n"
       "This is free software: you are free to change and redistribute it.\n"
       "There is NO WARRANTY, to the extent permitted by law.\n");
  printf("Written by %s\n", AUTHORS);
}

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
  size_t lines, words, chars, bytes, maxlen;
};

/*
print_to_var() IS NOT SAFE FOR CONCATENATING RAW INPUT
Do NOT use print_to_var() for input. Ever.

- ChatGPT, 2025

it's weird and quirky i know, and im keeping this as a relic

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
*/

static inline bool is_word_seperator(wchar_t wc) {
  if (iswspace(wc))
    return true;

  switch (wc) {
  case 0x00A0:
  case 0x2007:
  case 0x202F:
  case 0x2060:
    return true;
  default:
    return false;
  }
}

// i am declaring that i wrote the maxlen part correctly and the GNU people didnt!!
// (~66 diff in a 75 million long file is crazy tho)
struct wc count_word_fd(int fd) {
  const size_t BUF_SZ = 524288;
  unsigned char *buf = malloc(BUF_SZ);
  if (!buf) {
    fprintf(stderr, "wc: %s\n", strerror(errno));
    exit(1);
  }

  ssize_t r; // renamed for better readability, for my future self
  size_t bytesread = 0;
  size_t lines = 0, words = 0, chars = 0, maxlen = 0, curlen = 0;
  bool in_word = false;

  while ((r = read(fd, buf, BUF_SZ)) > 0) {
    bytesread += r;

    // single pass processing
    for (size_t i = 0; i < (size_t)r;) {
      unsigned char c = buf[i];

      if (c < 0x80) {
        chars++;
        i++;

        // ascii lane
        if (c == '\n') {
          lines++;
          if (curlen > maxlen)
            maxlen = curlen;
          curlen = 0;
          in_word = false;
        } else if (c == '\t') {
          curlen += 8 - (curlen % 8);
          in_word = false;
        } else {
          curlen++;
          if (c <= ' ')
            in_word = false;
          else if (!in_word) {
            words++;
            in_word = true;
          }
        }
      } else { // utf-8 lane
        int len = 1;
        wchar_t wc = 0;

        mbstate_t state;
        memset(&state, 0, sizeof(state));
        size_t result = mbrtowc(&wc, (char *)(buf + i), (size_t)r - i, &state);

        if (result == (size_t)-1 || result == (size_t)-2) {
          len = 1;
          wc = 0;
        } else {
          len = result;
        }

        chars++;
        i += len;

        if (wc == L'\n' || wc == L'\r') {
          if (wc == L'\n')
            lines++;
          if (curlen > maxlen)
            maxlen = curlen;
          curlen = 0;
          in_word = false;
        } else if (is_word_seperator(wc)) {
          if (iswprint(wc)) {
            int width = wcwidth(wc);
            if (width > 0)
              curlen += width;
          }
          in_word = false;
        } else {
          if (iswprint(wc)) {
            int width = wcwidth(wc);
            if (width > 0)
              curlen += width;
          }

          if (!in_word) {
            words++;
            in_word = true;
          }
        }
      }
    }
  }

  if (curlen > maxlen)
    maxlen = curlen;

  free(buf);

  struct wc billy; // willy's twin brother
  billy.lines = lines;
  billy.words = words;
  billy.chars = chars;
  billy.bytes = bytesread;
  billy.maxlen = maxlen;
  return billy;
}

struct wc count_word_mmap(int fd, size_t file_size) {
  void *data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data == MAP_FAILED) {
    fprintf(stderr, "wc: %s\n", strerror(errno));
    return count_word_fd(fd);
  }

  // oh dear kernel...
  madvise(data, file_size, MADV_SEQUENTIAL);

  unsigned char *buf = (unsigned char *)data;
  size_t lines = 0, words = 0, chars = 0, maxlen = 0, curlen = 0;
  bool in_word = false;

  mbstate_t state;
  memset(&state, 0, sizeof(state));

  for (size_t i = 0; i < file_size;) {
    unsigned char c = buf[i];

    // ascii lane
    if (c < 0x80) {
      chars++;
      i++;

      if (c == '\n') {
        lines++;
        if (curlen > maxlen)
          maxlen = curlen;
        curlen = 0;
        in_word = false;
      } else if (c == '\t') {
        curlen += 8 - (curlen % 8);
        in_word = false;
      } else {
        curlen++;
        if (c <= ' ') {
          in_word = false;
        } else if (!in_word) {
          words++;
          in_word = true;
        }
      }
    } else { // utf8 lane
      int len = 1;
      wchar_t wc = 0;

      size_t result = mbrtowc(&wc, (char *)(buf + i), file_size - i, &state);

      if (result == (size_t)-1 || result == (size_t)-2) {
        len = 1;
        wc = 0;
      } else {
        len = result;
      }

      chars++;
      i += len;

      if (wc == L'\n' || wc == L'\r') {
        if (wc == L'\n')
          lines++;
        if (curlen > maxlen)
          maxlen = curlen;
        curlen = 0;
        in_word = false;
      } else if (is_word_seperator(wc)) {
        if (iswprint(wc)) {
          int width = wcwidth(wc);
          if (width > 0)
            curlen += width;
        }
        in_word = false;
      } else {
        if (iswprint(wc)) {
          int width = wcwidth(wc);
          if (width > 0)
            curlen += width;
        }

        if (!in_word) {
          words++;
          in_word = true;
        }
      }
    }
  }

  if (curlen > maxlen)
    maxlen = curlen;

  munmap(data, file_size);

  struct wc willy;
  willy.lines = lines;
  willy.words = words;
  willy.chars = chars;
  willy.bytes = file_size;
  willy.maxlen = maxlen;
  return willy;
}

struct wc cw_wrapper(int fd) {
  struct stat st;
  if (fstat(fd, &st) == -1) {
    fprintf(stderr, "wc: %s\n", strerror(errno));
    return count_word_fd(fd);
  }

  if (S_ISREG(st.st_mode) && st.st_size > 65536) {
    return count_word_mmap(fd, st.st_size);
  } else {
    return count_word_fd(fd);
  }
}

int num_width(long x) {
  int width = 0;
  if (x <= 0) {
    width = 1;
    x = -x;
  }
  while (x > 0) {
    width++;
    x /= 10;
  }
  return width;
}

// why do wc from stdin and file formats differently wtf!!
void print_results(uint8_t flags, char *name, struct wc willer,
                   bool from_stdin) {
  bool first_print = true;

  int w1 = num_width(willer.lines);
  int w2 = num_width(willer.words);
  int w3 = num_width(willer.chars);
  int w4 = num_width(willer.bytes);
  int w5 = num_width(willer.maxlen);

  int width = 0;
  if (w1 > width && flags & P_LINES)
    width = w1;
  if (w2 > width && flags & P_WORDS)
    width = w2;
  if (w3 > width && flags & P_CHARS)
    width = w3;
  if (w4 > width && flags & P_BYTES)
    width = w4;
  if (w5 > width && flags & P_LENMX)
    width = w5;

  if (flags & P_DEFAULT) {
    printf("%7ld %7ld %7ld", willer.lines, willer.words, willer.bytes);
  }
  if (flags & P_LINES) {
    if (!from_stdin)
      if (!first_print) {
        printf("%*ld", width, willer.lines);
        first_print = false;
      } else {
        printf(" %*ld", width, willer.lines);
      }
    else
      printf("%7ld ", willer.lines);
  }
  if (flags & P_WORDS) {
    if (!from_stdin)
      if (!first_print) {
        printf("%*ld", width, willer.words);
        first_print = false;
      } else {
        printf(" %*ld", width, willer.words);
      }
    else
      printf("%7ld ", willer.words);
  }
  if (flags & P_CHARS) {
    if (!from_stdin)
      if (!first_print) {
        printf("%*ld", width, willer.chars);
        first_print = false;
      } else {
        printf(" %*ld", width, willer.chars);
      }
    else
      printf("%7ld ", willer.chars);
  }
  if (flags & P_BYTES) {
    if (!from_stdin)
      if (!first_print) {
        printf("%*ld", width, willer.bytes);
        first_print = false;
      } else {
        printf(" %*ld", width, willer.bytes);
      }
    else
      printf("%7ld ", willer.bytes);
  }
  if (flags & P_LENMX) {
    if (!from_stdin)
      if (!first_print) {
        printf("%*ld", width, willer.maxlen);
        first_print = false;
      } else {
        printf(" %*ld", width, willer.maxlen);
      }
    else
      printf("%7ld ", willer.maxlen);
  }

  if (!from_stdin && name != NULL && name[0] != '\0') {
    printf(" %s\n", name);
  } else {
    printf("\n");
  }
}

// great creativity! such a manificient name! what an unbelievable thinking
// behind naming this function! /s
void process_the_fucking_struct(int fd, struct wc williams[], int length,
                                int *count) {
  struct wc willer;
  willer = cw_wrapper(fd);

  if (*count < length) {
    williams[*count] = willer;
    (*count)++;
  }
}

int main(int argc, char *argv[]) {
  // set to user's locale
  setlocale(LC_CTYPE, "");

  // i dont trust gcc.. at all...
  uint8_t flags = 0;
  uint8_t when = 0;

  int files0_from_fd = 0;
  bool files0_from_stdin = false;

  int opt;
  while ((opt = getopt_long(argc, argv, "cmlLw", long_options, NULL)) != -1) {
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
    case 3:;
      size_t optarg_len = strlen(optarg);
      if (strncasecmp(optarg, "auto", optarg_len) == 0) {
        when |= T_AUTO;
      } else if (strncasecmp(optarg, "always", optarg_len) == 0) {
        when |= T_ALWY;
      } else if (strncasecmp(optarg, "only", optarg_len) == 0) {
        when |= T_ONLY;
      } else if (strncasecmp(optarg, "never", optarg_len) == 0) {
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
    case 4:;
      size_t optarglen = strlen(optarg);
      if (strncmp(optarg, "-", optarglen) == 0) {
        files0_from_stdin = true;
      } else if ((files0_from_fd = open(optarg, O_RDONLY)) == -1) {
        fprintf(stderr, "wc: %s: %s", optarg, strerror(errno));
        return 1;
      }
      break;
    case 1:
      print_help(argv[0]);
      return 0;
    case 2:
      print_version();
      return 0;
    case '?':
      fprintf(stderr, "Try '%s --help' for more information\n", argv[0]);
      return 1;
    }
  }

  if (flags == 0) {
    flags = P_DEFAULT;
  }
  if (when == 0) {
    when = T_AUTO;
  }

  struct wc williams[50];
  char names[255][1024];
  int count = 0;
  bool from_stdin = false;

  if (files0_from_fd != 0 || files0_from_stdin != false) {
    char *list[255];
    char buf[12288];
    ssize_t bytes = 0;
    size_t buf_used = 0;
    size_t name_count = 0;

    if (files0_from_stdin) {
      while ((bytes = read(STDIN_FILENO, buf + buf_used, sizeof(buf) - buf_used)) > 0) {
        buf_used += bytes;
        size_t start = 0;
        for (size_t i = 0; i < buf_used; i++) {
          if (buf[i] == '\0') {
            size_t len = i - start;
            if (len > 0) {
              list[name_count] = strndup(&buf[start], len);
              name_count++;
            }
            start = i + 1;
          }
        }

        if (start < buf_used) {
          memmove(buf, buf + start, buf_used - start);
          buf_used -= start;
        } else {
          buf_used = 0;
        }
      }
    } else {
      while ((bytes = read(files0_from_fd, buf + buf_used, sizeof(buf) - buf_used)) > 0) {
        buf_used += bytes;
        size_t start = 0;
        for (size_t i = 0; i < buf_used; i++) {
          if (buf[i] == '\0') {
            size_t len = i - start;
            if (len > 0)
            {
              list[name_count] = strndup(&buf[start], len);
              name_count++;
            }
            start = i + 1;
          }
        }

        if (start < buf_used) {
          memmove(buf, buf + start, buf_used - start);
          buf_used -= start;
        } else {
          buf_used = 0;
        }
      }
    }

    if (buf_used > 0) {
      // handle last entry if not null-terminated
      list[name_count] = strndup(buf, buf_used);
      name_count++;
    }

    if (bytes == -1)
    {
      fprintf(stderr, "wc: %s\n", strerror(errno));
      close(files0_from_fd);
      return 1;
    }

    if (!files0_from_stdin)
      close(files0_from_fd);

    for (size_t i = 0; i < name_count; i++) {
      int fd = open(list[i], O_RDONLY | O_CLOEXEC);
      if (fd == -1) {
        fprintf(stderr, "%s: %s: %s\n", argv[0], list[i], strerror(errno));
        return 1;
      }
      strncpy(names[count], list[i], sizeof(names[count]));
      process_the_fucking_struct(
          fd, williams, sizeof(williams) / sizeof(williams[0]), &count);
      close(fd);
      free(list[i]);// free the strdup'd memory
    }
  } else if (argc == optind) {
  do_stdin:;
    from_stdin = true;
    process_the_fucking_struct(STDIN_FILENO, williams,
                               sizeof(williams) / sizeof(williams[0]), &count);
    strcpy(names[count - 1], "");
  } else {
    if (argv[optind][0] == '-')
      goto do_stdin;
    else if (argv[optind][0] == '\0') {
      fprintf(stderr, "wc: invalid zero-length file name\n");
      return 1;
    }
    for (; optind < argc; optind++) {
      int fd = open(argv[optind], O_RDONLY | O_CLOEXEC);
      if (fd == -1) {
        fprintf(stderr, "%s: %s: %s\n", argv[0], argv[optind], strerror(errno));
        return 1;
      }
      strncpy(names[count], argv[optind], sizeof(names[count]));
      process_the_fucking_struct(
          fd, williams, sizeof(williams) / sizeof(williams[0]), &count);
      close(fd);
    }
  }

  if (count == 1) {
    print_results(flags, names[0], williams[0], from_stdin);
    if (when & T_ALWY)
      print_results(flags, "total", williams[0], from_stdin); // :troll:
  } else if (count > 1) {
    struct wc final = {0};
    size_t previous_len = 0;
    for (int i = 0; i < count; i++) {
      final.bytes += williams[i].bytes;
      final.chars += williams[i].chars;
      final.lines += williams[i].lines;
      // wouldve done tenary if i didnt have to set the previous len
      if (williams[i].maxlen > previous_len) {
        final.maxlen = williams[i].maxlen;
        previous_len = final.maxlen;
      } else {
        final.maxlen = previous_len;
      }
      final.words += williams[i].words;
    }

    char totalbuf[256];
    size_t pos = 0;

    int w1 = num_width(final.lines);
    int w2 = num_width(final.words);
    int w3 = num_width(final.chars);
    int w4 = num_width(final.bytes);
    int w5 = num_width(final.maxlen);

    int width = 0;
    if (w1 > width && flags & P_LINES)
      width = w1;
    if (w2 > width && flags & P_WORDS)
      width = w2;
    if (w3 > width && flags & P_CHARS)
      width = w3;
    if (w4 > width && flags & P_BYTES)
      width = w4;
    if (w5 > width && flags & P_LENMX)
      width = w5;

    if (flags & P_LINES)
      pos +=
          snprintf(totalbuf + pos, sizeof(totalbuf) - pos, "%*ld ", width, final.lines);
    if (flags & P_WORDS)
      pos +=
          snprintf(totalbuf + pos, sizeof(totalbuf) - pos, "%*ld ", width, final.words);
    if (flags & P_CHARS)
      pos +=
          snprintf(totalbuf + pos, sizeof(totalbuf) - pos, "%*ld ", width, final.chars);
    if (flags & P_BYTES)
      pos +=
          snprintf(totalbuf + pos, sizeof(totalbuf) - pos, "%*ld ", width, final.bytes);
    if (flags & P_LENMX)
      pos += snprintf(totalbuf + pos, sizeof(totalbuf) - pos, "%*ld ", width,
                      final.maxlen);
    if (flags & P_DEFAULT)
      pos += snprintf(totalbuf + pos, sizeof(totalbuf) - pos, "%7ld %7ld %7ld",
                      final.lines, final.words, final.bytes);

    totalbuf[pos] = 0;

    if (when & T_ONLY) {
      puts(totalbuf);
      return 0;
    }

    for (int i = 0; i < count; i++) {
      print_results(flags, names[i], williams[i], from_stdin);
    }
    if (when & T_AUTO || flags & T_ALWY) {
      printf("%s total\n", totalbuf);
    }
  }

  return 0;
}
