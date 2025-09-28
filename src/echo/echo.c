#include <linux/limits.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <ctype.h>

typedef enum {
    True = 1,
    False = 0
} Bool;

Bool noNewline = False;
Bool backslashEscapes = False;

struct help_entry {
    const char *opt;
    const char *desc;
};

static struct option long_options[] = {
    {"help", no_argument, 0, '1'},
    {0, no_argument, 0, 'n'},
    {0, no_argument, 0, 'e'},
    {0, no_argument, 0, 'E'},
    {0, 0, 0, 0}
};

static struct help_entry help_entries[] = {
    {"-n", "do not output the trailing newline"},
    {"-e", "enable interpretation of backslash escapes"},
    {"-E", "disable interpretation of backslash escapes (default)"},
    {"    --help", "display this help and exit"},
    {NULL, NULL} // sentinel
};

static struct help_entry backslash_entries[] = {
    {"\\\\", "backslash"},
    {"\\a", "alert (BEL)"},
    {"\\b", "backspace"},
    {"\\c", "produce no further output"},
    {"\\e", "escape"},
    {"\\f", "form feed"},
    {"\\n", "new line"},
    {"\\r", "carriage return"},
    {"\\t", "horizontal tab"},
    {"\\v", "vertical tab"},
    {"\\0NNN", "byte with octal value NNN (1 to 3 digits)"},
    {"\\xHH", "byte with hexadecimal value HH (1 to 2 digits)"},
    {NULL, NULL}
};

static int parse_escapes(const char *input, char *output, size_t out_size) {
    size_t j = 0;
    for (size_t i = 0; input[i] != '\0' && j + 1 < out_size; i++) {
        if (input[i] == '\\') {
            i++;
            switch (input[i]) {
                case 'n': output[j++] = '\n'; break;
                case 't': output[j++] = '\t'; break;
                case 'r': output[j++] = '\r'; break;
                case '\\': output[j++] = '\\'; break;
                case 'a': output[j++] = '\a'; break;
                case 'b': output[j++] = '\b'; break;
                case 'v': output[j++] = '\v'; break;
                case 'f': output[j++] = '\f'; break;
                    
                case 'c': output[j] = '\0'; return 1;

                // Octal escape: \0NNN (up to 3 octal digits)
                case '0': {
                    int val = 0, k = 0;
                    while (k < 3 && input[i] >= '0' && input[i] <= '7') {
                        val = val * 8 + (input[i] - '0');
                        i++; k++;
                    }
                    i--; // step back because loop advanced too far
                    output[j++] = (char)val;
                    break;
                }

                // Hex escape: \xNN (any number of hex digits, usually up to 2)
                case 'x': {
                    int val = 0, k = 0;
                    i++;
                    while (isxdigit((unsigned char)input[i]) && k < 2) {
                        if (isdigit((unsigned char)input[i]))
                            val = val * 16 + (input[i] - '0');
                        else
                            val = val * 16 + (tolower((unsigned char)input[i]) - 'a' + 10);
                        i++; k++;
                    }
                    i--; // backtrack once
                    output[j++] = (char)val;
                    break;
                }

                default: // unknown escape, just keep the backslash and char
                    output[j++] = '\\';
                    if (input[i] != '\0' && j + 1 < out_size)
                        output[j++] = input[i];
                break;
            }
        } else {
            output[j++] = input[i];
        }
    }
    output[j] = '\0';
    return 0;
}

void print_help(const char *name) {
    printf("Usage: %s [SHORT-OPTION]... [STRING]...\n", name);
    printf("  or:  %s LONG-OPTION\n", name);
    printf("Echo the STRING(s) to standard output.\n\n");

    // find longest option string
    int maxlen = 0;
    for (int i = 0; help_entries[i].opt; i++) {
        int len = (int)strlen(help_entries[i].opt);
        if (len > maxlen) maxlen = len;
    }

    // print each option aligned
    for (int i = 0; help_entries[i].opt; i++) {
        printf("  %-*s  %s\n", maxlen, help_entries[i].opt, help_entries[i].desc);
    }
    printf("\nIf -e is in effect, the following sequences are recongized:\n\n");

    maxlen = 0;
    for (int i = 0; backslash_entries[i].opt; i++) {
        int len = (int)strlen(backslash_entries[i].opt);
        if (len > maxlen) maxlen = len;
    }

    for (int i = 0; backslash_entries[i].opt; i++) {
        printf("  %-*s  %s\n", maxlen, backslash_entries[i].opt, backslash_entries[i].desc);
    }
    printf("\nYour shell may have its own version of echo, which usually supersedes\n");
    printf("the version described here. Please refer to your shell's documentation\n");
    printf("for details about the options it supports.\n\n");
    printf("Consider using the printf(1) command instead,\n");
    printf("as it avoids problems when outputting option-like strings.\n");
}

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt_long(argc, argv, "neE", long_options, 0)) != -1) {
        switch (opt) {
            case '1':
                print_help(argv[0]);
                return 0;
            case 'n':
                noNewline = True;
                break;
            case 'e':
                backslashEscapes = True;
                break;
            case 'E':
                backslashEscapes = False;
                break;
            default:
                break;
        }
    }
    char message[ARG_MAX] = {0};
    for (; optind < argc; optind++) {
        if (strlen(message) + strlen(argv[optind]) < sizeof(message)) {
            strncat(message, argv[optind], sizeof(message) - strlen(message) - 1);
            if (optind + 1 < argc)
            strncat(message, " ", sizeof(message) - strlen(message) - 1);
        }
    }
    char parsed[ARG_MAX];
    if (backslashEscapes) {
        parse_escapes(message, parsed, sizeof(parsed));
    }
    
    if (!noNewline) {
        strncat(message, "\n", sizeof(message) - strlen(message) - 1);
        if (backslashEscapes)
            strncat(parsed, "\n", sizeof(parsed) - strlen(parsed) - 1);
    }

    if (backslashEscapes)
        write(1, parsed, strlen(parsed));
    else
        write(1, message, strlen(message));
    
    return 0;
}
