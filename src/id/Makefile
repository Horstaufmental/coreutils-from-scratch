CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic
LDFLAGS = -lselinux

all: id

id: id.c
	@$(CC) $(CFLAGS) id.c $(MACROS) -o id $(LDFLAGS)

debug: id.c
	@$(CC) $(CFLAGS) -g id.c $(MACROS) -o id $(LDFLAGS)

clean:
	@rm -f id

.PHONY: clean
