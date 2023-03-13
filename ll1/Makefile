
CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2 -ansi
DFLAGS ?= -g
SFLAGS ?= -Os -s

.PHONY: ll

all: ll

ll: ll.c
	$(CC) $(CFLAGS) $(SFLAGS) -o $@ $^

clean:
	rm -rf *.o *.optimized *.out debug ll

run: ll
	cat lib.l - | ./ll
