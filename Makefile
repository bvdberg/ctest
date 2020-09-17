UNAME=$(shell uname)

CCFLAGS=-Wall -Wextra -Wconversion -Wredundant-decls -Wshadow -Wno-unused-parameter -O3
CC=clang
CXX=clang++

all: test test++

remake: clean all

%.cpp: %.c
	ln -fs $< $@

%.c.o: %.c ctest.h
	$(CC) $(CCFLAGS) -c -o $@ $<

%.cpp.o: %.cpp ctest.h
	$(CXX) $(CCFLAGS) -c -o $@ $<

test: main.c.o ctest.h mytests.c.o
	$(CC) $(LDFLAGS) main.c.o mytests.c.o -o test

test++: main.cpp.o ctest.h mytests.cpp.o
	$(CXX) $(LDFLAGS) main.cpp.o mytests.cpp.o -o test++

clean:
	rm -f test test++ *.o
