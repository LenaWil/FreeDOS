/* $id$

	Perform a I context (interactive command line)

	Return:
		0: on error
		else: cmdline, but context gets poped
*/

#include "../config.h"

#include <assert.h>
#include <string.h>

#include "../include/context.h"
#include "../include/misc.h"
#include "../include/output.h"

#pragma argsused
char *readInteractive(ctxtEC_t far * const ctxt)
{
	dprintf(("readInteractive()\n"));

	assert(ctxt);
	assert(ctxt->ctxt_type == EC_TAG_INTERACTIVE);

	cbreak;			/* Ignore a probably given ^Break */
	doCancel = doQuit = 0;

	if(doExit) {
		dprintf(("!! EXIT flag active\n"));
		return 0;
	}

	interactive = F(interactive) = iConsole();

	if(!interactive)
		return readcommandFromFile();

#ifdef FEATURE_ENHANCED_INPUT
	/* If redirected from file or so, should use normal one */
	return readcommandEnhanced();
#else
	return readcommandDOS();
#endif
}
