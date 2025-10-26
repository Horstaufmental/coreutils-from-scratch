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
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "print_help.h"

void print_help(const char *name)
{
    printf("Usage: %s [OPTION]... DIRECTORY...\n", name);
    printf("Create the DIRECTORY(ies), if they do not already exist.\n\n");
    printf("Mandatory arguments to long options are mandatory for short options too.\n");

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
