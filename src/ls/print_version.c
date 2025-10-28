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
#include <stdio.h>

#include "print_version.h"
#include "args.h"

void print_version() {
    printf("%s (%s) %s\n", PROGRAM_NAME, PROJECT_NAME, VERSION);
    printf("Copyright (C) 2025 %s\n", AUTHORS);
    puts("License GPLv3+: GNU GPL version 3 or later "
    "<https://gnu.org/licenses/gpl.html>.\n"
    "This is free software: you are free to change and redistribute it.\n"
    "There is NO WARRANTY, to the extent permitted by law.\n");
    printf("Written by %s\n", AUTHORS);
}
