# this Makefile is for UNIX

CC=gcc
CFLAGS=-I../../include -I../conio
LINT=lint

RM=rm -f


# targets:

all: lsm_desc.o

examples: foo

foo: foo.o lsm_desc.o
	$(CC) $(CFLAGS) -o $@ foo.o lsm_desc.o


# clean up:

clean:
	$(RM) *~ core

realclean: clean
	$(RM) *.o

distclean: realclean
	$(RM) foo
