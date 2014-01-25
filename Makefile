CWD=$(shell pwd)

AR ?= ar
CC ?= gcc
PREFIX ?= /usr/local
SCANBUILD ?= scan-build

CFLAGS = -c -O3 -Wall -std=c99

SRCS = src/transform.c
INCS = 
OBJS = $(SRCS:.c=.o)

LIBSTS = build/libsts.a
EXAMPLE = example

all: clean $(LIBSTS)

$(LIBSTS): $(OBJS) 
	@mkdir -p build
	$(AR) rcs $@ $(OBJS)

run: all transform
	@echo "\n\033[1;33m>>>\033[0m"
	./bin/transform
	@echo "\033[1;33m<<<\033[0m\n"
	make clean

transform: $(OBJS)
	@mkdir -p bin	
	$(CC) $(LDFLAGS) $(OBJS) -o bin/$@ 

install: all
	cp -f $(LIBSTS) $(PREFIX)/lib/libsts.a
	cp -f src/sts.h $(PREFIX)/include/sts.h

uninstall:
	rm -f $(PREFIX)/lib/libsts.a
	rm -f $(PREFIX)/include/sts.h

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

.SUFFIXES: .c .o
.c.o: 
	$(CC) $< $(CFLAGS) $(INCS) -c -o $@

clean-all: clean
	rm -rf deps/**/build

clean:
	find . -name "*.gc*" -exec rm {} \;
	rm -rf `find . -name "*.dSYM" -print`
	rm -rf bin $(OBJS)

.PHONY: all check test clean clean-all install uninstall
