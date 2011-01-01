/* Wrapper to the info-unzip API */

/* Copyright (C) 1998,1999,2000,2001 Jim Hall <jhall@freedos.org> */

/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int UzpMain( int argc, char **argv );	/* from InfoZip's Unzip */

int
unzip_file (char *zipfile, char *fromdir, char *destdir)
{
  int i;
  int ret;
  int uzp_argc = 6;
  char *uzp_argv[6] = {"uzp", "-q", "-o", "?", "-d", "test"};

  /* just a simple wrapper to test things */

  printf ("wrapper to Unzip\n");

  /* show args */

  uzp_argv[3] = "t\\hello.zip"; /* pointer assignment */

  for (i = 0; i < uzp_argc; i++)
    {
      printf ("uzp_argv[%d] = '%s'\n", i, uzp_argv[i]);
    }

  /* call unzip */

  printf ("calling UzpMain ...\n");
  ret = UzpMain (uzp_argc, uzp_argv);
  printf ("... return = %d\n", ret);

  return (ret);
}

#ifdef DEBUG
/* debugging code - creates a standalone test program */

int
main (int argc, char **argv)
{
  int ret;

  printf ("unzipping hello.zip ...\n");
  ret = unzip_file ("hello.zip", "t", "test");

  exit (ret);
}
#endif
