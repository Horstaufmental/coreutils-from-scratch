CC = gcc
CFLAGS = -v
MACROS = -DOPERATING_SYSTEM="\"GNU/Linux\""

all: uname

uname: uname.c
	$(CC) $(CFLAGS) uname.c $(MACROS) -o uname

clean:
	rm -f uname

.PHONY: clean
