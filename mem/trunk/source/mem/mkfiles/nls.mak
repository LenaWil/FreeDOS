MEM_NLS.H: ..\..\NLS\MEM.DE
    $(PERL) muschi.pl -th < ..\..\NLS\MEM.DE > MEM_NLS.H

MEM_NLS.OBJ: MEM.C MEM_NLS.H
    $(CC) $(CFLAGS) -DMUSCHI $(OBJOUT)MEM_NLS.OBJ MEM.C

MEM_DE.EXE: MEM_NLS.OBJ MEM_DE.OBJ prf.obj kitten.obj $(MEMSUPT)
    $(CC) $(LFLAGS) $(EXEOUT)MEM_DE.EXE MEM_NLS.OBJ MEM_DE.OBJ prf.obj kitten.obj $(MEMSUPT)
    $(UPX) MEM_DE.EXE

MEM_DE.OBJ: MEM_DE.C
    $(CC) $(CFLAGS) $(OBJOUT)MEM_DE.OBJ MEM_DE.C

MEM_DE.C: ..\..\NLS\MEM.DE
    $(PERL) muschi.pl -tc < ..\..\NLS\MEM.DE > MEM_DE.C

MEM_EN.EXE: MEM_NLS.OBJ MEM_EN.OBJ prf.obj kitten.obj $(MEMSUPT)
    $(CC) $(LFLAGS) $(EXEOUT)MEM_EN.EXE MEM_NLS.OBJ MEM_EN.OBJ prf.obj kitten.obj $(MEMSUPT)
    $(UPX) MEM_EN.EXE

MEM_EN.OBJ: MEM_EN.C
    $(CC) $(CFLAGS) $(OBJOUT)MEM_EN.OBJ MEM_EN.C

MEM_EN.C: ..\..\NLS\MEM.EN
    $(PERL) muschi.pl -tc < ..\..\NLS\MEM.EN > MEM_EN.C

MEM_ES.EXE: MEM_NLS.OBJ MEM_ES.OBJ prf.obj kitten.obj $(MEMSUPT)
    $(CC) $(LFLAGS) $(EXEOUT)MEM_ES.EXE MEM_NLS.OBJ MEM_ES.OBJ prf.obj kitten.obj $(MEMSUPT)
    $(UPX) MEM_ES.EXE

MEM_ES.OBJ: MEM_ES.C
    $(CC) $(CFLAGS) $(OBJOUT)MEM_ES.OBJ MEM_ES.C

MEM_ES.C: ..\..\NLS\MEM.ES
    $(PERL) muschi.pl -tc < ..\..\NLS\MEM.ES > MEM_ES.C

MEM_IT.EXE: MEM_NLS.OBJ MEM_IT.OBJ prf.obj kitten.obj $(MEMSUPT)
    $(CC) $(LFLAGS) $(EXEOUT)MEM_IT.EXE MEM_NLS.OBJ MEM_IT.OBJ prf.obj kitten.obj $(MEMSUPT)
    $(UPX) MEM_IT.EXE

MEM_IT.OBJ: MEM_IT.C
    $(CC) $(CFLAGS) $(OBJOUT)MEM_IT.OBJ MEM_IT.C

MEM_IT.C: ..\..\NLS\MEM.IT
    $(PERL) muschi.pl -tc < ..\..\NLS\MEM.IT > MEM_IT.C

MEM_NL.EXE: MEM_NLS.OBJ MEM_NL.OBJ prf.obj kitten.obj $(MEMSUPT)
    $(CC) $(LFLAGS) $(EXEOUT)MEM_NL.EXE MEM_NLS.OBJ MEM_NL.OBJ prf.obj kitten.obj $(MEMSUPT)
    $(UPX) MEM_NL.EXE

MEM_NL.OBJ: MEM_NL.C
    $(CC) $(CFLAGS) $(OBJOUT)MEM_NL.OBJ MEM_NL.C

MEM_NL.C: ..\..\NLS\MEM.NL
    $(PERL) muschi.pl -tc < ..\..\NLS\MEM.NL > MEM_NL.C

NLSSRCALL:  MEM_DE.C MEM_EN.C MEM_ES.C MEM_IT.C MEM_NL.C
NLSOBJALL:  MEM_DE.OBJ MEM_EN.OBJ MEM_ES.OBJ MEM_IT.OBJ MEM_NL.OBJ
NLSBINALL:  MEM_DE.EXE MEM_EN.EXE MEM_ES.EXE MEM_IT.EXE MEM_NL.EXE
