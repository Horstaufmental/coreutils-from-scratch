#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

struct help_entry {
  const char *opt;
  const char *desc;
};

static struct help_entry help_entries[] = {
    {"   --help", "display this help and exit"},
    {0, 0}
};

void print_help(const char *name) {
    printf("Usage: %s FILE\n"
           "  or:  %s OPTION\n", name, name);
    puts("Call the unlink function to remove the specified FILE.\n");
    
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
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "unlink: missing operand\n"
                        "Try '%s --help' for more information.\n", argv[0]);
        return 1;
    }
    if (argv[1] != NULL) {
        if (strcmp(argv[1], "--help") == 0) {
            print_help(argv[0]);
            return 0;
        }
        // for unrecognized long options
        if (argv[1][0] == '-' && argv[1][1] == '-' && argv[1][2] != '\0') {
            fprintf(stderr, "unlink: unrecognized option '%s'\n"
                            "Try '%s --help' for more information.\n", argv[1], argv[0]);
            return 1;
        }
        if (argv[1][0] == '-' && argv[1][1] == '-' && argv[1][2] == '\0') {
            fprintf(stderr, "unlink: missing operand\n"
                            "Try '%s --help' for more information.\n", argv[0]);
            return 1;
        }
        // for unrecognized short options
        if (argv[1][0] == '-' && argv[1][1] != '\0') {
            fprintf(stderr, "unlink: invalid option -- '%c'\n"
                            "Try '%s --help' for more information.\n", argv[1][1], argv[0]);
            return 1;
        }
        if (unlink(argv[1]) != 0) {
            fprintf(stderr, "unlink: cannot unlink '%s': %s\n", argv[1], strerror(errno));
            return 1;
        }
    }
    return 0;
}