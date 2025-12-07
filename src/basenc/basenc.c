#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>

#include "encoders.h"
#include "decoders.h"

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
