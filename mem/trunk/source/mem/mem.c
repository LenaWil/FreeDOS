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
*/

#pragma pack(1)   /*  Be sure to compile with word alignment OFF !!! */
#ifdef __WATCOMC__
#define PRAGMAS
#define getvect _dos_getvect
#define outportb outp
#define inportb inp
#define biosmemory _bios_memsize
#endif
#ifdef __TURBOC__
#pragma -a-
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

typedef unsigned char BYTE;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;

#define FALSE 0
#define TRUE  1

#define MT_NONE    0
#define MT_SYSCODE 1
#define MT_SYSDATA 2
#define MT_PROGRAM 3
#define MT_DEVICE  4
#define MT_ENV     5
#define MT_DATA    6
#define MT_FREE    7
#define MT_MAP     8

int num_lines = -1;

typedef struct device_header
{
    struct device_header far*next;
    ushort attr;
    ushort strategy_rutin;
    ushort interrupt_rutin;
    char name[8];
} DEVICE_HEADER;

typedef struct devinfo
{
    struct device_header far *addr;
    char devname[9];
    char *progname;
    ushort attr;
    uchar firstdrive, lastdrive;
    struct devinfo *next;
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

typedef struct minfo
{
    uchar type;
    ushort seg;
    ushort owner;
    ushort environment;
    char *name;
    ushort size;
#if 0 /* presently unused */
    uchar vecnum;
    uchar *vectors;
#endif
    struct minfo *next;
} MINFO;

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

#ifdef PRAGMAS

ulong get_dos_version(void);
#pragma aux get_dos_version = \
    "mov ax, 0x3000" \
    "int 0x21" \
value [bx ax]

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

#else

/* Get DOS version and OEM number. */
static ulong get_dos_version(void)
{
    union REGS regs;
    regs.x.ax = 0x3000;
    intdos(&regs, &regs);
    return ((ulong)regs.x.bx << 16) | regs.x.ax;
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

static char malloc_buffer[30000];
static size_t mbuffer_idx;

/* dumb malloc replacement without possibility to free. that's
   enough for us here */
void *malloc(size_t size)
{
    char *ret = &malloc_buffer[mbuffer_idx];
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
        fatal ("Out of memory.\n");
    return value;
}

#define xcalloc(nitems,size) (xmalloc((nitems) * (size)))

static unsigned long round_kb(unsigned long bytes)
{
    return ((bytes + 512) / 1024L);
}

static unsigned round_seg_kb(unsigned para)
{
    return ((para + 32) / 64);
}

static void convert(char *format, ulong num)
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
    BYTE OEMNumber, DOSMajor30;

    ulong dosver = get_dos_version();
    OEMNumber = dosver >> 24;
    DOSMajor30 = dosver & 0xff;

    if (DOSMajor30 == 0x00)
        DOSMajor30 = 0x01;              /* DOS 1.x. */

    switch (OEMNumber)
        {
	case 0xFD:
            return "FreeDOS";
        case 0xFF:
	    if (DOSMajor30 <= 6)
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
            return "An unknown operating system";

        }
}

static EMSINFO *ems_error(void)
{
    printf("EMS INTERNAL ERROR.\n");
    return NULL;
}

#define ems_frame() call_ems_driver_bx(0x41)
#define ems_size() call_ems_driver_dx(0x42)
#define ems_free() call_ems_driver_bx(0x42)
#define ems_version() call_ems_driver(0x46)
#define ems_num_handles() call_ems_driver_bx(0x4b)

static EMSINFO *check_ems(void)
{
    char far *int67;
    static EMSINFO *ems_static_pointer = NULL;
    static EMSINFO ems_static;
    EMSINFO *ems = ems_static_pointer;
    char *emsname = "EMMXXXX0";
    unsigned len;
    long tmp;
    
    if (ems != NULL)
        return ems;
    
    int67=(char far *)getvect(0x67);
    if (int67 == NULL)
        return ems;

    int67 = MK_FP(FP_SEG(int67),10);
    for (len = 8; len; emsname++,int67++,len--)
	if (*emsname != *int67)
            return ems;
            
    ems = ems_static_pointer = &ems_static;
    if ((tmp = ems_frame()) < 0)
        return ems_error();
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
            return ems_error();
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

    if (xms_available() != 0x80)
        return xms;
    xms_drv = get_xms_drv();
    xms = xms_static_pointer = &xms_static;

    total = xms_version();
    xms->verminor=total & 0xff;
    xms->vermajor=(total >> 8) & 0xff;
    xms->drv_vermajor=total >> 24;
    xms->drv_verminor=(total >> 16) & 0xff;
    xms->hma=xms_hma();

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
        if (xms->vermajor >= 3) {
            xms->largest=xms_extlargest();
            xms->free=xms_exttotalfree();
        }
    }
    if (!xms->largest)
        xms->largest=xms_largest();
    if (!xms->free)
        xms->free=xms_totalfree();
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
            fatal("SYSTEM MEMORY TRASHED!\n");

	}

    upper = xcalloc(1, sizeof(UPPERINFO));
    lastconseg = biosmemory()*64 - 1;
    while (mlist!=NULL && mlist->seg != lastconseg)
        mlist=mlist->next;
    
    if (mlist==NULL)
        fatal("MEM: UMB Corruption.\n");
    
    mlist=mlist->next;
    while (mlist!=NULL) {
        if (mlist->type == MT_FREE)
        {
	    upper->free+=mlist->size+1;
            if (mlist->size > upper->largest)
                upper->largest=mlist->size;

	}
        upper->total += mlist->size + 1;
        mlist=mlist->next;
    }
    return(upper);
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
        *dest++ = (ch <= ' ' ? ' ' : toupper(ch));
    }
}

static MINFO *search_sd(MINFO *mlist)
{
    ushort begin, end, i;
    SD_HEADER far *sd;
    static struct {char c; char *s;} sdtype[] = 
    {
      { 'F', " FILES" },
      { 'X', " FCBS" },
      { 'C', " BUFFERS" },
      { 'B', " BUFFERS" },
      { 'L', " LASTDRV" },
      { 'S', " STACKS" }
    };

    begin=mlist->seg + 1;
    end=mlist->seg + mlist->size;
    sd=MK_FP(begin, 0);
    while ((FP_SEG(sd) >= begin) && (FP_SEG(sd) < end))
        {
        mlist->next = xcalloc(1, sizeof(MINFO));
	mlist = mlist->next;
        mlist->seg=sd->start;
        mlist->size=sd->size;
        mlist->type=MT_DATA;
        if (sd->type == 'D' || sd->type == 'I')
            {
            mlist->name = xmalloc(10);
            mlist->name[0]=' ';
            check_name(&mlist->name[1], sd->name, 8);
            mlist->type=MT_DEVICE;
            break;
            }
        else
            {
            mlist->name = " ??????";
            for (i = 0; i< sizeof(sdtype)/sizeof(sdtype[0]); i++)
                {
                if (sdtype[i].c == sd->type)
                    {
                    mlist->name = sdtype[i].s;
                    break;
                    }
                }
            }
	sd=MK_FP(sd->start + sd->size, 0);
        }
    return(mlist);
}

static void register_dos_mcb(MINFO *mlist)
{
    if (!mlist->owner) {
        mlist->name="";
        mlist->type=MT_FREE;
    } else if (mlist->owner <= 0x0008) {
        MCB far *mcb = MK_FP(mlist->seg, 0);
        mlist->name = "DOS";
        if (mcb->name[0]=='S' && mcb->name[1]=='D') {
            mlist->type=MT_SYSDATA;
            mlist=search_sd(mlist);
        } else {
            mlist->type=MT_SYSCODE;
        }
    }
}

static void program_mcb(MINFO *mlist)
{
    MCB far *mcb;

    mlist->name = xmalloc(9);
    mcb = MK_FP(mlist->seg, 0);
    check_name(mlist->name, mcb->name, 8);
    mlist->environment=env_seg(mlist->seg);
    mlist->type=MT_PROGRAM;
}

static MINFO *register_mcb(MINFO *mlist)
{
    MCB far *mcb = MK_FP(mlist->seg, 0);
    
    mlist->owner=mcb->owner;
    mlist->size=mcb->size;
    register_dos_mcb(mlist);
    if (mlist->seg <= biosmemory()*64 - 1)
        {
        if (is_psp(mlist->seg))
            program_mcb(mlist);
        }
    else
        {
        if (mcb->owner > 0x0008)
            program_mcb(mlist);
        }
    return mlist;
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

    mlistroot = mlist = xcalloc(1, sizeof(MINFO));

    /* In LL in offset -02 there's pointer to first mem block (segment only). */
    cur_mcb=MK_FP(*(ushort far *)(dos_list_of_lists()-2), 0);

    while(cur_mcb->type == 'M')
    {
        mlist->seg = FP_SEG(cur_mcb);
        mlist=register_mcb(mlist);
        cur_mcb = MK_FP(FP_SEG(cur_mcb) + cur_mcb->size + 1, FP_OFF(cur_mcb));
        mlist->next=xcalloc(1, sizeof(MINFO));
        mlist=mlist->next;
    }
    if (cur_mcb->type != 'Z')
        fatal("MEM: The MCB chain is corrupted.\n");
    mlist->seg = FP_SEG(cur_mcb);
    register_mcb(mlist);
    set_upperlink(origlink);

    for (mlist=mlistroot; mlist!=NULL; mlist=mlist->next) {
        MINFO *mlistj;
        
        if (mlist->type == MT_PROGRAM) {
            /* now find MEM himself and environment and mark this as MT_FREE */
            if (mlist->seg+1 == _psp || mlist->owner == _psp) {
                /* thats MEM himself */
                mlist->type = MT_FREE;
                mlist->name = "";
            }
            for(mlistj=mlistroot;mlistj!=NULL;mlistj=mlistj->next) {
                if ((mlist->seg != mlistj->seg) && (mlistj->owner == mlist->seg+1)) {
                    mlistj->name = mlist->name;
                    if (mlist->environment == mlistj->seg+1) {
                        mlistj->type = MT_ENV;
                        if (mlist->seg + 1 == _psp) {
                            /* MEM environment */
                            mlistj->type=MT_FREE;
                            mlistj->name = "";
                        }
                    } else {
                        mlistj->type = MT_DATA;
                    }
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

    /* merge free blocks */
    for (mlist=mlistroot; mlist->next !=NULL;) {
        if (mlist->type == MT_FREE) {
            if (mlist->next->type == MT_FREE) {
                /* both free, merge */
                mlist->size += mlist->next->size + 1;
                mlist->next = mlist->next->next;
                continue;
            }
            if (mlist->seg < convtopseg)
                freemem += mlist->size + 1;
        }
        mlist=mlist->next;
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
        if (mlist->type == MT_FREE && mlist->size > largest)
            largest = mlist->size;
    return largest;
}    

static DEVINFO *make_dev_list(MINFO *mlist)
{
    DEVICE_HEADER far *cur_dev;
    DPB far*cur_dpb;
    DEVINFO *dlistroot, *dlist;
    MINFO *mlistroot = mlist;
    
    dlist = dlistroot = xcalloc(1, sizeof(DEVINFO));

    cur_dev = (DEVICE_HEADER far *)(dos_list_of_lists() + 0x22);
    
    while (FP_OFF(cur_dev) != 0xFFFF) {
	dlist->addr=cur_dev;
        dlist->attr=cur_dev->attr;
        dlist->progname="";
        check_name(dlist->devname, cur_dev->name, 8);
        cur_dev=cur_dev->next;
        if (FP_OFF(cur_dev) != 0xFFFF) {
            dlist->next=xcalloc(1, sizeof(DEVINFO));
            dlist=dlist->next;
        }
    }

    for (dlist=dlistroot;dlist!=NULL;dlist=dlist->next)
        for (mlist=mlistroot;mlist!=NULL;mlist=mlist->next)
            if (mlist->seg == FP_SEG(dlist->addr))
                dlist->progname = (mlist->name[0] == ' ') ?
                       &mlist->name[1] : mlist->name;

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
        if ((dlist->attr & 0x8000) == 0)
            dlist->devname[0]='\0';

        if (dlist->firstdrive != 0)
            {
            sprintf(dlist->devname, "%c:", dlist->firstdrive);
            if (dlist->lastdrive != 0)
                sprintf(&dlist->devname[2], " - %c:", dlist->lastdrive);
            }
        }

    return dlistroot;
}

#define dos_in_hma() (dos_in_hma_() & 0x10)

static void normal_list(unsigned memfree, UPPERINFO *upper)
{
    unsigned memory, memused, largest_executable;
    XMSINFO *xms;
    EMSINFO *ems;

    ems=check_ems();
    xms=check_xms();
    memory=biosmemory();
    memused=memory - round_seg_kb(memfree);
    printf("\n");
    printf("Memory Type        Total        Used        Free\n");
    printf("----------------  --------   ---------    --------\n");
    printf("Conventional%13uK%11uK%11uK\n", memory, memused, round_seg_kb(memfree));
    if (upper != NULL) {
	upper->free=round_seg_kb(upper->free);
	upper->total=round_seg_kb(upper->total);
	printf("Upper%20uK%11uK%11uK\n",
	       upper->total, upper->total-upper->free, upper->free);
    }
    if (xms != NULL) {
	convert("Extended (XMS)%11sK ", round_kb(xms->total));
	convert("%10sK ", round_kb(xms->total)-xms->free);
	convert("%10sK\n", xms->free);
    }
    printf("----------------  --------   ---------    --------\n");
    printf("\n");
    if (ems != NULL) {
	printf("Total Expanded (EMS) ");
	convert("%19sK ", ems->size * 16L);
	convert("(%10s bytes)\n", ems->size * 16384L);
	printf("Free Expanded (EMS)  ");
	convert("%19sK ", ems->free * 16L);
	convert("(%10s bytes)\n", ems->free * 16384L);
        printf("\n");
    }

    largest_executable = mcb_largest();

    printf("Largest executable program size%9uK ", round_seg_kb(largest_executable));
    convert("(%10s bytes)\n", (ulong)largest_executable*16);
    if (upper != NULL) {
	printf("Largest free upper memory block%9uK ", round_seg_kb(upper->largest));
        convert("(%10s bytes)\n", (ulong)upper->largest*16);
    }

    printf("%s is %sresident in the high memory area.\n",
           get_os(), dos_in_hma() ? "": "not ");
}    

static void print_entry(MINFO *entry)
{
    static char *typenames[]= { "", "system code", "system data",
                                "program", "device driver", "environment",
                                "data area", "free", "MT_MAP" };

    printf("  %04X%9lu   %-9s   %-13s\n",
           entry->seg, (ulong)entry->size*16, entry->name, typenames[entry->type]);
}

static void upper_list(void)
{
    MINFO *ml;

    printf("\nSegment   Size       Name         Type\n");
    printf(  "------- --------  ----------  -------------\n");
    for (ml=make_mcb_list(NULL);ml!=NULL;ml=ml->next)
        if ((ml->type != MT_NONE && ml->type < MT_ENV) || ml->type == MT_FREE)
		print_entry(ml);
}

static void full_list(void)
{
    MINFO *ml;
    
    printf("\nSegment   Size       Name         Type\n");
    printf(  "------- --------  ----------  -------------\n");
    for (ml=make_mcb_list(NULL);ml!=NULL;ml=ml->next)
/*        if (ml->type != MT_MAP) */
            print_entry(ml);
}

static void device_list(void)
{
    DEVINFO *dl;

    printf("\n   Address     Attr    Name       Program\n");
    printf(  " -----------  ------ ----------  ----------\n");
             /*  XXXX:XXXX    XXXX   XXXXXXXX    XXXXXXXX */
    for (dl=make_dev_list(make_mcb_list(NULL));dl!=NULL;dl=dl->next)
	printf("  %p    %04X   %-8s    %-8s\n", dl->addr, dl->attr,
                dl->devname, dl->progname);
}

static void ems_list(void)
{
    EMSINFO *ems;
    ushort i;
    static char handlename_sys[] = "SYSTEM  ";
    static char handlename_other[9];
    char *handlename;
    static char format[] = "  %-20s";
    
    ems=check_ems();
    if (ems==NULL)
    {
        printf("  EMS driver not installed in system.\n");
    }
    else
    {
	printf(format, "\nEMS driver version");
	printf("%1u.%1u\n", ems->vermajor, ems->verminor);
	printf(format, "EMS page frame");
	printf("%04X\n", ems->frame);
	printf(format, "Total EMS memory");
	printf("%lu bytes\n", ems->size * 16384L);
	printf(format, "Free EMS memory");
	printf("%lu bytes\n", ems->free * 16384L);
	printf(format, "Total handles");
	printf("%u\n", ems->totalhandle);
	printf(format, "Free handles");
	printf("%u\n", ems->freehandle);

	printf("\n  Handle   Pages    Size	 Name\n");
	printf(  " -------- ------  --------   ----------\n");
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
                }
                check_name(handlename, handlename, 8);
            }

            printf("%9u%7u%10lu%11s\n", ems->handles[i].handle,
                   ems->handles[i].pages, (ulong)ems->handles[i].pages * 16384L, handlename);
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

    if (xms==NULL)
	{
	printf("XMS driver not installed in system.\n");
	return;
	}

    printf("\nTesting XMS memory ...\n");

    lhandle = xms_query_a20();
    if (lhandle < 0) {
	printf("XMS INTERNAL ERROR.\n");
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

	printf("INT 2F AX=4309 supported\n");

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

    printf(format, "XMS version");
    printf("%u.%02u \t\t", xms->vermajor, xms->verminor);
    printf(format, "XMS driver version");
    printf("%u.%02u\n", xms->drv_vermajor, xms->drv_verminor);
    printf(format, "HMA state");
    printf("%s \t", (xms->hma) ? "exists" : "not exists");
    printf(format, "A20 line state");
    printf("%s\n", (xms->a20) ? "enabled" : "disabled");
    printf(format, "Free XMS memory");
    printf("%lu bytes\n", xms->free*1024L);
    printf(format, "Largest free XMS block");
    printf("%lu bytes\n", xms->largest*1024L);
    printf(format, "Free handles");
    printf("%u\n", xms->freehandle);
    printf("\n");
    if (xms->handles != NULL)
    {
	printf(" Block	 Handle	    Size     Locks\n");
	printf("------- --------  --------  -------\n");
	for (i=0, handle=xms->handles;handle!=NULL;handle=handle->next, i++)
	    printf("%7u %8u  %8lu  %7u\n", i, handle->handle,
		   handle->size, handle->locks);

    }

    upper = check_upper(make_mcb_list(NULL));
    if (upper != NULL)
    {
	printf(format, "Free upper memory");
	printf("%lu bytes\n", upper->free*16L);
	printf(format, "Largest upper block");
	printf("%lu bytes\n", upper->largest*16L);
    }
    else
    {
	printf("Upper memory not available\n");
    }

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

enum {F_HELP = 1, F_DEVICE = 2, F_EMS = 4, F_FULL = 8, F_UPPER = 16,
      F_XMS = 32, F_PAGE = 64 };

int main(int argc, char *argv[])
{
    uchar flags = 0;
    UPPERINFO *upper;
    unsigned memfree;
    int i = 1, j;
    char *argvi = argv[i];
    static struct {char c; uchar flag;} optype[] = 
    {
      { '?', F_HELP },
      { 'D', F_DEVICE },
      { 'E', F_EMS },
      { 'F', F_FULL },
      { 'P', F_PAGE },
      { 'U', F_UPPER },
      { 'X', F_XMS }
    };

    argc = argc;

    while (argvi != NULL && (flags & F_HELP)==0) {
        char ch = *argvi++;
        if (ch == '-' || ch == '/')
            ch = toupper(*argvi);
        else
            ch = '/';
        for (j = 0; j < sizeof(optype)/sizeof(optype[0]); j++) {
            if (ch == optype[j].c) {
                flags |= optype[j].flag;
                break;
            }
        }
        if (j == sizeof(optype)/sizeof(optype[0]))
            fatal("unknown option: %s\nUse /? for help\n", argvi);

        if (*++argvi == '\0')
        {
            i++;
            argvi = argv[i];
        }
    }

    upper=check_upper(make_mcb_list(&memfree));

    if (flags & F_PAGE)   num_lines=get_font_info();
    if (flags & F_DEVICE) device_list();
    if (flags & F_EMS)    ems_list();
    if (flags & F_FULL)   full_list();
    if (flags & F_UPPER)  upper_list();
    if (flags & F_XMS)    xms_list();
        
    if (flags & F_HELP)
	{
	printf("Displays the amount of used and free memory in your system.\n\n"
	       "Syntax: MEM [/E] [/F] [/D] [/U] [/X] [/P] [/?]\n"
	       "  /E  Reports all information about Expanded Memory\n"
	       "  /F  Full list of memory blocks\n"
	       "  /D  List of device drivers currently in memory\n"
	       "  /U  List of programs in conventional and upper memory\n"
	       "  /X  Reports all information about Extended Memory\n"
	       "  /P  Pauses after each screenful of information\n"
	       "  /?  Displays this help message\n");
	return 1;
	}

    normal_list(memfree, upper);
    return 0;
}

/*
        TE - some size optimizations for __TURBOC__
    
        as printf() is redefined in PRF.C to use no stream functions,
        rather calls DOS directly, these Stream operations are nowhere used,
        but happen to be in the executable.

        so we define some dummy functions here to save some precious bytes :-)

        this is in no way necessary, but saves us some 1500 bytes
*/

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

