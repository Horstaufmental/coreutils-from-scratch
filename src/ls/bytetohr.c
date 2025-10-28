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
#include "bytetohr.h"
#include <stdio.h>

void byteToHR(long long bytes, char *buffer, int bufferSize)
{
    const char *units[] = {"B", "K", "M", "G", "T", "P", "E"};
    long unsigned int uIndx = 0;
    double value = (double)bytes;

    while (value >= 1024 && uIndx < sizeof(units) / sizeof(units[0]) - 1)
    {
        value /= 1024;
        uIndx++;
    }
    snprintf(buffer, bufferSize, "%.1f%s", value, units[uIndx]);
}
