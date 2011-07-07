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

int
pkginstall (char *filename, char *dest)
{
  int ret;
  char **uzp_argv;
  int uzp_argc = 6;

  char command[_MAX_PATH];

  /* assign string pointers */

  uzp_argv[0] = "UNZIP";
  uzp_argv[1] = "-q";				/* quiet */
  uzp_argv[2] = "-o";				/* overwrite without prompt */
  uzp_argv[3] = filename;			/* zip file to extract */
  uzp_argv[4] = "-d";				/* extract to -d=dir */
  uzp_argv[5] = dest;				/* destination dir */

  /*
  cprintf ("DEBUG: about to call UzpMain . . . press a key");
  getch();
  */

  /* unzip */

  sprintf (command, "UzpMain -q -o -q %s -d %s", filename, dest);
  ret = system (command);
  /*
  ret = UzpMain (uzp_argc, uzp_argv);
  */

  /*
  cprintf ("DEBUG: %s -> %s (ok)", filename, dest);
  */

  return (ret);
}
