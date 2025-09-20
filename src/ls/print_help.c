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