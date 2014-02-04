CWD=$(shell pwd)

AR ?= ar
CC ?= gcc
PREFIX ?= /usr/local
SCANBUILD ?= scan-build

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

run: run-file-stream

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
	for file in $^; do ./$$file; done

bin/test/%: $(OBJS) test/%.o
	@mkdir -p bin/test
	$(CC) $(LDFLAGS) $^ -o $@ 

dsym-%: test/%.test.o 
	dsymutil bin/$(<:.o=)

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

include valgrind.mk
