# simple makefile for compiling FreeDOS Install program

CFLAGS=-ml -q
LFLAGS=-q

KITTENLIB=kitten\kitten.obj
# DFLATLIB=dfp100s\source\dflatp\dflatpm.lib

OBJS=strchar.obj window.obj yesno.obj

all: install.exe

install.exe: install.obj $(OBJS) kittenlib # dflatlib
	wcl $(LFLAGS) install.obj $(OBJS) $(KITTENLIB)

# deps:

.c.obj: .autodepend
	wcc $(CFLAGS) $<

kittenlib: .symbolic
	cd kitten
	wmake
	cd ..

# dflatlib: .symbolic
# 	cd dfp100s\source\dflatp
# 	wmake
# 	cd ..\..\..

# cleanup:

distclean: realclean .symbolic
	del *.exe
	cd kitten
	wmake distclean
	cd ..
	# cd dfp100s\source\dflatp
	# wmake distclean
	# cd ..\..\..

realclean: clean .symbolic
	del *.obj

clean: .symbolic
	del *.err
