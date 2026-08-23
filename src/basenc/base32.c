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

#include "encoders.h"
#include "decoders.h"
#include "meta.h"

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

    valid = (base32_decoding_table[c] & 0x80) == 0;

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
      {
        char buf[256];
        snprintf(buf, 256, "Usage: %s [OPTION].. [FILE]", argv[0]);
        print_help(buf, "Base32 encode or decode FILE, or standard input, to standard output.\n\n"
                   "With no FILE, or when FILE is -, read standard input.\n\n"
                   "Mandatory arguments to long options are mandatory for short options too.",
                   help_entries,
                   "The data are encoded as described for the base32 alphabet in RFC 4648.\n"
                   "When decoding, the input may contain newlines in addition to the bytes of\n"
                   "the format base32 alphabet.  Use --ignore-garbage to attempt to recover\n"
                   "from any other non-alphabet bytes in the encoded stream.");
        return 0;
      }
    case 2:
      print_version(PROGRAM_NAME, PROJECT_NAME, VERSION, AUTHORS);
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
                "base32: invalid wrap size: '%s'\n"
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

  if (decode)
    init_decode_table_wrapper(B_32);

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
            base32_decode(buffer, bytes_read, &output_length);
        if (!result) {
          return 1;
        }
        fputs((char *)result, stdout);
        free(result);
      } else {
        char *result =
            base32_encode((unsigned char *)buffer, bytes_read, &output_length);
        if (!result) {
          return 1;
        }
        print_wrap(result, wrap);
        free(result);
      }
    }
    if (bytes_read == (size_t)-1) {
      fprintf(stderr, "base32: failed to read from stdin: %s\n",
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
              base32_decode(buffer, bytes_read, &output_length);
          if (!result) {
            close(fd);
            return 1;
          }
          fputs((char *)result, stdout);
          free(result);
        } else {
          char *result = base32_encode((unsigned char *)buffer, bytes_read,
                                       &output_length);
          if (!result) {
            close(fd);
            return 1;
          }
          print_wrap(result, wrap);
          free(result);
        }
      }
      if (bytes_read == (size_t)-1) {
        fprintf(stderr, "base32: failed to read from file '%s': %s\n",
                argv[optind], strerror(errno));
        close(fd);
        return 1;
      }
      close(fd);
    }
  }

  return 0;
}
