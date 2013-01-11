UNAME=$(shell uname)

CCFLAGS=-Wall -Wextra -Wno-unused-parameter -O3
ifdef CTEST_COLOR_OK
CCFLAGS+=-DCTEST_COLOR_OK
endif
ifdef CTEST_MORE_COLORS
CCFLAGS+=-DCTEST_MORE_COLORS
endif

ifeq ($(UNAME), Darwin)
LDFLAGS=-Wl,-flat_namespace,-undefined,dynamic_lookup
endif

all: test

remake: clean all

%.o: %.c ctest.h
	gcc $(CCFLAGS) -c -o $@ $<

test: main.o ctest.h mytests.o
	gcc $(LDFLAGS) main.o mytests.o -o test

clean:
	rm -f test *.o

