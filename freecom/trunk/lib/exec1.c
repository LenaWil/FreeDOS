/*	$id$
	$Locker$	$Name$	$State$

	Decode the return code of the execution of an external program:
		return := input? input
					   : normal exit? errorlevel
					                : ^CBreak errorlevel

	This file bases on EXEC.C of FreeCOM v0.81 beta 1.

	$Log$
	Revision 1.1  2001/04/12 00:33:53  skaus
	chg: new structure
	chg: If DEBUG enabled, no available commands are displayed on startup
	fix: PTCHSIZE also patches min extra size to force to have this amount
	   of memory available on start
	bugfix: CALL doesn't reset options
	add: PTCHSIZE to patch heap size
	add: VSPAWN, /SWAP switch, .SWP resource handling
	bugfix: COMMAND.COM A:\
	bugfix: CALL: if swapOnExec == ERROR, no change of swapOnExec allowed
	add: command MEMORY
	bugfix: runExtension(): destroys command[-2]
	add: clean.bat
	add: localized CRITER strings
	chg: use LNG files for hard-coded strings (hangForEver(), init.c)
		via STRINGS.LIB
	add: DEL.C, COPY.C, CBREAK.C: STRINGS-based prompts
	add: fixstrs.c: prompts & symbolic keys
	add: fixstrs.c: backslash escape sequences
	add: version IDs to DEFAULT.LNG and validation to FIXSTRS.C
	chg: splitted code apart into LIB\*.c and CMD\*.c
	bugfix: IF is now using error system & STRINGS to report errors
	add: CALL: /N

 */

#include "../config.h"

#include <assert.h>
#include <dos.h>

#include "../include/command.h"

int decode_exec_result(int rc)
{	struct REGPACK rp;

	if (!rc) {
		rp.r_ax = 0x4d00;           /* get return code */
		intr(0x21, &rp);
		rc = rp.r_ax & 0xFF;
		if ((rp.r_ax & 0xFF00) == 0x100) {
			ctrlBreak = 1;
			if (!rc)
				rc = CBREAK_ERRORLEVEL;
		}
	}

	return rc;
}
