#
# Makefile for FreeDOS MEM
#
#
# You might need to change the following lines if you use another
# compiler than Turbo / Watcom C
#
# for Turbo C:
# assuming that a) tcc , tlink and nasm are all your path and
# b) these are configured the right way, for instance using a turboc.cfg
# file in the same directory with e.g. -IC:\TC\INCLUDE -LC:\TC\LIB.
#
# for Watcom: make sure it's setup correctly

#for WATCOM
CC	= wcl
CFLAGS  = -c -j -wx -ms -oahls -s -d1
LFLAGS  = -fm -"option statics"
MEMSUPT =
##for Turbo
#CC	= tcc
#CFLAGS  = -c -w -mt -a- -k- -N- -d -O -O2
#LFLAGS  = -mt -M
#MEMSUPT = memsupt.obj

mem.exe: mem.obj prf.obj $(MEMSUPT)
	$(CC) $(LFLAGS) mem.obj prf.obj $(MEMSUPT)
	upx --8086 --best mem.exe

.c.obj:
	$(CC) $(CFLAGS) $*.c

.asm.obj:
	nasm -fobj $*.asm

clean:
	del *.obj
	del *.map

distclean: clean
	del mem.exe

dist: mem.exe
	copy mem.exe ..\..\bin
	del mem.exe
