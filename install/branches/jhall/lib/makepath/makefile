# makefile for the _makepath() library of functions
# jhall1@isd.net


# macros:

LINT=lint

CC=gcc
CFLAGS=-I. -Wall

LDFLAGS=-L.
LDLIBS=-lmakepath

AR=ar
ARFLAGS=-rc
RANLIB=ranlib

SOURCES=makepath.c splitpth.c
OBJECTS=$(SOURCES:.c=.o)


# default target (create object files)

all: libmakepath.a

libmakepath.a: $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)
	$(RANLIB) $@

lint:
	$(LINT) $(CFLAGS) $(SOURCES)


# test target (create test program)

foo: all foo.o
	$(CC) $(CFLAGS) -o $@ foo.o $(LDFLAGS) $(LDLIBS)


# cleanup:

clean:
	$(RM) core

realclean: clean
	$(RM) $(OBJECTS) foo.o

distclean: realclean
	$(RM) foo libmakepath.a
