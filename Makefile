UNAME=$(shell uname)

CCFLAGS=-Wall -Wextra -Wno-unused-parameter -O3
ifeq ($(UNAME), Darwin)
LDFLAGS=-Wl,-flat_namespace,-undefined,dynamic_lookup
endif

all: test

%.o: %.c
	gcc $(CCFLAGS) -c -o $@ $<

test: main.o ctest.h mytests.o
	gcc $(LDFLAGS) main.o mytests.o -o test

clean:
	rm -f test *.o

