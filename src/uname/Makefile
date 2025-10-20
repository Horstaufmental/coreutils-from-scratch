CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic
MACROS = -DOPERATING_SYSTEM="\"GNU/Linux\""

all: uname

uname: uname.c
	$(CC) $(CFLAGS) uname.c $(MACROS) -o uname

debug: uname.c
	$(CC) $(CFLAGS) -g uname.c $(MACROS) -o uname

clean:
	rm -f uname

.PHONY: clean
