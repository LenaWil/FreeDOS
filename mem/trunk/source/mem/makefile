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


#FIXME: make a separate target MEMDBG.EXE with -DDEBUG
#FIXME: don't commit this file with the modified flags to OpenWatcom
#       to enable debugging symbols

#for WATCOM
#CC  = wcl
#CFLAGS  = -c -j -wx -ms -oahls -s -d2
#LFLAGS  = -fm -"option statics" -"debug all"
#MEMSUPT =
##for Turbo
CC = tcc
CFLAGS  = -f -c -w -ms -a- -k- -N- -d -O -O2
LFLAGS  = -ms -M -l3
MEMSUPT = memsupt.obj

all: mem.exe memtest.exe

# Watcom Make doesn't need quotes but Turbo Make does
!include "nlsbin.mak"

mem.exe: mem.obj prf.obj kitten.obj $(MEMSUPT)
    $(CC) $(LFLAGS) mem.obj prf.obj kitten.obj $(MEMSUPT)
    upx --8086 --best mem.exe

memtest.exe: memtest.obj
    $(CC) $(LFLAGS) memtest.obj

.c.obj:
    $(CC) $(CFLAGS) $<

.asm.obj:
    nasm -fobj $*.asm

clean:
    del *.obj
    del *.map

distclean: clean
    del mem.exe
    del memtest.exe

dist: mem.exe
    copy mem.exe ..\..\bin
    del mem.exe
