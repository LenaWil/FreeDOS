/* makepath.c */

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


/* builds a path from its component parts.  The new path is of the
   form X:/DIR/SUB/NAME.EXT */

/* the Borland C version of _makepath() allows any length arguments to
   come in and assumes that the path is long enough to hold it all.
   I'm sure Borland does error checking to not overflow the variable,
   but I'm not going to.  So there.  */

void
_makepath (char *path, const char *drive, const char *dir, const char *name,
	   const char *ext)
{
  char s[_MAX_PATH];
  int end;

#ifndef NDEBUG /* debugging statements */
  printf ("drive=%s\n", drive);
  printf ("dir=%s\n", dir);
  printf ("name=%s\n", name);
  printf ("ext=%s\n", ext);
#endif /* NDEBUG */

  /* copy the drive */

  /* I'm not putting any error checking here because this is UNIX and
     you can pretty much do what you want with path names. */

  strcpy (s, drive);

#ifndef NDEBUG
  printf ("(drive) s=%s\n", s);
#endif /* NDEBUG */

  /* concatenate the path */

  strcat (s, dir);

  end = strlen (s) - 1;

  if (end < 0)
    {
      /* empty string! */
      strcat (s, "/");
    }

  else if (s[end] != '/')
    {
      /* no trailing slash */
      strcat (s, "/");
    }

#ifndef NDEBUG
  printf ("(dir) s=%s\n", s);
#endif /* NDEBUG */

  /* concatenate the filename */

  /* name might be null */

  if (name[0])
    {
      strcat (s, name);
    }

#ifndef NDEBUG
  printf ("(name) s=%s\n", s);
#endif /* NDEBUG */

  /* concatenate the ext */

  /* ext might be null */

  if (ext[0])
    {
      if (ext[0] != '.')
	{
	  /* no leading dot */
	  strcat (s, ".");
	}
    }

  strcat (s, ext);

#ifndef NDEBUG
  printf ("(ext) s=%s\n", s);
#endif /* NDEBUG */

  /* Assign the path and return */

  /* path might be NULL */

  if (path)
    {
      strncpy (path, s, _MAX_PATH);
    }
}
