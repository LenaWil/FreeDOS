/*
 *  BATCH.C - batch file processor for COMMAND.COM.
 *
 *	Enter a batch file
 *
 *	Say: BATCH.BAT arg1 arg2 ...
 *
 *	The filename of the script itself is internally storred in its
 *	absolute form, so that CHDIR'ing and switching the current drive
 *	won't effect the script when continueing.
 *
 *	The arguments expands to:
 *		argv[0] == name of batch script as written on command line
 *		argv[1..N] == argument
 *	Arguments are delimited by the usual argument separators, such as
 *	whitespaces, but quotes are honored. When expanding quotes are
 *	restored in the same fashion as they had been written on command
 *	line, but most internal commands do ignore missing or embedded quotes.
 *
 *	Switching floppies however must be supported, e.g.:
 *	There is a INSTALL.BAT on disk #1, and there is a sequence like this:
 *	01: COPY *.* c:\fdos
 *	02: :disk2
 *	03: pause Please insert disk #2
 *	04: if not exist \diskid.2 goto disk2
 *	05: copy *.* c:\fdos
 *
 *	If there is an INSTALL.BAT script with exactly the same contents on
 *	disk #2, the execution of the script continues without problem, when
 *	the floppy gets changed within the PAUSE statement in line #03, with
 *	the lines #04 and following.
 *
 *	During execution of commands, the file is closed and to read the next
 *	one the file gets re-opened. Doing so also supports so-called self-
 *	modifying batch script, where a script modifies itself; a process that
 *	a) might be denied if SHARE or a similiar functionality has been
 *		activated, or
 *	b) because of pre-fetching and other buffering methods is not reflected
 *		at once during the batch file processing.
 *
 */

#include "../config.h"

#include <assert.h>
#include <limits.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <ctype.h>
//#include <string.h>

#include <dfn.h>

#include "../include/command.h"
#include "../include/context.h"
#include "../include/cmdline.h"
#include "../err_fcts.h"

/*
 *  Batch files are entitled to be "modifyable" and may span across
 *  floppy disks (you know: calling an external program that says
 *  "Please insert next floppy", then the INSTALL.BAT file on the
 *  the "next floppy" is accessed rather the originally opened one).
 *  The current implementation keeps the batchfile open, which is not
 *  the standard behaviour.
 */
int batch(char *fullname, char *firstword, char *param)
{
	assert(fullname);
	assert(firstword);
	assert(param);

	if((fullname = regStr(dfnexpand(fullname, 0))) == 0) {
		error_out_of_memory();
		return 1;
	}

	dprintf(("batch('%s', '%s', '%s')\n", fullname, firstword, param));

	if(!called)			/* when this batch file terminates,
							drop to interactive command line */
		if(ecMkc("CANCEL", (char*)0) != E_None)
			return 1;

	if(CTXT_INFO(CTXT_TAG_ARG, nummax)) {
		/* There are already arguments storred */
		char buf[(sizeof(unsigned) * 8 + 1 + 1) * 3 + 1];

		sprintf(buf, "%u %u %u"
		 , F(base_shiftlevel)
		 , CTXT_INFO(CTXT_TAG_ARG, nummax) + 1
		 , F(shiftlevel));
		assert(strlen(buf) < sizeof(buf));
		if(ecMkc("ARG ", buf, (char*)0) != E_None)
			return 1;
	} else
		if(ecMkc("ARG", (char*)0) != E_None)	/* reset all arguments */
			return 1;

		/* New base is the first non-used string */
	ctxtPush(CTXT_TAG_ARG, firstword);	/* argv[0] <-> name of script */
	F(base_shiftlevel) = CTXT_INFO(CTXT_TAG_ARG, nummax);
	F(shiftlevel) = 0;		/* each script has its own sh-lvl */

	if(setArguments(param))	 /* out of memory condition */
		return 1;

	ecMkB(fullname);		/* Default to pos & lnr := 0 */

	return 0;
}
