/*
 *  WHERE.C - path functions.
 *
 *
 *
 *  Comments:
 *
 *  07/15/95 (Tim Norman)
 *    started.
 *
 *  08/08/95 (Matt Rains)
 *    i have cleaned up the source code. changes now bring this source into
 *    guidelines for recommended programming practice.
 *
 *  12/12/95 (Steffan Kaiser & Tim Norman)
 *    added some patches to fix some things and make more efficient
 *
 *  1/6/96 (Tim Norman)
 *    fixed a stupid pointer mistake...  Thanks to everyone who noticed it!
 *
 *  8/1/96 (Tim Norman)
 *    fixed a bug when getenv returns NULL
 *
 *  8/7/96 (Steffan Kaiser and Tim Norman)
 *    speed improvements and bug fixes
 *
 *  8/27/96 (Tim Norman)
 *    changed code to use pointers directly into PATH environment variable
 *    rather than making our own copy.  This saves some memory, but requires
 *    we write our own function to copy pathnames out of the variable.
 *
 *  12/23/96 (Aaron Kaufman)
 *    Fixed a bug in get_paths() that did not point to the first PATH in the
 *    environment variable.
 *
 *  7/12/97 (Tim Norman)
 *    Apparently, Aaron's bugfix got lost, so I fixed it again.
 *
 *  16 July 1998 (John P. Price)
 *    Added stand alone code.
 *
 *  17 July 1998 (John P. Price)
 *    Rewrote find_which to use searchpath function
 *
 * 24-Jul-1998 (John P Price <linux-guru@gcfl.net>)
 * - fixed bug where didn't check all extensions when path was specified
 *
 * 27-Jul-1998 (John P Price <linux-guru@gcfl.net>)
 * - added config.h include
 *
 * 30-Jul-1998 (John P Price <linux-guru@gcfl.net>)
 * - fixed so that it find_which returns NULL if filename is not executable
 *   (does not have .bat, .com, or .exe extention). Before command would
 *   to execute any file with any extension (opps!)
 *
 * 2001/02/16 ska
 * add: command WHICH
 */

#include "config.h"

#include <assert.h>
#include <dir.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "command.h"
#include "cmdline.h"

#include "dfn.h"


/*
 *  searches for file using path info.
 *
 */
char *find_which(char *fname)
{
  static char *buf = NULL;

  free(buf);
  return buf = dfnsearch(fname, NULL, NULL);
}

#ifdef INCLUDE_CMD_WHICH

int cmd_which(char *rest)
{
	char **arg, *p;
	int argc, optc, i;

	if((arg = scanCmdline(rest, NULL, NULL, &argc, &optc)) == NULL)
		return E_Other;

	for(i = 0; i < argc; ++i) {
		assert(arg[i]);
		fputs(arg[i], stdout);
		if((p = find_which(arg[i])) != 0) {
			putchar('\t');
			puts(p);
		} else {
			putchar('\n');
		}
	}

	freep(arg);
	return E_None;
}

#endif

#ifdef DEBUG_STANDALONE
int main(int argc, char **argv)
{
  char *fullname;

  if ((fullname = find_which("deltree.exe")) != NULL)
  {
    printf("deltree.exe found at %s\n", fullname);
  }
  else
    printf("deltree.exe not found.\n");

  if ((fullname = find_which("deltree")) != NULL)
  {
    printf("deltree found at %s\n", fullname);
  }
  else
    printf("deltree not found.\n");

  if ((fullname = find_which("c:\windows\command\deltree")) != NULL)
  {
    printf("deltree found at %s\n", fullname);
  }
  else
    printf("deltree not found.\n");

  return 0;
}

#endif
