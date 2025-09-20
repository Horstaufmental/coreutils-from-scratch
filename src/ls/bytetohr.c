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