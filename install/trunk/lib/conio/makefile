# Makefile for the conio compatibility library
# Supports (some) conio functions under UNIX curses

# Copyright (C) 1998 Jim Hall, <jhall1@isd.net>
# This software is distributed under the GNU GPL.  See the file
# `COPYING' for more information.

SOURCES=conio.c interf.c io.c strio.c textwin.c
OBJS=$(SOURCES:.c=.o)

TESTS= wrap.c
TEST_OBJS=$(TESTS:.c=.o)

CC=gcc
CFLAGS=-Wall
LINT=lint
AR=ar
RANLIB=ranlib

RM=rm -f


# targets:

all: libconio.a

libconio.a: $(OBJS)
	$(AR) -rc $@ $(OBJS)
	$(RANLIB) $@

lint: $(SOURCES) $(TESTS)
	$(LINT) $(CFLAGS) $(SOURCES)
	$(LINT) $(CFLAGS) $(TESTS)

test: all $(TEST_OBJS)
	$(CC) $(CFLAGS) -L. -I. -o wrap $(TEST_OBJS) -lconio -lcurses
	./wrap


# clean up

clean:
	$(RM) *~

realclean: clean
	$(RM) *.o

distclean: realclean
	$(RM) *.a
	$(RM) wrap
