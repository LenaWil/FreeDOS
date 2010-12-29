# simple makefile for compiling kitten


CFLAGS=-mm -q


all: kitten.obj

kitten.lib: kitten.obj
	wlib kitten.lib kitten.obj

# deps:

.c.obj: .autodepend
	wcc $(CFLAGS) $<

# cleanup:

distclean: realclean .symbolic
	del *.lib

realclean: clean .symbolic
	del *.obj

clean: .symbolic
	del *.err
