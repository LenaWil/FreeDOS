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


/* Symbolic constants */

#define _MAX_PATH  80
#define _MAX_DRIVE 3			/* includes trailing colon */
#define _MAX_DIR   66			/* incl leading,trailing slashes */
#define _MAX_FNAME 9
#define _MAX_EXT   5			/* includes leading dot */


/* Functions */


/* splits a full DOS pathname into its DOS components. Takes a file
   path name as a string in the form X:/DIR/SUB/NAME.EXT and stores
   its components in the strings pointed to by drive, dir, name, ext.
   (drive is always returned as null.) */

/* Since this is assumed to be a compatibility library for UNIX, I
   will use '/' as my path separator, instead of '\\' */

void
_splitpath (const char *path, char *drive, char *dir, char *name,
	    char *ext);


/* builds a path from its component parts.  The new path is of the
   form X:/DIR/SUB/NAME.EXT */

void
_makepath (char *path, const char *drive, const char *dir, const char *name,
	   const char *ext);


/* Convert a pathname from relative to absolute.  This removes
   references to '/./', '/../' and extra '/' characters. */

/* uses UNIX realpath() */

char *_fullpath (char *buf, const char *path, int buflen);
