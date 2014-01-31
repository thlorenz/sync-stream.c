CWD=$(shell pwd)

AR ?= ar
CC ?= gcc
PREFIX ?= /usr/local
SCANBUILD ?= scan-build

CFLAGS = -c -O0 -Wall -std=c99

SRCS = $(wildcard src/*.c)
INCS = -Iinclude/
OBJS = $(SRCS:.c=.o)

LIBSTS = build/libsts.a
EXAMPLE = example

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

scp:
	scp -r examples include src test Makefile udesktop:tmp/sync-stream

grind: bin/test/stream
	valgrind --tool=memcheck --leak-check=yes $^

grind-report: bin/test/stream
	G_SLICE=always-malloc G_DEBUG=gc-friendly \
	valgrind -v --tool=memcheck --leak-check=full --num-callers=40 --log-file=valgrind.log $^

grind-chunk: bin/test/grind/chunk-new-free
	valgrind --tool=memcheck --leak-check=yes $^

rgrind-chunk: scp
	ssh udesktop 'cd tmp/sync-stream && make grind-chunk'
	
test: bin/test/stream 
	bin/test/stream

bin/test/grind/%: $(OBJS) test/grind/%.o
	@mkdir -p bin/test/grind
	$(CC) $(LDFLAGS) $^ -o $@ 

bin/test/%: $(OBJS) test/%.o
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

