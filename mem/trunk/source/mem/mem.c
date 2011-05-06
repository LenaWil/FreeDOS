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

    MEM 1.6, April 2004 (CVS revision 1.6)
    * minor output tweaks, don't upcase names anymore
    * try to detect UMB holes and don't count them as upper memory
    * display UMB holes as "reserved" in mem/f output
    * display version for "mem/?"


    MEM 1.7 beta, August 2004, Bart Oldeman (CVS revision 1.10)
    * kittenize (support localization)
    * Fix "UMB corruption" problem. As it turned out FreeCOM put the
      environment just below the top of conv mem where it used to be
      placed in an UMB.  That's not very nice in terms of memory
      consumption, but MEM should work in either case.
    * 64L -> 64 (optimization)
    * avoid NULL derefs.
    * Improved diagnostics (Eric Auer) Adjusted help.
    * Correct NULL check for loop that check the last conv mem seg.
    * update mem to 1.7beta and fix a bug when a free MCB looks like a
      PSP.

    Unreleased, September 2004, Bart Oldeman (CVS revision 1.11)
    * Replace getenv() by our own (~1700 bytes uncompressed).

    MEM 1.8 (alpha 1), 29 August 2005, David O'Shea (doshea revision 26)
    * make malloc() cause a fatal error if its buffer is full (this change
      reverted later)
    * include Interrupt Vector Table, BIOS Data Area and kernel areas for
      which no MCBs exist in the output of the new /DEBUG command
    * MINFO structure contains pointers to first and last DEVINFO
      structures that describe devices in the memory block and each
      DEVINFO contains a back-pointer to the MINFO of the memory
      block it lies in
    * provide details for STACKS, FILES, BUFFERS and LASTDRIVE blocks,
      although the STACKS details will not work with FreeDOS
    * show memory available via interrupt 15
    * add /ALL command which shows how much HMA is free, HMAMIN, etc.;
      HMAMIN is probed via a binary search of allocation/free attempts;
      without /ALL we still show whether MS-DOS is managing the HMA
    * add new /MODULE and /FREE commands which use new functions to take the
      full MINFO list and return a new filtered copy and re-implement /U
      this way
    * support long command-line option names such as /CLASSIFY in place
      of /C with shortest unique prefix matching
    * add /H[ELP] command as a synonym for /?
    * add /NOSUMMARY command to skip default output

    MEM 1.9a2 (alpha 2), 18 November 2005, David O'Shea (doshea revision 43)
    * add support for MUSCHI
    * restore support for Turbo C compiler by adding get_ext_mem_size()
      to MEMSUPT.ASM
    * replace minfo_typenames[] array and accesses to it with a function 
      containing a switch statement so we never have to pass a variable
      argument to the _() macro and hence so that MUSCHI works
    * classify_list() used to treat ml->seg as the amount of "SYSTEM" memory
      in pages before the first MINFO, but now there are actually a number of
      MINFO entries for all the reserved memory blocks, e.g. MT_IVT and
      MT_BDA, which are explicitly counted in classify_system()
    * correct malloc() to return NULL instead of trigering a fatal error in
      case of its buffer being full; allocators that want to have a fatal
      error triggered in case of insufficient memory can (and do) call
      xmalloc()
    * kitten-ize strings added in 1.8 alpha 1
    * a MEM.EXE compiled with -DDEBUG will no longer unconditionally
      print all debugging information, instead providing command-line
      options for the different debug flags (which start with /DBG);
      note that you can't debug the command-line parser code this way
      because you obviously need to have completed at least part of
      the parse to know that the option to debug it was specified, but
      you could add such a command-line option which, after parsing,
      causes flags to be reset to 0 and the parsing code to be
      re-invoked with debugging enabled; for now use #define
      DEBUG_PARSER
    * change method of generation of the help output so that new flags
      cannot be accidentally left undocumented in this output: for each
      entry in opts[] that corresponds to a unique flag, we either
      display the help string for the flag as returned by
      help_for_flag() or we show the name of the parameter with a note
      that no help text is available
    * version string in help output includes compile date/time, compiler
      information and an indication of whether DEBUG was defined
    * provide generic_split_list function to split MINFO list into two
      parts (conventional and upper) which is used to show some of the
      output split into these parts
    * in case of MEM /MODULE <name> being specified and <name> not
      existing in memory, we now return errorlevel 2 instead of 0;
      note that all fatal errors result in errorlevel 1
    * replace opt_flag_t enumeration with a set of #defines and a
      'typedef unsigned long' so we can force it to be long and
      #define masks/sets of flags
    * get_opts(): add documentation; allow one or more colons to be
      used in place of spaces between arguments; allow e.g.
      /MODULE<name> or /M<name> with no space/colon in between
    * provide /OLD option which causes /D to map to /DEVICE instead of
      /DEBUG and /F to map to /FULL instead of /FREE, i.e. causes /D
      and /F to take on their MEM 1.7 beta meanings instead of MS-DOS
      meanings
    * provide /SUMMARY option which always overrides /NOSUMMARY
    * avoid producing no output when /NOSUMMARY specified by itself
      or with /P

    MEM 1.9a3 (alpha 3), 15 April 2006, David O'Shea
    * kittenize "FILES=%u (%u in this block)" (4.12)
    * support "SYSTEM" as argument to "/MODULE" which will include
      everything we'd show under "/MODULE IO" and "/MODULE DOS"
    * remove comment on MT_DEVICE that is outdated due to the use of a
      tree data structure
    * update MUSCHI compile method to avoid #including a .c file
    * update changelog entries for MEM 1.7b onwards
    * prevent /DEBUG and /FREE from showing empty tables when there is no
      or no free upper memory respectively, instead they will explicitly
      indicate that the specified type of memory is not installed/free;
      kitten message 1.6 updated and 1.9 added
*/

#define MEM_MAJOR 1
#define MEM_MINOR 9
#define MEM_VER_SUFFIX "a3"

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

#define CONV_BYTES_PER_SEG	64UL

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
#define MT_DEVICE  11
#define MT_DOSDATA 12
#define MT_IFS     13

#ifdef MUSCHI
#include "mem_nls.h"
#define _(set,message_number,message) kittengets(set,message_number,muschi_ ## set ## _ ## message_number)
#else
#define _(set,message_number,message) kittengets(set,message_number,message)
#endif

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
    uchar classified; /* flag set to TRUE when entry processed for /C */
#if 0 /* presently unused */
    uchar vecnum;
    uchar *vectors;
#endif
    struct minfo *next;
    struct minfo *first_child;
#if 0 /* presently unused */
    struct minfo *caller;
#endif
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
 * The last segment address that is in conventional memory.
 */
unsigned int last_conv_seg;

void setup_globals(void)
{
    last_conv_seg = biosmemory() * CONV_BYTES_PER_SEG;
}

/*
 * What xms_available() returns if XMS is available.
 */
#define XMS_AVAILABLE_RESULT 0x80

/*
 * Return values from assembler function get_ext_mem_size().
 */
#define GET_EXT_MEM_SIZE_OK(result)	((result >> 16) == 0)
#define GET_EXT_MEM_SIZE_VALUE(result)	(result)
#define GET_EXT_MEM_SIZE_ERROR(result)	((result & 0xFF00) >> 8)

#define GET_EXT_MEM_SIZE_ERROR_INVALID_CMD	0x80
#define GET_EXT_MEM_SIZE_ERROR_UNSUPPORTED	0x86


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
extern ulong cdecl get_ext_mem_size(void);

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

#ifdef DEBUG
/*
 * Debug flags set on the command-line.
 */
int dbgdevaddr;
int dbghmamin;
int dbgcpu;
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
        return NULL;
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
        return ems; /* NULL */

    int67 = MK_FP(FP_SEG(int67),10);

    /*
     * The driver name is "EMMQXXX0" if EMM386 /NOEMS was used.  This
     * is true for both MS-DOS and FreeDOS EMM386.
     */
    if (!fnstreqn(int67, "EMMXXXX0", 8))
	return ems; /* NULL */
            
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
#ifdef DEBUG
    if (dbgcpu) {
	printf("check_xms: is_386\n");
    }
#endif
    xms->is_386 = is_386();
#ifdef DEBUG
    if (dbgcpu) {
	printf("check_xms: 386: %s\n", xms->is_386 ? "yes" : "no");
    }
#endif
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
    
#ifdef DEBUG
    if (dbgcpu) {
	printf("check_xms: total=%lu, check_e801\n", total);
    }
#endif
    if (total == 0)
        total = check_e801();
#ifdef DEBUG
    if (dbgcpu) {
	printf("check_xms: total=%lu, check_8800\n", total);
    }
#endif
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
#ifdef DEBUG
    if (dbgcpu) {
	printf("check_xms: xms_available\n");
    }
#endif
    if (xms_available() != XMS_AVAILABLE_RESULT)
        return xms;

#ifdef DEBUG
    if (dbgcpu) {
	printf("check_xms: get_xms_drv\n");
    }
#endif
    xms_drv = get_xms_drv();

#ifdef DEBUG
    if (dbgcpu) {
	printf("check_xms: xms_version\n");
    }
#endif
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
    /* assert(mlist!=NULL) -- comes from make_mcb_list */
    for (;;) {
        unsigned short seg = mlist->seg;
        if (seg == last_conv_seg)
            break;
        mlist=mlist->next;
        if (mlist==NULL) {
            fatal(_(0,2,"UMB Corruption: Chain doesn't reach top of low RAM at %dk. Last=0x%x.\n"),
                last_conv_seg/64, seg);
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

char *minfo_typename(int type)
{
    switch (type) {
    case MT_NONE:	return (_(3,0,""));
    case MT_FREE:	return (_(3,1,"free"));
    case MT_SYSCODE:	return (_(3,2,"system code"));
    case MT_SYSDATA:	return (_(3,3,"system data"));
    case MT_PROGRAM:	return (_(3,4,"program"));
    case MT_ENV:	return (_(3,5,"environment"));
    case MT_DATA:	return (_(3,6,"data area"));
    case MT_RESERVED:	return (_(3,7,"reserved"));
    case MT_IVT:	return (_(3,8,"interrupt vector table"));
    case MT_BDA:	return (_(3,9,"BIOS data area"));
    case MT_KERNEL:	return (_(3,10,"system data"));
    case MT_DEVICE:	return (_(3,11,"device driver"));
    case MT_DOSDATA:	return (_(3,12,"data area"));
    case MT_IFS:	return (_(3,13,"IFS"));
    default:		return (_(3,14,"(error)"));
    }
}

static void print_minfo_type_default(MINFO *entry)
{
    printf("%s", minfo_typename(entry->type));
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

    printf("FILES=%u", files_total_count);
    printf(_(4, 12, " (%u in this block)"), specific->count);
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
/*
 * FIXME: for FreeDOS, need to show STACKS in type column instead of
 * name since we do that for MS-DOS where we show STACKS=x,y but for FreeDOS
 * we don't know what x and y are.
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
/*
 * FIXME: offset into list of lists is different (1Bh) for DOS 3.0 and
 * doesn't exist in previous versions.
 */
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
	if (type == 'I'
	    && *(unsigned short far *)MK_FP(0, 0x40e) == mlist->seg)
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
	    mlist->type = MT_SYSCODE;
	    /* no  holes in conv mem */
	    if (seg == last_conv_seg || (seg > last_conv_seg
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

    mlist->name = xmalloc(MCB_NAME_MAX_LEN + 1);
    mcb = MK_FP(mlist->seg, 0);
    check_name(mlist->name, mcb->name, MCB_NAME_MAX_LEN);
    mlist->environment=env_seg(mlist->seg);
    mlist->type=MT_PROGRAM;
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
    unsigned freemem;
    ushort mlist_pid, find_pid;
    PSP far *psp;

    if(mlistroot!=NULL)
	return(mlistroot);

    origlink=get_upperlink();
    set_upperlink(1);

    mlistroot = mlist = new_minfo();

    /*
     * Set up BIOS memory blocks which are always fixed in size and location.
     */
    mlist->seg = FP_SEG(0);
    /* num. interrupts * sizeof(far ptr) / para */
    mlist->size = 256 * 4 / CONV_BYTES_PER_PAGE;
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
	    mlist_pid = mlist->seg+1;
	    psp = MK_FP(mlist_pid, 0);
	    if (psp->parent_pid != 0 && psp->parent_pid != mlist_pid) {
		/* this program has a parent program we should search for */
		find_pid = psp->parent_pid;
	    } else {
		find_pid = 0;
	    }

            for(mlistj=mlistroot;mlistj!=NULL;mlistj=mlistj->next) {
                if (mlist->seg != mlistj->seg) {
		    /* did mlist allocate mlistj? */
		    if (mlistj->owner == mlist_pid) {
			mlistj->name = mlist->name;
			mlistj->type = MT_ENV;
			if (mlist->environment != mlistj->seg+1)
			    mlistj->type = MT_DATA;
		    }
#if 0 /* process list functionality not complete yet */
		    /* did mlistj execute mlist? */
		    if (mlistj->seg + 1 == find_pid) {
			mlist->caller = mlistj;
		    }
#endif
		}
            }
        }

#if 0 /* presently unused */
        if (mlist->type != MT_SYSDATA)
            search_vectors(mlist);
#endif

    }

    freemem = 0;

    /* get free memory */
    for (mlist=mlistroot; mlist!=NULL; mlist=mlist->next) {
        if (show_minfo_as_free(mlist) && mlist->seg < last_conv_seg)
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
    ulong normal_addr = (ulong) FP_SEG(addr) * CONV_BYTES_PER_PAGE
                        + (ulong) FP_OFF(addr);

#ifdef DEBUG
    if (dbgdevaddr) {
	printf("is %05lX in range %05lX(%04X)-%05lX (program %s)\n",
	       normal_addr, base, minfo->seg, limit, minfo->name);
    }
#endif
    return (normal_addr >= base && normal_addr <= limit);
}

/*
 * Set the MINFO that a device is inside of.
 */
static void set_dev_minfo(DEVINFO *dlist, MINFO *mlist)
{
    if (dlist->minfo != NULL) {
	printf(_(1,4,
"Warning: device appears to be owned by multiple memory blocks (%s\n"
"and %s)\n"),
	       mlist->name, dlist->minfo->name);
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
                strcpy(dlist->devname, _(1,5,"(no drv)"));
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

typedef enum {
    memory_conv,
    memory_upper,
    memory_num_types
} memory_t;

char *memory_typename(memory_t type)
{
    switch (type) {
    case memory_conv:	return (_(9,0,"Conventional"));
    case memory_upper:	return (_(9,1,"Upper"));
    default:		return (_(9,2,"(error)"));
    }
}


/*
 * Given an MINFO list, split it into two separate MINFO lists, one for blocks
 * in conventional memory and one for blocks in upper memory.  Returns a
 * pointer to an array of two MINFOs, indexed by elements of memory_t, i.e.
 * memory_conv and memory_upper.
 */
MINFO **split_mlist_conv_upper(MINFO *mlisthead)
{
    MINFO **head;
    MINFO *tail[memory_num_types];
    memory_t memory_type;

    head = xcalloc(memory_num_types, sizeof(MINFO *));

    while (mlisthead != NULL) {
	/*
	 * Work out if this block is in conventional or upper memory.
	 */
	if (mlisthead->seg < last_conv_seg) {
	    memory_type = memory_conv;
	} else {
	    memory_type = memory_upper;
	}

	/*
	 * Duplicate the block and put it on the appropriate list.
	 */
	if (head[memory_type] != NULL) {
	    tail[memory_type]->next = minfo_dup(mlisthead);
	    tail[memory_type] = tail[memory_type]->next;
	} else {
	    head[memory_type] = tail[memory_type] = minfo_dup(mlisthead);
	}

	mlisthead = mlisthead->next;
    }

    /*
     * Make sure both lists are terminated properly and don't contain links
     * from the duplicated nodes.
     */
    for (memory_type = memory_conv; memory_type < memory_num_types;
	 memory_type++) {
	if (head[memory_type] != NULL) {
	    tail[memory_type]->next = NULL;
	}
    }

    return head;
}

/*
 * Flags to be ORed together and passed as 'flags' to filter_mlist().
 */
#define FILTER_MLIST_NO_FLAGS	0x0000

/*
 * If this flag is set, then if an MINFO entry matches the filter, all of the
 * entries below it will be included in the results, otherwise only entries
 * under it that match the filter will be included.
 */
#define FILTER_MLIST_EXPANDED	0x0001

/*
 * If this flag is set, then even if an MINFO entry doesn't match the
 * filter, if one or more of its children does match, then the parent
 * and the matching children are included in the results.
 */
#define FILTER_MLIST_SEARCH_CHILDREN 0x0002


typedef int (*mlist_filter_match)(MINFO *entry, void *data);

MINFO *filter_mlist(MINFO *mlisthead, int flags, mlist_filter_match is_match,
		    void *data)
{
    MINFO *filtered_head = NULL, *filtered_tail, *filtered_children;

    while (mlisthead != NULL) {
	if (flags & FILTER_MLIST_SEARCH_CHILDREN) {
	    filtered_children
		= filter_mlist(mlisthead->first_child, flags,
			       is_match, data);
	} else {
	    filtered_children = NULL;
	}
	/*
	 * filtered_children will be non-NULL if one or more of the children
	 * matches the filter and FILTER_MLIST_SEARCH_CHILDREN was specified,
	 * in which case we need to include the MINFO even if it doesn't
	 * match the filter itself.
	 */
	if (is_match(mlisthead, data) || filtered_children != NULL) {
	    /*
	     * Duplicate the MINFO element and put it on the end of the
	     * filtered list.
	     */
	    if (filtered_head != NULL) {
		filtered_tail->next = minfo_dup(mlisthead);
		filtered_tail = filtered_tail->next;
	    } else {
		filtered_head = filtered_tail = minfo_dup(mlisthead);
	    }
	    if (!is_match(mlisthead, data)) {
		/*
		 * In this case we must have flags &
		 * FILTER_MLIST_SEARCH_CHILDREN.
		 */
		filtered_tail->first_child = filtered_children;
	    } else if ((flags & FILTER_MLIST_EXPANDED) == 0) {
		/* filter the children too */
		filtered_tail->first_child
                    = filter_mlist(filtered_tail->first_child, flags,
				   is_match, data);
	    }
	}
	mlisthead = mlisthead->next;
    }

    if (filtered_head == NULL) {
	/*
	 * No entries match the given module name.
	 */
	return NULL;
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
 * Filter for /MODULE SYSTEM
 */
int filter_system(MINFO *entry, void *data)
{
    return (entry->type == MT_SYSCODE ||
	    entry->type == MT_SYSDATA ||
	    entry->type == MT_KERNEL);
}

/*
 * Filter for /U
 */
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

	    printf("%-38s%3uK", _(2,12,"Available space in High Memory Area"),
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
		    printf(_(2,13,"HMA is available via the XMS driver\n"));
		} else {
		    switch (XMS_HMA_BL(result)) {
		    case XMS_HMA_BL_NOT_IMPL:
			printf(_(2,14,
"HMA is not available via the XMS driver: not implemented by the driver\n"));
			break;
		    case XMS_HMA_BL_VDISK:
			printf(_(2,15,
"HMA is not available via the XMS driver: a VDISK device is present\n"));
			break;
		    case XMS_HMA_BL_NOT_EXIST:
			printf(_(2,16,
"HMA is not available via the XMS driver: HMA does not exist\n"));
			break;
		    case XMS_HMA_BL_IN_USE:
			printf(_(2,17,
"HMA is not available via the XMS driver: HMA already in use\n"));
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
			    if (dbghmamin) {
				printf("HMAMIN binary search: low=%5u "
				       "high=%5u test=%5u\n", low, high, test);
			    }
#endif
			    result = xms_hma_request(test);
			    if (XMS_HMA_AX(result) != XMS_HMA_AX_OK) {
				low = test;
			    } else {
				high = test;
				result = xms_hma_release();
				if (XMS_HMA_AX(result) != XMS_HMA_AX_FAILED) {
				    fatal(_(0,7,
"Fatal error: failed to free HMA, error code %02Xh\n"), XMS_HMA_BL(result));
				}
			    }
			}
			if (high == XMS_HMA_SIZE + 1) {
			    printf(_(2,18,
"HMA is not available via the XMS driver: HMAMIN is larger than HMA\n"));
			} else {
			    printf(_(2,19,
"HMA is available via the XMS driver, minimum TSR size (HMAMIN): %u bytes\n"),
				   high);
			}
			break;
		    default:
			printf(_(2,20,
"HMA is not available via the XMS driver: unknown error %02Xh\n"),
			       XMS_HMA_BL(result));
		    }
		}
	    } else {
		printf(_(2,21,
"HMA is not available as no XMS driver is loaded\n"));
	    }
	}
    }
}

static void int_15_info(void)
{
    ulong result = get_ext_mem_size();

    if (GET_EXT_MEM_SIZE_OK(result)) {
	printf("%-38s%3uK", _(2,22,"Memory accessible using Int 15h"),
	       round_kb(BYTES_PER_KB * GET_EXT_MEM_SIZE_VALUE(result)));
	convert(_(1,3," (%7s bytes)\n"),
		(ulong) GET_EXT_MEM_SIZE_VALUE(result) * BYTES_PER_KB);
    } else {
	printf(_(2,23,"Memory is not accessible using Int 15h (code %02h)\n"),
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

#ifdef DEBUG
    if (dbgcpu) {
	printf("check_ems\n");
    }
#endif
    ems=check_ems();
#ifdef DEBUG
    if (dbgcpu) {
	printf("ems=0x%04X\n",ems);
	printf("check_xms\n");
    }
#endif
    xms=check_xms();
#ifdef DEBUG
    if (dbgcpu) {
	printf("xms=0x%04X\n",xms);
	printf("biosmemory\n");
    }
#endif
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
/*
 * FIXME: this is inconsistent with MS-DOS 6.22 which shows 0K if no
 * UMBs; we appear to show nothing if no UMBs.
 */
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
/* FIXME: assert(level <= MAX_LEVEL);*/
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

    printf("                             %s%-8s%s",
	   pre_indent, entry->devname, post_indent);
    if (entry->minfo->type == MT_KERNEL) {
	printf(_(4,8,"system device driver\n"));
    } else {
	printf(_(4,9,"installed DEVICE=%s\n"), entry->minfo->name);
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

/*
 * Recursively classify items that belong to "SYSTEM".
 */
static void classify_system(MINFO *ml, unsigned *total_conv,
			    unsigned *total_umb)
{
    for (;ml!=NULL;ml=ml->next) {
        if (ml->owner == _psp || ml->type == MT_DOSDATA 
	    || ml->type == MT_RESERVED)
	    /*
	     * We ignore our own block and DOSDATA or RESERVED blocks.
	     */
            ml->classified = 1;
        else if (ml->type == MT_SYSCODE || ml->type == MT_SYSDATA
		 || ml->type == MT_KERNEL || ml->type == MT_IVT
		 || ml->type == MT_BDA || ml->type >= MT_DEVICE) {
            int size = ml->size + 1;
	    /*
	     * Device drivers come off our total as they are already included
	     * in their parent MINFO's size but we don't actually include them
	     * in "SYSTEM".
	     */
	    if (ml->type == MT_DEVICE)
		size = -(size - 1);
	    else
		ml->classified = 1;
            if (ml->seg < last_conv_seg)
                *total_conv += size;
            else
                *total_umb += size;
	    if (ml->first_child != NULL)
		classify_system(ml->first_child, total_conv, total_umb);
        }
    }
}

static void classify_modules(MINFO *ml)
{
    MINFO *ml2;
    unsigned total_conv, total_umb;

    for (;ml!=NULL;ml=ml->next) {
        if (ml->type > MT_FREE && !ml->classified) {
            total_conv = total_umb = 0;
            for (ml2 = ml; ml2 != NULL; ml2 = ml2->next) {
                if (!ml2->classified && ml2->type > MT_FREE &&
                    ml2->owner == ml->owner) {
                    ml2->classified = 1;
                    if (ml2->seg < last_conv_seg)
                        total_conv += ml2->size + 1;
                    else
                        total_umb += ml2->size + 1;
                }
            }
            print_classify_entry(ml->name, total_conv, total_umb);
        }
	if (ml->first_child != NULL)
	    classify_modules(ml->first_child);
    }
}

static void classify_list(unsigned convmemfree, unsigned umbmemfree)
{
    unsigned total_conv, total_umb;

    printf(_(4,0,
"\nModules using memory below 1 MB:\n\n"));
    printf(_(4,1,
  "  Name           Total           Conventional       Upper Memory\n"));
    printf(
  "  --------  ----------------   ----------------   ----------------\n");
    /* figure out code used by "SYSTEM" */
    total_conv = 0; total_umb = 0;
    classify_system(make_mcb_list(NULL), &total_conv, &total_umb);
    print_classify_entry(_(4,2,"SYSTEM"), total_conv, total_umb);
    /* generate output for other modules */
    classify_modules(make_mcb_list(NULL));
    print_classify_entry(_(4,3,"Free"), convmemfree, umbmemfree);
}

typedef void (*print_header_t)(memory_t memory_type);
typedef void (*print_footer_t)(MINFO *mlisthead);
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
    printf(_(4,4,"\nSegment       Total            Name          Type\n"));
    printf(        "-------  ----------------  ------------  -------------\n");
}

static void print_free_header(void)
{
    printf(_(4,6,"\nSegment       Total\n"));
    printf(        "-------  ----------------\n");
}

static void print_full_footer(ulong total)
{
    printf(        "         ----------------\n");
    printf(_(4,7,  "Total:"));
    print_classify_value("%11s", total);
    printf("\n");
}

/*
 * Similar to /DEBUG, but doesn't show devices, environment blocks,
 * data blocks, and some other things.
 */
/*
 * FIXME: give complete list of what is skipped in this comment and in
 * the user documentation.
 */
/* FIXME: Can this be renamed from UPPER to U<something else>?  It's
 * not really got anything to do with upper memory.  Currently the
 * user can only access this option via /U and not via /UPPER in
 * anticipation of finding a better name.
 */
static void upper_list(void)
{
    MINFO *mlisthead = make_mcb_list(NULL);

    /* ignore the return value, as we get the DEVINFO entries from mlisthead */
    (void) make_dev_list(mlisthead);

    /*
     * Filter the list to just the entries with the given name.
     */
    mlisthead = filter_mlist(mlisthead, FILTER_MLIST_NO_FLAGS,
			     filter_upper, NULL);

    print_full_header();

    generic_list(mlisthead, 0, print_minfo_full, NULL);
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

/*
 * For each memory type, display a header, use generic_list to display
 * that part of the split list, then display a footer.  The header and
 * footer are both optional.  The function used to print the header is
 * passed the memory type (conventional or upper) and the function
 * used to print the footer is passed the MINFO list so it can
 * calculate a memory total for display in the footer.  A blank line
 * is inserted between memory types.  If print_empty is supplied, it
 * is called when one of the split lists is empty instead of any of
 * the other functions.
 */
static void generic_split_list(MINFO **split,
			       print_header_t print_header,
			       print_footer_t print_footer,
			       print_header_t print_empty,
			       print_minfo_t print_minfo,
			       print_devinfo_t print_devinfo)
{
    memory_t memory_type;

    for (memory_type = memory_conv; memory_type < memory_num_types;
	 memory_type++) {
	if (memory_type > memory_conv) { /* if not the first type */
	    printf("\n");
	}
	if (split[memory_type] == NULL && print_empty != NULL) {
	    print_empty(memory_type);
	} else {
	    if (print_header != NULL) {
		print_header(memory_type);
	    }
	    generic_list(split[memory_type], 0,
			 print_minfo,
			 print_devinfo);
	    if (print_footer != NULL) {
		print_footer(split[memory_type]);
	    }
	}
    }
}

static void print_full_header_with_type(memory_t memory_type)
{
    printf(_(4, 10, "%s Memory Detail:\n"), memory_typename(memory_type));
    print_full_header();
}

static void print_full_empty(memory_t memory_type)
{
    printf(_(1, 9, "%s Memory is not accessible\n"), memory_typename(memory_type));
}

static void debug_list(void)
{
    MINFO *ml;

    ml = make_mcb_list(NULL);
    /* ignore the return value, as we get the DEVINFO entries from ml */
    (void) make_dev_list(ml);

    generic_split_list(split_mlist_conv_upper(ml),
		       print_full_header_with_type, NULL, print_full_empty,
		       print_minfo_full, print_devinfo_full);
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

    if (strcmpi(module_name, "SYSTEM") == 0) {
	/*
	 * Filter the lis to entries related to the OS which we would
	 * show as "IO" or "DOS" and show under "SYSTEM" in /CLASSIFY.
	 */
	mlisthead = filter_mlist(mlisthead,
				 FILTER_MLIST_EXPANDED |
				 FILTER_MLIST_SEARCH_CHILDREN,
				 filter_system, NULL);
    } else {
	/*
	 * Filter the list to just the entries with the given name.
	 */
	mlisthead = filter_mlist(mlisthead,
				 FILTER_MLIST_EXPANDED |
				 FILTER_MLIST_SEARCH_CHILDREN,
				 filter_by_module_name, module_name);
    }

    if (mlisthead == NULL) {
	printf(_(1,7,"%s is not currently in memory.\n"), module_name);
	exit(2);
    }

    printf(_(1,8,"%s is using the following memory:\n"), module_name);
    print_full_header();

    generic_list(mlisthead, 0, print_minfo_full, print_devinfo_full);

    print_full_footer(total_mem(mlisthead));

/*
 * FIXME: use split list (conventional/upper) for /MODULE?  We should
 * probably include a grand total as well as the conventional total
 * and upper total if we do split it.
 */
}

static void print_free_header_with_type(memory_t memory_type)
{
    printf(_(4, 11, "Free %s Memory:\n"), memory_typename(memory_type));
    print_free_header();
}

static void print_free_empty(memory_t memory_type)
{
    printf(_(1, 6, "No %s Memory is free\n"), memory_typename(memory_type));
}

static void print_free_footer(MINFO *mlisthead)
{
    print_full_footer(total_mem(mlisthead));
}

void free_list(void)
{
    MINFO *mlisthead = make_mcb_list(NULL);

    /* ignore the return value, as we get the DEVINFO entries from ml */
    (void) make_dev_list(mlisthead);

    /*
     * Filter the list to just the entries with the given name.  Whether or not
     * we pass FILTER_MLIST_EXPANDED is irrelevant as free blocks don't have
     * sub-blocks.
     */
    mlisthead = filter_mlist(mlisthead, FILTER_MLIST_NO_FLAGS,
			     filter_free, NULL);

    /*
     * Pass NULL for print_devinfo as we won't be printing any devices
     * - a free block won't have any devices linked off it.
     */
    generic_split_list(split_mlist_conv_upper(mlisthead),
		       print_free_header_with_type,
		       print_free_footer, print_free_empty,
		       print_minfo_free, NULL);
}

/*
 * FIXME: add support for showing a process listing/tree using this function.
 */
#if 0
void ps_list_from(MINFO *mlisthead, MINFO *root, unsigned int level)
{
    MINFO *mlist;
    char pre_indent[MAX_INDENT_SIZE + 1 + 99/*FIXME*/];
    char post_indent[MAX_INDENT_SIZE + 1 + 99/*FIXME*/];

    indent_setup(pre_indent, post_indent, level);
    for (mlist = mlisthead; mlist != NULL; mlist = mlist->next) {
	if (mlist->type == MT_PROGRAM && mlist->caller == root) {
	    printf("%s%s\n", pre_indent, mlist->name);
	    ps_list_from(mlisthead, mlist, level + 1);
	}
    }
}

void ps_list(void)
{
    MINFO *mlisthead = make_mcb_list(NULL);

    ps_list_from(mlisthead, NULL, 0);
}
#endif

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

int is_space_char(char c)
{
    /*
     * Don't need to actually list ' ' since the C runtime takes care of
     * splitting up argv[] based on spaces.
     */
    return (c == ':');
}

#define NO_SWITCH_CHAR '\0'

#define F_HELP       0x00000001UL
#define F_DEVICE     0x00000002UL
#define F_EMS	     0x00000004UL
#define F_FULL	     0x00000008UL
#define F_UPPER	     0x00000010UL
#define F_XMS	     0x00000020UL
#define F_PAGE	     0x00000040UL
#define F_CLASSIFY   0x00000080UL
#define F_DEBUG	     0x00000100UL
#define F_MODULE     0x00000200UL
#define F_FREE	     0x00000400UL
#define F_ALL	     0x00000800UL
#define F_NOSUMMARY  0x00001000UL
#define F_SUMMARY    0x00002000UL
#define F_DBGDEVADDR 0x00004000UL
#define F_DBGHMAMIN  0x00008000UL
#define F_OLD	     0x00010000UL
#define F_F          0x00020000UL
#define F_D          0x00040000UL
#define F_DBGCPU     0x00080000UL

typedef unsigned long opt_flag_t;

/*
 * Mask for values of opt_flag_t which cause additional output (on top of the
 * normal summary) to be produced.  If the user specifies /NOSUMMARY without
 * one of these flags it is considered an error because absolutley no output
 * would be generated.
 */
#define OPT_FLAG_MASK_OUTPUT (F_HELP | F_DEVICE | F_EMS | F_FULL |	\
			      F_UPPER | F_XMS | F_CLASSIFY | F_DEBUG |	\
			      F_MODULE | F_FREE)

typedef struct {
    char *s;
    opt_flag_t flag;
    char **value;
} opt_t;

typedef enum {
    GET_OPTS_STATE_SWITCH,
    GET_OPTS_STATE_VALUE
} get_opts_state_t;

/*
 * Each element of argv[] may contain one or more substrings.  Each of
 * these substrings may be an option, which starts with a character
 * for which is_switch_char() returns TRUE, a value which corresponds
 * to the option before it, or a sequence of whitespace-equivalent
 * characters for which is_space_char() returns TRUE.
 *
 * The beginning of an argv[] element is always the beginning of a
 * substring and the end of an argv[] element is always the end of a
 * substring.  A substring ends before a switch character, so an
 * argv[] element of "/FOO/BAR" would be parsed as two substrings
 * "/FOO" and "/BAR".  A substring consisting of whitespace-equivalent
 * characters serves only to split two substrings, so "/FOO:BAR" or
 * "/FOO::BAR", etc. would be parsed as substrings "/FOO", a sequence
 * of colons which are ignored, and "BAR".
 *
 * If a substring begins with a switch character, it is interpreted as
 * an option.  The part of the substring after the leading switch
 * character is compared to the 's' member of each entry in the array
 * 'opts'.  If the substring is a complete case-insensitive match for
 * exactly one 's' member then the corresponding 'flag' will be set in
 * the return value of the function.  If it is an exact match for more
 * than one 's' member then an error message is shown; you should not
 * set up 'opts' such that there are multiple array members with the
 * same 's' value.
 *
 * If the substring isn't an exact match for any 's' member, the
 * following holds: If the substring is an initial substring, i.e. a
 * prefix, of exactly one 's' member, e.g. the substring after removal
 * of the switch characer is "F" and an 's' member is equal to "FOO",
 * then the corresponding 'flag' will be set in the return value of
 * the function.  If it is a prefix for more than one 's' member,
 * e.g. the substring is "B" and it is a substring of both "BAR" and
 * "BAZ", then the user will be shown an error message indicating that
 * the option they specified is ambiguous and matches two or more
 * possible options.  The user needs to specify more characters of the
 * option name to avoid this.
 *
 * If the substring is not a prefix of any 's' member, then the
 * following holds: if the value of 's' is a prefix of the substring,
 * e.g. the substring is "FOOBAR" and 's' is "FOO", AND the
 * corresponding 'value' is not NULL (i.e. 's' is the text of an
 * option that requires a value), then the corresponding 'flag' will
 * be set in the return value of the function and the part of the
 * substring following 's' in the substring will be treated as a
 * separate substring for the purposes of setting 'value' as described
 * below.  If the multiple values of 's' are prefixes of the
 * substring, then we ignore all but the one with the longest string
 * length, e.g. if 's' values of "F" and "FOO" are seen, then if the
 * substring is "FOOBAR", we will ignore the fact that "F" is a prefix
 * of "FOOBAR" and act as if "FOO" was the only option that was a
 * prefix of "FOOBAR".  If there is more than one value of 's' that is
 * a prefix of the substring with the same length, this indicates that
 * 'opts' contains duplicate options and an error will be shown.
 *
 * If the substring is not an exact match for any 's' member, is not a
 * prefix of any 's' member, and does not have a value of 's' for an
 * option that takes a value as its prefix, then the user is given an
 * error message indicating that they specified an unrecognized
 * option.
 *
 * If the return value of the function is being set based on the
 * 'flag' member for an array member, then if the 'value' member for
 * the option is not NULL this indicates that the option requires a
 * value.  The pointer that 'value' points to will be set to point to
 * the substring.
 */
opt_flag_t get_opts (char *argv[], opt_t *opts, int opt_count)
{
    int arg_index = 1, opt_index, matched_index;
    int partial_match_count, exact_match_count;
    int prefix_match_len, new_prefix_match_len;
    char *char_ptr = argv[arg_index];
    char *switch_start, *switch_text_start, old_switch_char;
    opt_flag_t flags = 0;
    get_opts_state_t state = GET_OPTS_STATE_SWITCH;

    while (char_ptr != NULL && (flags & F_HELP)==0) {
	/*
	 * If state == GET_OPTS_STATE_SWITCH, we're at the start of a
	 * switch, which might be the start of argv[i] or somewhere
	 * inside it.  Otherwise we're at the start of a value for a
	 * switch, which is always at the start of argv[i].  */
	switch_start = char_ptr;

	/*
	 * A switch should start with a switch character.
	 */
	if (state == GET_OPTS_STATE_SWITCH) {
	    if (!is_switch_char(*char_ptr)) {
		fatal(_(0,8,"unknown option (expected a '/'): %s\n%s"),
		      switch_start, _(0,4,"Use /? for help\n"));
	    }
	    char_ptr++;
	} else { /* state == GET_OPTS_STATE_VALUE */
	    if (is_switch_char(*char_ptr)) {
		fatal(_(0,9,
"Expected a value after /%s, not another switch\n%s"), opts[matched_index].s,
		      _(0,4,"Use /? for help\n"));
	    }
	}

	/*
	 * Now get the text of the switch or value - scan for the end of the
	 * switch.
	 */
	switch_text_start = char_ptr;
	while (*char_ptr != '\0'
	       && !is_switch_char(*char_ptr)
	       && !is_space_char(*char_ptr)) {
	    char_ptr++;
	}

	/*
	 * If we're not at the end of the current argv[] element, we
	 * need to put a '\0' character at char_ptr so that string
	 * operations don't see the following argument.
	 */
	if (is_switch_char(*char_ptr) || is_space_char(*char_ptr)) {
	    old_switch_char = *char_ptr;
	    *char_ptr = '\0';
	} else {
	    old_switch_char = NO_SWITCH_CHAR;
	}

	prefix_match_len = 0;
	exact_match_count = 0;
	partial_match_count = 0;
	if (state == GET_OPTS_STATE_SWITCH) {
	    if (strlen(switch_text_start) == 0) {
		fatal(_(0, 14,
"Invalid option '%s': you must specify at least one letter of the\n"
"option name"), switch_start);
	    }
	    strupr(switch_text_start);
#ifdef DEBUG_PARSER
	    printf("%s: argument is [%s]\n", __FUNCTION__, switch_text_start);
#endif

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
#ifdef DEBUG_PARSER
			printf("%s: argument is exact match (number %u) for "
			       "[%s]\n", __FUNCTION__, exact_match_count,
			       opts[opt_index].s);
#endif
		    } else {
			partial_match_count++;
			if (exact_match_count == 0) {
			    matched_index = opt_index;
			}
#ifdef DEBUG_PARSER
			printf("%s: argument is partial match (number %u) for "
			       "[%s]\n", __FUNCTION__, partial_match_count,
			       opts[opt_index].s);
#endif
		    }
		} else if (strstr(switch_text_start, opts[opt_index].s)
			   == switch_text_start
			   && opts[opt_index].value != NULL) {
		    /*
		     * opts[opt_index].s does not begin with
		     * switch_text_start (nor are they equal), but
		     * switch_text_start begins with opts[opt_index].s
		     * and opts[opt_index] is an option that takes a
		     * value.  Note that we only set matched_index
		     * (i.e. treat this as actually being the option
		     * the user wanted to select) if (a) we've not
		     * seen an exact match [yet], (b) we've not seen a
		     * partial match [yet] and (c) we've not seen a
		     * longer match [yet].  If we see any of those
		     * three later, we will overwrite matched_index.
		     */
#ifdef DEBUG_PARSER
		    printf("%s: argument begins with [%s]\n", __FUNCTION__,
			   opts[opt_index].s);
#endif
		    if (exact_match_count == 0 && partial_match_count == 0) {
			new_prefix_match_len = strlen(opts[opt_index].s);
			if (new_prefix_match_len > prefix_match_len) {
			    prefix_match_len = new_prefix_match_len;
			    matched_index = opt_index;
			} else if (new_prefix_match_len == prefix_match_len) {
			    /*
			     * If there is another opts[].s which is the same
			     * length which is also a prefix of
			     * switch_text_start, we must have duplicate
			     * options.
			     */
			    fatal(_(0,10,"Internal error: option '%s' has "
				    "'%s' as a prefix\nplus another equal-"
				    "length prefix"),
				  switch_start, opts[opt_index].s);
			}
		    }
		}
	    }

	    /*
	     * Now figure out if we had ambiguity or no match at all.
	     * The normal cases are exact_match_count == 1 (in which
	     * case we don't care if there were any partial matches or
	     * prefixes) or exact_match_count == 0 &&
	     * partial_match_count == 1 (i.e. the user's input was a
	     * partial match for only one switch) or exact_match_count
	     * == 0 && partial_match_count == 0 && prefix_match_len !=
	     * 0 (i.e. the user's input had a switch as its prefix).
	     */
	    if (exact_match_count > 1) {
		/*
		 * This is a bug - there shouldn't be two options with the
		 * same text in opts[]!
		 */
		fatal(_(0,11,
"Internal error: option '%s' was an exact match for two\n"
"different switches\n"), switch_start);
	    } else if (exact_match_count == 0) {
		if (partial_match_count > 1) {
		    fatal(_(0,12,
"Error: option '%s' is ambiguous - it is a partial match for two\n"
"or more different options\n%s"), switch_start, _(0,4,"Use /? for help\n"));
		}
		if (partial_match_count == 0 && prefix_match_len == 0) {
		    fatal(_(0,5,"unknown option: %s\n%s"),
			  switch_start, _(0,4,"Use /? for help\n"));
		}
	    }
	    /*
	     * We had (a) only one exact match, (b) no exact match but
	     * one partial match, or (c) no exact match, no partial
	     * match, but one or more prefix matches (of which we
	     * would have ignored all but the longest).  In any
	     * case, matched_index will be set.
	     */
#ifdef DEBUG_PARSER
	    printf("%s: resolved to: [%s]\n", __FUNCTION__,
		   opts[matched_index].s);
#endif
	    flags |= opts[matched_index].flag;

	    /*
	     * If value is not NULL, then on the next iteration through this
	     * loop we need to get a value for the switch.
	     */
	    if (opts[matched_index].value != NULL) {
#ifdef DEBUG_PARSER
		printf("%s: expecting a value for this switch\n",
		       __FUNCTION__);
#endif
		state = GET_OPTS_STATE_VALUE;
	    }
	} else { /* state == GET_OPTS_STATE_VALUE */
#ifdef DEBUG_PARSER
	    printf("%s: got switch value [%s]\n", __FUNCTION__, switch_start);
#endif
	    *(opts[matched_index].value) = xstrdup(switch_start);
	    state = GET_OPTS_STATE_SWITCH;
	}

	if (old_switch_char != NO_SWITCH_CHAR) {
	    /*
	     * *char_ptr was originally a switch or space character,
	     * so restore it and continue through the loop to process
	     * the next switch in this argv[] element.
	     */
	    *char_ptr = old_switch_char;
	}

	if (exact_match_count == 0 && partial_match_count == 0
	    && prefix_match_len != 0) {
	    /*
	     * We just processed an option which takes a value, and
	     * the option and value are concatenated.  char_ptr points
	     * to the first character after the value, but we need to
	     * make it point to the first character after the option
	     * (i.e. the first character of the value) so that we can
	     * parse the value in the next loop iteration.  Note that
	     * the following code which handles trailing
	     * whitespace-like characters and the end of the argv[]
	     * element will do nothing since char_ptr will not be
	     * pointing to one of those types of characters.  */
	    char_ptr = switch_text_start + prefix_match_len;
	}

	/*
	 * Skip over any whitespace-like characters that came after the flag.
	 * If we're already at the end of the string,
	 * is_space_char(*char_ptr) will return FALSE.
	 */
	while (is_space_char(*char_ptr)) {
	    char_ptr++;
	}
	/*
	 * If we're at the end of the argv[] element, possibly after skipping
	 * over whitespace-like characters, move to next argv[] element.
	 */
	if (*char_ptr == '\0') {
            arg_index++;
            char_ptr = argv[arg_index];
        }
    }

    /*
     * If we are still in this state, it means we failed to get the required
     * value.
     */
    if (state == GET_OPTS_STATE_VALUE) {
	fatal(_(0,13,"Expected a value after /%s\n%s"),
	      opts[matched_index].s, _(0,4,"Use /? for help\n"));
    }

    return (flags);
}

/*
 * Return the help string for the given command-line flag/option.  If
 * there is no help string for the option, NULL is returned.
 */
char *help_for_flag(opt_flag_t opt)
{
    switch (opt) {
    case F_HELP:       return (_(7, 10,
"/?           Displays this help message"));
    case F_DEVICE:     return (_(7, 6,
"/DEV[ICE]    List of device drivers currently in memory"));
    case F_EMS:        return (_(7, 3,
"/E[MS]       Reports all information about Expanded Memory"));
    case F_FULL:       return (_(7, 4,
"/FU[LL]      Full list of memory blocks"));
    case F_UPPER:      return (_(7, 7,
"/U           List of programs in conventional and upper memory"));
    case F_XMS:        return (_(7, 8,
"/X[MS]       Reports all information about Extended Memory"));
    case F_PAGE:       return (_(7, 9,
"/P[AGE]      Pauses after each screenful of information"));
    case F_CLASSIFY:   return (_(7, 5,
"/C[LASSIFY]  Classify modules using memory below 1 MB"));
    case F_DEBUG:      return (_(7, 11,
"/DEB[UG]     Show programs and devices in conventional and upper memory"));
    case F_MODULE:     return (_(7, 12,
"/M <name> | /MODULE <name>\n"
"             Show memory used by the given program or driver"));
    case F_FREE:       return (_(7, 13,
"/FR[EE]      Show free conventional and upper memory blocks"));
    case F_ALL:        return (_(7, 14,
"/A[LL]       Show all details of high memory area (HMA)"));
    case F_NOSUMMARY:  return (_(7, 15,
"/N[OSUMMARY] Do not show the summary normally displayed when no other\n"
"             options are specified"));
    case F_SUMMARY:    return (_(7, 16,
"/S[UMMARY]   Negates the /NOSUMMARY option"));
    case F_OLD:        return (_(7, 18,
"/O[LD]       Compatability with FreeDOS MEM 1.7 beta"));
    case F_D:          return (_(7, 19,
"/D           Same as /DEBUG by default, same as /DEVICE if /OLD used"));
    case F_F:          return (_(7, 20,
"/F           Same as /FREE by default, same as /FULL if /OLD used"));
    default:           return (NULL);
    }
}

void show_help(opt_t *opts, int opt_count)
{
    int opt_index;
    opt_flag_t displayed_flags = 0;
    opt_flag_t flag;
    char *help_str;

    printf(_(7, 0, "FreeDOS MEM version %d.%d%s"),
	   MEM_MAJOR, MEM_MINOR, MEM_VER_SUFFIX);
#ifdef DEBUG
    printf(" DEBUG");
#endif
    printf(" [%s %s", __DATE__, __TIME__);
#ifdef __WATCOMC__
    printf(" Watcom C %u.%u", __WATCOMC__ / 100, __WATCOMC__ % 100);
#endif
#ifdef __TURBOC__
    printf(" Turbo C 0x%04X", __TURBOC__);
#endif
    printf("]\n");

    printf("%s\n\n%s\n",
	   _(7, 1, "Displays the amount of used and free memory in your system."),
	   _(7, 2, "Syntax: MEM [zero or more of the options shown below]"));
    
    for (opt_index = 0; opt_index < opt_count; opt_index++) {
	flag = opts[opt_index].flag;
	if (displayed_flags & flag) {
	    /*
	     * We've already displayed the provided help string for
	     * this flag so don't re-display it.
	     */
	    continue;
	}
	help_str = help_for_flag(flag);
	if (help_str != NULL) {
	    printf("%s\n", help_str);
	    /*
	     * Make sure we don't display this string again - the
	     * string returned by help_for_flag() should have covered
	     * both options.
	     */
	    displayed_flags |= flag;
	} else {
	    printf(_(7, 17, "/%-10s No help is available for this option\n"),
		   opts[opt_index].s);
	    /*
	     * If there is another option with the same flag, we'll
	     * need to display that one too.
	     */
	}
    }

    exit(1);
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
      { "D",		F_D,		NULL },
#ifdef DEBUG
      { "DBGCPU",       F_DBGCPU,       NULL },
      { "DBGDEVADDR",   F_DBGDEVADDR,   NULL },
      { "DBGHMAMIN",    F_DBGHMAMIN,    NULL },
#endif
      { "DEBUG",	F_DEBUG,	NULL },
      { "DEVICE",	F_DEVICE,	NULL },
      { "EMS",		F_EMS,		NULL },
      { "F",		F_F,		NULL },
      { "FREE",		F_FREE,		NULL },
      { "FULL",		F_FULL,		NULL },
      { "HELP",		F_HELP,		NULL },
      /*
       * Specify both /M and /MODULE in this array so the parser handles
       * /M<module_name>, i.e. the option without a separator before
       * the module name.
       */
      { "M",		F_MODULE,	&module_name },
      { "MODULE",	F_MODULE,	&module_name },
      { "NOSUMMARY",	F_NOSUMMARY,	NULL },
      { "OLD",		F_OLD,		NULL },
      { "PAGE",		F_PAGE,		NULL },
      { "SUMMARY",      F_SUMMARY,      NULL },
      { "U",		F_UPPER,	NULL },
      { "XMS",		F_XMS,		NULL }
    };

    setup_globals();

    /* avoid unused argument warning? */
    argc = argc;

    kittenopen("mem");

    flags = get_opts(argv, &opts, sizeof(opts) / sizeof(opts[0]));

#ifdef DEBUG
    /*
     * Set up global boolean flags used to enable debugging based on
     * the command-line options.
     */
    dbgcpu = ((flags & F_DBGCPU) == F_DBGCPU);
    dbgdevaddr = ((flags & F_DBGDEVADDR) == F_DBGDEVADDR);
    dbghmamin = ((flags & F_DBGHMAMIN) == F_DBGHMAMIN);
#endif

    /*
     * The /OLD flag selects the old meanings of /D and /F vs. the (new)
     * MS-DOS-like meanings:
     * /D => /OLD ? /DEVICE : /DEBUG
     * /F => /OLD ? /FULL : /FREE */
    if (flags & F_D) {
	if (flags & F_OLD) {
	    flags |= F_DEVICE;
	} else {
	    flags |= F_DEBUG;
	}
    }
    if (flags & F_F) {
	if (flags & F_OLD) {
	    flags |= F_FULL;
	} else {
	    flags |= F_FREE;
	}
    }

    /*
     * /SUMMARY cancels /NOSUMMARY and has a higher precedence.
     */
    if ((flags & F_SUMMARY) && (flags & F_NOSUMMARY)) {
	flags &= ~(F_SUMMARY | F_NOSUMMARY);
    }

    /*
     * If the user specified /NOSUMMARY but not one of the flags in
     * OPT_FLAG_MASK_OUTPUT then no output would be produced.
     */
    if ((flags & F_NOSUMMARY) && !(flags & OPT_FLAG_MASK_OUTPUT)) {
	fatal(_(0,6,
"The /NOSUMMARY option was specified, but no other output-producing options\n"
"were specified, so no output is being produced.\n%s"),
	      _(0,4,"Use /? for help\n"));
    }

    /*
     * /? or /HELP takes precedence over all other options and they
     * are ignored if it is specified.
     */
    if (flags & F_HELP)
      {
	show_help(&opts, sizeof(opts) / sizeof(opts[0]));
      }

    upper=check_upper(make_mcb_list(&memfree));

    if (flags & F_PAGE)   num_lines=get_font_info();

/*
 * FIXME: In MS-DOS, when /MODULE or /FREE are specified, nothing else
 * is shown.  We need to make sure no other flags are specified!  Or
 * should we allow those options plus something else?
 */
/*
 * FIXME: Perhaps when /MODULE is specified, the /DEBUG flag should
 * determine whether subtrees of devices are included?
 */
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

/*
 * FIXME: should the sizes shown to the user include sizeof(MCB)?  it
 * seems that we are not consistent in including the size of the MCB
 * between /CLASSIFY and /FULL; in MS-DOS we don't have inconsistency
 * between /C and /F but don't know what they DO do.
 */
