# this Makefile is for UNIX

LINT=lint
CC=gcc
CFLAGS=-I../include -I../lib/lsm -I../lib/makepath \
	-I/home/jhall/src/unzip -I/home/jhall/include

LDFLAGS=-L../lib/makepath -L/home/jhall/src/unzip -L/home/jhall/lib
LDLIBS=../lib/dat.o ../lib/lsm/lsm_desc.o -lmakepath -lunzip -lconio -lcurses

RM=/bin/rm -f

SOURCES=install.c bargraph.c cat.c getch_yn.c inst.c isfile.c repaint.c unz.c
OBJS=$(SOURCES:.c=.o)

# targets:

all: install.exe

install.exe: libs
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

libs: $(OBJS)

lint:
	$(LINT) $(CFLAGS) $(SOURCES)


# clean up:

clean:
	$(RM) *~

realclean: clean
	$(RM) $(OBJS)

distclean: realclean
	$(RM) install.exe
