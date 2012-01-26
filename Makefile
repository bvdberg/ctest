CCFLAGS=-Wall -Wextra -Wno-unused-parameter -O3

all: test

test: main.c ctest.h mytests.c
	gcc $(CCFLAGS) main.c mytests.c -o test -lrt

clean:
	rm -f test

