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
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "decoders.h"
#include "encoders.h"

#define PROGRAM_NAME "basenc"
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
    {"base64", no_argument, 0, 3},
    {"base64url", no_argument, 0, 4},
    {"base58", no_argument, 0, 5},
    {"base32", no_argument, 0, 6},
    {"base32hex", no_argument, 0, 7},
    {"base16", no_argument, 0, 8},
    {"base2msbf", no_argument, 0, 9},
    {"base2lsbf", no_argument, 0, 10},
    {"decode", no_argument, 0, 'd'},
    {"ignore-garbage", no_argument, 0, 'i'},
    {"wrap", required_argument, 0, 'w'},
    {"z85", no_argument, 0, 11},
};

static struct help_entry help_entries[] = {
  {"    --base64", "same as 'base64' program (RFC4648 section 4)"},
  {"    --base64url", "file- and url-safe base64 (RFC4648 section 5)"},
  {"    --base58", "visually unambiguous base58 encoding"},
  {"    --base32", "same as 'base32' program (RFC4648 section 6)"},
  {"    --base32hex", "extended hex alphabet base32 (RFC4648 section 7)"},
  {"    --base16", "hex encoding (RFC4648 section 8)"},
  {"    --base2msbf", "bit string with most significant bit (msb) first"},
  {"    --base2lsbf", "bit string with least significant bit (lsb) first"},
  {"-d, --decode", "decode data"},
  {"-i, --ignore-garbage", "when decoding, ignore non-alphabet characters"},
  {"-w, --wrap=COLS", "wrap encoded lines after COLS character (default 76).\n"
   "                     Use 0 to disable line wrapping"},
  {"    --z85", "ascii85-like encoding (ZeroMQ spec:32/Z85);\n"
   "             when encoding, input length must be multiple of 4;\n"
   "             when decoding, input length must be multiple of 5"},
  {"    --help", "display this help and exit"},
  {"   --version", "output version information and exit"},
  {NULL, NULL}
};

void print_help(const char *name)
{
  printf("Usage: %s [OPTION]... [FILE]\n", name);
  printf("basenc encode or decode FILE, or standard input, to standard output.\n\n");

  printf("With no FILE, or when FILE is -, read standard input.\n\n"
         "Mandatory arguments to long options are mandatory for short options too.\n");

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
  fputs("\nWhen decoding, the input may contain newlines in addition to the bytes of\n"
        "the formal alphabet. Use --ignore-garbage to attempt to recover\n"
        "from any other non-alphabet bytes in the encoded stream.\n",
        stdout);
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

unsigned char *decode_wrapper(unsigned int base, const char *data,
                              size_t input_length, size_t *output_length) {
  init_decode_table_wrapper(base);
  switch (base) {
  case B_64:
    return base64_decode(data, input_length, output_length);
  case B_64URL:
    return base64url_decode(data, input_length, output_length);
  case B_58:
    return base58_decode(data, input_length, output_length);
  case B_32:
    return base32_decode(data, input_length, output_length);
  case B_32HEX:
    return base32hex_decode(data, input_length, output_length);
  case B_16:
    return base16_decode(data, input_length, output_length);
  case B_2MSB:
    return base2msbf_decode(data, input_length, output_length);
  case B_2LSB:
    return base2lsbf_decode(data, input_length, output_length);
  case B_Z85:
    return z85_decode(data, input_length, output_length);
  }
  return NULL;
}

char *encode_wrapper(unsigned int base, const unsigned char *data,
                              size_t input_length, size_t *output_length) {
  switch (base) {
  case B_64:
    return base64_encode(data, input_length, output_length);
  case B_64URL:
    return base64url_encode(data, input_length, output_length);
  case B_58:
    return base58_encode(data, input_length, output_length);
  case B_32:
    return base32_encode(data, input_length, output_length);
  case B_32HEX:
    return base32hex_encode(data, input_length, output_length);
  case B_16:
    return base16_encode(data, input_length, output_length);
  case B_2MSB:
    return base2msbf_encode(data, input_length, output_length);
  case B_2LSB:
    return base2lsbf_encode(data, input_length, output_length);
  case B_Z85:
    return z85_encode(data, input_length, output_length);
  }
  return NULL;
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

void clean_garbage(char *data, size_t *input_length, unsigned int base) {
  size_t j = 0;
  for (size_t i = 0; i < *input_length; i++) {
    unsigned char c = (unsigned char)data[i];
    bool valid = false;

    if (base == B_64) {
      valid = (base64_decoding_table[c] & 0x80) == 0;
    } else if (base == B_64URL) {
      valid = (base64url_decoding_table[c] & 0x80) == 0;
    } else if (base == B_58) {
      valid = (base58_decoding_table[c] & 0x80) == 0;
    } else if (base == B_32) {
      valid = (base32_decoding_table[c] & 0x80) == 0 || c == '=';
    } else if (base == B_32HEX) {
      valid = (base32hex_decoding_table[c] & 0x80) == 0 || c == '=';
    } else if (base == B_16) {
      valid = (base16_decoding_table[c] & 0x80) == 0;
    } else if (base == B_Z85) {
      valid = (z85_decoding_table[c] != 0xFF);
    }

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
  unsigned int base = 0;

  int opt;
  while ((opt = getopt_long(argc, argv, "diw:", long_options, NULL)) != -1) {
    switch (opt) {
    case 1:
      print_help(argv[0]);
      return 0;
    case 2:
      print_version();
      return 0;
    case 3:
      base = B_64;
      break;
    case 4:
      base = B_64URL;
      break;
    case 5:
      base = B_58;
      break;
    case 6:
      base = B_32;
      break;
    case 7:
      base = B_32HEX;
      break;
    case 8:
      base = B_16;
      break;
    case 9:
      base = B_2MSB;
      break;
    case 10:
      base = B_2LSB;
      break;
    case 11:
      base = B_Z85;
      break;
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
                "basenc: invalid wrap size: '%s'\n"
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

  if (base == 0) {
    fprintf(stderr,
            "basenc: missing encoding type\n"
            "Try '%s --help' for more information.\n",
            argv[0]);
    return 1;
  }

  if (argc == optind) {
stdin_mode:;
    char buffer[8192];
    size_t bytes_read;
    while ((bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
      size_t output_length;
      if (decode) {
        if (ignore_garbage) {
          clean_garbage(buffer, &bytes_read, base);
        }
        unsigned char *result = decode_wrapper(base, buffer, bytes_read, &output_length);
        if (!result) {
          return 1;
        }
        fputs((char *)result, stdout);
        free(result);
      } else {
        char *result = encode_wrapper(base, (unsigned char *)buffer, bytes_read, &output_length);
        if (!result) {
          return 1;
        }
        print_wrap(result, wrap);
        free(result);
      }
    }
    if (bytes_read == (size_t)-1) {
      fprintf(stderr, "basenc: failed to read from stdin: %s\n",
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
        fprintf(stderr, "basenc: failed to open file '%s': %s\n", argv[optind],
                strerror(errno));
        return 1;
      }
      char buffer[8192];
      size_t bytes_read;
      while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        size_t output_length;
        if (decode) {
          if (ignore_garbage) {
            clean_garbage(buffer, &bytes_read, base);
          }
          unsigned char *result = decode_wrapper(base, buffer, bytes_read, &output_length);
          if (!result) {
            close(fd);
            return 1;
          }
          fputs((char *)result, stdout);
          free(result);
        } else {
          char *result = encode_wrapper(base, (unsigned char *)buffer, bytes_read, &output_length);
          if (!result) {
            close(fd);
            return 1;
          }
          print_wrap(result, wrap);
          free(result);
        }
      }
      if (bytes_read == (size_t)-1) {
        fprintf(stderr, "basenc: failed to read from file '%s': %s\n",
                argv[optind], strerror(errno));
        close(fd);
        return 1;
      }
      close(fd);
    }
  }

  return 0;
}
