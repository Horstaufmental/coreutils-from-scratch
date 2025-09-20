CC = gcc
CFLAGS = -Wall -Wextra -std=c99
DEPS = main.c longformat.h bytetohr.c
OUTDIR = bin
TARGET = $(OUTDIR)/ls

all: $(TARGET)
debug: CFLAGS += -g
debug: all

$(TARGET): $(DEPS) | $(OUTDIR)
	$(CC) $(CFLAGS) -o $@ $(DEPS)

$(OUTDIR):
	mkdir -p $(OUTDIR)

clean:
	rm -rf $(OUTDIR)

.PHONY: all clean