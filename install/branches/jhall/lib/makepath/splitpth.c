/* splitpth.c */

/* part of a _splitpath(), _makepath() compatibility for UNIX */

/* Copyright (C) 1999 Jim Hall, jhall1@isd.net */

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
#include <string.h>

#include "makepath.h"


/* Symbolic constants */

#define NDEBUG				/* UNCOMMENT to turn ON debugging */


/* Functions */


/* splits a full DOS pathname into its DOS components. Takes a file
   path name as a string in the form X:/DIR/SUB/NAME.EXT and stores
   its components in the strings pointed to by drive, dir, name, ext.
   (drive is always returned as null.) */

/* Since this is assumed to be a compatibility library for UNIX, I
   will use '/' as my path separator, instead of '\\' */

void
_splitpath (const char *path, char *drive, char *dir, char *name,
	    char *ext)
{
  char _drive[_MAX_DRIVE];
  char _dir[_MAX_DIR];
  char _name[_MAX_FNAME];
  char _ext[_MAX_EXT];
  char *p;
  int pathlen;
  int i, j, m;

  /* Determine length of path */

  pathlen = strlen(path);

#ifndef NDEBUG /* debugging statements */
  printf ("%s is %d\n", path, pathlen);
#endif /* NDEBUG */

  /* Find the name (starts at i) */

  for (i = pathlen-1; (i > 0) && (path[i] != '/'); i--) ;
  i = (i < 0 ? 0 : i+1);

  /* Find the ext (starts at j) */

  for (j = i; (j < pathlen) && (path[j] != '.'); j++) ;

  /* Find the drive */
  /* Find the dir (starts at m) */

  m = 0;
  strcpy (_drive, "");

  if (path[1] == ':')
    {
      /* Aha! This has a drive letter */
      strncat (_drive, path, 2);
      m = 2;
    }

  /* Save values */

  if (drive)
    {
      strncpy (drive, _drive, _MAX_DRIVE);
    }

  p = &path[m];
  strcpy (_dir, "");
  strncat (_dir, p, i-m);
  if (dir)
    {
      strncpy (dir, _dir, _MAX_DIR);
    }

  p = &path[i];
  strcpy (_name, "");
  strncat (_name, p, j-i);
  if (name)
    {
      strncpy (name, _name, _MAX_FNAME);
    }

  p = &path[j];
  strcpy (_ext, "");
  strncat (_ext, p, pathlen-j);
  if (ext)
    {
      strncpy (ext, _ext, _MAX_EXT);
    }

#ifndef NDEBUG
  printf ("_drive=%s\n", _drive);
  printf ("_dir=%s\n", _dir);
  printf ("_name=%s\n", _name);
  printf ("_ext=%s\n", _ext);
#endif /* DEBUG */
}
