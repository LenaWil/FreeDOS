# Simple makefile for compiling FreeDOS Install program.

# Assumes that you first downloaded & unzipped InfoZip's Unzip 5.52 in
# the unzip552 directory, in the Install source tree.

CC=wcc
CL=wcl
CFLAGS=-ml -q
LFLAGS=-q

OBJS=kitten.obj strchar.obj window.obj yesno.obj unz.obj

all: install.exe

install.exe: install.obj $(OBJS)
	$(CL) $(LFLAGS) install.obj $(OBJS)

# deps:

.c.obj: .autodepend
	$(CC) $(CFLAGS) $<

kitten.obj: .symbolic
	$(CC) $(CFLAGS) -Ikitten kitten\kitten.c

test: install.exe .symbolic
	cd t
	mktest
	..\install
	deltree *.dir
	cd ..

# cleanup:

distclean: realclean .symbolic
	-del *.exe

realclean: clean .symbolic
	-del *.obj

clean: .symbolic
	-del *.err
