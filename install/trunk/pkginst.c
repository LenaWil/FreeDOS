/* pkginst.c */

/* Function to install a FreeDOS package. This is written as a wrapper
   so we can change it later if we decide to switch to RPM or APT or
   some other packaging system. */

/* Copyright (c) 2011 Jim Hall <jhall@freedos.org> */

#include <stdio.h>
#include <stdlib.h>

#include <conio.h>				/* getch */

#include "pkginst.h"


int UzpMain ( int argc, char **argv );		/* from Info-Zip's Unzip */

/*
  pkginstall()

  INPUT:
  filename = package file to install
  dest = destination dir to install into

  RETURN:
  non-zero if successful, zero if fail
*/

int
pkginstall (char *filename, char *dest, int do_source)
{
  int ret;

  char command[_MAX_PATH];
  /*
  char **uzp_argv;
  int uzp_argc = 6;
  */

  /* assign string pointers */

  /*
  uzp_argv[0] = "UNZIP";
  uzp_argv[1] = "-q";
  uzp_argv[2] = "-o";
  uzp_argv[3] = filename;
  uzp_argv[4] = "-d";
  uzp_argv[5] = dest;
  */

  /* unzip */

  if (do_source)
    {
      /* cputs ("DEBUG: include source code"); */
      sprintf (command, "UNZIP -q -o %s -d %s", filename, dest);
    }
  else
    {
      /* cputs ("DEBUG: skip source code"); */
      sprintf (command, "UNZIP -q -o -C %s -x SOURCE\\* -d %s", filename, dest);
    }
  ret = system (command);

  /*
  cprintf ("DEBUG: about to call UzpMain() . . . press a key");
  getch();
  ret = UzpMain (uzp_argc, uzp_argv);
  cprintf ("DEBUG: %s -> %s (ok)", filename, dest);
  */

  /* return code from UzpMain() or system() will be 0 if success,
     nonzero if ok, because this is what a shell ERRORLEVEL would
     be. this is opposite from what pkginstall() is supposed to
     return, so fix the value. */

  /* BUG: OpenWatcom system() seems to always return ok (0), even if
     an error had occurred */

  if (ret == 0)
    {
      /* cputs ("DEBUG: ok"); */
      return (1);
    }
  else
    {
      /* cputs ("DEBUG: error"); */
      return (0);
    }
}
