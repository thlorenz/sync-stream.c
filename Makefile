CWD=$(shell pwd)

AR ?= ar
CC ?= gcc
PREFIX ?= /usr/local
SCANBUILD ?= scan-build

CFLAGS = -c -O3 -Wall -std=c99

EE = deps/ee.c/build/ee.a
EEINC = deps/ee.c

LDFLAGS += -l$(EE)
SRCS = src/readable.c
OBJS = $(SRCS:.c=.o)
INCS = -I $(EEINC)/
CLIB = node_modules/.bin/clib

LIBSTREAM = build/libstream.a
EXAMPLE = example

all: clean $(LIBSTREAM)

$(LIBSTREAM): $(OBJS)
	@mkdir -p build
	$(AR) rcs $@ $(OBJS)

readable: $(EE) $(OBJS)
	@mkdir -p bin
	$(CC) $(LDFLAGS) $(OBJS) -o bin/$@


install: all
	cp -f $(LIBSTREAM) $(PREFIX)/lib/libstream.a
	cp -f src/stream.h $(PREFIX)/include/stream.h

uninstall:
	rm -f $(PREFIX)/lib/libstream.a
	rm -f $(PREFIX)/include/stream.h

check:
	$(SCANBUILD) $(MAKE) test

test: $(EE) test.o $(OBJS)
	@mkdir -p bin
	$(CC) $(OBJS) test.o -o bin/$@
	bin/$@

$(EXAMPLE): clean $(EE) $(OBJS) example.o
	@mkdir -p bin
	$(CC) $(OBJS) example.o -o bin/$@
	bin/$@

# clibs
$(CLIB):
	npm install

$(EE): $(CLIB)
	$(CLIB) install thlorenz/ee.c -o deps/
	cd deps/ee.c && make build

.SUFFIXES: .c .o
.c.o: 
	$(CC) $< $(CFLAGS) $(INCS) -c -o $@

clean-all: clean
	rm -f $(OBJS)

clean:
	find . -name "*.gc*" -exec rm {} \;
	rm -rf `find . -name "*.dSYM" -print`
	rm -rf bin src/*.o *.o

.PHONY: all check test clean clean-all install uninstall
