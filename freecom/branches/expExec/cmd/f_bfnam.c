/* $id$

	Returns the absolute filename of the currently active batch script
	or ""  if none.

*/

#include "../config.h"

#include "../include/command.h"
#include "../include/context.h"
#include "../include/misc.h"

#pragma argsused
char *fct_batchname(char *param)
{	ctxtEC_Batch_t far *bc = ecLastB();

	return bc? edupstr(bc->ec_fname): 0;
}
