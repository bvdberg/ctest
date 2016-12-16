UNAME=$(shell uname)

CC=gcc
LD=gcc

CCFLAGS=-std=c99 -Werror -Wall -Wextra -Wformat=2 -Wno-format-nonliteral -Wno-unused-parameter -O3

ifeq ($(UNAME), Darwin)
CC=clang
LD=clang
endif

all: test

remake: clean all

%.o: %.c ctest.h
	$(CC) $(CCFLAGS) -c -o $@ $<

test: main.o ctest.h mytests.o
	$(LD) $(LDFLAGS) main.o mytests.o -o test

clean:
	rm -f test *.o

