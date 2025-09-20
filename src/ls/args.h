#ifndef ARGS_H
#define ARGS_H

#include <getopt.h>

#define true 1
#define false 0

int includeALL = false;
int includeALLshort = false;
int longFormat = false;
int humanReadable = false;

static struct option long_options[] = {
    {"all", no_argument, 0, 'a'},
    {"almost-all", no_argument, 0, 'A'},
    {"help", no_argument, 0, 1},
    {0, no_argument, 0, 'l'},
    {0, 0, 0, 0}
};

#endif