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

#include <dos.h>
#include <string.h>

#include "splitdir.h"

int UzpMain( int argc, char **argv );	/* from InfoZip's Unzip */

int
unzip_file (char *zipfile, char *fromdir, char *destdir)
{
  int i;
  int ret;
  int uzp_argc = 4;
  char *uzp_argv[] = {"UNZIP.EXE", "?", "-d", "test"};

  char zdrive[_MAX_DRIVE];
  char zdir[_MAX_DIR];
  char full_zipfile[_MAX_PATH];

  /* show args */

  splitdir (fromdir, zdrive, zdir);
  _makepath (full_zipfile, zdrive, zdir, zipfile, NULL);

  uzp_argv[1] = full_zipfile;
  uzp_argv[3] = destdir;

#ifdef DEBUG
  /* debugging */

  printf ("\n");
  for (i = 0; i < uzp_argc; i++)
    {
      printf ("[%d]'%s'", i, uzp_argv[i]);
    }
#endif

  /* call unzip */

  /* ret = UzpMain (uzp_argc, uzp_argv); */
  sleep (1);

  return (ret);
}
