CWD=$(shell pwd)

AR ?= ar
CC ?= gcc
PREFIX ?= /usr/local
SCANBUILD ?= scan-build
uname = $(shell uname -s)

CFLAGS = -c -g -O0 -Wall -std=c99

SRCS = $(wildcard src/*.c)
INCS = -Iinclude/
OBJS = $(SRCS:.c=.o)

TEST_SRCS = $(wildcard test/*.c)
TESTS = $(addprefix bin/,$(TEST_SRCS:.c=))

LIBSTS = build/libsts.a
EXAMPLE = example

show:
	@echo src: $(TEST_SRCS)
	@echo bin:  $(TESTS)

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

test: $(TESTS) 
	set -e; for file in $^; do echo "\n\033[00;32m+++ $$file +++\033[00m\n" && ./$$file; done

bin/test/%: $(OBJS) test/%.o
	@mkdir -p bin/test
	$(CC) $(LDFLAGS) $^ -o $@ 
ifeq ($(uname),Darwin)
	dsymutil $@ 
endif

.SUFFIXES: .c .o
.c.o: 
	mkdir -p bin/test
	$(CC) $< $(CFLAGS) $(INCS) -o $@

clean-all: clean
	rm -rf deps/**/build

clean:
	find . -name "*.gc*" -exec rm {} \;
	rm -rf `find . -name "*.dSYM" -print`
	rm -rf bin $(OBJS)
	rm -rf examples/*.o
	rm -rf test/*.o
	rm -f  test/fixtures/sample.out.txt

.PHONY: clean

include valgrind.mk
