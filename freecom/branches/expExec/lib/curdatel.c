/*	$id$
	$Locker$	$Name$	$State$

	Get the current date in a long format (day_of_week date)
	in a dynamically allocated buffer

	This file bases on MISC.C of FreeCOM v0.81 beta 1.

	$Log$
	Revision 1.1.4.3  2001/07/16 20:28:38  skaus
	Update #9

	Revision 1.1.4.2  2001/07/05 22:18:34  skaus
	Update #5
	
	Revision 1.1.4.1  2001/06/19 20:42:23  skaus
	Update #1
	
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
#include <stdlib.h>
#include <string.h>

#include <dynstr.h>

#include "../include/command.h"
#include "../err_fcts.h"
#include "../include/misc.h"
#include "../include/nls.h"
#include "../strings.h"

char *curDateLong(void)
{
	struct dosdate_t d;
	char *date, *p;

	_dos_getdate(&d);

	if((date = nls_makedate(0, d.year, d.month, d.day)) == 0) {
		error_out_of_memory();
		return 0;
	}

	if((p = getString(TEXT_WEEKDAY_SHORT_NAME_SUNDAY + d.dayofweek)) == 0
	 || !StrAppChr(p, ' ')
	 || !StrCat(p, date)) {
		myfree(p);
		myfree(date);
		error_out_of_memory();
		return 0;
	}

	myfree(date);

	return p;
}
