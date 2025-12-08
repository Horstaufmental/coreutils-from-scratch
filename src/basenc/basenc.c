#include <bits/types/struct_iovec.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "decoders.h"
#include "encoders.h"

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

int main(int argc, char *argv[]) {
  bool ignore_garbage = false;
  bool decode = false;
  int wrap = false;
  unsigned int base = 0;

  int opt;
  while ((opt = getopt_long(argc, argv, "diw:", long_options, NULL)) != -1) {
    switch (opt) {
    case 1:
      return 0;
    case 2:
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

  if (opt == 0) {
    fprintf(stderr,
            "basenc: missing encoding type\n"
            "Try '%s --help' for more information.\n",
            argv[0]);
    return 1;
  }

  if (argc == optind) {
    if (decode) {

    }
  }
}
