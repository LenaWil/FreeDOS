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

#include <stdio.h>			/* for system(), free() */
#include "unzip.h"			/* for UzpMain() */
#include "dir.h"
#include <conio.h>                  /* Quick hack to set cursor for errors */


int
unzip_file (const char *zipfile, const char *fromdir, char *destdir)
{
  char full_zipfile[MAXPATH];		/* path to a zipfile */
  int ret;

  /* -d will extract into a given path.  -o will force overwrite if
     the files already exist */

  /* set 3=full_zipfile and 5=destdir */

  char *argv[7] = {"unz.c", "-qq", "-o", "full_zipfile", "-d", "destdir", NULL};

  /* create the zip file name */

  /* usage is _makepath(newpath, drive, dir, name, ext) */

  /* .zip extension not needed as Unzip will add it if not there.  The
     DAT file may also have the .zip extension anyway. */

  fnmerge (full_zipfile, "", fromdir, zipfile, "");

  /* set 3=full_zipfile and 5=destdir */

  argv[3] = full_zipfile;		/* pointer assignment */
  argv[5] = destdir;			/* pointer assignment */

  /* Set the cursor to location so errors can be read, not proper fix. */
  gotoxy(2, 11);

  ret = UzpMain (7, argv);		/* the Unzip code */

  /* Done */

  return (ret);
}
