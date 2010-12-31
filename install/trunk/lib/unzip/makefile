## Makefile to compile unzip.lib using Watcom C
## Works with Unzip 552

## Jim Hall <jhall@freedos.org>

CFLAGS=-ml -q -DDLL

.c.obj: .autodepend
	wcc $(CFLAGS) $<

OBJS1=api.obj crc32.obj crctab.obj crypt.obj envargs.obj explode.obj
OBJS2=extract.obj fileio.obj globals.obj inflate.obj list.obj match.obj
OBJS3=msdos.obj process.obj ttyio.obj unreduce.obj unshrink.obj unzip.obj
OBJS4=zipinfo.obj

unzip.lib: $(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4)
	wlib unzip.lib @unzip.lst

msdos.obj:
	wcc $(CFLAGS) msdos\msdos.c

distclean: realclean .symbolic
	del unzip.lib

realclean: clean .symbolic
	del *.obj

clean: .symbolic
	del *.err
