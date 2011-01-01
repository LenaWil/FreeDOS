# Simple makefile for compiling FreeDOS Install program.

# Assumes that you first downloaded & unzipped InfoZip's Unzip 5.52 in
# the unzip552 directory, in the Install source tree.

#DEBUG=-DDEBUG

CC=wcc
CL=wcl
CFLAGS=-ml -q $(DEBUG)
LFLAGS=-q

UNZIP=unzip552

OBJS=kitten.obj splitdir.obj strchar.obj window.obj yesno.obj unz.obj

all: install.exe

install.exe: install.obj $(OBJS) unziplib
	$(CL) $(LFLAGS) install.obj $(OBJS) $(UNZIP)\unzip.lib

unziplib: .symbolic
	cd $(UNZIP)
	$(MAKE)
	cd ..

# deps:

.c.obj: .autodepend
	$(CC) $(CFLAGS) $<

kitten.obj: .symbolic
	$(CC) $(CFLAGS) -Ikitten kitten\kitten.c

test: install.exe .symbolic
	cd t
	mktest
	..\install C:\TEST
	deltree /y *.dir
	cd ..

# cleanup:

distclean: realclean .symbolic
	-del *.exe
	cd $(UNZIP)
	$(MAKE) distclean
	cd ..

realclean: clean .symbolic
	-del *.obj

clean: .symbolic
	-del *.err
