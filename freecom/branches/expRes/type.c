/*
 *  TYPE.C - type internal command
 *
 *  Comments:
 *
 *  07/08/1998 (John P. Price)
 *    started.
 *
 * 07/12/98 (Rob Lake)
 *    - Changed error messages
 *
 * 27-Jul-1998 (John P Price <linux-guru@gcfl.net>)
 * - added config.h include
 *
 * 10-Aug-1998 ska
 * - added ^Break checking
 *
 * 1999/01/24 ska
 * add: support for CP/M style device names (openf.h)
 */

#include "config.h"

#ifdef INCLUDE_CMD_TYPE

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "command.h"
#include "strings.h"
#include "cmdline.h"

#include "openf.h"

#pragma argsused
int cmd_type(char *rest)
{
  char buf[256];
  char **argv;
  int argc, opts, ec = E_None;
  FILE *f;

  if((argv = scanCmdline(rest, NULL, NULL, &argc, &opts)) == NULL)
    return 1;

  /* Because no option was passed into scanCmdline() no
    option can have been processed */
  assert(opts == 0);

  if(!argc) {
    error_req_param_missing();
    ec = E_Useage;
    goto errRet;
  }

  for(argc = 0; argv[argc]; ++argc) {
    if ((f = fdevopen(rest, "rt")) == NULL)
    {
    error_sfile_not_found(rest);
    ec = E_Other;
    break;
    }

    while (fgets(buf, sizeof(buf), f))
    {
      if(cbreak) {
      fclose(f);
        ec = E_CBreak;
      goto errRet;
    }
    fputs(buf, stdout);
    }
    fclose(f);
    if(cbreak) {
      ec = E_CBreak;
      break;
    }
  }

errRet:
  freep(argv);

  return ec;
}

#endif
