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
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PROGRAM_NAME "base64"
#define PROJECT_NAME "coreutils from scratch"
#define AUTHORS "Horstaufmental"
#define VERSION "1.0"

struct help_entry {
  const char *opt;
  const char *desc;
};

static struct option long_options[] = {
    {"help", no_argument, 0, 1},
    {"version", no_argument, 0, 2},
    {"decode", no_argument, 0, 'd'},
    {"ignore-garbage", no_argument, 0, 'i'},
    {"wrap", required_argument, 0, 'w'},
};

static struct help_entry help_entries[] = {  
    {"-d, --decode", "decode data"},
    {"-i, --ignore-garbage", "when decoding, ignore non-alphabet characters"},
    {"-w, --wrap=COLS",
     "wrap encoded lines after COLS character (default 76).\n"
     "                     Use 0 to disable line wrapping"},
    {"    --help", "display this help and exit"},
    {"    --version", "output version information and exit"},
    {NULL, NULL}};

void print_help(const char *name) {
  printf("Usage: %s [OPTION]... [FILE]\n", name);
  printf("basenc encode or decode FILE, or standard input, to standard "
         "output.\n\n");

  printf("With no FILE, or when FILE is -, read standard input.\n\n"
         "Mandatory arguments to long options are mandatory for short options "
         "too.\n");

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
  fputs("\nWhen decoding, the input may contain newlines in addition to the "
        "bytes of\n"
        "the formal alphabet. Use --ignore-garbage to attempt to recover\n"
        "from any other non-alphabet bytes in the encoded stream.\n",
        stdout);
}

void print_version() {
  printf("%s (%s) %s\n", PROGRAM_NAME, PROJECT_NAME, VERSION);
  printf("Copyright (C) 2025 %s\n", AUTHORS);
  puts("License GPLv3+: GNU GPL version 3 or later "
       "<https://gnu.org/licenses/gpl.html>.\n"
       "This is free software: you are free to change and redistribute it.\n"
       "There is NO WARRANTY, to the extent permitted by law.\n");
  printf("Written by %s\n", AUTHORS);
}

static const char base64_encoding_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static char base64_decoding_table[256];

void init_base64_decoding_table(void) {
  memset(base64_decoding_table, 0x80, 256);
  for (int i = 0; i < 64; ++i) {
    base64_decoding_table[(unsigned char)base64_encoding_table[i]] = i;
  }
}

char *base64_encode(const unsigned char *data, size_t input_length,
                    size_t *output_length) {
  *output_length = 4 * ((input_length + 2) / 3);
  char *encoded_data = (char *)malloc(*output_length + 1);
  if (!encoded_data) {
    fprintf(stderr, "base64: failed to encode data: %s\n", strerror(errno));
    return NULL;
  }

  size_t i, j;
  for (i = 0, j = 0; i < input_length;) {
    uint32_t octet_a = i < input_length ? data[i++] : 0;
    uint32_t octet_b = i < input_length ? data[i++] : 0;
    uint32_t octet_c = i < input_length ? data[i++] : 0;

    uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

    encoded_data[j++] = base64_encoding_table[(triple >> 3 * 6) & 0x3F];
    encoded_data[j++] = base64_encoding_table[(triple >> 2 * 6) & 0x3F];
    encoded_data[j++] = base64_encoding_table[(triple >> 1 * 6) & 0x3F];
    encoded_data[j++] = base64_encoding_table[(triple >> 0 * 6) & 0x3F];
  }

  int mod = input_length % 3;
  if (mod == 1) {
    encoded_data[*output_length - 2] = '=';
    encoded_data[*output_length - 1] = '=';
  } else if (mod == 2) {
    encoded_data[*output_length - 1] = '=';
  }

  encoded_data[*output_length] = '\0';
  return encoded_data;
}

unsigned char *base64_decode(const char *data, size_t input_length,
                             size_t *output_length) {
  if (input_length % 4 != 0) {
    fputs("base64: invalid input\n", stderr);
    return NULL;
  }

  *output_length = input_length / 4 * 3;

  if (data[input_length - 1] == '=')
    (*output_length)--;
  if (data[input_length - 2] == '=')
    (*output_length)--;

  unsigned char *decoded_data = malloc(*output_length);
  if (!decoded_data) {
    fprintf(stderr, "base64: failed to decode data: %s\n", strerror(errno));
    return NULL;
  }

  size_t i, j;
  for (i = 0, j = 0; i < input_length;) {
    uint32_t sextet_a =
        data[i] == '=' ? (uint32_t)0 & i++ : (uint32_t)base64_decoding_table[(unsigned char)data[i++]];
    uint32_t sextet_b =
        data[i] == '=' ? (uint32_t)0 & i++ : (uint32_t)base64_decoding_table[(unsigned char)data[i++]];
    uint32_t sextet_c =
        data[i] == '=' ? (uint32_t)0 & i++ : (uint32_t)base64_decoding_table[(unsigned char)data[i++]];
    uint32_t sextet_d =
        data[i] == '=' ? (uint32_t)0 & i++ : (uint32_t)base64_decoding_table[(unsigned char)data[i++]];

    uint32_t triple = (sextet_a << 3 * 6) | (sextet_b << 2 * 6) |
                      (sextet_c << 1 * 6) | (sextet_d << 0 * 6);

    if (j < *output_length)
      decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
    if (j < *output_length)
      decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
    if (j < *output_length)
      decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
  }

  return decoded_data;
}

void print_wrap(char *data, int wrap) {
  int count = 0;

  for (int i = 0; data[i] != '\0'; i++) {
    putchar(data[i]);
    count++;
    if (wrap > 0 && count >= wrap) {
      putchar('\n');
      count = 0;
    }
  }
}

void clean_garbage(char *data, size_t *input_length) {
  size_t j = 0;
  for (size_t i = 0; i < *input_length; i++) {
    unsigned char c = (unsigned char)data[i];
    bool valid = false;

    valid = (base64_decoding_table[c] & 0x80) == 0;

    if (valid) {
      data[j++] = data[i];
    }
  }
  *input_length = j;
}

int main(int argc, char *argv[]) {
  bool ignore_garbage = false;
  bool decode = false;
  int wrap = 76;

  int opt;
  while ((opt = getopt_long(argc, argv, "diw:", long_options, NULL)) != -1) {
    switch (opt) {
    case 1:
      print_help(argv[0]);
      return 0;
    case 2:
      print_version();
      return 0;
    case 'd':
      decode = true;
      break;
    case 'i':
      ignore_garbage = true;
      break;
    case 'w':;
      char *endptr;
      wrap = (int)strtol(optarg, &endptr, 10);
      if (endptr == optarg) {
        fprintf(stderr,
                "base64: invalid wrap size: '%s'\n"
                "Try '%s --help' for more information.\n",
                optarg, argv[1]);
        return 1;
      }
      break;
    default:
      fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
      return 1;
    }
  }

  if (argc == optind) {
  stdin_mode:;
    char buffer[8192];
    size_t bytes_read;
    while ((bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
      size_t output_length;
      if (decode) {
        if (ignore_garbage) {
          clean_garbage(buffer, &bytes_read);
        }
        unsigned char *result =
            base64_decode(buffer, bytes_read, &output_length);
        if (!result) {
          return 1;
        }
        fputs((char *)result, stdout);
        free(result);
      } else {
        char *result = base64_encode((unsigned char *)buffer, bytes_read,
                                      &output_length);
        if (!result) {
          return 1;
        }
        print_wrap(result, wrap);
        free(result);
      }
    }
    if (bytes_read == (size_t)-1) {
      fprintf(stderr, "base64: failed to read from stdin: %s\n",
              strerror(errno));
      return 1;
    }
    if (optind < argc) {
      optind++;
      goto fd_mode;
    }
  } else {
    for (; optind < argc; optind++) {
    fd_mode:;
      if (strcmp(argv[optind], "-") == 0) {
        goto stdin_mode;
      }
      int fd = open(argv[optind], O_RDONLY);
      if (fd == -1) {
        fprintf(stderr, "base64: failed to open file '%s': %s\n", argv[optind],
                strerror(errno));
        return 1;
      }
      char buffer[8192];
      size_t bytes_read;
      while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        size_t output_length;
        if (decode) {
          if (ignore_garbage) {
            clean_garbage(buffer, &bytes_read);
          }
          unsigned char *result =
              base64_decode(buffer, bytes_read, &output_length);
          if (!result) {
            close(fd);
            return 1;
          }
          fputs((char *)result, stdout);
          free(result);
        } else {
          char *result = base64_encode((unsigned char *)buffer,
                                        bytes_read, &output_length);
          if (!result) {
            close(fd);
            return 1;
          }
          print_wrap(result, wrap);
          free(result);
        }
      }
      if (bytes_read == (size_t)-1) {
        fprintf(stderr, "base64: failed to read from file '%s': %s\n",
                argv[optind], strerror(errno));
        close(fd);
        return 1;
      }
      close(fd);
    }
  }

  return 0;
}
