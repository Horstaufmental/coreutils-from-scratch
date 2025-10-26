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
#ifndef PRINTHELP_H
#define PRINTHELP_H

struct help_entry
{
    const char *opt;
    const char *desc;
};

static struct help_entry help_entries[] = {
    {"-a, --all", "show hidden and 'dot' files. Use this twice to also\n"
                  "              show the '.' and '..' directories"},
    {"-A, --almost-all", "equivalent to --all; included for compatibility with `ls -A`"},
    {"-h, --human-readable", "with -l, print sizes in human readable format (e.g., 1K 234M 2G)"},
    {"-l", "display extended file metadata as a table"},
    {"--help", "display this help and exit"}};

void print_help(const char *name);

#endif
