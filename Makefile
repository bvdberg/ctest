CCFLAGS=-Wall -Wextra -Wno-unused-parameter -O3
LDFLAGS=-Wl,-flat_namespace,-undefined,dynamic_lookup

all: test

%.o: %.c
	gcc $(CCFLAGS) -c -o $@ $<

test: main.o ctest.h mytests.o
	gcc $(LDFLAGS) main.o mytests.o -o test

clean:
	rm -f test

