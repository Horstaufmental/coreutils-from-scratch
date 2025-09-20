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