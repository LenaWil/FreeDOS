NLSSRC\DE: NLSSRC
    md NLSSRC\DE

NLSOBJ\DE: NLSOBJ
    md NLSOBJ\DE

NLSBIN\DE: NLSBIN
    md NLSBIN\DE

#FIXME: -fe= is a Watcom-specific flag; -e is the TCC version
NLSBIN\DE\MEM.EXE: NLSBIN\DE NLSOBJ\DE\MEM.OBJ prf.obj kitten.obj $(MEMSUPT)
    $(CC) $(LFLAGS) NLSOBJ\DE\MEM.OBJ prf.obj kitten.obj $(MEMSUPT) -fe=$@

#FIXME: -fo= is a Watcom-specific flag; -o is the TCC version
NLSOBJ\DE\MEM.OBJ: NLSOBJ\DE mem.c NLSSRC\DE\MUSCHI.C
# pass -I so we pick up muschi.c from the right directory
    $(CC) $(CFLAGS) -DMUSCHI -INLSSRC\DE mem.c -fo=$@

NLSSRC\DE\MUSCHI.C: ..\..\NLS\MEM.DE NLSSRC\DE
    perl muschi.pl NLSSRC\DE\MUSCHI < ..\..\NLS\MEM.DE 

NLSSRC\EN: NLSSRC
    md NLSSRC\EN

NLSOBJ\EN: NLSOBJ
    md NLSOBJ\EN

NLSBIN\EN: NLSBIN
    md NLSBIN\EN

#FIXME: -fe= is a Watcom-specific flag; -e is the TCC version
NLSBIN\EN\MEM.EXE: NLSBIN\EN NLSOBJ\EN\MEM.OBJ prf.obj kitten.obj $(MEMSUPT)
    $(CC) $(LFLAGS) NLSOBJ\EN\MEM.OBJ prf.obj kitten.obj $(MEMSUPT) -fe=$@

#FIXME: -fo= is a Watcom-specific flag; -o is the TCC version
NLSOBJ\EN\MEM.OBJ: NLSOBJ\EN mem.c NLSSRC\EN\MUSCHI.C
# pass -I so we pick up muschi.c from the right directory
    $(CC) $(CFLAGS) -DMUSCHI -INLSSRC\EN mem.c -fo=$@

NLSSRC\EN\MUSCHI.C: ..\..\NLS\MEM.EN NLSSRC\EN
    perl muschi.pl NLSSRC\EN\MUSCHI < ..\..\NLS\MEM.EN 

NLSSRC\ES: NLSSRC
    md NLSSRC\ES

NLSOBJ\ES: NLSOBJ
    md NLSOBJ\ES

NLSBIN\ES: NLSBIN
    md NLSBIN\ES

#FIXME: -fe= is a Watcom-specific flag; -e is the TCC version
NLSBIN\ES\MEM.EXE: NLSBIN\ES NLSOBJ\ES\MEM.OBJ prf.obj kitten.obj $(MEMSUPT)
    $(CC) $(LFLAGS) NLSOBJ\ES\MEM.OBJ prf.obj kitten.obj $(MEMSUPT) -fe=$@

#FIXME: -fo= is a Watcom-specific flag; -o is the TCC version
NLSOBJ\ES\MEM.OBJ: NLSOBJ\ES mem.c NLSSRC\ES\MUSCHI.C
# pass -I so we pick up muschi.c from the right directory
    $(CC) $(CFLAGS) -DMUSCHI -INLSSRC\ES mem.c -fo=$@

NLSSRC\ES\MUSCHI.C: ..\..\NLS\MEM.ES NLSSRC\ES
    perl muschi.pl NLSSRC\ES\MUSCHI < ..\..\NLS\MEM.ES 

NLSSRC\IT: NLSSRC
    md NLSSRC\IT

NLSOBJ\IT: NLSOBJ
    md NLSOBJ\IT

NLSBIN\IT: NLSBIN
    md NLSBIN\IT

#FIXME: -fe= is a Watcom-specific flag; -e is the TCC version
NLSBIN\IT\MEM.EXE: NLSBIN\IT NLSOBJ\IT\MEM.OBJ prf.obj kitten.obj $(MEMSUPT)
    $(CC) $(LFLAGS) NLSOBJ\IT\MEM.OBJ prf.obj kitten.obj $(MEMSUPT) -fe=$@

#FIXME: -fo= is a Watcom-specific flag; -o is the TCC version
NLSOBJ\IT\MEM.OBJ: NLSOBJ\IT mem.c NLSSRC\IT\MUSCHI.C
# pass -I so we pick up muschi.c from the right directory
    $(CC) $(CFLAGS) -DMUSCHI -INLSSRC\IT mem.c -fo=$@

NLSSRC\IT\MUSCHI.C: ..\..\NLS\MEM.IT NLSSRC\IT
    perl muschi.pl NLSSRC\IT\MUSCHI < ..\..\NLS\MEM.IT 

NLSSRC\NL: NLSSRC
    md NLSSRC\NL

NLSOBJ\NL: NLSOBJ
    md NLSOBJ\NL

NLSBIN\NL: NLSBIN
    md NLSBIN\NL

#FIXME: -fe= is a Watcom-specific flag; -e is the TCC version
NLSBIN\NL\MEM.EXE: NLSBIN\NL NLSOBJ\NL\MEM.OBJ prf.obj kitten.obj $(MEMSUPT)
    $(CC) $(LFLAGS) NLSOBJ\NL\MEM.OBJ prf.obj kitten.obj $(MEMSUPT) -fe=$@

#FIXME: -fo= is a Watcom-specific flag; -o is the TCC version
NLSOBJ\NL\MEM.OBJ: NLSOBJ\NL mem.c NLSSRC\NL\MUSCHI.C
# pass -I so we pick up muschi.c from the right directory
    $(CC) $(CFLAGS) -DMUSCHI -INLSSRC\NL mem.c -fo=$@

NLSSRC\NL\MUSCHI.C: ..\..\NLS\MEM.NL NLSSRC\NL
    perl muschi.pl NLSSRC\NL\MUSCHI < ..\..\NLS\MEM.NL 

NLSSRCALL:  NLSSRC\DE\MUSCHI.C NLSSRC\EN\MUSCHI.C NLSSRC\ES\MUSCHI.C NLSSRC\IT\MUSCHI.C NLSSRC\NL\MUSCHI.C
NLSOBJALL:  NLSOBJ\DE\MEM.OBJ NLSOBJ\EN\MEM.OBJ NLSOBJ\ES\MEM.OBJ NLSOBJ\IT\MEM.OBJ NLSOBJ\NL\MEM.OBJ
NLSBINALL:  NLSBIN\DE\MEM.EXE NLSBIN\EN\MEM.EXE NLSBIN\ES\MEM.EXE NLSBIN\IT\MEM.EXE NLSBIN\NL\MEM.EXE

# make the base directories:
NLSSRC NLSOBJ NLSBIN:
    md $(

