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
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

#include "meta.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define BUFSIZ 8192

static struct option long_options[] = {
    {"bytes", required_argument, NULL, 'c'},
    {"lines", required_argument, NULL, 'n'},
    {"quiet", no_argument, NULL, 'q'},
    {"silent", no_argument, NULL, 'q'},
    {"verbose", no_argument, NULL, 'v'},
    {"zero-terminated", no_argument, NULL, 'z'},
    {"help", no_argument, NULL, 1},
    {"version", no_argument, NULL, 2},
    {NULL, 0, NULL, 0}
};

static struct help_entry help_entries[] = {
    {
        "-c, --bytes=[-]NUM", "print the first NUM bytes of each file;\n"
        "                               with the leading '-', print all but the last\n"
        "                               NUM bytes of each file"
    },
    {
        "-n, --lines=[-]NUM", "print the first NUM lines instead of the first 10;\n"
        "                               with the leading '-', print all but the last\n"
        "                               NUM lines of each file"
    },
    {"-q, --quiet, --silent", "never print headers giving file names"},
    {"-v, --verbose", "always print headers giving file names"},
    {"-z, --zero-terminated", "line delimiter is NUL, not newline"},
    {"    --help", "display this help and exit"},
    {"    --version", "output version information and exit"},
    {NULL, NULL}
};

struct count {
    bool enabled;
    int negative;
    uint64_t value;
};

struct options {
    struct count bytes;
    struct count lines;
    bool quiet;
    bool verbose;
    bool nul_termed;
};

struct ring {
    unsigned char *buf;
    size_t cap;  // capacity
    size_t len;  // current len of bytes
    size_t head; // index of oldest element
};

struct line {
    unsigned char *data;
    size_t len;
};

struct line_ring {
    struct line *buf;
    size_t cap;
    size_t len;
    size_t head;
};

int ring_init(struct ring *r, const size_t cap) {
    r->buf = malloc(cap);
    if (!r->buf)
        return 1;

    r->cap = cap;
    r->len = 0;
    r->head = 0;
    return 0;
}

void ring_free(const struct ring *r) {
    free(r->buf);
}

int ring_push(struct ring *r, const unsigned char byte, unsigned char *evicted) {
    if (r->len < r->cap) {
        r->buf[(r->head + r->len) % r->cap] = byte;
        r->len++;
        return 0; // nothing evicted
    }

    *evicted = r->buf[r->head];
    r->buf[r->head] = byte;
    r->head = (r->head + 1) % r->cap;
    return 1; // evicted
}

int line_ring_init(struct line_ring *r, const size_t cap) {
    r->buf = calloc(cap, sizeof(struct line));
    if (!r->buf) return -1;
    r->cap = cap;
    r->len = 0;
    r->head = 0;
    return 0;
}

void line_ring_free(const struct line_ring *r) {
    for (size_t i = 0; i < r->len; i++) {
        const size_t idx = (r->head + i) % r->cap;
        free(r->buf[idx].data);
    }
    free(r->buf);
}

int line_ring_push(
    struct line_ring *r,
    unsigned char *data,
    const size_t len,
    struct line *evicted
) {
    if (r->len < r->cap) {
        const size_t idx = (r->head + r->len) % r->cap;
        r->buf[idx].data = data;
        r->buf[idx].len = len;
        r->len++;
        return 0;
    }

    *evicted = r->buf[r->head];
    r->buf[r->head].data = data;
    r->buf[r->head].len = len;
    r->head = (r->head + 1) % r->cap;
    return 1;
}

ssize_t read_line_fd(
    const int fd,
    unsigned char **out,
    size_t *out_len,
    const unsigned char delim
) {
    size_t cap = 128;
    size_t len = 0;
    unsigned char *buf = malloc(cap);
    if (!buf) return -1;

    while (1) {
        unsigned char c;
        const ssize_t n = read(fd, &c, 1);
        if (n == 0) break;
        if (n < 0) {
            free(buf);
            return -1;
        }

        if (len == cap) {
            cap *= 2;
            unsigned char *tmp = realloc(buf, cap);
            if (!tmp) {
                free(buf);
                return -1;
            }
            buf = tmp;
        }

        buf[len++] = c;
        if (c == delim) break;
    }

    if (len == 0) {
        free(buf);
        return 0;
    }

    *out = buf;
    *out_len = len;
    return (ssize_t)len;
}

int parse_count(const char *arg, int *neg, uint64_t *out, const bool is_lines) {
    const char *p = arg;
    uint64_t mul = 1;
    char *end;

    *neg = 0;
    while (isspace((unsigned char)*p)) p++;

    if (*p == '-') {
        *neg = 1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    if (*p == '-' || *p == '+') {
        fprintf(stderr, "head: invalid number of %s: '%s'\n", (is_lines ? "lines" : "bytes"), arg);
        return -1;
    }

    errno = 0;
    const uint64_t num = strtoull(p, &end, 10);
    if (p == end || errno == ERANGE) {
        fprintf(stderr, "head: cannot parse number of %s: %s\n",
                (is_lines ? "lines" : "bytes"), strerror(errno));
        return -1;
    }

    if (*end) {
        if (!strcasecmp(end, "b")) mul = 512;
        else if (!strcasecmp(end, "kB")) mul = 1000ULL;
        else if (!strcasecmp(end, "MB")) mul = 1000ULL * 1000;
        else if (!strcasecmp(end, "GB")) mul = 1000ULL * 1000 * 1000;
        else if (!strcasecmp(end, "TB")) mul = 1000ULL * 1000 * 1000 * 1000;
        else if (!strcasecmp(end, "PB")) mul = 1000ULL * 1000 * 1000 * 1000 * 1000;
        else if (!strcasecmp(end, "EB")) mul = 1000ULL * 1000 * 1000 * 1000 * 1000 * 1000;

        else if (!strcasecmp(end, "K") || !strcasecmp(end, "KiB")) mul = 1024ULL;
        else if (!strcasecmp(end, "M") || !strcasecmp(end, "MiB")) mul = 1024ULL * 1024;
        else if (!strcasecmp(end, "G") || !strcasecmp(end, "GiB")) mul = 1024ULL * 1024 * 1024;
        else if (!strcasecmp(end, "T") || !strcasecmp(end, "TiB")) mul = 1024ULL * 1024 * 1024 * 1024;
        else if (!strcasecmp(end, "P") || !strcasecmp(end, "PiB")) mul = 1024ULL * 1024 * 1024 * 1024 * 1024;
        else if (!strcasecmp(end, "E") || !strcasecmp(end, "EiB")) mul = 1024ULL * 1024 * 1024 * 1024 * 1024 * 1024;

        else {
            fprintf(stderr, "head: invalid number of %s: '%s'\n", (is_lines ? "lines" : "bytes"), end);
            return -1;
        }
    }

    if (num > UINT64_MAX / mul) {
        fprintf(stderr, "head: invalid number of %s: '%s': Value too large for defined data type\n",
        (is_lines ? "lines" : "bytes"), arg);
        return -1;
    }

    *out = num * mul;
    return 0;
}

int read_stdin_bytes_first(const uint64_t max_bytes, const bool *term) {
    unsigned char buf[BUFSIZ];
    uint64_t written = 0;

    while (written < max_bytes) {
        errno = 0;
        const ssize_t n = read(STDIN_FILENO, buf, BUFSIZ);
        if (n == -1) {
            fprintf(stderr, "head: cannot read from standard input: %s\n", strerror(errno));
            return -1;
        } else if (n == 0) {
            break;
        }

        const uint64_t remaining = max_bytes - written;
        const uint64_t to_write = MIN((uint64_t)n, remaining);

        uint64_t actual_write = to_write;
        if (*term) {
            for (uint64_t i = 0; i < to_write; i++) {
                if (buf[i] == '\0') {
                    actual_write = i + 1;
                    break;
                }
            }
        }

        if (write(STDOUT_FILENO, buf, actual_write) == -1) {
            fprintf(stderr, "head: failed to write output: %s\n", strerror(errno));
            return -1;
        }
        written += actual_write;

        if (*term && actual_write < to_write) {
            break;
        }
    }

    return 0;
}

int read_stdin_all_but_last(const uint64_t tail_len) {
    unsigned char buf[BUFSIZ];
    ssize_t n;

    if (tail_len == 0) {
        while ((n = read(STDIN_FILENO, buf, BUFSIZ)) > 0) {
            if (write(STDOUT_FILENO, buf, n) == -1) {
                fprintf(stderr, "head: failed to write output: %s\n", strerror(errno));
            }
        }
        if (n < 0) {
            fprintf(stderr, "head: cannot read from standard input: %s\n", strerror(errno));
            return -1;
        }
        return 0;
    }

    struct ring r;
    if (ring_init(&r, tail_len) != 0) {
        fprintf(stderr, "head: failed to allocate memory\n");
        return -1;
    }

    while ((n = read(STDIN_FILENO, buf, BUFSIZ)) > 0) {
        for (ssize_t i = 0; i < n; i++) {
            unsigned char evicted;
            if (ring_push(&r, buf[i], &evicted)) {
                if (write(STDOUT_FILENO, &evicted, 1) == -1) {
                    ring_free(&r);
                    fprintf(stderr, "head: failed to write output: %s\n", strerror(errno));
                    return -1;
                }
            }
        }
    }
    if (n < 0) {
        fprintf(stderr, "head: failed to read: %s\n", strerror(errno));
        return -1;
    }

    ring_free(&r);
    return 0;
}

int head_bytes_first(const char *file, const uint64_t max_bytes, const bool *term) {
    unsigned char buf[BUFSIZ];
    uint64_t written = 0;

    const int fd = open(file, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "head: cannot open '%s' for reading: %s\n", file, strerror(errno));
        return -1;
    }

    while (written < max_bytes) {
        errno = 0;
        const ssize_t n = read(fd, buf, BUFSIZ);
        if (n == -1) {
            fprintf(stderr, "head: cannot read '%s': %s\n", file, strerror(errno));
            close(fd);
            return -1;
        } else if (n == 0) {
            break;
        }

        const uint64_t remaining = max_bytes - written;
        const uint64_t to_write = MIN((uint64_t)n, remaining);

        uint64_t actual_write = to_write;
        if (*term) {
            for (uint64_t i = 0; i < to_write; i++) {
                if (buf[i] == '\0') {
                    actual_write = i + 1;
                    break;
                }
            }
        }

        if (write(STDOUT_FILENO, buf, actual_write) == -1) {
            fprintf(stderr, "head: failed to write output: %s\n", strerror(errno));
            close(fd);
            return -1;
        }
        written += actual_write;

        if (*term && actual_write < to_write) {
            break;
        }
    }

    close(fd);
    return 0;
}

int head_bytes_all_but_last(const char *file, const uint64_t tail_len) {
    unsigned char buf[BUFSIZ];
    ssize_t n;

    const int fd = open(file, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "head: cannot open '%s' for reading: %s\n", file, strerror(errno));
        return -1;
    }

    if (tail_len == 0) {
        while ((n = read(fd, buf, BUFSIZ)) > 0) {
            if (write(STDOUT_FILENO, buf, n) == -1) {
                close(fd);
                fprintf(stderr, "head: failed to write output: %s\n", strerror(errno));
                return -1;
            }
        }
        if (n < 0) {
            close(fd);
            fprintf(stderr, "head: cannot read from standard input: %s\n", strerror(errno));
            return -1;
        }
        return 0;
    }

    struct ring r;
    if (ring_init(&r, tail_len) != 0) {
        fprintf(stderr, "head: failed to allocate memory\n");
        return -1;
    }

    while ((n = read(fd, buf, BUFSIZ)) > 0) {
        for (ssize_t i = 0; i < n; i++) {
            unsigned char evicted;
            if (ring_push(&r, buf[i], &evicted)) {
                if (write(STDOUT_FILENO, &evicted, 1) == -1) {
                    close(fd);
                    ring_free(&r);
                    fprintf(stderr, "head: failed to write output: %s\n", strerror(errno));
                    return -1;
                }
            }
        }
    }
    if (n < 0) {
        close(fd);
        fprintf(stderr, "head: failed to read '%s': %s\n", file, strerror(errno));
        return -1;
    }

    close(fd);
    ring_free(&r);
    return 0;
}

int head_bytes(const char *file, const struct options opts, const bool term) {
    if (file == NULL) {
        if (!opts.bytes.negative)
            return read_stdin_bytes_first(opts.bytes.value, &term);
        else
            return read_stdin_all_but_last(opts.bytes.value);
    } else {
        if (!opts.bytes.negative)
            return head_bytes_first(file, opts.bytes.value, &term);
        else
            return head_bytes_all_but_last(file, opts.bytes.value);
    }
}

int read_stdin_lines_first(const uint64_t max_lines, const unsigned char delim) {
    unsigned char buf[BUFSIZ];
    uint64_t lines = 0;

    if (max_lines == 0) {
        return 0;
    }

    while (lines < max_lines) {
        errno = 0;
        const ssize_t n = read(STDIN_FILENO, buf, BUFSIZ);
        if (n == -1) {
            fprintf(stderr, "head: cannot read from standard input: %s\n", strerror(errno));
            return -1;
        } else if (n == 0) {
            break;
        }

        for (ssize_t i = 0; i < n; i++) {
            if (buf[i] == delim) {
                lines++;
                if (lines == max_lines) {
                    break;
                }
            }
            if (write(STDOUT_FILENO, &buf[i], 1) == -1) {
                fprintf(stderr, "head: failed to write output: %s\n", strerror(errno));
                return -1;
            }
        }
    }

    return 0;
}

int read_lines_all_but_last(const uint64_t tail_len, const unsigned char delim) {
    if (tail_len == 0) {
        unsigned char buf[BUFSIZ];
        ssize_t n;
        while ((n = read(STDIN_FILENO, buf, BUFSIZ)) > 0)
            if (write(STDOUT_FILENO, buf, n) == -1) {
                fprintf(stderr, "head: failed to write output: %s\n", strerror(errno));
                return -1;
            }
        return 0;
    }

    struct line_ring r;
    if (line_ring_init(&r, tail_len) != 0) {
        fprintf(stderr, "head: failed to allocate memory\n");
        return -1;
    }

    while (1) {
        unsigned char *line = NULL;
        size_t len = 0;
        const ssize_t n = read_line_fd(STDIN_FILENO, &line, &len, delim);
        if (n <= 0) break;

        struct line evicted;
        if (line_ring_push(&r, line, len, &evicted)) {
            if (write(STDOUT_FILENO, evicted.data, evicted.len) == -1) {
                line_ring_free(&r);
                fprintf(stderr, "head: failed to write output: %s\n", strerror(errno));
                return -1;
            }
            free(evicted.data);
        }
    }

    line_ring_free(&r);
    return 0;
}

int head_lines_first(const char *file, const uint64_t max_lines, const char delim) {
    unsigned char buf[BUFSIZ];
    uint64_t lines = 0;

    const int fd = open(file, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "head: cannot open '%s' for reading: %s\n", file, strerror(errno));
        return -1;
    }

    while (lines < max_lines) {
        errno = 0;
        const ssize_t n = read(fd, buf, BUFSIZ);
        if (n == -1) {
            close(fd);
            fprintf(stderr, "head: cannot read from standard input: %s\n", strerror(errno));
            return -1;
        } else if (n == 0) {
            break;
        }

        for (ssize_t i = 0; i < n; i++) {
            if (buf[i] == delim) {
                lines++;
                if (lines == max_lines) {
                    break;
                }
            }
            if (write(STDOUT_FILENO, &buf[i], 1) == -1) {
                close(fd);
                fprintf(stderr, "head: failed to write output: %s\n", strerror(errno));
                return -1;
            }
        }
    }

    close(fd);
    return 0;
}

int head_lines_all_but_last(const char *file, const uint64_t tail_len, const unsigned char delim) {
    const int fd = open(file, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "head: cannot open '%s' for reading: %s\n", file, strerror(errno));
        return -1;
    }

    if (tail_len == 0) {
        unsigned char buf[BUFSIZ];
        ssize_t n;
        while ((n = read(fd, buf, BUFSIZ)) > 0)
            if (write(STDOUT_FILENO, buf, n) == -1) {
                close(fd);
                fprintf(stderr, "head: failed to write output: %s\n", strerror(errno));
                return -1;
            }
        return 0;
    }

    struct line_ring r;
    if (line_ring_init(&r, tail_len) != 0) {
        close(fd);
        fprintf(stderr, "head: failed to allocate memory\n");
        return -1;
    }

    while (1) {
        unsigned char *line = NULL;
        size_t len = 0;
        const ssize_t n = read_line_fd(fd, &line, &len, delim);
        if (n <= 0) break;

        struct line evicted;
        if (line_ring_push(&r, line, len, &evicted)) {
            if (write(STDOUT_FILENO, evicted.data, evicted.len) == -1) {
                close(fd);
                line_ring_free(&r);
                fprintf(stderr, "head: failed to write output: %s\n", strerror(errno));
                return -1;
            }
            free(evicted.data);
        }
    }

    close(fd);
    line_ring_free(&r);
    return 0;
}

int head_lines(const char *file, const struct options opts, const char delim) {
    if (file == NULL) {
        if (!opts.lines.negative) return read_stdin_lines_first(opts.lines.value, delim);

        return read_lines_all_but_last(opts.lines.value, delim);
    }
    if (!opts.lines.negative) return head_lines_first(file, opts.lines.value, delim);

    return head_lines_all_but_last(file, opts.lines.value, delim);
}

int main(int argc, char *argv[]) {
    struct options opts = {0};
    // default
    opts.lines.value = 10;
    opts.lines.negative = 0;
    opts.lines.enabled = true;

    int opt;
    while ((opt = getopt_long(argc, argv, "c:n:qvz", long_options, NULL)) != -1) {
        switch (opt) {
            case 'c':
                if (parse_count(optarg, &opts.bytes.negative, &opts.bytes.value, false) != 0) {
                    return -1;
                }
                opts.bytes.enabled = true;

                opts.lines.enabled = false;
                opts.lines.negative = 0;
                opts.lines.value = 0;
                break;
            case 'n':
                if (parse_count(optarg, &opts.lines.negative, &opts.lines.value, true) != 0) {
                    return -1;
                }
                opts.lines.enabled = true;

                opts.bytes.enabled = false;
                opts.bytes.negative = 0;
                opts.bytes.value = 0;
                break;
            case 'q':
                opts.quiet = true;
                opts.verbose = false;
                break;
            case 'v':
                opts.verbose = true;
                opts.quiet = false;
                break;
            case 'z':
                opts.nul_termed = true;
                break;
            case 1:
                {
                    char buf[256];
                    snprintf(buf, 256, "Usage: %s [OPTION]... [FILE]...", argv[0]);
                    print_help(buf, "Print the first 10 lines of each FILE to standard output.\n"
                                "With more than one FILE, precede each with a header giving the file name.\n\n"
                                "With no FILE, or when FILE is -, read standard input.\n\n"
                                "Mandatory arguments to long options are mandatory for short options too.",
                                help_entries,
                                "NUM may have a multiplier suffix:\n"
                                "b 512, kB 1000, K 1024, MB 1000*1000, M 1024*1024,\n"
                                "GB 1000*1000*1000, G 1024*1024*1024, and so on for T, P, E, Z, Y, R, Q.\n"
                                "Binary prefixes can be used, too: KiB=K, MiB=M, and so on.");
                    return 0;
                }
            case 2:
                print_version(PROGRAM_NAME, PROJECT_NAME, VERSION, AUTHORS);
                return 0;
            default:
                fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
                return 1;
        }
    }
    const char delim = opts.nul_termed ? '\0' : '\n';

    if (argc == optind) {
        if (opts.verbose)
            puts("==> standard input <==");
        if (opts.bytes.enabled) {
            if (head_bytes(NULL, opts, opts.nul_termed) == -1)
                return -1;
        } else {
            if (head_lines(NULL, opts, delim) == -1)
                return -1;
        }
    } else {
        bool first = true;
        const bool more_than_one = (argc - optind > 1) ? true : false;

        for (; optind < argc; optind++) {
            if (more_than_one) {
                if (!first) {
                    putchar('\n');
                }
                if (!opts.quiet) {
                    if (!strcmp(argv[optind], "-"))
                        puts("==> standard input <==\n");
                    else
                        printf("==> %s <==\n", argv[optind]);
                }
            } else if (opts.verbose) {
                if (!strcmp(argv[optind], "-"))
                    puts("==> standard input <==");
                else
                    printf("==> %s <==\n", argv[optind]);
            }
            first = false;

            if (!strcmp(argv[optind], "-")) {
                if (opts.bytes.enabled) {
                    if (head_bytes(NULL, opts, opts.nul_termed) == -1)
                        return -1;
                } else {
                    if (head_lines(NULL, opts, delim) == -1)
                        return -1;
                }
            } else {
                if (opts.bytes.enabled) {
                    if (head_bytes(argv[optind], opts, opts.nul_termed) == -1)
                        return -1;
                } else {
                    if (head_lines(argv[optind], opts, delim) == -1)
                        return -1;
                }
            }
        }
    }

    return 0;
}
