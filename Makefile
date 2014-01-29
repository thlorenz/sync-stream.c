CWD=$(shell pwd)

AR ?= ar
CC ?= gcc
PREFIX ?= /usr/local
SCANBUILD ?= scan-build

CFLAGS = -c -O3 -Wall -std=c99

SRCS = $(wildcard src/*.c)
INCS = -Iinclude/
OBJS = $(SRCS:.c=.o)

LIBSTS = build/libsts.a
EXAMPLE = example

all: clean $(LIBSTS)

$(LIBSTS): $(OBJS) 
	@mkdir -p build
	$(AR) rcs $@ $(OBJS)

run: run-pipe-thru

run-file-stream: bin/file-stream-transform
	@echo "\n\033[1;33m>>>\033[0m"
	$<	
	@echo "\n\033[1;33m<<<\033[0m\n"
	make clean

run-pipe-thru: bin/pipe-thru-transforms
	@echo "\n\033[1;33m>>>\033[0m"
	$<	
	@echo "\n\033[1;33m<<<\033[0m\n"
	make clean

bin/pipe-thru-transforms: $(OBJS) examples/pipe-thru-transforms.o
	@mkdir -p bin	
	$(CC) $(LDFLAGS) $(OBJS) examples/pipe-thru-transforms.o -o $@ 

bin/file-stream-transform: $(OBJS) examples/file-stream-transform.o
	@mkdir -p bin	
	$(CC) $(LDFLAGS) $(OBJS) examples/file-stream-transform.o -o $@ 

install: all
	cp -f $(LIBSTS) $(PREFIX)/lib/libsts.a
	cp -f src/sts.h $(PREFIX)/include/sts.h

uninstall:
	rm -f $(PREFIX)/lib/libsts.a
	rm -f $(PREFIX)/include/sts.h

check: all
	$(SCANBUILD) $(MAKE) test

test: bin/test/stream 
	bin/test/stream

bin/test/stream: $(OBJS) test/stream.o
	@mkdir -p bin/test
	$(CC) $(LDFLAGS) $^ -o $@ 

.SUFFIXES: .c .o
.c.o: 
	$(CC) $< $(CFLAGS) $(INCS) -o $@

clean-all: clean
	rm -rf deps/**/build

clean:
	find . -name "*.gc*" -exec rm {} \;
	rm -rf `find . -name "*.dSYM" -print`
	rm -rf bin $(OBJS)
	rm -rf examples/*.o
	rm -rf test/*.o

.PHONY: all check test clean clean-all install uninstall
