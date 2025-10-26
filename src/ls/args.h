/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This file is part of coreutils from scratch.
 * Copyright (c) 2025 Horstaufmental
 *
 * coreutils from scratch is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * coreutils from scratch is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */
#ifndef ARGS_H
#define ARGS_H

#include <getopt.h>
#include <stdbool.h>

extern bool includeALL;
extern bool includeALLshort;
extern bool longFormat;
extern bool humanReadable;

static struct option long_options[] = {
    {"all", no_argument, 0, 'a'},
    {"almost-all", no_argument, 0, 'A'},
    {"help", no_argument, 0, 1},
    {0, no_argument, 0, 'l'},
    {0, 0, 0, 0}
};

#endif
