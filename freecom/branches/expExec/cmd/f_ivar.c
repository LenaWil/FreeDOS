/* $Id$

	function: ivar(varname)

	Return:
		0: on error (error message already displayed)
		else: pointer to dynamically allocated value of internal variable
*/

#include "../config.h"

#include <string.h>

#include "../include/command.h"
#include "../include/context.h"
#include "../include/cmdline.h"
#include "../include/misc.h"

char *fct_ivar(char *param)
{	char *buf;

	strupr(param);

	return ctxtGetS(2, CTXT_TAG_IVAR, param, &buf) == 0
		? buf
		: 0;
}
