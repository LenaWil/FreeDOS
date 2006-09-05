/*---------------------------------------------------------------------------

  unzpriv.h

  This header file contains private (internal) macros, typedefs, prototypes
  and global-variable declarations used by all of the UnZip source files.
  In a prior life it was part of the main unzip.h header, but now it is only
  included by that header if UNZIP_INTERNAL is defined.

  ---------------------------------------------------------------------------*/



#ifndef __unzpriv_h   /* prevent multiple inclusions */
#define __unzpriv_h

/* First thing: Signal all following code that we compile UnZip utilities! */
#ifndef UNZIP
#  define UNZIP
#endif

/* GRR 960204:  MORE defined here in preparation for removal altogether */
#if 0
#ifndef MORE
#  define MORE
#endif
#endif

/* GRR 960218:  testing a new feature...definitely *not* ready for primetime */
#if (defined(TIMESTAMP) && !defined(UNIX))
#  undef TIMESTAMP
#endif

/* fUnZip should never need to be reentrant */
#ifdef FUNZIP
#  ifdef REENTRANT
#    undef REENTRANT
#  endif
#  ifdef DLL
#    undef DLL
#  endif
#endif

#if (defined(DLL) && !defined(REENTRANT))
#  define REENTRANT
#endif

#ifndef DYNAMIC_CRC_TABLE
#  define DYNAMIC_CRC_TABLE
#endif

#if (defined(DYNAMIC_CRC_TABLE) && !defined(REENTRANT))
#  ifndef DYNALLOC_CRCTAB
#    define DYNALLOC_CRCTAB
#  endif
#endif

#ifndef MINIX            /* Minix needs it after all the other includes (?) */
#  include <stdio.h>
#endif
#include <ctype.h>       /* skip for VMS, to use tolower() function? */
#include <errno.h>       /* used in mapname() */
#ifdef USE_STRINGS_H
#  include <strings.h>   /* strcpy, strcmp, memcpy, index/rindex, etc. */
#else
#  include <string.h>    /* strcpy, strcmp, memcpy, strchr/strrchr, etc. */
#endif
#if (defined(MODERN) && !defined(NO_LIMITS_H))
#  include <limits.h>    /* GRR:  EXPERIMENTAL!  (can be deleted) */
#endif

/* this include must be down here for SysV.4, for some reason... */
#include <signal.h>      /* used in unzip.c, fileio.c */


/*---------------------------------------------------------------------------
    API (DLL) section:
  ---------------------------------------------------------------------------*/

#ifdef DLL
#  define MAIN  UzpMain   /* was UzpUnzip */
#  ifdef OS2API
#    undef Info
#    define REDIRECTC(c)             varputchar(__G__ c)
#    define REDIRECTPRINT(buf,size)  varmessage(__G__ buf, size)
#    define FINISH_REDIRECT()        finish_REXX_redirect(__G)
#  else
#    define REDIRECTC(c)
#    define REDIRECTPRINT(buf,size)  0
#    define FINISH_REDIRECT()
#  endif
#endif

/*---------------------------------------------------------------------------
    Acorn RISCOS section:
  ---------------------------------------------------------------------------*/

#ifdef RISCOS
#  include "acorn/riscos.h"
#endif

/*---------------------------------------------------------------------------
    Amiga section:
  ---------------------------------------------------------------------------*/

#ifdef AMIGA
#  include "amiga/amiga.h"
#endif

/*---------------------------------------------------------------------------
    AOS/VS section (somewhat similar to Unix, apparently):
  ---------------------------------------------------------------------------*/

#ifdef AOS_VS
#  ifdef FILEIO_C
#    include "aosvs/aosvs.h"
#  endif
#endif

/*---------------------------------------------------------------------------
    Atari ST section:
  ---------------------------------------------------------------------------*/

#ifdef ATARI
#  include <time.h>
#  include <stat.h>
#  include <fcntl.h>
#  include <limits.h>
#  define SYMLINKS
#  define EXE_EXTENSION  ".tos"
#  define DATE_FORMAT    DF_MDY
#  define DIR_END        '/'
#  define INT_SPRINTF
#  define timezone      _timezone
#  define lenEOL        2
#  define PutNativeEOL  {*q++ = native(CR); *q++ = native(LF);}
#  undef SHORT_NAMES
#endif

/*---------------------------------------------------------------------------
    Human68k/X68000 section:
  ---------------------------------------------------------------------------*/

#ifdef __human68k__    /* DO NOT DEFINE DOS_OS2 HERE!  If Human68k is so much */
#  include <time.h>    /*  like MS-DOS and/or OS/2, create DOS_H68_OS2 macro. */
#  include <fcntl.h>
#  include <io.h>
#  include <conio.h>
#  include <jctype.h>
#  include <sys/stat.h>
#  define DATE_FORMAT   DF_YMD   /* Japanese standard */
      /* GRR:  these EOL macros are guesses */
#  define lenEOL        2
#  define PutNativeEOL  {*q++ = native(CR); *q++ = native(LF);}
#  define EXE_EXTENSION ".exe"   /* just a guess... */
#endif

/*---------------------------------------------------------------------------
    Mac section:
  ---------------------------------------------------------------------------*/

#ifdef THINK_C
#  define MACOS
#  ifndef __STDC__            /* if Think C hasn't defined __STDC__ ... */
#    define __STDC__ 1        /*   make sure it's defined: it needs it */
#  else
#    if !__STDC__             /* sometimes __STDC__ is defined as 0; */
#      undef __STDC__         /*   it needs to be 1 or required header */
#      define __STDC__ 1      /*   files are not properly included. */
#    endif /* !__STDC__ */
#  endif
#  define CREATOR  'KAHL'
#  define MAIN     _dummy_main
#endif /* THINK_C */

#ifdef MPW
#  define MACOS
#  include <Errors.h>
#  include <Files.h>
#  include <Memory.h>
#  include <Quickdraw.h>
#  include <ToolUtils.h>
#  ifdef fileno
#    undef fileno
#  endif
#  ifdef MCH_MACINTOSH
#    define CREATOR     'Manx'
#  else
#    define CREATOR     'MPS '
#  endif
#endif /* MPW */

#ifdef MACOS
#  include <fcntl.h>            /* O_BINARY for open() w/o CR/LF translation */
#  define fileno(x)     ((x) == stdout ? 1 : ((x) == stderr ? 2 : (short)(x)))
#  define open(x,y)     macopen((x), (y), G.gnVRefNum, G.glDirID)
#  define fopen(x,y)    macfopen((x), (y), G.gnVRefNum, G.glDirID)
#  define close         macclose
#  define fclose(x)     macclose(fileno((x)))
#  define read          macread
#  define write         macwrite
#  define lseek         maclseek
#  define creat(x,y)    maccreat((x), G.gnVRefNum, G.glDirID, G.gostCreator, G.gostType)
#  define stat(x,y)     macstat((x), (y), G.gnVRefNum, G.glDirID)
#  define dup
#  ifndef MCH_MACINTOSH
#    define NO_STRNICMP
#  endif
#  define DIR_END ':'
#  define DATE_FORMAT   DF_MDY
#  define lenEOL        1
#  define PutNativeEOL  *q++ = native(CR);
#  define MALLOC_WORK
#  define INT_SPRINTF

#  ifdef THINK_C
#    define fgets       wfgets
#    define fflush(f)
#    define fprintf     wfprintf
#    define fputs(s,f)  wfprintf((f), "%s", (s))
#    define isatty(f)   (((f) >= 0) || ((f) <= 2))
#    define printf      wprintf
#    ifdef putc
#      undef putc
#    endif
#    define putc(c,f)   wfprintf((f), "%c", (c))
#    define getenv      macgetenv
#  endif

#  ifndef isascii
#    define isascii(c)  ((unsigned char)(c) <= 0x3F)
#  endif

#  include "macstat.h"
#  include "macdir.h"

#  ifdef CR
#    undef  CR
#  endif

typedef struct _ZipExtraHdr {
    ush header;               /*    2 bytes */
    ush data;                 /*    2 bytes */
} ZIP_EXTRA_HEADER;

typedef struct _MacInfoMin {
    ush header;               /*    2 bytes */
    ush data;                 /*    2 bytes */
    ulg signature;            /*    4 bytes */
    FInfo finfo;              /*   16 bytes */
    ulg lCrDat;               /*    4 bytes */
    ulg lMdDat;               /*    4 bytes */
    ulg flags ;               /*    4 bytes */
    ulg lDirID;               /*    4 bytes */
                              /*------------*/
} MACINFOMIN;                 /* = 40 bytes for size of data */

typedef struct _MacInfo {
    ush header;               /*    2 bytes */
    ush data;                 /*    2 bytes */
    ulg signature;            /*    4 bytes */
    FInfo finfo;              /*   16 bytes */
    ulg lCrDat;               /*    4 bytes */
    ulg lMdDat;               /*    4 bytes */
    ulg flags ;               /*    4 bytes */
    ulg lDirID;               /*    4 bytes */
    char rguchVolName[28];    /*   28 bytes */
                              /*------------*/
} MACINFO;                    /* = 68 bytes for size of data */
#endif /* MACOS */

/*---------------------------------------------------------------------------
    MS-DOS and OS/2 section:
  ---------------------------------------------------------------------------*/

#ifdef WIZUNZIP
#  define MSWIN
#  ifdef MORE
#    undef MORE
#  endif
#endif

#if (defined(_MSC_VER) || (defined(M_I86) && !defined(__WATCOMC__)))
#  ifndef MSC
#    define MSC               /* This should work for older MSC, too!  */
#  endif
#endif

#ifdef MSWIN
#  include "wingui\wizunzip.h"
#endif

#if (defined(MSDOS) || defined(OS2))
#  include <sys/types.h>      /* off_t, time_t, dev_t, ... */
#  include <sys/stat.h>
#  include <io.h>             /* lseek(), open(), setftime(), dup(), creat() */
#  include <time.h>           /* localtime() */
#  include <fcntl.h>          /* O_BINARY for open() w/o CR/LF translation */

#  ifdef OS2                  /* defined for all OS/2 compilers */
#    include "os2/os2cfg.h"
#  else
#    include "msdos/doscfg.h"
#  endif

#  ifdef __GO32__
#    define DIR_END '/'
#  else
#    define DIR_END '\\'
#  endif
#  ifndef WIN32
#    define DATE_FORMAT   dateformat()
#  endif
#  define lenEOL          2
#  define PutNativeEOL    {*q++ = native(CR); *q++ = native(LF);}
#  define USE_EF_UX_TIME
#endif /* MSDOS || OS2 */

/*---------------------------------------------------------------------------
    MTS section (piggybacks UNIX, I think):
  ---------------------------------------------------------------------------*/

#ifdef MTS
#  include <sys/types.h>      /* off_t, time_t, dev_t, ... */
#  include <sys/stat.h>
#  include <sys/file.h>       /* MTS uses this instead of fcntl.h */
#  include <timeb.h>
#  include <time.h>
#  include <unix.h>           /* some important non-ANSI routines */
#  define mkdir(s,n) (-1)     /* no "make directory" capability */
#  define EBCDIC              /* set EBCDIC conversion on */
#  define NO_STRNICMP         /* unzip's is as good the one in MTS */
#  define USE_FWRITE
#  define close_outfile()  fclose(G.outfile)   /* can't set time on files */
#  define umask(n)            /* don't have umask() on MTS */
#  define FOPWT         "w"   /* open file for writing in TEXT mode */
#  define DATE_FORMAT   DF_MDY
#  define lenEOL        1
#  define PutNativeEOL  *q++ = native(LF);
#endif /* MTS */

/*---------------------------------------------------------------------------
    Win32 section:
  ---------------------------------------------------------------------------*/

#ifdef WIN32  /* NT and Win95 */
#  include "win32/w32cfg.h"
#endif

/*---------------------------------------------------------------------------
    TOPS-20 section:
  ---------------------------------------------------------------------------*/

#ifdef TOPS20
#  include <sys/types.h>        /* off_t, time_t, dev_t, ... */
#  include <sys/stat.h>
#  include <sys/param.h>
#  include <sys/time.h>
#  include <sys/timeb.h>
#  include <sys/file.h>
#  include <timex.h>
#  include <monsym.h>           /* get amazing monsym() macro */
   extern int open(), close(), read();
   extern int stat(), unlink(), jsys(), fcntl();
   extern long lseek(), dup(), creat();
#  define strchr    index       /* GRR: necessary? */
#  define strrchr   rindex
#  define REALLY_SHORT_SYMS
#  define NO_MKDIR
#  define DIR_BEG       '<'
#  define DIR_END       '>'
#  define DIR_EXT       ".directory"
#  define DATE_FORMAT   DF_MDY
#  define EXE_EXTENSION ".exe"  /* just a guess... */
#endif /* TOPS20 */

/*---------------------------------------------------------------------------
    Unix section:
  ---------------------------------------------------------------------------*/

#ifdef UNIX
#  include <sys/types.h>       /* off_t, time_t, dev_t, ... */
#  include <sys/stat.h>

#  ifndef COHERENT
#    include <fcntl.h>         /* O_BINARY for open() w/o CR/LF translation */
#  else /* COHERENT */
#    ifdef _I386
#      include <fcntl.h>       /* Coherent 4.0.x, Mark Williams C */
#    else
#      include <sys/fcntl.h>   /* Coherent 3.10, Mark Williams C */
#    endif
#    define SHORT_SYMS
#    ifndef __COHERENT__       /* Coherent 4.2 has tzset() */
#      define tzset  settz
#    endif
#  endif /* ?COHERENT */

#  ifndef NO_PARAM_H
#    ifdef NGROUPS_MAX
#      undef NGROUPS_MAX       /* SCO bug:  defined again in <sys/param.h> */
#    endif
#    ifdef BSD
#      define TEMP_BSD         /* may be defined again in <sys/param.h> */
#      undef BSD
#    endif
#    include <sys/param.h>     /* conflict with <sys/types.h>, some systems? */
#    ifdef TEMP_BSD
#      undef TEMP_BSD
#      ifndef BSD
#        define BSD
#      endif
#    endif
#  endif /* !NO_PARAM_H */

#  ifdef __osf__
#    define DIRENT
#    ifdef BSD
#      undef BSD
#    endif
#  endif /* __osf__ */

#  ifdef BSD
#    include <sys/time.h>
#    include <sys/timeb.h>
#    ifdef _AIX
#      include <time.h>
#    endif
#  else
#    include <time.h>
     struct tm *gmtime(), *localtime();
#  endif

#  if (defined(BSD4_4) || (defined(SYSV) && defined(MODERN)))
#    include <unistd.h>        /* this includes utime.h on SGIs */
#    ifdef BSD4_4
#      include <utime.h>
#    endif
#  endif

#  if (defined(V7) || defined(pyr_bsd))
#    define strchr   index
#    define strrchr  rindex
#  endif
#  ifdef V7
#    define O_RDONLY 0
#    define O_WRONLY 1
#    define O_RDWR   2
#  endif

#  ifdef MINIX
#    include <stdio.h>
#  endif
#  define DATE_FORMAT   DF_MDY    /* GRR:  customize with locale.h somehow? */
#  define lenEOL        1
#  define PutNativeEOL  *q++ = native(LF);
#  define SCREENLINES   screenlines()
#  define USE_EF_UX_TIME
#endif /* UNIX */

/*---------------------------------------------------------------------------
    VM/CMS and MVS section:
  ---------------------------------------------------------------------------*/

#ifdef CMS_MVS
#  include "vmmvs.h"
#endif

/*---------------------------------------------------------------------------
    VMS section:
  ---------------------------------------------------------------------------*/

#ifdef VMS
#  include <types.h>                    /* GRR:  experimenting... */
#  include <stat.h>
#  include <time.h>                     /* the usual non-BSD time functions */
#  include <file.h>                     /* same things as fcntl.h has */
#  include <unixio.h>
#  include <rms.h>
#  define _MAX_PATH (NAM$C_MAXRSS+1)    /* to define FILNAMSIZ below */
#  ifdef RETURN_CODES  /* VMS interprets standard PK return codes incorrectly */
#    define RETURN(ret) return_VMS(__G__ (ret))   /* verbose version */
#    define EXIT(ret)   return_VMS(__G__ (ret))
#  else
#    define RETURN      return_VMS                /* quiet version */
#    define EXIT        return_VMS
#  endif
#  define DIR_BEG       '['
#  define DIR_END       ']'
#  define DIR_EXT       ".dir"
#  define DATE_FORMAT   DF_MDY
#  define lenEOL        1
#  define PutNativeEOL  *q++ = native(LF);
#  define SCREENLINES   screenlines()
#  if ((!defined(__VMS_VER)) || (__VMS_VER < 70000000))
#    define NO_GMTIME           /* gmtime() of earlier VMS C RTLs is broken */
#  else
#    if (!defined(NO_EF_UX_TIME) && !defined(USE_EF_UX_TIME))
#      define USE_EF_UX_TIME
#    endif
#  endif
#endif /* VMS */





/*************/
/*  Defines  */
/*************/

#define UNZIP_VERSION     20   /* compatible with PKUNZIP 2.0 */
#define VMS_UNZIP_VERSION 42   /* if OS-needed-to-extract is VMS:  can do */

#if (defined(MSDOS) || defined(OS2))
#  define DOS_OS2
#endif

#if (defined(DOS_OS2) || defined(WIN32))
#  define DOS_OS2_W32
#  define DOS_W32_OS2          /* historical:  don't use */
#endif

#if (defined(DOS_OS2_W32) || defined(__human68k__))
#  define DOS_H68_OS2_W32
#endif

#if (defined(TOPS20) || defined(VMS))
#  define T20_VMS
#endif

#if (defined(MSDOS) || defined(T20_VMS))
#  define DOS_T20_VMS
#endif

/* clean up with a few defaults */
#ifndef DIR_END
#  define DIR_END '/'       /* last char before program name (or filename) */
#endif
#ifndef DATE_FORMAT
#  define DATE_FORMAT   DF_MDY  /* defaults to US convention */
#endif
#ifndef RETURN
#  define RETURN  return        /* only used in main() */
#endif
#ifndef EXIT
#  define EXIT    exit
#endif

/* OS-specific exceptions to the "ANSI <--> INT_SPRINTF" rule */

#if (!defined(PCHAR_SPRINTF) && !defined(INT_SPRINTF))
#  if (defined(SYSV) || defined(CONVEX) || defined(NeXT) || defined(BSD4_4))
#    define INT_SPRINTF      /* sprintf() returns int:  SysVish/Posix */
#  endif
#  if (defined(DOS_OS2_W32) || defined(VMS) || defined(AMIGA) || defined(_AIX))
#    define INT_SPRINTF      /* sprintf() returns int:  ANSI */
#  endif
#  if (defined(ultrix) || defined(__ultrix)) /* Ultrix 4.3 and newer */
#    if (defined(POSIX) || defined(__POSIX))
#      define INT_SPRINTF    /* sprintf() returns int:  ANSI/Posix */
#    endif
#    ifdef __GNUC__
#      define PCHAR_SPRINTF  /* undetermined actual return value */
#    endif
#  endif
#  if (defined(__osf__) || defined(CMS_MVS))
#    define INT_SPRINTF      /* sprintf() returns int:  ANSI/Posix */
#  endif
#  if defined(sun)
#    define PCHAR_SPRINTF    /* sprintf() returns char *:  SunOS cc *and* gcc */
#  endif
#endif

/* defaults that we hope will take care of most machines in the future */

#if (!defined(PCHAR_SPRINTF) && !defined(INT_SPRINTF))
#  ifdef __STDC__
#    define INT_SPRINTF      /* sprintf() returns int:  ANSI */
#  endif
#  ifndef INT_SPRINTF
#    define PCHAR_SPRINTF    /* sprintf() returns char *:  BSDish */
#  endif
#endif

#define MSG_STDERR(f)  (f & 1)        /* bit 0:  0 = stdout, 1 = stderr */
#define MSG_INFO(f)    ((f & 6) == 0) /* bits 1 and 2:  0 = info */
#define MSG_WARN(f)    ((f & 6) == 2) /* bits 1 and 2:  1 = warning */
#define MSG_ERROR(f)   ((f & 6) == 4) /* bits 1 and 2:  2 = error */
#define MSG_FATAL(f)   ((f & 6) == 6) /* bits 1 and 2:  (3 = fatal error) */
#define MSG_ZFN(f)     (f & 0x0008)   /* bit 3:  1 = print zipfile name */
#define MSG_FN(f)      (f & 0x0010)   /* bit 4:  1 = print filename */
#define MSG_LNEWLN(f)  (f & 0x0020)   /* bit 5:  1 = leading newline if !SOL */
#define MSG_TNEWLN(f)  (f & 0x0040)   /* bit 6:  1 = trailing newline if !SOL */
#define MSG_MNEWLN(f)  (f & 0x0080)   /* bit 7:  1 = trailing newline if MPW */
/* the following are subject to change */
#define MSG_NO_WGUI(f) (f & 0x0100)   /* bit 8:  1 = skip if Windows GUI */
#define MSG_NO_AGUI(f) (f & 0x0200)   /* bit 9:  1 = skip if Acorn GUI */
#define MSG_NO_DLL2(f) (f & 0x0400)   /* bit 10:  1 = skip if OS/2 DLL */
#define MSG_NO_NDLL(f) (f & 0x0800)   /* bit 11:  1 = skip if WIN32 DLL */
#define MSG_NO_WDLL(f) (f & 0x1000)   /* bit 12:  1 = skip if Windows DLL */

#if (defined(MORE) && !defined(SCREENLINES))
#  ifdef DOS_OS2_W32
#    define SCREENLINES 25  /* can be (should be) a function instead */
#  else
#    define SCREENLINES 24  /* VT-100s are assumed to be minimal hardware */
#  endif
#endif

#define DIR_BLKSIZ  64      /* number of directory entries per block
                             *  (should fit in 4096 bytes, usually) */
#ifndef WSIZE
#  define WSIZE     0x8000  /* window size--must be a power of two, and */
#endif                      /*  at least 32K for zip's deflate method */

#ifndef INBUFSIZ
#  if (defined(MED_MEM) || defined(SMALL_MEM))
#    define INBUFSIZ  2048  /* works for MS-DOS small model */
#  else
#    define INBUFSIZ  8192  /* larger buffers for real OSes */
#  endif
#endif

#ifndef __16BIT__
#  define nearmalloc  malloc
#  define nearfree    free
#  ifndef __IBMC__
#    define near
#    define far
#  endif
#endif

#if (defined(USE_ZLIB) && !defined(USE_OWN_CRCTAB))
#  ifdef DYNALLOC_CRCTAB
#    undef DYNALLOC_CRCTAB
#  endif
#endif

#if (defined(USE_ZLIB) && defined(ASM_CRC))
#  undef ASM_CRC
#endif

/* Logic for case of small memory, length of EOL > 1:  if OUTBUFSIZ == 2048,
 * OUTBUFSIZ>>1 == 1024 and OUTBUFSIZ>>7 == 16; therefore rawbuf is 1008 bytes
 * and transbuf 1040 bytes.  Have room for 32 extra EOL chars; 1008/32 == 31.5
 * chars/line, smaller than estimated 35-70 characters per line for C source
 * and normal text.  Hence difference is sufficient for most "average" files.
 * (Argument scales for larger OUTBUFSIZ.)
 */
#ifdef SMALL_MEM          /* i.e., 16-bit OSes:  MS-DOS, OS/2 1.x, etc. */
#  define LoadFarString(x)       fLoadFarString(__G__ (x))
#  define LoadFarStringSmall(x)  fLoadFarStringSmall(__G__ (x))
#  define LoadFarStringSmall2(x) fLoadFarStringSmall2(__G__ (x))
#  if (defined(_MSC_VER) && (_MSC_VER >= 600))
#    define zfstrcpy(dest, src)  _fstrcpy((dest), (src))
#  endif
#  ifndef Far
#    define Far far  /* __far only works for MSC 6.00, not 6.0a or Borland */
#  endif
#  define OUTBUFSIZ INBUFSIZ
#  if (lenEOL == 1)
#    define RAWBUFSIZ (OUTBUFSIZ>>1)
#  else
#    define RAWBUFSIZ ((OUTBUFSIZ>>1) - (OUTBUFSIZ>>7))
#  endif
#  define TRANSBUFSIZ (OUTBUFSIZ-RAWBUFSIZ)
   typedef short  shrint;            /* short/int or "shrink int" (unshrink) */
#else
#  define zfstrcpy(dest, src)    strcpy((dest), (src))
#  define LoadFarString(x)       x
#  define LoadFarStringSmall(x)  x
#  define LoadFarStringSmall2(x) x
#  ifdef MED_MEM
#    define OUTBUFSIZ 0xFF80         /* can't malloc arrays of 0xFFE8 or more */
#    define TRANSBUFSIZ 0xFF80
     typedef short  shrint;
#  else
#    define OUTBUFSIZ (lenEOL*WSIZE) /* more efficient text conversion */
#    define TRANSBUFSIZ (lenEOL*OUTBUFSIZ)
     typedef int  shrint;            /* for efficiency/speed, we hope... */
#  endif
#  define RAWBUFSIZ OUTBUFSIZ
#endif /* ?SMALL_MEM */

#ifndef Far
#  define Far
#endif

#ifndef MAIN
#  define MAIN  main
#endif

#if (defined(SFX) && !defined(NO_ZIPINFO))
#  define NO_ZIPINFO
#endif

#if (!defined(COPYRIGHT_CLEAN) && !defined(USE_SMITH_CODE))
#  define COPYRIGHT_CLEAN
#endif

#ifndef O_BINARY
#  define O_BINARY  0
#endif

#ifndef PIPE_ERROR
#  define PIPE_ERROR (errno == EPIPE)
#endif

/* File operations--use "b" for binary if allowed or fixed length 512 on VMS */
#ifdef VMS
#  define FOPR  "r","ctx=stm"
#  define FOPM  "r+","ctx=stm","rfm=fix","mrs=512"
#  define FOPW  "w","ctx=stm","rfm=fix","mrs=512"
#endif /* VMS */

#ifdef CMS_MVS
#  define FOPW "wb,recfm=f"
#  ifdef MVS
#    define FOPWT "w,lrecl=133"
#  else
#    define FOPWT "w"
#  endif
#endif /* CMS_MVS */

#ifdef TOPS20          /* TOPS-20 MODERN?  You kidding? */
#  define FOPW "w8"
#endif /* TOPS20 */

/* Defaults when nothing special has been defined previously. */
#ifdef MODERN
#  ifndef FOPR
#    define FOPR "rb"
#  endif
#  ifndef FOPM
#    define FOPM "r+b"
#  endif
#  ifndef FOPW
#    define FOPW "wb"
#  endif
#  ifndef FOPWT
#    define FOPWT "wt"
#  endif
#else /* !MODERN */
#  ifndef FOPR
#    define FOPR "r"
#  endif
#  ifndef FOPM
#    define FOPM "r+"
#  endif
#  ifndef FOPW
#    define FOPW "w"
#  endif
#  ifndef FOPWT
#    define FOPWT "w"
#  endif
#endif /* ?MODERN */

/*
 * If <limits.h> exists on most systems, should include that, since it may
 * define some or all of the following:  NAME_MAX, PATH_MAX, _POSIX_NAME_MAX,
 * _POSIX_PATH_MAX.
 */
#ifdef DOS_OS2
#  include <limits.h>
#endif

#ifndef PATH_MAX
#  ifdef MAXPATHLEN
#    define PATH_MAX      MAXPATHLEN    /* in <sys/param.h> on some systems */
#  else
#    ifdef _MAX_PATH
#      define PATH_MAX    _MAX_PATH
#    else
#      if FILENAME_MAX > 255
#        define PATH_MAX  FILENAME_MAX  /* used like PATH_MAX on some systems */
#      else
#        define PATH_MAX  1024
#      endif
#    endif /* ?_MAX_PATH */
#  endif /* ?MAXPATHLEN */
#endif /* !PATH_MAX */

#define FILNAMSIZ  PATH_MAX

#ifdef SHORT_SYMS                   /* Mark Williams C, ...? */
#  define extract_or_test_files     xtr_or_tst_files
#  define extract_or_test_member    xtr_or_tst_member
#endif

#ifdef REALLY_SHORT_SYMS            /* TOPS-20 linker:  first 6 chars */
#  define process_cdir_file_hdr     XXpcdfh
#  define process_local_file_hdr    XXplfh
#  define extract_or_test_files     XXxotf  /* necessary? */
#  define extract_or_test_member    XXxotm  /* necessary? */
#  define check_for_newer           XXcfn
#  define overwrite_all             XXoa
#  define process_all_files         XXpaf
#  define extra_field               XXef
#  define explode_lit8              XXel8
#  define explode_lit4              XXel4
#  define explode_nolit8            XXnl8
#  define explode_nolit4            XXnl4
#  define cpdist8                   XXcpdist8
#  define inflate_codes             XXic
#  define inflate_stored            XXis
#  define inflate_fixed             XXif
#  define inflate_dynamic           XXid
#  define inflate_block             XXib
#  define maxcodemax                XXmax
#endif

#ifdef RISCOS
#  define ZSUFX           "/zip"
#else
#  define ZSUFX           ".zip"
#endif

#define CENTRAL_HDR_SIG   "\001\002"   /* the infamous "PK" signature bytes, */
#define LOCAL_HDR_SIG     "\003\004"   /*  sans "PK" (so unzip executable not */
#define END_CENTRAL_SIG   "\005\006"   /*  mistaken for zipfile itself) */
#define EXTD_LOCAL_SIG    "\007\010"   /* [ASCII "\113" == EBCDIC "\080" ??] */

#define SKIP              0    /* choice of activities for do_string() */
#define DISPLAY           1
#define DS_FN             2
#define EXTRA_FIELD       3
#define DS_EF             3
#ifdef AMIGA
#  define FILENOTE        4
#endif

#define DOES_NOT_EXIST    -1   /* return values for check_for_newer() */
#define EXISTS_AND_OLDER  0
#define EXISTS_AND_NEWER  1

#define ROOT              0    /* checkdir() extract-to path:  called once */
#define INIT              1    /* allocate buildpath:  called once per member */
#define APPEND_DIR        2    /* append a dir comp.:  many times per member */
#define APPEND_NAME       3    /* append actual filename:  once per member */
#define GETPATH           4    /* retrieve the complete path and free it */
#define END               5    /* free root path prior to exiting program */

/* version_made_by codes (central dir):  make sure these */
/*  are not defined on their respective systems!! */
#define FS_FAT_           0    /* filesystem used by MS-DOS, OS/2, Win32 */
#define AMIGA_            1
#define VMS_              2
#define UNIX_             3
#define VM_CMS_           4
#define ATARI_            5    /* what if it's a minix filesystem? [cjh] */
#define FS_HPFS_          6    /* filesystem used by OS/2, NT */
#define MAC_              7
#define Z_SYSTEM_         8
#define CPM_              9
#define TOPS20_           10
#define FS_NTFS_          11   /* filesystem used by Windows NT */
#define QDOS_MAYBE_       12   /* a bit premature, but somebody once started */
#define ACORN_            13   /* Archimedes Acorn RISC OS */
#define FS_VFAT_          14   /* filesystem used by Windows 95 */
#define MVS_              15
#define BEBOX_            16
#define NUM_HOSTS         17   /* index of last system + 1 */

#define STORED            0    /* compression methods */
#define SHRUNK            1
#define REDUCED1          2
#define REDUCED2          3
#define REDUCED3          4
#define REDUCED4          5
#define IMPLODED          6
#define TOKENIZED         7
#define DEFLATED          8
#define NUM_METHODS       9    /* index of last method + 1 */
/* don't forget to update list_files() appropriately if NUM_METHODS changes */

#define PK_OK             0    /* no error */
#define PK_COOL           0    /* no error */
#define PK_GNARLY         0    /* no error */
#define PK_WARN           1    /* warning error */
#define PK_ERR            2    /* error in zipfile */
#define PK_BADERR         3    /* severe error in zipfile */
#define PK_MEM            4    /* insufficient memory */
#define PK_MEM2           5    /* insufficient memory */
#define PK_MEM3           6    /* insufficient memory */
#define PK_MEM4           7    /* insufficient memory */
#define PK_MEM5           8    /* insufficient memory */
#define PK_NOZIP          9    /* zipfile not found */
#define PK_PARAM          10   /* bad or illegal parameters specified */
#define PK_FIND           11   /* no files found */
#define PK_DISK           50   /* disk full */
#define PK_EOF            51   /* unexpected EOF */

#define IZ_DIR            76   /* potential zipfile is a directory */
#define IZ_CREATED_DIR    77   /* directory created: set time and permissions */
#define IZ_VOL_LABEL      78   /* volume label, but can't set on hard disk */
#define IZ_EF_TRUNC       79   /* local extra field truncated (PKZIP'd) */

#define DF_MDY            0    /* date format 10/26/91 (USA only) */
#define DF_DMY            1    /* date format 26/10/91 (most of the world) */
#define DF_YMD            2    /* date format 91/10/26 (a few countries) */

/* extra-field ID values, little-endian: */
#define EF_AV        0x0007    /* PKWARE's authenticity verification ID */
#define EF_OS2       0x0009    /* OS/2 extended attributes ID */
#define EF_PKVMS     0x000c    /* PKWARE's VMS ID */
#define EF_ASIUNIX   0x756e    /* ASi/PKWARE's Unix ID ("nu") */
#define EF_IZVMS     0x4d49    /* Info-ZIP's VMS ID ("IM") */
#define EF_IZUNIX    0x5855    /* Info-ZIP's Unix ID ("UX") */
#define EF_VMCMS     0x4704    /* VM/CMS Extra Field ID ("\004G") */
#define EF_MVS       0x470f    /* MVS Extra Field ID ("\017G") */
#define EF_SPARK     0x4341    /* David Pilling's Acorn/SparkFS ID ("AC") */
#define EF_MD5       0x4b46    /* Fred Kantor's MD5 ID ("FK") */
#define EF_ACL       0x4C41    /* OS/2 ACL ID */

#define EB_HEADSIZE       4    /* length of extra field block header */
#define EB_ID             0    /* offset of block ID in header */
#define EB_LEN            2    /* offset of data length field in header */

#define EB_UX_MINLEN      8    /* minimal "UX" field contains atime, mtime */
#define EB_UX_FULLSIZE    12   /* full "UX" field (atime, mtime, uid, gid) */
#define EB_UX_ATIME       0    /* offset of atime in "UX" extra field data */
#define EB_UX_MTIME       4    /* offset of mtime in "UX" extra field data */
#define EB_UX_UID         8    /* byte offset of UID in "UX" field data */
#define EB_UX_GID         10   /* byte offset of GID in "UX" field data */

/*---------------------------------------------------------------------------
    True sizes of the various headers, as defined by PKWARE--so it is not
    likely that these will ever change.  But if they do, make sure both these
    defines AND the typedefs below get updated accordingly.
  ---------------------------------------------------------------------------*/
#define LREC_SIZE   26   /* lengths of local file headers, central */
#define CREC_SIZE   42   /*  directory headers, and the end-of-    */
#define ECREC_SIZE  18   /*  central-dir record, respectively      */

#define MAX_BITS    13                 /* used in unshrink() */
#define HSIZE       (1 << MAX_BITS)    /* size of global work area */

#define LF     10        /* '\n' on ASCII machines; must be 10 due to EBCDIC */
#define CR     13        /* '\r' on ASCII machines; must be 13 due to EBCDIC */
#define CTRLZ  26        /* DOS & OS/2 EOF marker (used in fileio.c, vms.c) */

#ifdef EBCDIC
#  define native(c) ebcdic[(c)]
#  define NATIVE    "EBCDIC"
#endif

#if (defined(CRAY) && defined(ZMEM))
#  undef ZMEM
#endif

#ifdef ZMEM
#  undef ZMEM
#  define memcpy(dest,src,len)   bcopy(src,dest,len)
#  define memzero                bzero
#else
#  define memzero(dest,len)      memset(dest,0,len)
#endif

#ifdef VMS
#  define ENV_UNZIP       "UNZIP_OPTS"     /* names of environment variables */
#  define ENV_ZIPINFO     "ZIPINFO_OPTS"
#else /* !VMS */
#  ifdef RISCOS
#    define ENV_UNZIP     "Unzip$Options"
#    define ENV_ZIPINFO   "Zipinfo$Options"
#  else /* !RISCOS */
#    define ENV_UNZIP     "UNZIP"          /* the standard names */
#    define ENV_ZIPINFO   "ZIPINFO"
#  endif /* ?RISCOS */
#endif /* ?VMS */
#define ENV_UNZIP2        "UNZIPOPT"     /* alternate names, for zip compat. */
#define ENV_ZIPINFO2      "ZIPINFOOPT"

#if (!defined(QQ) && !defined(NOQQ))
#  define QQ
#endif

#ifdef QQ                         /* Newtware version:  no file */
#  define QCOND     (!G.qflag)   /*  comments with -vq or -vqq */
#else                             /* Bill Davidsen version:  no way to */
#  define QCOND     (which_hdr)   /*  kill file comments when listing */
#endif

#ifdef OLD_QQ
#  define QCOND2    (G.qflag < 2)
#else
#  define QCOND2    (!G.qflag)
#endif

#ifndef TRUE
#  define TRUE      1   /* sort of obvious */
#endif
#ifndef FALSE
#  define FALSE     0
#endif

#ifndef SEEK_SET
#  define SEEK_SET  0
#  define SEEK_CUR  1
#  define SEEK_END  2
#endif

#if (defined(UNIX) && defined(S_IFLNK) && !defined(MTS))
#  define SYMLINKS
#  ifndef S_ISLNK
#    define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)
#  endif
#endif /* UNIX && S_IFLNK && !MTS */

#ifndef S_ISDIR
#  ifdef CMS_MVS
#    define S_ISDIR(m)  (FALSE)
#  else
#    define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
# endif
#endif

#ifndef IS_VOLID
#  define IS_VOLID(m)  ((m) & 0x08)
#endif





/**************/
/*  Typedefs  */
/**************/

#ifdef NO_UID_GID
#  ifdef UID_USHORT
     typedef unsigned short  uid_t;    /* TI SysV.3 */
     typedef unsigned short  gid_t;
#  else
     typedef unsigned int    uid_t;    /* SCO Xenix */
     typedef unsigned int    gid_t;
#  endif
#endif

#if (defined(WIN32) || defined(sgi) || defined(BSD4_4) || defined(ATARI))
   typedef struct utimbuf ztimbuf;
#else
   typedef struct ztimbuf {
       time_t actime;        /* new access time */
       time_t modtime;       /* new modification time */
   } ztimbuf;
#endif

typedef struct min_info {
    long offset;
    ulg compr_size;          /* compressed size (needed if extended header) */
    ulg crc;                 /* crc (needed if extended header) */
    int hostnum;
    unsigned file_attr;      /* local flavor, as used by creat(), chmod()... */
    unsigned encrypted : 1;  /* file encrypted: decrypt before uncompressing */
    unsigned ExtLocHdr : 1;  /* use time instead of CRC for decrypt check */
    unsigned textfile : 1;   /* file is text (according to zip) */
    unsigned textmode : 1;   /* file is to be extracted as text */
    unsigned lcflag : 1;     /* convert filename to lowercase */
    unsigned vollabel : 1;   /* "file" is an MS-DOS volume (disk) label */
} min_info;

typedef struct VMStimbuf {
    char *revdate;    /* (both roughly correspond to Unix modtime/st_mtime) */
    char *credate;
} VMStimbuf;

/*---------------------------------------------------------------------------
    Zipfile work area declarations.
  ---------------------------------------------------------------------------*/

#ifdef MALLOC_WORK
   union work {
     struct {                 /* unshrink(): */
       shrint *Parent;          /* (8193 * sizeof(short)) */
       uch *value;
       uch *Stack;
     } shrink;
     uch *Slide;              /* explode(), inflate(), unreduce() */
   };
#else /* !MALLOC_WORK */
   union work {
     struct {                 /* unshrink(): */
       shrint Parent[HSIZE];    /* (8192 * sizeof(short)) == 16KB minimum */
       uch value[HSIZE];        /* 8KB */
       uch Stack[HSIZE];        /* 8KB */
     } shrink;                  /* total = 32KB minimum; 80KB on Cray/Alpha */
     uch Slide[WSIZE];        /* explode(), inflate(), unreduce() */
   };
#endif /* ?MALLOC_WORK */

#define slide  G.area.Slide

#ifdef DLL
#  define redirSlide G.redirect_pointer
#else
#  define redirSlide G.area.Slide
#endif

/*---------------------------------------------------------------------------
    Zipfile layout declarations.  If these headers ever change, make sure the
    xxREC_SIZE defines (above) change with them!
  ---------------------------------------------------------------------------*/

   typedef uch   local_byte_hdr[ LREC_SIZE ];
#      define L_VERSION_NEEDED_TO_EXTRACT_0     0
#      define L_VERSION_NEEDED_TO_EXTRACT_1     1
#      define L_GENERAL_PURPOSE_BIT_FLAG        2
#      define L_COMPRESSION_METHOD              4
#      define L_LAST_MOD_FILE_TIME              6
#      define L_LAST_MOD_FILE_DATE              8
#      define L_CRC32                           10
#      define L_COMPRESSED_SIZE                 14
#      define L_UNCOMPRESSED_SIZE               18
#      define L_FILENAME_LENGTH                 22
#      define L_EXTRA_FIELD_LENGTH              24

   typedef uch   cdir_byte_hdr[ CREC_SIZE ];
#      define C_VERSION_MADE_BY_0               0
#      define C_VERSION_MADE_BY_1               1
#      define C_VERSION_NEEDED_TO_EXTRACT_0     2
#      define C_VERSION_NEEDED_TO_EXTRACT_1     3
#      define C_GENERAL_PURPOSE_BIT_FLAG        4
#      define C_COMPRESSION_METHOD              6
#      define C_LAST_MOD_FILE_TIME              8
#      define C_LAST_MOD_FILE_DATE              10
#      define C_CRC32                           12
#      define C_COMPRESSED_SIZE                 16
#      define C_UNCOMPRESSED_SIZE               20
#      define C_FILENAME_LENGTH                 24
#      define C_EXTRA_FIELD_LENGTH              26
#      define C_FILE_COMMENT_LENGTH             28
#      define C_DISK_NUMBER_START               30
#      define C_INTERNAL_FILE_ATTRIBUTES        32
#      define C_EXTERNAL_FILE_ATTRIBUTES        34
#      define C_RELATIVE_OFFSET_LOCAL_HEADER    38

   typedef uch   ec_byte_rec[ ECREC_SIZE+4 ];
/*     define SIGNATURE                         0   space-holder only */
#      define NUMBER_THIS_DISK                  4
#      define NUM_DISK_WITH_START_CENTRAL_DIR   6
#      define NUM_ENTRIES_CENTRL_DIR_THS_DISK   8
#      define TOTAL_ENTRIES_CENTRAL_DIR         10
#      define SIZE_CENTRAL_DIRECTORY            12
#      define OFFSET_START_CENTRAL_DIRECTORY    16
#      define ZIPFILE_COMMENT_LENGTH            20


   typedef struct local_file_header {                 /* LOCAL */
       uch version_needed_to_extract[2];
       ush general_purpose_bit_flag;
       ush compression_method;
       ush last_mod_file_time;
       ush last_mod_file_date;
       ulg crc32;
       ulg csize;
       ulg ucsize;
       ush filename_length;
       ush extra_field_length;
   } local_file_hdr;

#if 0
   typedef struct central_directory_file_header {     /* CENTRAL */
       uch version_made_by[2];
       uch version_needed_to_extract[2];
       ush general_purpose_bit_flag;
       ush compression_method;
       ush last_mod_file_time;
       ush last_mod_file_date;
       ulg crc32;
       ulg csize;
       ulg ucsize;
       ush filename_length;
       ush extra_field_length;
       ush file_comment_length;
       ush disk_number_start;
       ush internal_file_attributes;
       ulg external_file_attributes;
       ulg relative_offset_local_header;
   } cdir_file_hdr;
#endif /* 0 */

   typedef struct end_central_dir_record {            /* END CENTRAL */
       ush number_this_disk;
       ush num_disk_start_cdir;
       ush num_entries_centrl_dir_ths_disk;
       ush total_entries_central_dir;
       ulg size_central_directory;
       ulg offset_start_central_directory;
       ush zipfile_comment_length;
   } ecdir_rec;


/* Huffman code lookup table entry--this entry is four bytes for machines
   that have 16-bit pointers (e.g. PC's in the small or medium model).
   Valid extra bits are 0..13.  e == 15 is EOB (end of block), e == 16
   means that v is a literal, 16 < e < 32 means that v is a pointer to
   the next table, which codes e - 16 bits, and lastly e == 99 indicates
   an unused code.  If a code with e == 99 is looked up, this implies an
   error in the data. */

struct huft {
    uch e;                /* number of extra bits or operation */
    uch b;                /* number of bits in this code or subcode */
    union {
        ush n;            /* literal, length base, or distance base */
        struct huft *t;   /* pointer to next level of table */
    } v;
};


typedef struct _APIDocStruct {
    char *compare;
    char *function;
    char *syntax;
    char *purpose;
} APIDocStruct;




/*************/
/*  Globals  */
/*************/

#if (defined(OS2) && !defined(FUNZIP))
#  include "os2/os2.h"
#endif

/*
    no longer supported (though code is available separately)

    #if (defined(WINDLL) && !defined(FUNZIP))
    #  include "win16dll/windll.h"
    #endif
 */

#include "globals.h"



/*************************/
/*  Function Prototypes  */
/*************************/

/*---------------------------------------------------------------------------
    Functions in unzip.c (initialization routines):
  ---------------------------------------------------------------------------*/

/* int main                      OF((int argc, char **argv));  */
int    unzip                     OF((__GPRO__ int argc, char **argv));
int    uz_opts                   OF((__GPRO__ int *pargc, char ***pargv));
int    usage                     OF((__GPRO__ int error));

/*---------------------------------------------------------------------------
    Functions in process.c (main driver routines):
  ---------------------------------------------------------------------------*/

int      process_zipfiles        OF((__GPRO));
/* static int    do_seekable     OF((__GPRO__ int lastchance)); */
/* static int    find_ecrec      OF((__GPRO__ long searchlen)); */
int      uz_end_central          OF((__GPRO));
int      process_cdir_file_hdr   OF((__GPRO));
int      get_cdir_ent            OF((__GPRO));
int      process_local_file_hdr  OF((__GPRO));
unsigned ef_scan_for_izux        OF((uch *ef_buf, unsigned ef_len,
                                     ztimbuf *z_utim, ush *z_uidgid));

/*---------------------------------------------------------------------------
    Functions in zipinfo.c (`zipinfo-style' listing routines):
  ---------------------------------------------------------------------------*/

int      zi_opts                 OF((__GPRO__ int *pargc, char ***pargv));
int      zi_end_central          OF((__GPRO));
int      zipinfo                 OF((__GPRO));
/* static int      zi_long       OF((__GPRO__ ulg endprev)); */
/* static int      zi_short      OF((__GPRO)); */
/* static char    *zi_time       OF((__GPRO__
                                     const ush *datez, const ush *timez,
                                     const time_t *modtimez, char *d_t_str)); */

/*---------------------------------------------------------------------------
    Functions in list.c (generic zipfile-listing routines):
  ---------------------------------------------------------------------------*/

int      list_files              OF((__GPRO));
int      ratio                   OF((ulg uc, ulg c));
void     fnprint                 OF((__GPRO));

/*---------------------------------------------------------------------------
    Functions in fileio.c:
  ---------------------------------------------------------------------------*/

int      open_input_file      OF((__GPRO));
int      open_outfile         OF((__GPRO));                    /* also vms.c */
void     undefer_input        OF((__GPRO));
void     defer_leftover_input OF((__GPRO));
unsigned readbuf              OF((__GPRO__ char *buf, register unsigned len));
int      readbyte             OF((__GPRO));
int      fillinbuf            OF((__GPRO));
#ifdef FUNZIP
   int   flush                OF((__GPRO__ ulg size));  /* actually funzip.c */
#else
   int   flush                OF((__GPRO__ uch *buf, ulg size, int unshrink));
#endif
/* static int  disk_error     OF((__GPRO)); */
void     handler              OF((int signal));
time_t   dos_to_unix_time     OF((unsigned ddate, unsigned dtime));
int      check_for_newer      OF((__GPRO__ char *filename)); /* os2,vmcms,vms */
int      do_string            OF((__GPRO__ unsigned int len, int option));
ush      makeword             OF((uch *b));
ulg      makelong             OF((uch *sig));
int      zstrnicmp            OF((register const char *s1,
                                  register const char *s2,
                                  register unsigned n));
#ifdef REGULUS
   int zstat                  OF((char *p, struct stat *s));
#endif
#ifdef ZMEM   /* MUST be ifdef'd because of conflicts with the standard def. */
   zvoid *memset OF((register zvoid *, register int, register unsigned int));
   zvoid *memcpy OF((register zvoid *, register const zvoid *,
                     register unsigned int));
#endif
#ifdef SMALL_MEM
   char *fLoadFarString       OF((__GPRO__ const char Far *sz));
   char *fLoadFarStringSmall  OF((__GPRO__ const char Far *sz));
   char *fLoadFarStringSmall2 OF((__GPRO__ const char Far *sz));
   #ifndef zfstrcpy
     char Far * Far zfstrcpy  OF((char Far *s1, const char Far *s2));
   #endif
#endif


/*---------------------------------------------------------------------------
    Functions in extract.c:
  ---------------------------------------------------------------------------*/

int    extract_or_test_files     OF((__GPRO));
/* static int   store_info               OF((void)); */
/* static int   extract_or_test_member   OF((__GPRO)); */
/* static int   TestExtraField           OF((__GPRO__ uch *ef,
                                             unsigned ef_len)); */
/* static int   test_OS2                 OF((__GPRO__ uch *eb,
                                             unsigned eb_size)); */
int    memextract                OF((__GPRO__ uch *tgt, ulg tgtsize,
                                     uch *src, ulg srcsize));
int    memflush                  OF((__GPRO__ uch *rawbuf, ulg size));
char  *fnfilter                  OF((char *raw, uch *space));

/*---------------------------------------------------------------------------
    Decompression functions:
  ---------------------------------------------------------------------------*/

int    explode                   OF((__GPRO));                  /* explode.c */
int    huft_free                 OF((struct huft *t));          /* inflate.c */
int    huft_build                OF((__GPRO__ unsigned *b, unsigned n,
                                     unsigned s, ush *d, ush *e,
                                     struct huft **t, int *m));
#ifdef USE_ZLIB
   int    UZinflate              OF((__GPRO));                  /* inflate.c */
#  define inflate_free(x)        inflateEnd(&((struct Globals *)(&G))->dstrm)
#else
   int    inflate                OF((__GPRO));                  /* inflate.c */
   int    inflate_free           OF((__GPRO));                  /* inflate.c */
#endif /* ?USE_ZLIB */
void   unreduce                  OF((__GPRO));                 /* unreduce.c */
/* static void  LoadFollowers    OF((__GPRO__ f_array *follower, uch *Slen));
                                                                * unreduce.c */
int    unshrink                  OF((__GPRO));                 /* unshrink.c */
/* static void  partial_clear    OF((__GPRO));                  * unshrink.c */

/*---------------------------------------------------------------------------
    Internal API functions (only included in DLL versions):
  ---------------------------------------------------------------------------*/

#ifdef DLL
   void     setFileNotFound       OF((__GPRO));                     /* api.c */
   int      unzipToMemory         OF((__GPRO__ char *zip, char *file,
                                      UzpBuffer *retstr));          /* api.c */
   int      redirect_outfile      OF((__GPRO));                     /* api.c */
   int      writeToMemory         OF((__GPRO__ uch *rawbuf, ulg size));
                                                                    /* api.c */
   /* this obsolescent entry point kept for compatibility: */
   int      UzpUnzip              OF((int argc, char **argv));/* use UzpMain */
#ifdef OS2API
   int      varmessage            OF((__GPRO__ uch *buf, ulg size));
                                                                /* rexxapi.c */
   int      varputchar            OF((__GPRO__ int c));         /* rexxapi.c */
   int      finish_REXX_redirect  OF((__GPRO));                 /* rexxapi.c */
#endif
#ifdef API_DOC
   void     APIhelp               OF((__GPRO__ int argc, char **argv));
#endif                                                          /* apihelp.c */
/*
    no longer supported (though code is available separately)

    #ifdef WINDLL
       int      WINDLLMAIN            OF((struct ScFileList *pGZipFileList,
                                          int argc, char **argv));
    #endif
 */
#endif /* DLL */

/*---------------------------------------------------------------------------
    Acorn RISCOS-only functions:
  ---------------------------------------------------------------------------*/

#ifdef RISCOS
   int   isRISCOSexfield     OF((void *extra_field));             /* acorn.c */
   void  setRISCOSexfield    OF((char *path, void *extra_field)); /* acorn.c */
   void  printRISCOSexfield  OF((int isdir, void *extra_field));  /* acorn.c */
#endif

/*---------------------------------------------------------------------------
    Human68K-only functions:
  ---------------------------------------------------------------------------*/

#ifdef __human68k__
   void  InitTwentyOne       OF((void));
#endif

/*---------------------------------------------------------------------------
    Macintosh-only functions:
  ---------------------------------------------------------------------------*/

#ifdef MACOS
   int     macmkdir   OF((char *, short, long));                    /* mac.c */
   short   macopen    OF((char *, short, short, long));             /* mac.c */
   FILE   *macfopen   OF((char *, char *, short, long));            /* mac.c */
   short   maccreat   OF((char *, short, long, OSType, OSType));    /* mac.c */
   short   macread    OF((short, char *, unsigned));                /* mac.c */
   long    macwrite   OF((short, char *, unsigned));                /* mac.c */
   short   macclose   OF((short));                                  /* mac.c */
   long    maclseek   OF((short, long, short));                     /* mac.c */
   char   *macgetenv  OF((char *));                                 /* mac.c */
   char   *wfgets     OF((char *, int, FILE *));                    /* mac.c */
   void    wfprintf   OF((FILE *, char *, ...));                    /* mac.c */
   void    wprintf    OF((char *, ...));                            /* mac.c */
#endif

/*---------------------------------------------------------------------------
    MSDOS-only functions:
  ---------------------------------------------------------------------------*/

#if (defined(__GO32__) || (defined(MSDOS) && defined(__EMX__)))
   unsigned _dos_getcountryinfo(void *);                          /* msdos.c */
#if (!defined(__DJGPP__) || (__DJGPP__ < 2))
   void _dos_setftime(int, unsigned short, unsigned short);       /* msdos.c */
   void _dos_setfileattr(char *, int);                            /* msdos.c */
   unsigned _dos_creat(char *, unsigned, int *);                  /* msdos.c */
   void _dos_getdrive(unsigned *);                                /* msdos.c */
   unsigned _dos_close(int);                                      /* msdos.c */
#endif /* !__DJGPP__ || (__DJGPP__ < 2) */
#endif

/*---------------------------------------------------------------------------
    OS/2-only functions:
  ---------------------------------------------------------------------------*/

#ifdef OS2  /* GetFileTime conflicts with something in Win32 header files */
#  if (defined(REENTRANT) && defined(USETHREADID))
   ulg   GetThreadId      OF((void));
#  endif
   int   GetCountryInfo   OF((void));                               /* os2.c */
   long  GetFileTime      OF((char *name));                         /* os2.c */
/* static void  SetPathInfo  OF((char *path, ush moddate, ush modtime,
                                 int flags));                          os2.c */
/* static int   SetEAs       OF((__GPRO__ char *path, void *eablock)); os2.c */
/* static int   IsFileNameValid OF((char *name));                      os2.c */
/* static void  map2fat         OF((char *pathcomp, char **pEndFAT));  os2.c */
/* static int   SetLongNameEA   OF((char *name, char *longname));      os2.c */
/* static void  InitNLS         OF((void));                            os2.c */
   int   IsUpperNLS       OF((int nChr));                           /* os2.c */
   int   ToLowerNLS       OF((int nChr));                           /* os2.c */
   void  DebugMalloc      OF((void));                               /* os2.c */
#endif

/*---------------------------------------------------------------------------
    TOPS20-only functions:
  ---------------------------------------------------------------------------*/

#ifdef TOPS20
   int upper               OF((char *s));                        /* tops20.c */
   int enquote             OF((char *s));                        /* tops20.c */
   int dequote             OF((char *s));                        /* tops20.c */
   int fnlegal             OF(());  /* error if prototyped(?) */ /* tops20.c */
#endif

/*---------------------------------------------------------------------------
    VM/CMS- and MVS-only functions:
  ---------------------------------------------------------------------------*/

#ifdef CMS_MVS
   FILE  *vmmvs_open_infile  OF((__GPRO));                        /* vmmvs.c */
   extent getVMMVSexfield    OF((char *type, uch *ef_block, unsigned datalen));
#endif

/*---------------------------------------------------------------------------
    VMS-only functions:
  ---------------------------------------------------------------------------*/

#ifdef VMS
   int    check_format        OF((__GPRO));                         /* vms.c */
   int    find_vms_attrs      OF((__GPRO));                         /* vms.c */
/* int    open_outfile        OF((__GPRO));           * (see fileio.c) vms.c */
/* int    flush               OF((__GPRO__ uch *rawbuf, unsigned size,
                                  int final_flag));   * (see fileio.c) vms.c */
#ifdef RETURN_CODES
   void   return_VMS          OF((__GPRO__ int zip_error));         /* vms.c */
#else
   void   return_VMS          OF((int zip_error));                  /* vms.c */
#endif
#ifdef VMSCLI
   ulg    vms_unzip_cmdline   OF((int *, char ***));            /* cmdline.c */
#endif
#endif

/*---------------------------------------------------------------------------
    Miscellaneous/shared functions:
  ---------------------------------------------------------------------------*/

struct Globals *globalsCtor  OF((void));                        /* globals.c */

void     envargs         OF((__GPRO__ int *Pargc, char ***Pargv,
                             char *envstr, char *envstr2));     /* envargs.c */
void     mksargs         OF((int *argcp, char ***argvp));       /* envargs.c */

int      match           OF((char *s, char *p, int ic));          /* match.c */
int      iswild          OF((char *p));                           /* match.c */

#ifndef USE_ZLIB
   ulg near *get_crc_table  OF((void));                /* funzip.c, crctab.c */
   ulg      crc32           OF((ulg crc, const uch *buf,
                                extent len));       /* AS source, or crc32.c */
#endif /* !USE_ZLIB */

int      dateformat      OF((void));              /* currently, only msdos.c */
void     version         OF((__GPRO));                              /* local */
int      mapattr         OF((__GPRO));                              /* local */
int      mapname         OF((__GPRO__ int renamed));                /* local */
int      checkdir        OF((__GPRO__ char *pathcomp, int flag));   /* local */
char    *do_wild         OF((__GPRO__ char *wildzipfn));            /* local */
char    *GetLoadPath     OF((__GPRO));                              /* local */
#if (defined(MORE) && (defined(UNIX) || defined(VMS)))
   int screenlines       OF((void));                                /* local */
#endif
#ifndef MTS /* macro in MTS */
   void  close_outfile   OF((__GPRO));                              /* local */
#endif
#ifdef SYSTEM_SPECIFIC_CTOR
   void  SYSTEM_SPECIFIC_CTOR   OF((__GPRO));                       /* local */
#endif





/************/
/*  Macros  */
/************/

#ifndef MAX
#  define MAX(a,b)   ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#  define MIN(a,b)   ((a) < (b) ? (a) : (b))
#endif

#ifdef DEBUG
#  define Trace(x)   fprintf x
#else
#  define Trace(x)
#endif

#ifdef DEBUG_TIME
#  define TTrace(x)  fprintf x
#else
#  define TTrace(x)
#endif

#ifdef NO_DEBUG_IN_MACROS
#  define MTrace(x)
#else
#  define MTrace(x)  Trace(x)
#endif

#if (defined(UNIX) || defined(T20_VMS)) /* generally old systems */
#  define ToLower(x)   ((char)(isupper((int)x)? tolower((int)x) : x))
#else
#  define ToLower      tolower          /* assumed "smart"; used in match() */
#endif

#ifdef USE_STRM_INPUT
   /* ``Replace'' the unbuffered UNIX style I/O function with similar
    * standard C functions from <stdio.h>.
    */
#  define read(fd,buf,n) fread((buf),1,(n),(FILE *)(fd))
#  define lseek(fd,o,w) fseek((FILE *)(fd),(o),(w))
#  define close(fd) fclose((FILE *)(fd))
#endif /* USE_STRM_INPUT */

/* The return value of the Info() "macro function" is never checked in
 * UnZip. Otherwise, to get the same behaviour as for (*G.message)(), the
 * Info() definition for "FUNZIP" would have to be corrected:
 * #define Info(buf,flag,sprf_arg) \
 *      (fprintf((flag)&1? stderr : stdout, \
 *               (char *)(sprintf sprf_arg, (buf))) == EOF)
 */
#ifndef Info   /* may already have been defined for redirection */
#  ifdef FUNZIP
#    define Info(buf,flag,sprf_arg) \
     fprintf((flag)&1? stderr : stdout, (char *)(sprintf sprf_arg, (buf)))
#  else
#    ifdef INT_SPRINTF  /* optimized version for "int sprintf()" flavour */
#      define Info(buf,flag,sprf_arg) \
       (*G.message)((zvoid *)&G, (uch *)(buf), (ulg)sprintf sprf_arg, (flag))
#    else          /* generic version, does not use sprintf() return value */
#      define Info(buf,flag,sprf_arg) \
       (*G.message)((zvoid *)&G, (uch *)(buf), \
                     (ulg)(sprintf sprf_arg, strlen((char *)(buf))), (flag))
#    endif
#  endif
#endif /* !Info */

#ifndef FUNZIP   /* used only in inflate.c */
#  define MESSAGE(str,len,flag)  (*G.message)((zvoid *)&G,(str),(len),(flag))
#endif

#if 0            /* Optimization: use the (const) result of crc32(0L,NULL,0) */
#  define CRCVAL_INITIAL  crc32(0L, NULL, 0)
#else
#  define CRCVAL_INITIAL  0L
#endif

/*
 *  Seek to the block boundary of the block which includes abs_offset,
 *  then read block into input buffer and set pointers appropriately.
 *  If block is already in the buffer, just set the pointers.  This macro
 *  is used by uz_end_central (process.c), zi_end_central (zipinfo.c) and
 *  do_string (fileio.c).  A slightly modified version is embedded within
 *  extract_or_test_files (extract.c).  readbyte() and readbuf() (fileio.c)
 *  are compatible.  NOTE THAT abs_offset is intended to be the "proper off-
 *  set" (i.e., if there were no extra bytes prepended); cur_zipfile_bufstart
 *  contains the corrected offset.
 *
 *  Since ZLSEEK is never used during decompression, it is safe to use the
 *  slide[] buffer for the error message.
 *
 *  The awkward "%cbs_offset" construct is provided for the obnoxious Solaris
 *  compiler, which wants to do macro substitution inside strings.
 */

#ifndef ZLSEEK
#  ifdef USE_STRM_INPUT
#    define _ZLS_RELOAD(abs_offset) {\
         MTrace((stderr, "ZLSEEK: %cbs_offset = %ld, G.extra_bytes = %ld\n",\
           'a', (abs_offset), G.extra_bytes));\
         fseek(G.zipfd,(LONGINT)bufstart,SEEK_SET);\
         G.cur_zipfile_bufstart = ftell(G.zipfd);\
         MTrace((stderr,\
           "       request = %ld, (abs+extra) = %ld, inbuf_offset = %ld\n",\
           request, ((abs_offset)+G.extra_bytes), inbuf_offset));\
         MTrace((stderr, "       bufstart = %ld, cur_zipfile_bufstart = %ld\n",\
           bufstart, G.cur_zipfile_bufstart));\
         if ((G.incnt = fread((char *)G.inbuf,1,INBUFSIZ,G.zipfd)) <= 0)\
             return(PK_EOF);\
         G.inptr = G.inbuf + (int)inbuf_offset;\
         G.incnt -= (int)inbuf_offset;\
     }
#  else /* !USE_STRM_INPUT */
#    define _ZLS_RELOAD(abs_offset) {\
         MTrace((stderr, "ZLSEEK: %cbs_offset = %ld, G.extra_bytes = %ld\n",\
           'a', (abs_offset), G.extra_bytes));\
         G.cur_zipfile_bufstart = lseek(G.zipfd,(LONGINT)bufstart,SEEK_SET);\
         MTrace((stderr,\
           "       request = %ld, (abs+extra) = %ld, inbuf_offset = %ld\n",\
           request, ((abs_offset)+G.extra_bytes), inbuf_offset));\
         MTrace((stderr, "       bufstart = %ld, cur_zipfile_bufstart = %ld\n",\
           bufstart, G.cur_zipfile_bufstart));\
         if ((G.incnt = read(G.zipfd,(char *)G.inbuf,INBUFSIZ)) <= 0)\
             return(PK_EOF);\
         G.inptr = G.inbuf + (int)inbuf_offset;\
         G.incnt -= (int)inbuf_offset;\
     }
#  endif /* ?USE_STRM_INPUT */

#  define ZLSEEK(abs_offset) {\
       LONGINT request = (abs_offset) + G.extra_bytes;\
       LONGINT inbuf_offset = request % INBUFSIZ;\
       LONGINT bufstart = request - inbuf_offset;\
\
       if (request < 0) {\
           Info(slide, 1, ((char *)slide, LoadFarStringSmall(SeekMsg),\
             LoadFarString(ReportMsg)));\
           return(PK_BADERR);\
       } else if (bufstart != G.cur_zipfile_bufstart)\
           _ZLS_RELOAD(abs_offset)\
       else {\
           G.incnt += (G.inptr-G.inbuf) - (int)inbuf_offset;\
           G.inptr = G.inbuf + (int)inbuf_offset;\
       }\
   }
#endif /* !ZLSEEK */

#define SKIP_(length) if(length&&((error=do_string(__G__ length,SKIP))!=0))\
  {error_in_archive=error; if(error>1) return error;}

/*
 *  Skip a variable-length field, and report any errors.  Used in zipinfo.c
 *  and unzip.c in several functions.
 *
 *  macro SKIP_(length)
 *      ush length;
 *  {
 *      if (length && ((error = do_string(length, SKIP)) != 0)) {
 *          error_in_archive = error;   /-* might be warning *-/
 *          if (error > 1)              /-* fatal *-/
 *              return (error);
 *      }
 *  }
 *
 */


#ifdef FUNZIP
#  define FLUSH(w)  flush(__G__ (ulg)(w))
#  define NEXTBYTE  getc(G.in)   /* redefined in crypt.h if full version */
#else
#  define FLUSH(w)  if (G.mem_mode) memflush(__G__ redirSlide,(ulg)(w)); \
     else flush(__G__ redirSlide,(ulg)(w),0)
#  define NEXTBYTE  (--G.incnt >= 0 ? (int)(*G.inptr++) : readbyte(__G))
#endif


#define READBITS(nbits,zdest) {if(nbits>G.bits_left) {int temp; G.zipeof=1;\
  while (G.bits_left<=8*(sizeof(G.bitbuf)-1) && (temp=NEXTBYTE)!=EOF) {\
  G.bitbuf|=(ulg)temp<<G.bits_left; G.bits_left+=8; G.zipeof=0;}}\
  zdest=(shrint)((ush)G.bitbuf&mask_bits[nbits]);G.bitbuf>>=nbits;\
  G.bits_left-=nbits;}

/*
 * macro READBITS(nbits,zdest)    * only used by unreduce and unshrink *
 *  {
 *      if (nbits > G.bits_left) {  * fill G.bitbuf, 8*sizeof(ulg) bits *
 *          int temp;
 *
 *          G.zipeof = 1;
 *          while (G.bits_left <= 8*(sizeof(G.bitbuf)-1) &&
 *                 (temp = NEXTBYTE) != EOF) {
 *              G.bitbuf |= (ulg)temp << G.bits_left;
 *              G.bits_left += 8;
 *              G.zipeof = 0;
 *          }
 *      }
 *      zdest = (shrint)((ush)G.bitbuf & mask_bits[nbits]);
 *      G.bitbuf >>= nbits;
 *      G.bits_left -= nbits;
 *  }
 *
 */


/* GRR:  should change name to STRLOWER and use StringLower if possible */

/*
 *  Copy the zero-terminated string in str1 into str2, converting any
 *  uppercase letters to lowercase as we go.  str2 gets zero-terminated
 *  as well, of course.  str1 and str2 may be the same character array.
 */
#ifdef __human68k__
#  define TOLOWER(str1, str2) \
   { \
       char *p=(str1), *q=(str2); \
       uch c; \
       while ((c = *p++) != '\0') { \
           if (iskanji(c)) { \
               if (*p == '\0') \
                   break; \
               *q++ = c; \
               *q++ = *p++; \
           } else \
               *q++ = isupper(c) ? tolower(c) : c; \
       } \
       *q = '\0'; \
   }
#else
#  define TOLOWER(str1, str2) \
   { \
       char  *p, *q; \
       p = (str1) - 1; \
       q = (str2); \
       while (*++p) \
           *q++ = (char)(isupper((int)(*p))? tolower((int)(*p)) : *p); \
       *q = '\0'; \
   }
#endif
/*
 *  NOTES:  This macro makes no assumptions about the characteristics of
 *    the tolower() function or macro (beyond its existence), nor does it
 *    make assumptions about the structure of the character set (i.e., it
 *    should work on EBCDIC machines, too).  The fact that either or both
 *    of isupper() and tolower() may be macros has been taken into account;
 *    watch out for "side effects" (in the C sense) when modifying this
 *    macro.
 */


#ifndef native
#  define native(c)   (c)
#  define A_TO_N(str1)
#else
#  ifndef NATIVE
#    define NATIVE     "native chars"
#  endif
#  define A_TO_N(str1) {register uch *p;\
     for (p=str1; *p; p++) *p=native(*p);}
#endif
/*
 *  Translate the zero-terminated string in str1 from ASCII to the native
 *  character set. The translation is performed in-place and uses the
 *  "native" macro to translate each character.
 *
 *  NOTE:  Using the "native" macro means that is it the only part of unzip
 *    which knows which translation table (if any) is actually in use to
 *    produce the native character set.  This makes adding new character set
 *    translation tables easy, insofar as all that is needed is an appropriate
 *    "native" macro definition and the translation table itself.  Currently,
 *    the only non-ASCII native character set implemented is EBCDIC, but this
 *    may not always be so.
 */




/**********************/
/*  Global constants  */
/**********************/

   extern ush near  mask_bits[];
   extern char     *fnames[2];

#ifdef EBCDIC
   extern const uch ebcdic[];
#endif

   extern char Far  CentSigMsg[];
   extern char Far  EndSigMsg[];
   extern char Far  SeekMsg[];
   extern char Far  FilenameNotMatched[];
   extern char Far  ExclFilenameNotMatched[];
   extern char Far  ReportMsg[];

#ifndef SFX
   extern char Far  CompiledWith[];
#endif /* !SFX */



/***********************************/
/*  Global (shared?) RTL variables */
/***********************************/

#ifdef DECLARE_ERRNO
   extern int       errno;
#endif


#endif /* !__unzpriv_h */
