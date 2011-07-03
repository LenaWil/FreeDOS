# Simple makefile for compiling FreeDOS Install program.

# Assumes that you first downloaded & unzipped InfoZip's Unzip 5.52 in
# the unzip552 directory, in the Install source tree.

#DEBUG=-DDEBUG

CC=wcc
CL=wcl
CFLAGS=-ml -q $(DEBUG)
LFLAGS=-q

UNZIP=unzip552

OBJS=kitten.obj strchar.obj window.obj yesno.obj unz.obj

install.exe: install.obj $(OBJS) $(UNZIP)\unzip.lib
	$(CL) $(LFLAGS) install.obj $(OBJS) $(UNZIP)\unzip.lib

test: install.exe .symbolic
	cd t
	mktest
	..\install C:\TEST
	deltree /y *.dir
	cd ..

# deps:

.c.obj: .autodepend
	$(CC) $(CFLAGS) $<

$(UNZIP)\unzip.lib: .symbolic
	cd $(UNZIP)
	$(MAKE)
	cd ..

kitten.obj: .symbolic
	$(CC) $(CFLAGS) -Ikitten kitten\kitten.c

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
