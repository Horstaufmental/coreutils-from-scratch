#include <bits/types/struct_iovec.h>
#include <sys/mman.h>
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
#include <unistd.h>
#include <wchar.h>
#include <sys/stat.h>

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

struct help_entry
{
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

struct wc
{
  size_t lines, words, chars, bytes, maxlen;
};

/*
print_to_var() IS NOT SAFE FOR CONCATENATING RAW INPUT
Do NOT use print_to_var() for input. Ever.

- ChatGPT, 2025

it's weird and quirky i know, and im keeping this as a relic of my stupidity

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

struct wc count_word_fd(int fd)
{
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

  while ((r = read(fd, buf, sizeof(BUF_SZ))) > 0)
  {
    bytesread += r;

    // single pass processing
    for (size_t i = 0; i < (size_t)r; ) {
      unsigned char c = buf[i];

      if (c < 0x80) {
        chars++;
        i++;

        // ascii lane
        if (c == '\n') {
          lines++;
          if (curlen > maxlen) maxlen = curlen;
          curlen = 0;
          in_word = true;
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
        if ((c & 0xE0) == 0xC0) len = 2;
        else if ((c & 0xF0) == 0xE0) len = 3;
        else if ((c & 0xF8) == 0xF0) len = 4;

        if (i + len > (size_t)r) {
          len = 1;
        }

        chars++;
        curlen++;
        i += len;

        if (!in_word) {
          words++;
          in_word = true;
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

  for (size_t i = 0; i < file_size; ) {
    unsigned char c = buf[i];

    // ascii lane
    if (c < 0x80) {
      chars++;
      i++;

      if (c == '\n') {
        lines++;
        if (curlen > maxlen) maxlen = curlen;
        curlen = 0;
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
      if ((c & 0xE0) == 0xC0) len = 2;
      else if ((c & 0xF0) == 0xE0) len = 3;
      else if ((c & 0xF8) == 0xF0) len = 4;

      if (i + len > file_size)
        len = 1;

      chars++;
      curlen++;
      i += len;

      if (!in_word) {
        words++;
        in_word = true;
      }
    }
  }

  if (curlen > maxlen) maxlen = curlen;

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

// why do wc from stdin and file formats differently wtf!!
void print_results(uint8_t flags, char *name, struct wc willer, bool from_stdin)
{
  bool first_print = true;

  if (flags & P_DEFAULT)
  {
    printf("%6ld %7ld %7ld", willer.lines, willer.words, willer.bytes);
  }
  if (flags & P_LINES)
  {
    if (!from_stdin)
      if (!first_print)
      {
        printf("%ld", willer.lines);
        first_print = false;
      }
      else
      {
        printf(" %ld", willer.lines);
      }
    else
      printf("%ld ", willer.lines);
  }
  if (flags & P_WORDS)
  {
    if (!from_stdin)
      if (!first_print)
      {
        printf("%ld", willer.words);
        first_print = false;
      }
      else
      {
        printf(" %ld", willer.words);
      }
    else
      printf("%6ld ", willer.words);
  }
  if (flags & P_CHARS)
  {
    if (!from_stdin)
      if (!first_print)
      {
        printf("%ld", willer.chars);
        first_print = false;
      }
      else
      {
        printf(" %ld", willer.chars);
      }
    else
      printf("%6ld ", willer.chars);
  }
  if (flags & P_BYTES)
  {
    if (!from_stdin)
      if (!first_print)
      {
        printf("%ld", willer.bytes);
        first_print = false;
      }
      else
      {
        printf(" %ld", willer.bytes);
      }
    else
      printf("%6ld ", willer.bytes);
  }
  if (flags & P_LENMX)
  {
    if (!from_stdin)
      if (!first_print)
      {
        printf("%ld", willer.maxlen);
        first_print = false;
      }
      else
      {
        printf(" %ld", willer.maxlen);
      }
    else
      printf("%6ld ", willer.maxlen);
  }

  if (!from_stdin && name != NULL && name[0] != '\0')
  {
    printf(" %s\n", name);
  }
  else
  {
    printf("\n");
  }
}

// great creativity! such a manificient name! what an unbelievable thinking behind
// naming this function! /s
void process_the_fucking_struct(int fd, struct wc williams[], int length, int *count)
{
  struct wc willer;
  willer = cw_wrapper(fd);

  if (*count < length)
  {
    williams[*count] = willer;
    (*count)++;
  }
}

int main(int argc, char *argv[])
{
  // set to user's locale
  setlocale(LC_CTYPE, "");

  // i dont trust gcc.. at all...
  uint8_t flags = 0;
  uint8_t when = 0;

  int opt;
  while ((opt = getopt_long(argc, argv, "cmlLw", long_options, NULL)) != -1)
  {
    switch (opt)
    {
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
      if (strncasecmp(optarg, "auto", optarg_len) == 0)
      {
        when |= T_AUTO;
      }
      else if (strncasecmp(optarg, "always", optarg_len) == 0)
      {
        when |= T_ALWY;
      }
      else if (strncasecmp(optarg, "only", optarg_len) == 0)
      {
        when |= T_ONLY;
      }
      else if (strncasecmp(optarg, "never", optarg_len) == 0)
      {
        when |= T_NEVR;
      }
      else
      {
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

  if (flags == 0)
  {
    flags = P_DEFAULT;
  }
  if (when == 0)
  {
    when = T_AUTO;
  }

  struct wc williams[50];
  char names[50][1024];
  int count = 0;
  bool from_stdin = false;
  if (argc == optind)
  {
  do_stdin:;
    from_stdin = true;
    process_the_fucking_struct(STDIN_FILENO, williams, sizeof(williams) / sizeof(williams[0]), &count);
    strcpy(names[count - 1], "");
  }
  else
  {
    if (argv[optind][0] == '-')
      goto do_stdin;
    else if (argv[optind][0] == '\0')
    {
      fprintf(stderr, "wc: invalid zero-length file name\n");
      return 1;
    }
    for (; optind < argc; optind++)
    {
      int fd = open(argv[optind], O_RDONLY | O_CLOEXEC);
      if (fd == -1)
      {
        fprintf(stderr, "%s: %s: %s\n", argv[0], argv[optind], strerror(errno));
        return 1;
      }
      strncpy(names[count], argv[optind], sizeof(names[count]));
      process_the_fucking_struct(fd, williams, sizeof(williams) / sizeof(williams[0]), &count);
      close(fd);
    }
  }

  if (count == 1)
  {
    print_results(flags, names[0], williams[0], from_stdin);
    if (when & T_ALWY)
      print_results(flags, "total", williams[0], from_stdin); // :troll:
  }
  else if (count > 1)
  {
    struct wc final = {0};
    size_t previous_len = 0;
    for (int i = 0; i < count; i++)
    {
      final.bytes += williams[i].bytes;
      final.chars += williams[i].chars;
      final.lines += williams[i].lines;
      // wouldve done tenary if i didnt have to set the previous len
      if (williams[i].maxlen > previous_len)
      {
        final.maxlen = williams[i].maxlen;
        previous_len = final.maxlen;
      }
      else
      {
        final.maxlen = previous_len;
      }
      final.words += williams[i].words;
    }

    char totalbuf[256];
    size_t pos = 0;

    if (flags & P_LINES)
      pos += snprintf(totalbuf + pos, sizeof(totalbuf) - pos, "%ld ", final.lines);
    if (flags & P_WORDS)
      pos += snprintf(totalbuf + pos, sizeof(totalbuf) - pos, "%ld ", final.words);
    if (flags & P_CHARS)
      pos += snprintf(totalbuf + pos, sizeof(totalbuf) - pos, "%ld ", final.chars);
    if (flags & P_BYTES)
      pos += snprintf(totalbuf + pos, sizeof(totalbuf) - pos, "%ld ", final.bytes);
    if (flags & P_LENMX)
      pos += snprintf(totalbuf + pos, sizeof(totalbuf) - pos, "%ld ", final.maxlen);
    if (flags & P_DEFAULT)
      pos += snprintf(totalbuf + pos, sizeof(totalbuf) - pos, "%ld %ld %ld",
                      final.lines, final.words, final.bytes);
                  
    totalbuf[pos] = 0;

    if (when & T_ONLY)
    {
      puts(totalbuf);
      return 0;
    }

    for (int i = 0; i < count; i++)
    {
      print_results(flags, names[i], williams[i], from_stdin);
    }
    if (when & T_AUTO || flags & T_ALWY)
    {
      printf("%s total\n", totalbuf);
    }
  }

  return 0;
}
