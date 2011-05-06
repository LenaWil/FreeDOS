/*  Mem

    Copyright (c) Express Software 1997.
    All Rights Reserved.

    Created by: Joseph Cosentino.
    With help from Michal Meller for the convmemfree routine.
    
    Clean up and bug fixes March 2001, Bart Oldeman, with a patch
    applied from Martin Stromberg.
    Thanks to Arkady V.Belousov for suggestions.

    April 2001, tom ehlert
    moved to small memory model to cure some strange bugs
    change largest executable program size from mem_free to
    largest free MCB
        
    Added (for Win98) with the help of Alain support for INT2F AX=4309 (Win98)

    Bart: moved to tiny model and added malloc checks. Then moved
    it back to the small model.

    August 2001, Bart Oldeman. Licensed under the GPL; see \doc\mem\license.txt.
    Bug fixes + XMS enhancements from Martin and Tom.

    MEM 1.4, Mar 2003:
    * use XXMS functions if possible for > 64 MB configurations (bug #1045)
    * implemented /p (initially by Bryan Reifsnyder) (part of bug #1467)
    * XMS version reported as x.xx (bug #1598)
    * code cleanups to reduce size including:
      merging of duplicate code
      disable unused code (search_vectors())
      use our own version of malloc and printf (parts done by Tom Ehlert)
      less use of long ints.
      use UPX.
    * remove dependency on TASM (using a minimal amount of external assembly
      (NASM) or #pragma aux)
    * MEM's own environment is now correctly counted as part of the free space
    * fixed all compiler warnings.
    * options can be concatenated without spaces as in mem/f/e/x/p
    * fixes to bugs and changes noted by Arkady (that are not mentioned above):
      - main: diagnosed all wrong options.
      - check_name: now checks memory block size.
      - removed all strupr(), adapted check_name() with names copying.
      - EMS_list: EMS driver version showed as unsigned values.
      - search_vectors: missed FAR modifier; FP_SEG(ivp) shifted by one.
      - search_sd: check for end should be "<=" instead "<".
      - register_mcb: forgot "mlist =" before "search_sd()".
      - xms_list: free_handles_tmp used outside of "if XMS call successful".
      - XMS and XMS driver minor version should be two BCD digit long.
      - make_mcb_list: added marking as free all MEM's MCBs, not only environment.
      - check_upper: UMB size counted incorrectly.
      - main: command line processed before actions.
      - added support for '-' options prefix.

    MEM 1.5, April 2004
    * implement /c option
    * fix display of sub mcb blocks; detect EBDA, DRIVEDATA and SECTORBUF
    * fix display of device names with garbage beyond the '\0'
    * fix problem with FD EMM386 NOEMS "Internal EMS Error": EMS memory was
      available, just no page frame.
    * merge prf.c with newest version in the kernel source
    * made output more MS compatible, display extended memory without an
      XMS driver installed as well, do NOT count MEM environment as free space,
      don't merge free blocks in the output anymore.
    * various small cleanups

    MEM 1.6, April 2004
    * minor output tweaks, don't upcase names anymore
    * try to detect UMB holes and don't count them as upper memory
    * display UMB holes as "reserved" in mem/f output
    * display version for "mem/?"
*/

#define MEM_MAJOR 1
#define MEM_MINOR 8

/*  Be sure to compile with word alignment OFF !!! */
#if defined(_MSC_VER)
#define asm __asm
#if _MSC_VER >= 700
#pragma warning(disable:4103)
#endif
#pragma pack(1)
#elif defined(_QC) || defined(__WATCOMC__)
#pragma pack(1)
#elif defined(__ZTC__)
#pragma ZTC align 1
#elif defined(__TURBOC__) && (__TURBOC__ > 0x202)
#pragma option -a-
#endif

#ifdef __WATCOMC__
#define PRAGMAS
#define getvect _dos_getvect
#define outportb outp
#define inportb inp
#define biosmemory _bios_memsize
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dos.h>
#include <mem.h>
#include <string.h>
#include <ctype.h>
#include <bios.h>
#include <conio.h>
#include "kitten.h"

typedef unsigned char BYTE;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;

#define FALSE 0
#define TRUE  1

#define OEM_FREEDOS 0xFD

/*
 * Size of a page in conventional memory and in EMS.
 */
#define CONV_BYTES_PER_PAGE	16UL
#define EMS_BYTES_PER_PAGE	16UL*1024UL

#define BYTES_PER_KB		1024UL

/* Types for MINFO.type */
#define MT_NONE    0
#define MT_FREE    1
#define MT_SYSCODE 2
#define MT_SYSDATA 3
#define MT_PROGRAM 4
#define MT_ENV     5
#define MT_DATA    6
#define MT_RESERVED 7 /* check_upper() doesn't include entries with a type
		       * of MT_RESERVED or higher in total size */
#define MT_IVT     8 /* inferred - Interrupt Vector Table */
#define MT_BDA     9 /* inferred - BIOS Data Area */
#define MT_KERNEL  10 /* inferred - area between BIOS and first MCB */
#define MT_DEVICE  11 /* values equal to or above this one are treated as
		       * subordinate to other entries and are hence
		       * indented by print_entry() */
#define MT_DOSDATA 12
#define MT_IFS     13

/* FIXME: NLS the new entries */
static char *minfo_typenames[] = {
    "",
    "free",
    "system code",
    "system data",
    "program",
    "environment",
    "data area",
    "reserved",
    "interrupt vector table",
    "BIOS data area",
    "system data",
    "device driver",
    "data area",
    "IFS"
};

#define _(set,message_number,message) kittengets(set,message_number,message)

int num_lines = -1;

typedef struct device_header
{
    struct device_header far*next;
    ushort attr;
    ushort strategy_rutin;
    ushort interrupt_rutin;
    char name[8];
} DEVICE_HEADER;

/* forward declarations as these structures link between each other */
struct devinfo;
struct minfo;

/* is a character device, as opposed to a block device */
#define DEVICE_ATTR_IS_CHAR 0x8000

typedef struct devinfo
{
    struct device_header far *addr;
    char devname[9];
    struct minfo *minfo;
    ushort attr;
    uchar firstdrive, lastdrive;
    struct devinfo *next;
    struct devinfo *next_in_minfo;
} DEVINFO;

typedef struct dpb
{
    uchar drive_num;
    uchar unit_number;
    ushort bytes_in_sector;
    uchar lsec_num;
    uchar log_base;
    ushort reserved_num;
    uchar FAT_num;
    ushort rootentry_num;
    ushort first_sector;
    ushort largest_cluster;
    ushort sectors_in_FAT;
    ushort root_firstsector;
    DEVICE_HEADER far*device_addr;
    uchar media_desc;
    uchar block_flag;
    struct dpb far*next_dpb;
} DPB;

typedef struct
{
    uchar type;
    ushort start;
    ushort size;
    uchar unused[3];
    char name[8];
} SD_HEADER;

typedef struct
{
    uchar type;
    ushort owner;
    ushort size;
    uchar unused[3];
    char name[8];
} MCB;

typedef struct
{
    uchar dummy1[22]; /* fields we're not interested in */
    ushort parent_pid;
    uchar file_handles[20];
    ushort env_seg;
    ulong dummy2;
    ushort dummy3;
    ulong dummy4;
    ulong previous_psp; /* default FFFFFFFFh in DOS 3.x */
    uchar dummy5[68];
    uchar cmd_tail_len;
    uchar cmd_tail[127];
} PSP;
    

#define MCB_NAME_MAX_LEN 8

#define OWNER_FREE	0x0000
#define OWNER_DOS	0x0008

typedef void (*print_minfo_field)(struct minfo *entry);

/*
 * MINFO elements in the list returned by make_mcb_list do not overlap.
 * For a particular MINFO 'm', the linked list pointed to by 'first_child'
 * contains entries that all overlap with m but not with each other.
 */
typedef struct minfo
{
    uchar type;
    ushort seg;
    ushort owner;
    ushort environment;
    char *name;
    ushort size;
    uchar classified;
#if 0 /* presently unused */
    uchar vecnum;
    uchar *vectors;
#endif
    struct minfo *next;
    struct minfo *first_child;
    struct devinfo *first_dev;
    struct devinfo *last_dev;
    print_minfo_field print_name;
    print_minfo_field print_type;
    void *specific;
} MINFO;

/*
 * Format of data at start of STACKS code segment (if present)
 * Comments are from Table 01634 in Ralf Brown's Interrupt List 60
 */
typedef struct
{
    ushort unknown;		/* ??? */
    ushort count;		/* number of stacks (the x in STACKS=x,y) */
    ushort array_size;		/* size of stack control block array (should
				 * be 8*x */
    ushort stack_size;		/* size of each stack (the y in STACKS=x,y) */
    void far *dseg;		/* pointer to STACKS data segment */
    				/* offset in STACKS data segment of ... */
    ushort array_offset;	/* ... stack control block array */
    ushort last_offset;		/* ... last element of that array */
    ushort free_offset;		/* ... the entry in that array for the next
				 * stack to be allocated */
} STACKS;

/*
 * Format of data at start of FILES segment
 */
typedef struct files
{
    struct files far *next;
    ushort count;
} FILES;

/*
 * Value of FILES= in config.sys.  We add to this each time we see a FILES
 * sub-block.
 */
ushort files_total_count = 5;

/*
 * Information pointed to by MINFO.specific
 */
typedef struct
{
    ushort count;
    ushort stack_size;
} STACKSINFO;

typedef struct
{
    ushort count;
    ushort secondary_count;
} BUFFERSINFO;

typedef struct
{
    ushort count;
} FILESINFO;

typedef struct
{
    ushort count;
} LASTDRIVEINFO;

typedef struct ems_handle
{
    ushort handle;
    ushort pages;
} EMS_HANDLE;

typedef struct xms_handle
{
    ushort handle;
    ulong size;
    ushort locks;
    struct xms_handle *next;
} XMS_HANDLE;

struct MCBheader {
    char type;
    unsigned int pid;
    unsigned int length;
    char reserved[3];
    char progname[8];
};

typedef struct {
    uchar vermajor, verminor;
    unsigned totalhandle, freehandle, usehandle, frame;
    unsigned size, free;
    EMS_HANDLE *handles;
} EMSINFO;

typedef struct {
    ulong total, free, largest;
    unsigned freehandle;
    uchar hma;
    uchar is_386;
    uchar vermajor, verminor, a20;
    uchar drv_vermajor, drv_verminor;
    XMS_HANDLE *handles;
} XMSINFO;

typedef struct upperinfo {
    unsigned total, free, largest;
} UPPERINFO;
    
/* structures for INT 2F AX=4309 */
typedef struct{
    unsigned char flag;
    unsigned char locks;
    unsigned long xmsAddrKb;
    unsigned long xmsBlkSize;
} XMS_HANDLE_DESCRIPTOR;

typedef struct{
    unsigned char checkByte;
    unsigned char sizeOfDesc;
    unsigned short numbOfHandles;
    XMS_HANDLE_DESCRIPTOR far* xmsHandleDescr;
} XMS_HANDLE_TABLE;

#define SMAP  0x534d4150UL
struct e820map 
{
    ulong baselow;
    ulong basehigh;
    ulong lenlow;
    ulong lenhigh;
    ulong type;
};

typedef void far (*xms_drv_t)(void);
void far (*xms_drv)(void);

/*
 * What xms_available() returns if XMS is available.
 */
#define XMS_AVAILABLE_RESULT 0x80

#ifdef PRAGMAS

uchar get_oem_number(void);
#pragma aux get_oem_number = \
    "mov ax, 0x3000" \
    "int 0x21" \
value [bh]

char dos_in_hma_(void);
#pragma aux dos_in_hma_ = \
    "mov ax, 0x3306" \
    "xor dx, dx" \
    "int 0x21" \
value [dh];

char far * dos_list_of_lists(void);
#pragma aux dos_list_of_lists = \
    "mov ah, 0x52" \
    "int 0x21" \
value [es bx];

uchar get_upperlink(void);
#pragma aux get_upperlink = \
    "mov ax, 0x5802" \
    "int 0x21" \
value [al]

long dos_set_upperlink(uchar link);
#pragma aux dos_set_upperlink = \
    "mov ax, 0x5803" \
    "int 0x21" \
    "sbb dx, dx" \
parm [bx] value [dx ax] ;

int call_ems_driver(unsigned char ah);
#pragma aux call_ems_driver = \
    "int 0x67" \
parm [ah] value [ax]

long call_ems_driver_bx(unsigned char ah);
#pragma aux call_ems_driver_bx = \
    "int 0x67" \
    "cwd" \
parm [ah] value [dx bx]

long call_ems_driver_dx(unsigned char ah);
#pragma aux call_ems_driver_dx = \
    "int 0x67" \
    "mov bx, dx" \
    "cwd" \
parm [ah] value [dx bx]

long ems_get_pages(EMS_HANDLE far *handle);
#pragma aux ems_get_pages = \
    "mov ah, 0x4d" \
    "int 0x67" \
    "cwd" \
value [dx bx] parm [es di]

int ems_get_handle_name(unsigned handle, char far *handlename);
#pragma aux ems_get_handle_name = \
    "mov ax, 0x5300" \
    "int 0x67" \
value [ax] parm [dx] [es di]

long ems_total_num_handles(void);
#pragma aux ems_total_num_handles = \
    "mov ax, 0x5402" \
    "int 0x67" \
    "cwd" \
value [dx bx]

unsigned char xms_available(void);
#pragma aux xms_available = \
    "mov ax, 0x4300" \
    "int 0x2f" \
value [al]

XMS_HANDLE_TABLE far* get_xmsHanTab(void);
#pragma aux get_xmsHanTab = \
    "mov ax, 0x4309" \
    "int 0x2f" \
    "cmp al, 0x43" \
    "je ok" \
    "xor bx, bx" \
    "mov es, bx" \
"ok:" \
value [es bx];

xms_drv_t get_xms_drv(void);
#pragma aux get_xms_drv = \
    "mov ax,0x4310" \
    "int 0x2f" \
value [es bx];

ulong call_xms_driver_dx_bl_al(unsigned char ah, ushort dx);
#pragma aux call_xms_driver_dx_bl_al = \
    "call dword ptr [xms_drv]" \
    "mov ah, bl" \
parm [ah] [dx] value [dx ax] modify [bx];

ulong call_xms_driver_bx_ax(unsigned char ah, ushort dx);
#pragma aux call_xms_driver_bx_ax = \
    "call dword ptr [xms_drv]" \
parm [ah] [dx] value [bx ax];

ulong call_xms_driver_edx(unsigned char ah, ushort dx);
#pragma aux call_xms_driver_edx = \
    "call dword ptr [xms_drv]" \
    ".386" \
    "test bl, bl" \
    "jz noerror" \
    "xor edx, edx" \
"noerror:" \
    "mov ax, dx" \
    "shr edx, 16" \
    ".8086" \
parm [ah] [dx] value [dx ax] modify [bx]

ulong call_xms_driver_eax(unsigned char ah, ushort dx);
#pragma aux call_xms_driver_eax = \
    "call dword ptr [xms_drv]" \
    ".386" \
    "mov edx, eax" \
    "test bl, bl" \
    "jz noerror" \
    "xor edx, edx" \
"noerror:" \
    "mov ax, dx" \
    "shr edx, 16" \
    ".8086" \
parm [ah] [dx] value [dx ax] modify [bx]

unsigned check_8800(void);
#pragma aux check_8800 =\
    "mov ax, 0x8800" \
    "int 0x15" \
    "jnc noerror" \
    "xor ax, ax" \
"noerror:"\
value [ax] modify [si]

ulong check_e801(void);
#pragma aux check_e801 =\
    "mov ax, 0xe801" \
    "xor bx, bx" \
    "int 0x15" \
    "jnc noerror" \
    "or bx, bx" \
    "jnz noerror" \
    "xor ax, ax" \
"noerror:" \
    "mov dx, 1024" \
    "mul dx" \
    "add dx, bx" \
value [dx ax] modify [si bx]

int is_386_(void);
#pragma aux is_386_ = \
    "pushf" \
    "pushf" \
    "pop ax" \
    "or ax, 7000h" \
    "push ax"\
    "popf"\
    "pushf"\
    "pop ax"\
    "popf"\
value [ax];

/* and now we can use some help from the linux kernel */
/* adapted from v 2.4.1 linux/arch/i386/boot/setup.S */
ulong check_e820(struct e820map far *e820map, ulong counter);
#pragma aux check_e820 = \
     ".386" \
     "shl ebx, 16" \
     "mov bx, ax" \
     "mov eax, 0x0000e820"        /* e820, upper word zeroed    */ \
     "mov edx, 0x534d4150"        /* ascii 'SMAP'               */ \
     "mov ecx, 20"                /* size of the e820map        */ \
     "int 15h"                    /* make the call              */ \
     "jc no386"                   /* fall to e801 if it fails   */ \
     "cmp eax, 0x534d4150"        /* check the return is `SMAP` */ \
     "je yes386"                  /* fall to e801 if it fails   */ \
"no386:" \
     "xor ebx, ebx" \
"yes386:" \
     "mov ax, bx"  \
     "shr ebx, 16" \
     ".8086" \
parm [es di] [bx ax] value [bx ax] modify [si];

/* FIXME: make NASM version */
/*
 * Get the extended memory size.  Ralf Brown's Interrupt List notes that
 * some BIOSes don't properly set the carry flag on failure.  If this
 * happens, we'll incorrectly show memory as available on systems that
 * do not support extended memory.
 *
 * If (result >> 16) != 0, then result & 0xFF00 = ah = error code,
 * else result = number of continuous KB starting at absolute address 100000h.
 */
ulong get_ext_mem_size(void);
#pragma aux get_ext_mem_size =			\
    "mov ah, 0x88"				\
    "int 15h"					\
    "xor dx, dx"				\
    "jnc success"				\
    "mov dx, 1"					\
"success:"					\
value [dx ax];

/* FIXME: put this where NASM version can use it */
#define GET_EXT_MEM_SIZE_OK(result)	((result >> 16) == 0)
#define GET_EXT_MEM_SIZE_VALUE(result)	(result)
#define GET_EXT_MEM_SIZE_ERROR(result)	((result & 0xFF00) >> 8)

#define GET_EXT_MEM_SIZE_ERROR_INVALID_CMD	0x80
#define GET_EXT_MEM_SIZE_ERROR_UNSUPPORTED	0x86

#else

/* Get OEM number. */
static uchar get_oem_number(void)
{
    union REGS regs;
    regs.x.ax = 0x3000;
    intdos(&regs, &regs);
    return regs.h.bh;
}

static char dos_in_hma_(void)
{
    union REGS regs;

    regs.x.ax = 0x3306;
    regs.x.dx = 0;
    intdos(&regs, &regs);
    return regs.h.dh & 0x10;
}

/* Get pointer to LL struct. */
static char far * dos_list_of_lists(void)
{
    union REGS regs;
    struct SREGS sregs;

    regs.h.ah = 0x52;
    intdosx(&regs, &regs, &sregs);
    return MK_FP(sregs.es, regs.x.bx);
}

static uchar get_upperlink(void)
{
    union REGS regs;

    regs.x.ax = 0x5802;
    intdos(&regs, &regs);
    return regs.h.al;
}

static long dos_set_upperlink(uchar link)
{
    union REGS regs;

    regs.x.ax = 0x5803;
    regs.x.bx = link;
    intdos(&regs, &regs);
    if (regs.x.cflag)
        return regs.x.ax | 0xffff0000L;
    else
        return regs.x.ax;
}

static int call_ems_driver(unsigned char ah)
{
    union REGS regs;
    regs.h.ah = ah;
    int86(0x67, &regs, &regs);
    return regs.x.ax;
}

static long call_ems_driver_bx(unsigned char ah)
{
    union REGS regs;
    regs.h.ah = ah;
    int86(0x67, &regs, &regs);
    return ((long)(int)regs.x.ax & 0xffff0000UL) | regs.x.bx;
}

static long call_ems_driver_dx(unsigned char ah)
{
    union REGS regs;
    regs.h.ah = ah;
    int86(0x67, &regs, &regs);
    return ((long)(int)regs.x.ax & 0xffff0000UL) | regs.x.dx;
}

static long ems_get_pages(EMS_HANDLE far *handle)
{
    union REGS regs;
    struct SREGS sregs;
        
    regs.h.ah = 0x4d;
    regs.x.di = FP_OFF(handle);
    sregs.es = FP_SEG(handle);
    int86x(0x67, &regs, &regs, &sregs);
    return ((long)(int)regs.x.ax & 0xffff0000UL) | regs.x.bx;
}

static int ems_get_handle_name(unsigned handle, char far *handlename)
{
    union REGS regs;
    struct SREGS sregs;
        
    regs.x.ax = 0x5300;
    regs.x.dx = handle;
    regs.x.di = FP_OFF(handlename);
    sregs.es = FP_SEG(handlename);
    int86x(0x67, &regs, &regs, &sregs);
    return regs.x.ax;
}

static long ems_total_num_handles(void)
{
    union REGS regs;
        
    regs.x.ax = 0x5402;
    int86(0x67, &regs, &regs);
    return ((long)(int)regs.x.ax & 0xffff0000UL) | regs.x.bx;
}

static unsigned char xms_available(void)
{
    union REGS regs;
        
    regs.x.ax = 0x4300;
    int86(0x2f, &regs, &regs);
    return regs.h.al;
}

static XMS_HANDLE_TABLE far* get_xmsHanTab(void)
{
    union REGS regs;
    struct SREGS sregs;
    
    regs.x.ax = 0x4309;
    int86x(0x2f, &regs, &regs, &sregs);
    if (regs.h.al == 0x43)
        return MK_FP(sregs.es, regs.x.bx);
    else
        return NULL;
}

static xms_drv_t get_xms_drv(void)
{
    union REGS regs;
    struct SREGS sregs;

    regs.x.ax = 0x4310;
    int86x(0x2f, &regs, &regs, &sregs);
    return MK_FP(sregs.es, regs.x.bx);
}

extern ulong cdecl call_xms_driver_dx_bl_al(unsigned char rah, ushort rdx);
extern ulong cdecl call_xms_driver_bx_ax(unsigned char rah, ushort rdx);
extern ulong cdecl call_xms_driver_edx(unsigned char rah, ushort rdx);
extern ulong cdecl call_xms_driver_eax(unsigned char rah, ushort rdx);
extern int cdecl is_386_(void);
extern ulong cdecl check_e820(struct e820map far *e820map, ulong counter);

static unsigned check_8800(void)
{
    union REGS regs;
        
    regs.x.ax = 0x8800;
    int86(0x15, &regs, &regs);
    if (regs.x.cflag)
        return 0;
    else
        return regs.x.ax;
}

static ulong check_e801(void)
{
    union REGS regs;
        
    regs.x.ax = 0xe801;
    regs.x.bx = 0;
    int86(0x15, &regs, &regs);
    if (regs.x.cflag || regs.x.bx == 0)
        return 0;
    else
        return regs.x.ax * 1024UL + regs.x.bx * 65536UL;
}

#endif

#define HMA_FREE_NOT_DOS 0x0000 /* DOS is not in HMA */
#define HMA_FREE_UNKNOWN 0xFFFF /* DOS doesn't support querying free HMA */

static unsigned int dos_hma_free(void)
{
    union REGS regs;

    regs.x.ax = 0x4A01;
    regs.x.bx = HMA_FREE_UNKNOWN;
    int86(0x2F, &regs, &regs);
    return regs.x.bx;
}

#ifdef __WATCOMC__
/* WATCOM's getenv is case-insensitive which wastes a lot of space
   for our purposes. So here's a simple case-sensitive one */
char *getenv(const char *name)
{
  char **envp, *ep;
  const char *np;
  char ec, nc;

  for (envp = environ; (ep = *envp) != NULL; envp++) {
    np = name;
    do {
      ec = *ep++;
      nc = *np++;
      if (nc == 0) {
        if (ec == '=')
          return ep;
        break;
      }
    } while (ec == nc);
  }
  return NULL;
}
#endif

#define MALLOC_BUFFER_SIZE 30000
static char malloc_buffer[MALLOC_BUFFER_SIZE];
static size_t mbuffer_idx;

/* dumb malloc replacement without possibility to free. that's
   enough for us here */
void *malloc(size_t size)
{
    char *ret = &malloc_buffer[mbuffer_idx];

    if (mbuffer_idx + size > MALLOC_BUFFER_SIZE) {
/* FIXME: NLS */
	fatal(_(0,2,"Out of memory!\n"));
    }
    mbuffer_idx += size;
    return ret;
}

#define calloc(nitems,size) (malloc((nitems) * (size)))

void free(void *foo)
{
    foo = foo;
}

static void fatal(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    printf("MEM: ");
    vprintf(format, ap);
    va_end(ap);
    exit(1);
}

static void *xmalloc (size_t size)
{
    register void *value = malloc (size);
    if (value == NULL)
        fatal (_(0,0,"Out of memory. Need %ld more bytes.\n"), (long int)size);
    return value;
}

#define xcalloc(nitems,size) (xmalloc((nitems) * (size)))

static char *xstrdup (char *src)
{
    char *dst = xmalloc(strlen(src) + 1);
    strcpy(dst, src);
    return dst;
}

static unsigned long round_kb(unsigned long bytes)
{
    return ((bytes + 512) / 1024L);
}

static unsigned round_seg_kb(unsigned para)
{
    return ((para + 32) / 64);
}

/*
 * As for printf(), but format may only contain a single format specifier,
 * which must be "%s" and is replaced with the string form of num with commas
 * separating groups of three digits.
 *
 * e.g. convert("%s bytes", 1234567) -> "1,234,567 bytes"
 */
static void convert(const char *format, ulong num)
{
    int c, i, j, n;
    char des[4*sizeof(ulong)];

    n = sprintf(des, "%lu", num);
    /* insert commas in the string */
    c = 3;
    for (i = n - 3; i > 0; i--) {
        if (c%3==0) {
            for (j = n; j >= i; j--)
                des[j+1] = des[j];
            des[i]=',';
            n++;
        }
        c++;
    }
    printf(format, des);
}

static char* get_os(void)
{
    switch (get_oem_number())
        {
	case OEM_FREEDOS:
            return "FreeDOS";
        case 0xFF:
	    if (_osmajor <= 6)
                return "MS-DOS";
            else
		return "Windows";
        case 0x00:
            return "PC-DOS";
        case 0xEE:
            return "DR-DOS";
        case 0xEF:
            return "Novell";
        case 0x66:
	    return "PTS-DOS";
        case 0x5E:
            return "RxDOS";
        default:
            return _(1,0,"An unknown operating system");

        }
}

static EMSINFO *ems_error(void)
{
    printf(_(5,0,"EMS INTERNAL ERROR.\n"));
    return NULL;
}

#define ems_frame() call_ems_driver_bx(0x41)
#define ems_size() call_ems_driver_dx(0x42)
#define ems_free() call_ems_driver_bx(0x42)
#define ems_version() call_ems_driver(0x46)
#define ems_num_handles() call_ems_driver_bx(0x4b)

/*
 * Returns TRUE if the far string and near string which are both of length
 * max (not NUL-terminated) are equal, FALSE otherwise.
 */
int fnstreqn(const char far *s1, const char *s2, size_t max)
{
    for (; max > 0; max--) {
	if (*s1 != *s2) {
	    return FALSE;
	}
	s1++;
	s2++;
    }
    return TRUE;
}

static EMSINFO *check_ems(void)
{
    char far *int67;
    static EMSINFO *ems_static_pointer = NULL;
    static EMSINFO ems_static;
    EMSINFO *ems = ems_static_pointer;
    long tmp;
    
    if (ems != NULL)
        return ems;
    
    int67=(char far *)getvect(0x67);
    if (int67 == NULL)
        return ems;

    int67 = MK_FP(FP_SEG(int67),10);

    /*
     * The driver name is "EMMQXXX0" if EMM386 /NOEMS was used.  This
     * is true for both MS-DOS and FreeDOS EMM386.
     */
    if (!fnstreqn(int67, "EMMXXXX0", 8))
	return ems;
            
    ems = ems_static_pointer = &ems_static;
    /* no frame is not an error -- FD EMM386 has that for noems */
    if ((tmp = ems_frame()) < 0)
        tmp = 0;
    ems->frame = (ushort)tmp;

    if ((tmp = ems_version()) < 0)
        return ems_error();
    ems->vermajor = ((ushort)tmp >> 4) & 0xf;
    ems->verminor = (ushort)tmp & 0xf;

    if ((tmp = ems_size()) < 0)
        return ems_error();
    ems->size = (ushort)tmp;

    if ((tmp = ems_free()) < 0)
        return ems_error();
    ems->free = (ushort)tmp;

    if ((tmp = ems_num_handles()) < 0)
        return ems_error();
    ems->handles=xmalloc((ushort)tmp*sizeof(EMS_HANDLE));

    if ((tmp = ems_get_pages(ems->handles)) < 0)
        return ems_error();
    ems->usehandle=(ushort)tmp;

    if (ems->vermajor >= 4)
        {
        if ((tmp = ems_total_num_handles()) < 0)
            ems->totalhandle=ems->usehandle;
        else
            ems->totalhandle = (unsigned)tmp;
        }
    else
        {
        ems->totalhandle=ems->usehandle;
        }

    ems->freehandle=ems->totalhandle - ems->usehandle;

    return(ems);
}

/* check for 386+ before doing e820 stuff */
/* load value 0x7000 into FLAGS register */
/* then check whether all bits are set */
#define is_386() ((is_386_() & 0x7000) == 0x7000)

#define xms_version() (call_xms_driver_bx_ax(0, 0))
#define xms_hma() ((uchar)(call_xms_driver_dx_bl_al(0, 0)>>16))
#define xms_largest() ((ushort)call_xms_driver_bx_ax(8, 0))
#define xms_totalfree() ((ushort)(call_xms_driver_dx_bl_al(8, 0)>>16))

#define XMS_HMA_SIZE ((64 * 1024) - 16)
#define XMS_HMA_AX(result) (result & 0xFFFF)
#define XMS_HMA_BL(result) ((result >> 16) & 0xFF)
#define XMS_HMA_AX_OK		0x0001
#define XMS_HMA_AX_FAILED	0x0001
#define XMS_HMA_BL_NOT_IMPL	0x80
#define XMS_HMA_BL_VDISK	0x81
#define XMS_HMA_BL_NOT_EXIST	0x90
#define XMS_HMA_BL_IN_USE	0x91
#define XMS_HMA_BL_HMAMIN	0x92
#define XMS_HMA_BL_NOT_IN_USE	0x93

#define xms_hma_request(amount) (call_xms_driver_bx_ax(0x01, amount))
#define xms_hma_release() (call_xms_driver_bx_ax(0x02, 0))

static ulong xms_exthandlesize(ushort handle)
{
    return call_xms_driver_edx(0x8e, handle);
}

static ulong xms_exttotalfree(void)
{
    return call_xms_driver_edx(0x88, 0);
}

static ulong xms_extlargest(void)
{
    return call_xms_driver_eax(0x88, 0);
}

static XMSINFO *check_xms(void)
{
    struct e820map e820map;
    ulong total;
    static XMSINFO *xms_static_pointer = NULL;
    static XMSINFO xms_static;
    XMSINFO *xms = xms_static_pointer;

    if (xms != NULL) return xms;

    xms = xms_static_pointer = &xms_static;
    total = 0;
    e820map.lenlow = 0;
    xms->is_386 = is_386();
    if (xms->is_386) {
        /* yes: we have a 386! and can use ax=0xe820 for int15 */
        ulong counter = 0;

        do {
            counter = check_e820(&e820map, counter);
            /* has to be system memory above 1 Meg. */
            if (e820map.type == 1 && e820map.baselow >= 1024*1024UL)
                total += e820map.lenlow;
        } while (counter != 0); /* check to see if ebx is set to EOF  */
    }
    if (total == 0)
        total = check_e801();
    if (total == 0) {
        total = check_8800();
	if (total == 0)
	    /* Try the CMOS approach required by Alain from Ralf */
            if (*(uchar far *)MK_FP(0xF000, 0xFFFE) == 0xFC) { /*is_AT*/
                outportb(0x70, 0x17);
                total = inportb(0x71);
                outportb(0x70, 0x18);
                total |= inportb(0x71) << 8;
	    }
        total *= 1024L;
    }

    xms->total=total;
    if (xms_available() != XMS_AVAILABLE_RESULT)
        return xms;

    xms_drv = get_xms_drv();

    total = xms_version();
    xms->verminor=total & 0xff;
    xms->vermajor=(total >> 8) & 0xff;
    xms->drv_vermajor=total >> 24;
    xms->drv_verminor=(total >> 16) & 0xff;
    xms->hma=xms_hma();

    if (xms->is_386 && xms->vermajor >= 3) {
	xms->largest=xms_extlargest();
	xms->free=xms_exttotalfree();
    }
    if (!xms->largest)
        xms->largest=xms_largest();
    if (!xms->free)
        xms->free=xms_totalfree();
    return(xms);
}

static int set_upperlink(uchar link)
{
    long res = dos_set_upperlink(link);
    
    if (res < 0) {
        if ((ushort)res == 1)
            return 0;
        return -1;
    }
    return 1;
}

static UPPERINFO *check_upper(MINFO *mlist)
{
    static UPPERINFO *upper = NULL;
    uchar origlink;
    ushort lastconseg;

    if (upper!=NULL)
        return upper;
    
    origlink=get_upperlink();
    switch (set_upperlink(1))
        {
	case 1:
            set_upperlink(origlink);
            break;

        case 0:
            return upper;
            
        case -1:
            fatal(_(0,1,"SYSTEM MEMORY TRASHED! (int 21.5803 failure)\n"));

	}

    upper = xcalloc(1, sizeof(UPPERINFO));
    lastconseg = biosmemory()*64;
    /* assert(mlist!=NULL) -- comes from make_mcb_list */
    for (;;) {
        unsigned short seg = mlist->seg;
        if (seg == lastconseg)
            break;
        mlist=mlist->next;
        if (mlist==NULL) {
            fatal(_(0,2,"UMB Corruption: Chain doesn't reach top of low RAM at %dk. Last=0x%x.\n"),
                lastconseg/64, seg);
        }
    }

    mlist=mlist->next;
    while (mlist!=NULL) {
        unsigned size = mlist->size + 1;
        if (mlist->type == MT_FREE)
        {
            upper->free += size;
            if (size > upper->largest)
                upper->largest = size;

	}
	if (mlist->type < MT_RESERVED)
	    upper->total += size;
        mlist=mlist->next;
    }
    return(upper);
}

static void print_minfo_name_default(MINFO *entry)
{
    printf("%-8s", entry->name);
}

static void print_minfo_type_default(MINFO *entry)
{
    printf("%s", _(3,entry->type,minfo_typenames[entry->type]));
}

static void print_minfo_type_stacks(MINFO *entry)
{
    STACKSINFO *specific = (STACKSINFO *) entry->specific;

    printf("STACKS=%u,%u", specific->count, specific->stack_size);
}

static void print_minfo_type_buffers(MINFO *entry)
{
    BUFFERSINFO *specific = (BUFFERSINFO *) entry->specific;

    printf("BUFFERS=%u,%u", specific->count, specific->secondary_count);
}

static void print_minfo_type_files(MINFO *entry)
{
    FILESINFO *specific = (FILESINFO *) entry->specific;

    printf("FILES=%u (%u in this block)", files_total_count, specific->count);
}

static void print_minfo_type_lastdrive(MINFO *entry)
{
    LASTDRIVEINFO *specific = (LASTDRIVEINFO *) entry->specific;
    char c;

    c = specific->count + 'A' - 1;
    if (c > 'Z') {
	c = 'Z';
    } else if (c < 'A') {
	c = '?';
    }
    printf("LASTDRIVE=%c", c);
}

static MINFO *new_minfo(void)
{
    MINFO *entry = xcalloc(1, sizeof(MINFO));

    entry->name = "";
    entry->print_name = print_minfo_name_default;
    entry->print_type = print_minfo_type_default;
    return (entry);
}

#define is_psp(mcb) (*(ushort far*)MK_FP(mcb+1, 0) == 0x20CD)

static unsigned env_seg(unsigned mcb)
{
    unsigned env = *(ushort far *)MK_FP(mcb+1, 0x2C);
    return (((MCB far *)MK_FP(env-1,0))->owner==mcb+1 ? env : 0);
}

#if 0 /* presently unused */
static void search_vectors(MINFO *m)
{
    ushort i, begin, end, iv;
    uchar far *ivp;
    uchar vectors[256];

    begin=m->seg + 1;
    end=begin + m->size;
    for (i=0;i<256;i++)
        {
        ivp = *(uchar far * far *)MK_FP(0, i*4);
	iv = FP_SEG(ivp) + FP_OFF(ivp) >> 4;
        if ((iv >= begin) && (iv < end))
            vectors[m->vecnum++]=(uchar)i;
	}
    if (m->vecnum != 0)
	{
	m->vectors = xmalloc(m->vecnum);
	memcpy(m->vectors, vectors, m->vecnum);
	}
}
#endif

static void check_name(char *dest, const char far *src, size_t length)
{
    dest[length] = '\0';
    while(length--) {
        unsigned char ch = (unsigned char)*src++;
        *dest++ = (ch == '\0' ? '\0' : ch <= ' ' ? ' ' : ch);
    }
}

static void sd_stacks(MINFO *mlist)
{
    STACKS far *stacks;
    STACKSINFO *specific;

    mlist->name = "STACKS";

    /*
     * FreeDOS doesn't set up the beginning of the STACKS code segment in
     * the way that MS-DOS does.
     */
/* FIXME: for FreeDOS, need to show STACKS in type column instead of
 * name since we do that for MS-DOS where we show STACKS=x,y.
 */
    if (get_oem_number() != OEM_FREEDOS) {
	stacks = MK_FP(mlist->seg, 0);
	specific = xmalloc(sizeof(STACKSINFO));
	specific->count = stacks->count;
	specific->stack_size = stacks->stack_size;
	mlist->specific = specific;
	mlist->print_type = print_minfo_type_stacks;
    }
}

static void sd_buffers(MINFO *mlist)
{
    BUFFERSINFO *specific;
    char far *list_of_lists = dos_list_of_lists();

    mlist->name = "BUFFERS";
    specific = xmalloc(sizeof(BUFFERSINFO));
    /*
     * These values are only in the list of lists for MS-DOS version 4.x
     * or higher.  We don't need to check for that version though as earlier
     * versions don't have subsegment descriptors either.
     */
    specific->count = *(ushort far *) (list_of_lists + 0x3F);
    specific->secondary_count = *(ushort far *) (list_of_lists + 0x41);
    mlist->specific = specific;
    mlist->print_type = print_minfo_type_buffers;
}

static void sd_files(MINFO *mlist)
{
    FILES far *files;
    FILESINFO *specific;

    specific = xmalloc(sizeof(FILESINFO));
    mlist->name = "FILES";
    files = MK_FP(mlist->seg, 0);
    specific->count = files->count;
    files_total_count += files->count;
    mlist->specific = specific;
    mlist->print_type = print_minfo_type_files;
}

static void sd_lastdrive(MINFO *mlist)
{
    LASTDRIVEINFO *specific;

    specific = xmalloc(sizeof(LASTDRIVEINFO));
    mlist->name = "LASTDRV";
/* FIXME: offset is 1Bh for DOS 3.0; doesn't exist in previous versions */
    specific->count = * (uchar far *) (dos_list_of_lists() + 0x21);
    mlist->specific = specific;
    mlist->print_type = print_minfo_type_lastdrive;
}

static MINFO *search_sd(MINFO *parent)
{
    MINFO *first_child, *mlist;

    ushort begin, end, i;
    SD_HEADER far *sd;
    static struct {char c; char *s;} sdtype[] = 
    {
      { 'E', "DEVICE" },
      { 'F', "FILES" },
      { 'X', "FCBS" },
      { 'C', "BUFFERS" },
      { 'B', "BUFFERS" },
      { 'L', "LASTDRV" },
      { 'S', "STACKS" }
    };

    begin=parent->seg + 1;
    end=parent->seg + parent->size;
    sd=MK_FP(begin, 0);
    mlist = NULL;
    while ((FP_SEG(sd) >= begin) && (FP_SEG(sd) < end))
        {
        char type = sd->type;
	if (mlist == NULL) {
	    mlist = first_child = new_minfo();
	} else {
	    mlist->next = new_minfo();
	    mlist = mlist->next;
	}
        mlist->owner = mlist->seg = sd->start;
        mlist->size=sd->size;
        mlist->type=MT_DOSDATA;
	if (type == 'I' && *(unsigned short far *)MK_FP(0, 0x40e) == mlist->seg)
            mlist->name = "EBDA";
	else if (type == 'D' || (type == 'I' && _osmajor < 7))
            {
            mlist->name = xmalloc(10);
            check_name(mlist->name, sd->name, 8);
            mlist->type=sd->type == 'D' ? MT_DEVICE : MT_IFS;
            }
        else if (type == 'I')
            {
            /* this is a hack but SECTORBUFs can only have this size
	       and DRIVEDATA areas never */
            mlist->name = mlist->size == 34 ? "SECTORBUF" : "DRIVEDATA";
            }
	else if (type == 'S')
	    sd_stacks(mlist);
	else if (type == 'C' || type == 'B')
	    sd_buffers(mlist);
	else if (type == 'F')
	    sd_files(mlist);
	else if (type == 'L')
	    sd_lastdrive(mlist);
        else
            {
            mlist->name = " ??????";
            for (i = 0; i< sizeof(sdtype)/sizeof(sdtype[0]); i++)
                {
                if (sdtype[i].c == sd->type)
                    {
                    mlist->name = sdtype[i].s;
                    }
                }
            }
	sd=MK_FP(sd->start + sd->size, 0);
        }
    return(first_child);
}

static void register_dos_mcb(MINFO *mlist)
{
    if (mlist->owner == OWNER_FREE) {
        mlist->type=MT_FREE;
    } else if (mlist->owner <= OWNER_DOS) {
        MCB far *mcb = MK_FP(mlist->seg, 0);
        mlist->name = "DOS";
        if (mcb->name[0]=='S' && mcb->name[1]=='D') {
            mlist->type=MT_SYSDATA;
            mlist->first_child=search_sd(mlist);
        } else {
	    /* can be either system code or an UMB hole */
	    unsigned seg = mlist->seg + 1;
	    unsigned convmemsize = biosmemory()*64;
	    mlist->type = MT_SYSCODE;
	    /* no  holes in conv mem */
	    if (seg == convmemsize || (seg > convmemsize
	    /* the heuristic: starts at 512 byte boundary, 
	       has size a multiple of 512 (32 paragraphs) */
		&& ((seg & 511) == 0) && ((mlist->size & 31) == 0))) {
		mlist->name = "";
		mlist->type = MT_RESERVED;
	    }
        }
    }
}

static void program_mcb(MINFO *mlist)
{
    MCB far *mcb;
#if 0
    PSP far *psp;
#endif

    mlist->name = xmalloc(MCB_NAME_MAX_LEN + 1);
    mcb = MK_FP(mlist->seg, 0);
    check_name(mlist->name, mcb->name, MCB_NAME_MAX_LEN);
    mlist->environment=env_seg(mlist->seg);
    mlist->type=MT_PROGRAM;
#if 0
    /*
     * This is test code.  At some point we should use the parent PID
     * to provide a process tree if requested by the user via a (new)
     * command-line option.
     */
    psp = MK_FP(mlist->seg + 1, 0);
    printf("name=%8s,pid=%04X, ppid=%04X\n", mlist->name,
	   mlist->seg + 1, psp->parent_pid);
#endif
}

static void register_mcb(MINFO *mlist)
{
    MCB far *mcb = MK_FP(mlist->seg, 0);

    mlist->owner=mcb->owner;
    mlist->size=mcb->size;

    /*
     * The call to is_psp() below is redundant as owner should always
     * be equal to seg + 1.
     */
    if (mlist->owner && (is_psp(mlist->seg) || mlist->owner == mlist->seg + 1))
        program_mcb(mlist);
    else
        register_dos_mcb(mlist);
}

static MINFO *make_mcb_list(unsigned *convmemfree)
{
    MCB far *cur_mcb;
    uchar origlink;
    MINFO *mlist;
    static MINFO *mlistroot = NULL;
    unsigned freemem, convtopseg;

    if(mlistroot!=NULL)
	return(mlistroot);

    origlink=get_upperlink();
    set_upperlink(1);

    mlistroot = mlist = new_minfo();

    /*
     * Set up BIOS memory blocks which are always fixed in size and location.  */
    mlist->seg = FP_SEG(0);
    mlist->size = 256 * 4 / CONV_BYTES_PER_PAGE; /* num. interrupts * sizeof(far ptr) / para */
    mlist->type = MT_IVT;
    mlist->next = new_minfo();
    mlist->next->seg = mlist->size;
    mlist = mlist->next;

    mlist->size = 0x30;
    mlist->type = MT_BDA;
    mlist->next = new_minfo();
    mlist->next->seg = mlist->seg + mlist->size;
    mlist = mlist->next;

    mlist->name = "IO";
    /*
     * Get the segment address of the first MCB (which will be for
     * DOS) from the list of lists.  The memory between the end of the
     * BIOS Data Area and the start of the first DOS MCB is the
     * kernel.
     */
    mlist->size = *(ushort far *)(dos_list_of_lists()-2) - mlist->seg;
    mlist->type = MT_KERNEL;
    mlist->next = new_minfo();
    mlist = mlist->next;


    /* In LL in offset -02 there's pointer to first mem block (segment only). */
    cur_mcb=MK_FP(*(ushort far *)(dos_list_of_lists()-2), 0);

    while(cur_mcb->type == 'M')
    {
        mlist->seg = FP_SEG(cur_mcb);
        register_mcb(mlist);
        cur_mcb = MK_FP(FP_SEG(cur_mcb) + cur_mcb->size + 1, FP_OFF(cur_mcb));
        mlist->next=new_minfo();
        mlist=mlist->next;
    }
    if (cur_mcb->type != 'Z')
        fatal(_(0,3,"The MCB chain is corrupted (no Z MCB after last M MCB, but %c at seg 0x%x).\n"),
            cur_mcb->type, cur_mcb);
    mlist->seg = FP_SEG(cur_mcb);
    register_mcb(mlist);
    set_upperlink(origlink);

    for (mlist=mlistroot; mlist!=NULL; mlist=mlist->next) {
        MINFO *mlistj;
        
        if (mlist->type != MT_RESERVED && mlist->next != NULL &&
            mlist->next->type == MT_RESERVED) {
	    /* adjust to make the reserved area clear */
	    mlist->next->seg++;
	}
        if (mlist->type == MT_PROGRAM) {
            for(mlistj=mlistroot;mlistj!=NULL;mlistj=mlistj->next) {
                if ((mlist->seg != mlistj->seg)
                    && (mlistj->owner == mlist->seg+1)) {
                    mlistj->name = mlist->name;
                    mlistj->type = MT_ENV;
                    if (mlist->environment != mlistj->seg+1)
                        mlistj->type = MT_DATA;
                }
            }
        }

#if 0 /* presently unused */
        if (mlist->type != MT_SYSDATA)
            search_vectors(mlist);
#endif

    }

    convtopseg = biosmemory()*64;
    freemem = 0;

    /* get free memory */
    for (mlist=mlistroot; mlist!=NULL; mlist=mlist->next) {
        if (show_minfo_as_free(mlist) && mlist->seg < convtopseg)
            freemem += mlist->size + 1;
    }
    
    *convmemfree = freemem;
    return(mlistroot);
}

/*
    return largest possible segment size
*/

static unsigned mcb_largest(void)
{
    MINFO *mlist;
    unsigned largest = 0;

    for (mlist=make_mcb_list(NULL); mlist!=NULL ;mlist = mlist->next)
        if (mlist->type == MT_FREE || mlist->seg + 1 == _psp) {
            unsigned size = mlist->size;
            if (mlist->type != MT_FREE && mlist->next != NULL &&
		mlist->next->type == MT_FREE)
                size += mlist->next->size + 1; /* adjustment for MEM himself */
            if (size > largest) {
                largest = size;
            }
        }
    return largest;
}    

int addr_in_block(void far *addr, MINFO *minfo)
{
    ulong base = (ulong) minfo->seg * CONV_BYTES_PER_PAGE;
    ulong limit = base + ((ulong) minfo->size) * CONV_BYTES_PER_PAGE;
    ulong normal_addr = (ulong) FP_SEG(addr) * CONV_BYTES_PER_PAGE + (ulong) FP_OFF(addr);

#ifdef DEBUG
    printf("is %05lX in range %05lX(%04X)-%05lX (program %s)\n",
	   normal_addr, base, minfo->seg, limit, minfo->name);
#endif
    return (normal_addr >= base && normal_addr <= limit);
}

/*
 * Set the MINFO that a device is inside of.
 */
static void set_dev_minfo(DEVINFO *dlist, MINFO *mlist)
{
    if (dlist->minfo != NULL) {
/* FIXME: NLS */
	printf("Warning: device appears to be owned by multiple "
	       "memory blocks (%s and %s)\n", mlist->name,
	       dlist->minfo->name);
	dlist->next_in_minfo = NULL;
    }
    dlist->minfo = mlist;
    /*
     * Maintain per-MCB list of devices.
     */
    dlist->next_in_minfo = NULL;
    if (mlist->first_dev == NULL) {
	mlist->first_dev = dlist;
	mlist->last_dev = dlist;
    } else {
	mlist->last_dev->next_in_minfo = dlist;
	mlist->last_dev = dlist;
    }
}

static DEVINFO *make_dev_list(MINFO *mlist)
{
    DEVICE_HEADER far *cur_dev;
    DPB far*cur_dpb;
    DEVINFO *dlistroot, *dlist;
    MINFO *mlistroot = mlist;
    MINFO *mchild;
    int found_in_child;
    
    dlist = dlistroot = xcalloc(1, sizeof(DEVINFO));

    cur_dev = (DEVICE_HEADER far *)(dos_list_of_lists() + 0x22);
    
    while (FP_OFF(cur_dev) != 0xFFFF) {
	dlist->addr=cur_dev;
        dlist->attr=cur_dev->attr;
        dlist->minfo=NULL;
        dlist->next_in_minfo=NULL;
        check_name(dlist->devname, cur_dev->name, 8);
        cur_dev=cur_dev->next;
        if (FP_OFF(cur_dev) != 0xFFFF) {
            dlist->next=xcalloc(1, sizeof(DEVINFO));
            dlist=dlist->next;
        }
    }

    /*
     * For each device, scan mlist for the memory block containing the
     * device so we can get the program name.  For mlist entries with
     * children, we scan the children first, because if one of the children
     * matches, the parent will also match, but we are more interested in
     * which child matches.
     */
    for (dlist=dlistroot;dlist!=NULL;dlist=dlist->next)
        for (mlist=mlistroot;mlist!=NULL;mlist=mlist->next) {
	    found_in_child = FALSE;
	    for (mchild=mlist->first_child;mchild!=NULL;mchild=mchild->next) {
		if (addr_in_block(dlist->addr, mchild)) {
		    found_in_child = TRUE;
		    set_dev_minfo(dlist, mchild);
		}
	    }
	    if (!found_in_child && addr_in_block(dlist->addr, mlist)) {
		set_dev_minfo(dlist, mlist);
	    }
	}

    for  (cur_dpb = *((DPB far *far*)dos_list_of_lists());
          FP_OFF(cur_dpb) != 0xFFFF; cur_dpb=cur_dpb->next_dpb)
	{
        for (dlist=dlistroot;dlist!=NULL && dlist->addr != cur_dpb->device_addr;
                 dlist=dlist->next)
	    ;
        
        if (dlist!=NULL)
	{
            uchar drive_num = cur_dpb->drive_num+'A';
	    if (dlist->firstdrive==0)
                dlist->firstdrive=drive_num;
            else
                dlist->lastdrive=drive_num;
            }
        }

    for (dlist=dlistroot;dlist!=NULL;dlist=dlist->next)
        {
	if ((dlist->attr & DEVICE_ATTR_IS_CHAR) == 0)
            {
	    if (dlist->firstdrive != 0)
                {
                sprintf(dlist->devname, "%c:", dlist->firstdrive);
                if (dlist->lastdrive != 0)
                    sprintf(&dlist->devname[2], " - %c:", dlist->lastdrive);
                }
            else
                {
/* FIXME: NLS */
                strcpy(dlist->devname, "(no drv)");
                }
            }
        }

    return dlistroot;
}

MINFO *minfo_dup(MINFO *minfo)
{
    MINFO *new_minfo;

    new_minfo = xmalloc(sizeof(MINFO));
    memcpy(new_minfo, minfo, sizeof(MINFO));
    return new_minfo;
}

typedef int (*mlist_filter_match)(MINFO *entry, void *data);

MINFO *filter_mlist(MINFO *mlisthead, mlist_filter_match is_match, void *data)
{
    MINFO *filtered_head, *filtered_tail;

    /*
     * Move mlisthead to the first matching entry.
     */
    while (mlisthead != NULL && !is_match(mlisthead, data)) {
	mlisthead = mlisthead->next;
    }

    if (mlisthead == NULL) {
	/*
	 * No entries match the given module name.
	 */
	return NULL;
    }

    filtered_head = filtered_tail = minfo_dup(mlisthead);
    mlisthead = mlisthead->next;

    while (mlisthead != NULL) {
	if (is_match(mlisthead, data)) {
	    /*
	     * Duplicate the MINFO element and put it on the end of the
	     * filtered list.
	     */
	    filtered_tail->next = minfo_dup(mlisthead);
	    filtered_tail = filtered_tail->next;
	}
	mlisthead = mlisthead->next;
    }

    /*
     * The 'next' field will have whatever value the MINFO it was duplicated
     * from had, which wouldn't be NULL unless it was the last entry in the
     * original list.
     */
    filtered_tail->next = NULL;

    return filtered_head;
}

int filter_by_module_name(MINFO *entry, void *data)
{
    return (strcmpi(entry->name, (char *) data) == 0);
}

/*
 * Filter for /U
 */
/* FIXME: this shows things like STACKS that should not be included; need an option to filter_mlist which indicates whether we should have an expanded view (i.e. include everything linked under an MINFO that matches the filter) or a collapsed view (only show things that are linked under an MINFO if they also match the filter) where we'd use collapsed for this view but expanded for /MODULE? */
int filter_upper(MINFO *entry, void *data)
{
    return (entry->type != MT_NONE
	    && (entry->type < MT_ENV
		|| entry->type == MT_DEVICE));
}

#define dos_in_hma() (dos_in_hma_() & 0x10)

static void print_normalized_ems_size(unsigned n)
{
    if (n > 624) /* 9984 is the highest "K" value */
	convert("%5sM ", (n + 32) / 64);
    else
	convert("%5sK ", n * 16);
    convert(_(1,2,"(%s bytes)\n"), n * 16384L);
}

static void print_normal_entry(char *text, unsigned long total, 
			       unsigned long used, unsigned long free)
{
    printf("%-17s", text);
    convert("%8sK ", total);
    convert("%9sK ", used);
    convert("%9sK\n", free);
}

/*
 * There are a number of possibilities:
 *
 * - DOS is resident in and hence manging the HMA
 *   - in this case we ask DOS how much space is free in the HMA
 * - DOS is not managing the HMA
 *   - in this case an XMS driver (e.g. HIMEM) may be managing it and we
 *     can find out about it by trying to allocate the HMA
 *     - in this case the HMA may already be in use in which case the
 *       whole area has been allocated to a program
 *     - the XMS driver may not implement HMA
 *     - the XMS driver may see that the HMA contains a VDISK header
 *     - the XMS driver may report HMA does not exist
 *     - otherwise it is all free
 *       - there may be a minimum size for TSRs to be allowed HMA
 *   - otherwise (no XMS driver) HMA is not available
 */
static void show_hma_info(int show_hma_free)
{
    unsigned int high, low, test;

    if (dos_in_hma()) {
	if (show_hma_free) {
	    unsigned int hma_free = dos_hma_free();

/* FIXME: NLS */
	    printf("%-38s%3uK", _(2,10,"Available space in High Memory Area"),
		   round_kb(hma_free));
	    convert(_(1,3," (%7s bytes)\n"), hma_free);
	}
        printf(_(2,11,"%s is resident in the high memory area.\n"), get_os());
    } else {
	if (show_hma_free) {
	    if (xms_available() == XMS_AVAILABLE_RESULT) {
		ulong result;

		xms_drv = get_xms_drv();
		/*
		 * Try requesting a small amount of memory so as to
		 * simultaneoulsy check if HMA is available and whether HMAMIN
		 * is set.
		 */
		result = xms_hma_request(1);
		if (XMS_HMA_AX(result) == XMS_HMA_AX_OK) {
/* FIXME: NLS */
		    printf("HMA is available via the XMS driver\n");
		} else {
		    switch (XMS_HMA_BL(result)) {
		    case XMS_HMA_BL_NOT_IMPL:
/* FIXME: NLS */
			printf("HMA is not available via the XMS driver: not implemented by the driver\n");
			break;
		    case XMS_HMA_BL_VDISK:
/* FIXME: NLS */
			printf("HMA is not available via the XMS driver: a VDISK device is present\n");
			break;
		    case XMS_HMA_BL_NOT_EXIST:
/* FIXME: NLS */
			printf("HMA is not available via the XMS driver: HMA does not exist\n");
			break;
		    case XMS_HMA_BL_IN_USE:
/* FIXME: NLS */
			printf("HMA is not available via the XMS driver: HMA already in use\n");
			break;
		    case XMS_HMA_BL_HMAMIN:
			/*
			 * HMA is available but HMAMIN is set, so try to
			 * determine what HMAMIN is set to.
			 */
			low = 0;
			high = XMS_HMA_SIZE + 1;
			while (high - low > 1) {
			    test = ((ulong) high + (ulong) low) / 2;
#ifdef DEBUG
			    printf("HMAMIN binary search: low=%5u high=%5u "
				   "test=%5u\n", low, high, test);
#endif
			    result = xms_hma_request(test);
			    if (XMS_HMA_AX(result) != XMS_HMA_AX_OK) {
				low = test;
			    } else {
				high = test;
				result = xms_hma_release();
				if (XMS_HMA_AX(result) != XMS_HMA_AX_FAILED) {
				    /* FIXME: NLS and use fatal function/terminate */
				    printf("Fatal: failed to free HMA, error "
					   "code %02Xh\n", XMS_HMA_BL(result));
				}
			    }
			}
			if (high == XMS_HMA_SIZE + 1) {
/* FIXME: NLS */
			    printf("HMA is not available via the XMS driver: HMAMIN is larger than HMA\n");
			} else {
/* FIXME: NLS */
			    printf("HMA is available via the XMS driver; minimum TSR size (HMAMIN) is %u bytes\n", high);
			}
			break;
		    default:
/* FIXME: NLS */
			printf("HMA is not available via the XMS driver: unknown error %02Xh\n",
			       XMS_HMA_BL(result));
		    }
		}
	    } else {
/* FIXME: NLS */
		printf("HMA is not available as no XMS driver is loaded\n");
	    }
	}
    }
}

static void int_15_info(void)
{
    ulong result = get_ext_mem_size();

    if (GET_EXT_MEM_SIZE_OK(result)) {
/* FIXME: NLS */
	printf("%-38s%3uK", _(2,10,"Memory accessible using Int 15h"),
	       round_kb(BYTES_PER_KB * GET_EXT_MEM_SIZE_VALUE(result)));
	convert(_(1,3," (%7s bytes)\n"),
		(ulong) GET_EXT_MEM_SIZE_VALUE(result) * BYTES_PER_KB);
    } else {
/* FIXME: NLS */
	printf("Memory is not accessible using Int 15h (code %02h)\n",
	       GET_EXT_MEM_SIZE_ERROR(result));
    }
}

static void normal_list(unsigned memfree, UPPERINFO *upper, int show_hma_free,
			int show_int_15)
{
    unsigned memory, memused, largest_executable, reserved;
    unsigned umbfree = 0, umbtotal = 0;
    unsigned long xms_total_k;
    XMSINFO *xms;
    EMSINFO *ems;

    ems=check_ems();
    xms=check_xms();
    memory=biosmemory();
    memfree=round_seg_kb(memfree);
    memused=memory - memfree;
    printf("\n");
    printf(_(2,0,"Memory Type        Total       Used       Free\n"));
    printf(      "----------------  --------   --------   --------\n");
    print_normal_entry(_(2,1,"Conventional"), memory, memused, memfree);
    if (upper) {
	umbfree=round_seg_kb(upper->free);
	umbtotal=round_seg_kb(upper->total);
    }
    print_normal_entry(_(2,2,"Upper"), umbtotal, umbtotal-umbfree, umbfree);
    reserved = 1024 - memory - umbtotal;
    print_normal_entry(_(2,3,"Reserved"), reserved, reserved, 0);
    xms_total_k = round_kb(xms->total);
    print_normal_entry(_(2,4,"Extended (XMS)"), xms_total_k, xms_total_k - xms->free,
		       xms->free);
    printf("----------------  --------   --------   --------\n");
    print_normal_entry(_(2,5,"Total memory"), 1024 + xms_total_k,
		       1024 - memfree - umbfree + xms_total_k - xms->free,
		       memfree + umbfree + xms->free);
    printf("\n");
    print_normal_entry(_(2,6,"Total under 1 MB"), 1024 - reserved,
	   memused + umbtotal - umbfree, memfree + umbfree);
    printf("\n");
    if (ems != NULL) {
	printf("%-36s",_(2,7,"Total Expanded (EMS)"));
	print_normalized_ems_size(ems->size);
	printf("%-36s",_(2,8,"Free Expanded (EMS)"));
	print_normalized_ems_size(ems->free);
        printf("\n");
    }

    /*
     * We only show the amount of memory available via INT 15 if /DEBUG
     * was specified to mimic MS-DOS.
     */
    if (show_int_15) {
	int_15_info();
    }

    largest_executable = mcb_largest();

    printf("%-38s%3uK", _(2,9,"Largest executable program size"), round_seg_kb(largest_executable));
    convert(_(1,3," (%7s bytes)\n"), (ulong)largest_executable
	    * CONV_BYTES_PER_PAGE);
/* FIXME: this is inconsistent with MS-DOS 6.22 which shows 0K if no UMBs */
    if (upper != NULL) {
	printf("%-38s%3uK", _(2,10,"Largest free upper memory block"), round_seg_kb(upper->largest));
	convert(_(1,3," (%7s bytes)\n"), (ulong)upper->largest
		* CONV_BYTES_PER_PAGE);
    }

    show_hma_info(show_hma_free);
}

/*
 * Minimum (top) level is 0, and maximum is 2 when we have a device under a
 * child memory block.
 */
#define MAX_LEVEL 2

/*
 * Number of characters to indent by per level.
 */
#define INDENT_PER_LEVEL 2

/*
 * Maximum number of characters of indent.
 */
#define MAX_INDENT_SIZE (MAX_LEVEL * INDENT_PER_LEVEL)

static void indent_setup(char *pre_indent, char *post_indent,
			 unsigned int level)
{
/* FIXME:assert(level <= MAX_LEVEL);*/
    /* fill buffers with the maximum number of space characters we might
       need */
    memset(pre_indent, ' ', MAX_INDENT_SIZE);
    memset(post_indent, ' ', MAX_INDENT_SIZE);
    /* put the terminating NUL character at the right location */
    pre_indent[level * INDENT_PER_LEVEL] = '\0';
    post_indent[(MAX_LEVEL - level) * INDENT_PER_LEVEL] = '\0';
}

/*
 * Print the specified value in conventional memory pages (of 16
 * bytes) as bytes and kilobytes, e.g "16,384   (16K)".
 *
 * format must be valid for convert() which means it must include a
 * "%s".  On top of the width of that string will be an additional 8
 * characters that are used to show the value in kilobytes, so for a
 * value less than 1000K there will be two spaces of leading padding,
 * e.g. "  (999K)".
 *
 * e.g. print_classify_value("%7s", 999999)
 *      results in: "999,999  (999K)"
 *                  \_____/\______/
 *                     /       \
 *  "%7s": 7 characters         always 8 characters wide
 *
 * You should therefore typically pass at least "%7s" and allow 7+8=15
 * characters to allow enough room for a normal conventional memory
 * block's size to be displayed.
 */
static void print_classify_value(const char *format, unsigned n)
{
    char kbuf[8];
    convert(format, n*CONV_BYTES_PER_PAGE);
    sprintf(kbuf, "(%uK)", round_seg_kb(n));
    printf("%8s", kbuf);
}

static void print_minfo_full(MINFO *entry, unsigned int level)
{
    char pre_indent[MAX_INDENT_SIZE + 1];
    char post_indent[MAX_INDENT_SIZE + 1];

    indent_setup(pre_indent, post_indent, level);
#if 0
    printf("  %04X%9lu   %s",
           entry->seg, (ulong)entry->size * CONV_BYTES_PER_PAGE,
	   pre_indent);
#endif
    printf("  %04X", entry->seg);
    print_classify_value("%11s", (ulong)entry->size);
    printf("  %s", pre_indent);
    entry->print_name(entry);
    printf("%s  ", post_indent);
    entry->print_type(entry);
    printf("\n");
}

static void print_minfo_free(MINFO *entry, unsigned int level)
{
    printf("  %04X", entry->seg);
    print_classify_value("%11s", (ulong)entry->size);
    printf("\n");
}

static void print_devinfo_full(DEVINFO *entry, unsigned int level)
{
    char pre_indent[MAX_INDENT_SIZE + 1];
    char post_indent[MAX_INDENT_SIZE + 1];

    indent_setup(pre_indent, post_indent, level);

/* FIXME: make sure entry->minfo != NULL */
    if (entry->minfo->type == MT_KERNEL) {
	printf("                             %s%-8s%ssystem device driver\n",
	       pre_indent, entry->devname, post_indent);
    } else {
	printf("                             %s%-8s%sinstalled DEVICE=%s\n",
	       pre_indent, entry->devname, post_indent, entry->minfo->name);
    }
}

static void print_classify_entry(char *name, unsigned total_conv, unsigned total_umb)
{
    printf("  %-9s", name);
    print_classify_value("%9s", total_conv + total_umb);
    print_classify_value("%11s", total_conv);
    print_classify_value("%11s", total_umb);
    printf("\n");
}

static void classify_list(unsigned convmemfree, unsigned umbmemfree)
{
    MINFO *ml, *ml2;
    unsigned total_conv, total_umb;
    unsigned convtopseg = biosmemory()*64;

    printf(_(4,0,"\nModules using memory below 1 MB:\n\n"));
    printf(_(4,1,
	   "  Name           Total           Conventional       Upper Memory\n"));
    printf("  --------  ----------------   ----------------   ----------------\n");
    /* figure out code used by "SYSTEM" */
    ml = make_mcb_list(NULL);
    total_conv = ml->seg; total_umb = 0;
    for (ml=make_mcb_list(NULL);ml!=NULL;ml=ml->next) {
        if (ml->owner == _psp || ml->type == MT_DOSDATA 
	    || ml->type == MT_RESERVED)
            ml->classified = 1;
        else if (ml->type == MT_SYSCODE || ml->type == MT_SYSDATA
		 || ml->type == MT_KERNEL || ml->type == MT_IVT
		 || ml->type == MT_BDA || ml->type >= MT_DEVICE) {
            int size = ml->size + 1;
	    if (ml->type == MT_DEVICE)
		size = -(size - 1);
	    else
		ml->classified = 1;
            if (ml->seg < convtopseg)
                total_conv += size;
            else
                total_umb += size;
        }
    }
    print_classify_entry(_(4,2,"SYSTEM"), total_conv, total_umb);
    for (ml=make_mcb_list(NULL);ml!=NULL;ml=ml->next)
        if (ml->type > MT_FREE && !ml->classified) {
            total_conv = total_umb = 0;
            for (ml2 = ml; ml2 != NULL; ml2 = ml2->next) {
                if (!ml2->classified && ml2->type > MT_FREE &&
                    ml2->owner == ml->owner) {
                    ml2->classified = 1;
                    if (ml2->seg < convtopseg)
                        total_conv += ml2->size + 1;
                    else
                        total_umb += ml2->size + 1;
                }
            }
            print_classify_entry(ml->name, total_conv, total_umb);
        }

    print_classify_entry(_(4,3,"Free"), convmemfree, umbmemfree);
}

typedef void (*print_minfo_t)(MINFO *entry, unsigned int level);
typedef void (*print_devinfo_t)(DEVINFO *entry, unsigned int level);

static void generic_list(MINFO *ml, unsigned int level,
			 print_minfo_t print_minfo,
			 print_devinfo_t print_devinfo)
{
    DEVINFO *dl;

    for (;ml!=NULL;ml=ml->next) {
	if (print_minfo) {
	    print_minfo(ml, level);
	}
	if (print_devinfo) {
	    for (dl=ml->first_dev;dl!=NULL;dl=dl->next_in_minfo) {
		if (print_devinfo != NULL) {
		    print_devinfo(dl, level);
		}
	    }
	}
	if (ml->first_child) {
	    generic_list(ml->first_child, level + 1,
			 print_minfo, print_devinfo);
	}
    }
}    

static void print_full_header(void)
{
/* FIXME: NLS - increased width of "Name" field by 2 */
    printf(_(4,4,"\nSegment       Total            Name          Type\n"));
    printf(        "-------  ----------------  ------------  -------------\n");
}

static void print_free_header(void)
{
/* FIXME: NLS */
    printf(_(4,4,"\nSegment       Total\n"));
    printf(        "-------  ----------------\n");
}

static void print_full_footer(ulong total)
{
/* FIXME: NLS */
    printf(        "         ----------------\n");
    printf(        "Total:");
    print_classify_value("%11s", total);
    printf("\n");
}

static void upper_list(void)
{
    MINFO *mlisthead = make_mcb_list(NULL);

    /* ignore the return value, as we get the DEVINFO entries from mlisthead */
    (void) make_dev_list(mlisthead);

    /*
     * Filter the list to just the entries with the given name.
     */
    mlisthead = filter_mlist(mlisthead, filter_upper, NULL);

    print_full_header();

    generic_list(mlisthead, 0, print_minfo_full, print_devinfo_full);
}

static void full_list(void)
{
    print_full_header();
    generic_list(make_mcb_list(NULL), 0, print_minfo_full, NULL);
}

static void device_list(void)
{
    DEVINFO *dl;
    char *progname;

    printf(_(4,5,"\n   Address     Attr    Name       Program\n"));
    printf(        " -----------  ------ ----------  ----------\n");
             /*  XXXX:XXXX    XXXX   XXXXXXXX    XXXXXXXX */
    for (dl=make_dev_list(make_mcb_list(NULL));dl!=NULL;dl=dl->next) {
        progname = (dl->minfo != NULL) ? dl->minfo->name : "";
	printf("  %p    %04X   %-8s    %-8s\n", dl->addr, dl->attr,
                dl->devname, progname);
    }
}

static void debug_list(void)
{
    MINFO *ml;

/* FIXME: NLS and pick correct text */
/* FIXME: high mem not shown separately */
    printf("Conventional Memory Detail:\n");

    ml = make_mcb_list(NULL);
    /* ignore the return value, as we get the DEVINFO entries from ml */
    (void) make_dev_list(ml);

    print_full_header();
    generic_list(ml, 0, print_minfo_full, print_devinfo_full);
}

static void ems_list(void)
{
    EMSINFO *ems;
    ushort i;
    static char handlename_other[9];
    char *handlename, *handlename_sys;
    static char format[] = "  %-20s";
    
    ems=check_ems();
    if (ems==NULL)
    {
        printf(_(5,1,"  EMS driver not installed in system.\n"));
    }
    else
    {
	printf(format, _(5,2,"\nEMS driver version"));
	printf("%1u.%1u\n", ems->vermajor, ems->verminor);
	if (ems->frame) {
	    printf(format, _(5,3,"EMS page frame"));
	    printf("%04X\n", ems->frame);
	}
	printf(format, _(5,4,"Total EMS memory"));
	printf(_(1,1,"%lu bytes\n"), ems->size * EMS_BYTES_PER_PAGE);
	printf(format, _(5,5,"Free EMS memory"));
	printf(_(1,1,"%lu bytes\n"), ems->free * EMS_BYTES_PER_PAGE);
	printf(format, _(5,6,"Total handles"));
	printf("%u\n", ems->totalhandle);
	printf(format, _(5,7,"Free handles"));
	printf("%u\n", ems->freehandle);

	printf(_(5,8,"\n  Handle   Pages    Size       Name\n"));
	printf(        " -------- ------  --------   ----------\n");
	handlename_sys = _(4,2,"SYSTEM");
        for (i=0;i<ems->usehandle;i++)
        {
            handlename = handlename_sys;
            if (ems->vermajor >= 4)
            {
                if (ems->handles[i].handle != 0)
                {
                    handlename = handlename_other;
                    memset(handlename, 0, 9);
                    ems_get_handle_name(ems->handles[i].handle, handlename);
		    check_name(handlename, handlename, 8);
                }
            }

            printf("%9u%7u%10lu%11s\n", ems->handles[i].handle,
                   ems->handles[i].pages, (ulong)ems->handles[i].pages
		   * EMS_BYTES_PER_PAGE, handlename);
        }
    }
}

static long xms_common(unsigned char al, signed char bl, ushort result)
{
     return (al != 0 ? result : (long)bl << 24);
}

static long xms_handleinfo(ushort handle)
{
     /* Get handle information */
     ulong bx_ax = call_xms_driver_bx_ax(0xe, handle);
     return xms_common((uchar)bx_ax, (uchar)(bx_ax >> 16),
                       (ushort)(bx_ax >> 16));
}

/* Get handle size information */

#define xms_handlesize(handle) ((ushort)(call_xms_driver_dx_bl_al(0xe, handle) >> 16))

static long xms_alloc(ushort kbytes)
{
     /* "Allocate extended memory block" */
     ulong dx_bl_al = call_xms_driver_dx_bl_al(0x9, kbytes);
     return xms_common((uchar)dx_bl_al,
                       (uchar)(dx_bl_al>>8), (ushort)(dx_bl_al>>16));
}

static void xms_free(ushort handle)
{
     /* "Free extended memory block" */
     call_xms_driver_dx_bl_al(0xa, handle);
}

static long xms_query_a20(void)
{
    ulong bx_ax = call_xms_driver_bx_ax(0x7, 0);
    return (((uchar)(bx_ax>>16))!=0 ? ((long)bx_ax<<8) : (uchar)(bx_ax));
}

static void xms_list(void)
{
    UPPERINFO *upper;
    XMSINFO *xms;
    XMS_HANDLE *handle = NULL;
    ushort i;
    long lhandle;
    static char format[] = "%-26s";
    XMS_HANDLE_TABLE far* xmsHanTab;

    xms = check_xms();

    if (xms_drv==NULL)
	{
	printf(_(6,0,"XMS driver not installed in system.\n"));
	return;
	}

    printf(_(6,1,"\nTesting XMS memory ...\n"));

    lhandle = xms_query_a20();
    if (lhandle < 0) {
	printf(_(6,2,"XMS INTERNAL ERROR.\n"));
	return;
    }
    xms->a20 = lhandle & 0xff;

    /*
    // 01/4/27 tom + alain
    //
    // although the 'old' method to search the handle table should be OK,
    // it crashes somehow and for unknown reason under Win98. So, a 'new' method to
    // query all handles was implemented, using INT 2F, AX=4309
       test support for INT2F AX=4309 first */
    xmsHanTab = get_xmsHanTab();
    if (xmsHanTab != NULL    && /* test returned OK */
	xmsHanTab->sizeOfDesc == sizeof(XMS_HANDLE_DESCRIPTOR)) /* assert correct size */
    {
	XMS_HANDLE_DESCRIPTOR far* descr = xmsHanTab->xmsHandleDescr;

	printf(_(6,3,"INT 2F AX=4309 supported\n"));

        for (i=0;i<xmsHanTab->numbOfHandles;i++,descr++)
        {
            if (descr->flag != 0x01 && /* not free */
                descr->xmsBlkSize != 0)   /* and takes memory */
            {
                if (handle==NULL)
                    xms->handles=handle=xmalloc(sizeof(XMS_HANDLE));
                else {
                    handle->next=xmalloc(sizeof(XMS_HANDLE));
                    handle=handle->next;
                }
                handle->handle=FP_OFF(descr);
                handle->size=descr->xmsBlkSize*1024L;
                handle->locks=descr->locks;
                handle->next=NULL;
            }
        }
    }
    else
    {
        /* old method */
        /* query all handles 0..0xffff */

        for (i=0;i<65535u;i++)
        {
            /* Get handle information */
            if ((lhandle = xms_handleinfo(i)) >= 0) {
                uchar free_handles_tmp = lhandle & 0xff;
                if (handle==NULL)
                    xms->handles=handle=xcalloc(1, sizeof(XMS_HANDLE));
                else {
                    handle->next=xcalloc(1, sizeof(XMS_HANDLE));
                    handle=handle->next;
                }
                handle->handle=i;
                if (xms->vermajor >= 3 && xms->is_386)
                    handle->size=xms_exthandlesize(i);
                if (!handle->size)
                    handle->size=xms_handlesize(i);
                handle->size *= 1024L;
                handle->locks=lhandle >> 8;
                handle->next=NULL;
                if (xms->freehandle < free_handles_tmp)
                {
                    xms->freehandle = free_handles_tmp;
                }
            }
        }
    }

    /* First try to get a handle of our own. */
    /* First we try 1kB. I'm not sure if XMS driver
       must support a zero sized allocate. */
    /* "Allocate extended memory block" */
    if (((lhandle = xms_alloc(1)) >= 0) ||
	/* Now try a zero sized allocate just in case there was no free memory. */
	((lhandle = xms_alloc(0)) >= 0))
    {
        long info = xms_handleinfo((ushort)lhandle);
	/* else nothing worked out. Use whatever we got from the loop above. */
	/* We can't do much if the free call fails, so it ends here. */
	if (info >= 0)
	    /* Hey! We got some info. Put it in a safe place. */
	    xms->freehandle = (uchar)info + 1;
	/* Add one for the handle we have allocated. */
	xms_free((ushort)lhandle);
    }

    printf(format, _(6,4,"XMS version"));
    printf("%u.%02u \t\t", xms->vermajor, xms->verminor);
    printf(format, _(6,5,"XMS driver version"));
    printf("%u.%02u\n", xms->drv_vermajor, xms->drv_verminor);
    printf(format, _(6,6,"HMA state"));
    printf("%s \t", (xms->hma) ? _(6,7,"exists") : _(6,8,"does not exist"));
    printf(format, _(6,9,"A20 line state"));
    printf("%s\n", (xms->a20) ? _(6,10,"enabled") : _(6,11,"disabled"));
    printf(format, _(6,12,"Free XMS memory"));
    printf(_(1,1,"%lu bytes\n"), xms->free*1024L);
    printf(format, _(6,13,"Largest free XMS block"));
    printf(_(1,1,"%lu bytes\n"), xms->largest*1024L);
    printf(format, _(6,14,"Free handles"));
    printf("%u\n", xms->freehandle);
    printf("\n");
    if (xms->handles != NULL)
    {
	printf(_(6,15," Block   Handle     Size     Locks\n"));
	printf(       "------- --------  --------  -------\n");
	for (i=0, handle=xms->handles;handle!=NULL;handle=handle->next, i++)
	    printf("%7u %8u  %8lu  %7u\n", i, handle->handle,
		   handle->size, handle->locks);

    }

    upper = check_upper(make_mcb_list(NULL));
    if (upper != NULL)
    {
	printf(format, _(6,16,"Free upper memory"));
	printf(_(1,1,"%lu bytes\n"), upper->free*CONV_BYTES_PER_PAGE);
	printf(format, _(6,17,"Largest upper block"));
	printf(_(1,1,"%lu bytes\n"), upper->largest*CONV_BYTES_PER_PAGE);
    }
    else
    {
	printf(_(6,18,"Upper memory not available\n"));
    }

}

ulong total_mem(MINFO *mlist)
{
    ulong result = 0;

    while (mlist != NULL) {
	result += mlist->size;
	mlist = mlist->next;
    }
    return result;
}

void module_list(char *module_name)
{
    MINFO *mlisthead = make_mcb_list(NULL);

    /*
     * The parser will not indicate /MODULE was specified without setting
     * module_name.
     */
/*FIXME: assert(module_name != NULL);*/

    /* ignore the return value, as we get the DEVINFO entries from mlisthead */
    (void) make_dev_list(mlisthead);

    /*
     * Filter the list to just the entries with the given name.
     */
    mlisthead = filter_mlist(mlisthead, filter_by_module_name, module_name);
    if (mlisthead == NULL) {
/* FIXME: NLS */
	printf("%s is not currently in memory.\n", module_name);
	return;
    }

/* FIXME: NLS */
    printf("%s is using the following memory:\n", module_name);
    print_full_header();

    generic_list(mlisthead, 0, print_minfo_full, print_devinfo_full);

    print_full_footer(total_mem(mlisthead));
}

/*
 * Obviously MCBs that are actually free should be shown as such, but we also
 * show our own MCB as free because the user isn't too interested in how much
 * memory MEM takes.  Note that MEM /DEBUG or MEM /FULL should still show
 * MEM in their output, but MEM /FREE should show MEM's block as free and
 * the plain MEM output should include MEM's own MCB in the free memory total.
 */
int show_minfo_as_free(MINFO *entry)
{
    return (entry->type == MT_FREE || (entry->seg + 1 == _psp));
}

int filter_free(MINFO *entry, void *data)
{
    return (show_minfo_as_free(entry));
}

void free_list(void)
{
    MINFO *mlisthead = make_mcb_list(NULL);

    /* ignore the return value, as we get the DEVINFO entries from ml */
    (void) make_dev_list(mlisthead);

    /*
     * Filter the list to just the entries with the given name.
     */
    mlisthead = filter_mlist(mlisthead, filter_free, NULL);
    if (mlisthead == NULL) {
/* FIXME: NLS */
	printf("There is no free memory!\n");
	return;
    }

/* FIXME: MS-DOS splits into conventional and upper; we should too */
/* FIXME: NLS */
    printf("Free Conventional Memory:\n");

    print_free_header();

    /*
     * Pass NULL for print_devinfo as we won't be printing any devices
     * - a free block won't have any devices linked off it.
     */
    generic_list(mlisthead, 0, print_minfo_free, NULL);

    print_full_footer(total_mem(mlisthead));
}

/* function to obtain the number of lines on the screen...added by brian reifsnyder.  */
static uchar get_font_info(void)
{
    uchar number_of_lines = *((uchar far *)MK_FP(0x40, 0x84));
    if (number_of_lines == 0)
        number_of_lines = 25;
    else
        number_of_lines++;
    return number_of_lines;
}

int is_switch_char(char c)
{
    return (c == '-' || c == '/');
}

#define NO_SWITCH_CHAR '\0'

typedef enum {
    F_HELP = 1,
    F_DEVICE = 2,
    F_EMS = 4,
    F_FULL = 8,
    F_UPPER = 16,
    F_XMS = 32,
    F_PAGE = 64,
    F_CLASSIFY = 128,
    F_DEBUG = 256,
    F_MODULE = 512,
    F_FREE = 1024,
    F_ALL = 2048,
    F_NOSUMMARY = 4096
} opt_flag_t;

typedef struct {
    char *s;
    opt_flag_t flag;
    char **value;
} opt_t;

typedef enum {
    GET_OPTS_STATE_SWITCH,
    GET_OPTS_STATE_VALUE
} get_opts_state_t;

int get_opts (char *argv[], opt_t *opts, int opt_count)
{
    int arg_index = 1, opt_index, matched_index;
    int partial_match_count, exact_match_count;
    char *char_ptr = argv[arg_index];
    char *switch_start, *switch_text_start, old_switch_char;
    opt_flag_t flags = 0;
    get_opts_state_t state = GET_OPTS_STATE_SWITCH;

    while (char_ptr != NULL && (flags & F_HELP)==0) {
	/*
	 * If state == GET_OPTS_STATE_SWITCH, we're at the start of a
	 * switch, which might be the start of argv[i] or somewhere
	 * inside it.  Otherwise we're at the start of a value for a
	 * switch, which is always at the start of argv[i].
	 */
	switch_start = char_ptr;

	/*
	 * A switch should start with a switch character.
	 */
	if (state == GET_OPTS_STATE_SWITCH) {
	    if (!is_switch_char(*char_ptr)) {
/* FIXME: NLS */
		fatal(_(0,4,"unknown option (expected a '/'): %s\nUse /? for "
			"help\n"), switch_start);
	    }
	    char_ptr++;
	} else { /* state == GET_OPTS_STATE_VALUE */
	    if (is_switch_char(*char_ptr)) {
/* FIXME: NLS */
		fatal(_(0,4,"Expected a value after /%s, not another switch\n"
			"Use /? for help\n"),
		      opts[matched_index].s);
	    }
	}

	/*
	 * Now get the text of the switch or value - scan for the end of the
	 * switch.
	 */
	switch_text_start = char_ptr;
	while (*char_ptr != '\0' && !is_switch_char(*char_ptr)) {
	    char_ptr++;
	}

	/*
	 * If we're not at the end of the current argv[] element, we
	 * need to put a '\0' character at char_ptr so that string
	 * operations don't see the following argument.
	 */
	if (is_switch_char(*char_ptr)) {
	    old_switch_char = *char_ptr;
	    *char_ptr = '\0';
	} else {
	    old_switch_char = NO_SWITCH_CHAR;
	}

	if (state == GET_OPTS_STATE_SWITCH) {
	    strupr(switch_text_start);
#ifdef DEBUG
	    printf("%s: argument is [%s]\n", __FUNCTION__, switch_text_start);
#endif

	    exact_match_count = 0;
	    partial_match_count = 0;
	    for (opt_index = 0; opt_index < opt_count; opt_index++) {
		if (strstr(opts[opt_index].s, switch_text_start)
		    == opts[opt_index].s) {
		    /*
		     * opts[opt_index].s begins with switch_text_start, so
		     * we check if we have an exact match (the string
		     * lengths are the same) or a partial one.
		     * matched_index needs to contain the index into
		     * opts[] of the matching switch, or one of them if
		     * there is ambiguity.  An exact match has precedence
		     * over a partial one, i.e. we set matched_index on an
		     * exact match regardless of whether there are any
		     * partial matches, and we only set it on a partial
		     * match if there haven't been any exact ones.
		     */
		    if (strlen(opts[opt_index].s)
			== strlen(switch_text_start)) {
			exact_match_count++;
			matched_index = opt_index;
#ifdef DEBUG
			printf("%s: argument is exact match (number %u) for "
			       "[%s]\n", __FUNCTION__, exact_match_count,
			       opts[opt_index].s);
#endif
		    } else {
			partial_match_count++;
			if (exact_match_count == 0) {
			    matched_index = opt_index;
			}
#ifdef DEBUG
			printf("%s: argument is partial match (number %u) for "
			       "[%s]\n", __FUNCTION__, partial_match_count,
			       opts[opt_index].s);
#endif
		    }
		}
	    }

	    /*
	     * Now figure out if we had ambiguity or no match at all.  The
	     * normal cases are exact_match_count == 1 (in which case we
	     * don't care if there were any partial matches) or
	     * exact_match_count == 0 && partial_match_count == 1
	     * (i.e. the user's input was a partial match for only one
	     * switch).
	     */
	    if (exact_match_count > 1) {
		/*
		 * This is a bug - there shouldn't be two options with the
		 * same text in opts[]!
		 */
/* FIXME: NLS */
		fatal(_(0,4,"Internal error: option '%s' was an exact match "
			"for two different switches\n"), switch_start);
	    } else if (exact_match_count == 0) {
		if (partial_match_count > 1) {
/* FIXME: NLS */
		    fatal(_(0,4,"Error: option '%s' is ambiguous - it is a "
			    "partial match for two different switches\n"),
			  switch_start);
		}
		if (partial_match_count == 0) {
		    fatal(_(0,4,"unknown option: %s\nUse /? for help\n"),
			  switch_start);
		}
	    }
	    /*
	     * We had only one exact match or no exact match but one
	     * partial match.  In either case, matched_index will be set.
	     */
#ifdef DEBUG
	    printf("%s: resolved to: [%s]\n", __FUNCTION__,
		   opts[matched_index].s);
#endif
	    flags |= opts[matched_index].flag;

	    /*
	     * If value is not NULL, then on the next iteration through this
	     * loop we need to get a value for the switch.
	     */
	    if (opts[matched_index].value != NULL) {
#ifdef DEBUG
		printf("%s: expecting a value for this switch\n",
		       __FUNCTION__);
#endif
		state = GET_OPTS_STATE_VALUE;
	    }
	} else { /* state == GET_OPTS_STATE_VALUE */
#ifdef DEBUG
	    printf("%s: got switch value [%s]\n", __FUNCTION__, switch_start);
#endif
	    *(opts[matched_index].value) = xstrdup(switch_start);
	    state = GET_OPTS_STATE_SWITCH;
	}

	if (old_switch_char == NO_SWITCH_CHAR) {
	    /*
	     * *char_ptr was originally '\0' so move to next argv[] element.
	     */
            arg_index++;
            char_ptr = argv[arg_index];
        } else {
	    /*
	     * *char_ptr was originally a switch character, so restore it and
	     * continue through the loop to process the next switch in this
	     * argv[] element.
	     */
	    *char_ptr = old_switch_char;
	}
    }

    /*
     * If we are still in this state, it means we failed to get the required
     * value.
     */
    if (state == GET_OPTS_STATE_VALUE) {
/* FIXME: NLS */
	fatal(_(0,4,"Expected a value after /%s\n"
		"Use /? for help\n"),
	      opts[matched_index].s);
    }

    return (flags);
}

int main(int argc, char *argv[])
{
    opt_flag_t flags;
    UPPERINFO *upper;
    unsigned memfree;
    static char *module_name = NULL;
    static opt_t opts[] = 
    {
      { "?",		F_HELP,		NULL },
      { "ALL",		F_ALL,		NULL },
      { "C",		F_CLASSIFY,	NULL },
      { "CLASSIFY",	F_CLASSIFY,	NULL },
      { "D",		F_DEBUG,	NULL },
      { "DEBUG",	F_DEBUG,	NULL },
      { "DEVICE",	F_DEVICE,	NULL },
      { "EMS",		F_EMS,		NULL },
      { "F",		F_FULL,		NULL },
      { "FREE",		F_FREE,		NULL },
      { "FULL",		F_FULL,		NULL },
      { "HELP",		F_HELP,		NULL },
      { "MODULE",	F_MODULE,	&module_name },
      { "NOSUMMARY",	F_NOSUMMARY,	NULL },
      { "PAGE",		F_PAGE,		NULL },
      { "U",		F_UPPER,	NULL },
      { "XMS",		F_XMS,		NULL }
    };

    /* avoid unused argument warning? */
    argc = argc;

    kittenopen("mem");

    flags = get_opts(argv, &opts, sizeof(opts) / sizeof(opts[0]));

    upper=check_upper(make_mcb_list(&memfree));

    if (flags & F_PAGE)   num_lines=get_font_info();

/*
 * FIXME: In MS-DOS, when /M is specified, nothing else is shown.
 * We need to make sure no other flags are specified!
 */

    /* FIXME: should probably have /DEBUG determine whether /MODULE includes a subtree of devices */

    if (flags & F_MODULE) {
	module_list(module_name);
	return 0;
    }
    if (flags & F_FREE) {
	free_list();
	return 0;
    }

    if (flags & F_DEBUG)  debug_list();
    if (flags & F_DEVICE) device_list();
    if (flags & F_EMS)    ems_list();
    if (flags & F_FULL)   full_list();
    if (flags & F_UPPER)  upper_list();
    if (flags & F_XMS)    xms_list();
    if (flags & F_CLASSIFY)    classify_list(memfree, upper ? upper->free : 0);
        
    if (flags & F_HELP)
      {
	printf(_(7, 0, "FreeDOS MEM version %d.%d\n"),
	       MEM_MAJOR, MEM_MINOR);
/* FIXME: update valid command-line options */
	printf("%s\n\n%s\n  %s\n  %s\n  %s\n  %s\n  %s\n  %s\n  %s\n  %s\n",
	_(7, 1, "Displays the amount of used and free memory in your system."),
	_(7, 2, "Syntax: MEM [/E] [/F] [/C] [/D] [/U] [/X] [/P] [/?]"),
	_(7, 3, "/E  Reports all information about Expanded Memory"),
	_(7, 4, "/F  Full list of memory blocks"),
	_(7, 5, "/C  Classify modules using memory below 1 MB"),
	_(7, 6, "/D  List of device drivers currently in memory"),
	_(7, 7, "/U  List of programs in conventional and upper memory"),
	_(7, 8, "/X  Reports all information about Extended Memory"),
	_(7, 9, "/P  Pauses after each screenful of information"),
	_(7, 10, "/?  Displays this help message"));
	return 1;
      }

    if (!(flags & F_NOSUMMARY)) {
        normal_list(memfree, upper, flags & F_ALL, flags & F_DEBUG);
    }
    return 0;
}

/*
        TE - some size optimizations for __TURBOC__
    
        as printf() is redefined in PRF.C to use no stream functions,
        rather calls DOS directly, these Stream operations are nowhere used,
        but happen to be in the executable.

        so we define some dummy functions here to save some precious bytes :-)

        this is in no way necessary, but saves us some 1500 bytes */

#ifdef __TURBOC__

#define UNREFERENCED_PARAMETER(x) if (x);

int _Cdecl flushall(void){return 0;}

int _Cdecl fprintf(FILE *__stream, const char *__format, ...)
{ UNREFERENCED_PARAMETER (__stream);
 UNREFERENCED_PARAMETER ( __format);    return 0;}
int _Cdecl fseek(FILE *__stream, long __offset, int __whence)
{ UNREFERENCED_PARAMETER (__stream);
 UNREFERENCED_PARAMETER (__offset);
 UNREFERENCED_PARAMETER ( __whence);
 return 0;}

int _Cdecl setvbuf(FILE *__stream, char *__buf, int __type, size_t __size)
{ UNREFERENCED_PARAMETER (__stream);
 UNREFERENCED_PARAMETER ( __buf);
 UNREFERENCED_PARAMETER ( __type);
 UNREFERENCED_PARAMETER ( __size);   return 0;}

void _Cdecl _xfflush (void){}
void _Cdecl _setupio (void){}

#endif


/* FIXME: should the sizes shown to the user include sizeof(MCB)?  it seems that we are not consistent in including the size of the MCB between /CLASSIFY and /FULL; in MS-DOS we don't have inconsistency between /C and /F but don't know what they DO do */
