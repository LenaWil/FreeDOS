## is the base path ("root") of Turbo C++ v1.01
CC_BASE_PATH = O:\TC101

## Where the pre-compiled SUPPL files are located
## See DOCS\SUPPL.TXT for more information about this library
SUPPL_INC_PATH = ..\SUPPL
SUPPL_LIB_PATH = $(SUPPL_INC_PATH)

## Program locations
BINPATH = $(CC_BASE_PATH)\BIN
LIBPATH = $(CC_BASE_PATH)\LIB
INCLUDEPATH = $(CC_BASE_PATH)\INCLUDE;$(SUPPL_INC_PATH)

## Which programs to use
CC = $(BINPATH)\TCC +$(CFG)
AR = $(BINPATH)\Tlib
LD = $(BINPATH)\Tlink
NASM = nasm.exe

## Add -DDEBUG=1 below to enable debug support for assembly files
NASMFLAGS =
## Add -DDEBUG=1 below to enable FreeCOM-debug support for C files
## Add -DNDEBUG=1 below to disable any debug (even assert)
CFLAGS =

## Memory model of FreeCOM
SHELL_MMODEL=s

!if $(XMS_SWAP)
__XMS_SWAP = -DXMS_SWAP=1
!endif

# Default configuration
# added strings.h here because $(CFG) is included everywhere already
## Add -D_NO__DOS_DATE if your compiler does not have no dosdate_t (*)
## Add -D_NO__DOS_TIME if your compiler does not have no dostime_t (*)
## Add -D_NO_FMEMCHR if your compiler does not have no _fmemchr() (*)
## Add -D_NO_FMEMCMP if your compiler does not have no _fmemcmp() (*)
##
## (*) Note: Should be detected by CONFIG.H automatically for Turbo C
##     and Borland C compilers.
##
$(CFG): $(CFG_DEPENDENCIES) ..\CONFIG.MAK
  copy &&|
-1-
-a
-f-
-ff-
-K
-w+
-O
-Z
-b-
-I$(INCLUDEPATH)
-L$(LIBPATH)
-m$(SHELL_MMODEL)
-DNDEBUG
-UDEBUG
$(__XMS_SWAP)
| $(CFG)


#-k-

#		*Implicit Rules*
.c.exe:
  $(CC) $< 
.c.obj:
  $(CC) -c {$< }
.asm.obj:
	$(NASM) $(NASMFLAGS) -f obj -DMODEL=$(SHELL_MMODEL) $<
