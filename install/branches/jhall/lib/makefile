# this Makefile is for UNIX

LINT=lint
CC=gcc
CFLAGS=-I../include

RM=rm -f

SOURCES=dat.c
OBJS=$(SOURCES:.c=.o)

# targets:

all: libs

libs: $(OBJS)
	(cd lsm; $(MAKE) all)
	(cd makepath; $(MAKE) all)

lint:
	$(LINT) $(CFLAGS) $(SOURCES)


# clean up:

clean:
	$(RM) *~

realclean: clean
	$(RM) $(OBJS)

distclean: realclean
	(cd lsm; $(MAKE) $@)
	(cd makepath; $(MAKE) $@)
