/* Functions to install a disk series, or a package */

/* Copyright (C) 1998-2001 Jim Hall <jhall@freedos.org> */

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
#include <stdlib.h>			/* for system(), free() */
#include <conio.h>			/* DOS conio */

#include "catgets.h"			/* DOS catopen, catgets */
#include "bargraph.h"			/* for bargraph() */
#include "box.h"			/* box() */
#include "dat.h"			/* data file functions */
#include "cat.h"			/* for cat_file() */
#include "inst.h"			/* for this file */
#include "isfile.h"			/* for isfile() */
#include "lsm.h"			/* Linux LSM files */
#include "repaint.h"			/* for repaint() */
#include "sel_list.h"			/* select_yn() */
#include "unz.h"			/* for UzpMain() */

/* Globals */

extern nl_catd cat;			/* (install.c) language catalog */


inst_t
set_install (const char *diskset, char *fromdir, char *destdir)
{
  /* Variables */

  char endfile[_MAX_PATH];		/* marks end of series */
  char descfile[_MAX_PATH];		/* description file */
  char datfile[_MAX_PATH];		/* current DAT file */
  char ext[_MAX_EXT];			/* file extension */
  char *s;
  int disknum = 0;			/* current disk number */
  int ch;
  inst_t ret;				/* return: no. of errors,warnings */
  inst_t this;				/* no. of errors,warnings */

  /* Create the filenames */

  _makepath (endfile, "", fromdir, diskset, "END");
  _makepath (descfile, "", fromdir, diskset, "TXT");

  /* Print the name of the series we are working on */

  repaint_empty();
  s = catgets (cat, 3, 2, "Installing series: ");
  gotoxy (2, 3);
  cputs (s);
  cputs (diskset);

  /* Install while we have disks to work from.  Since we will reach an
     exit condition within the loop, we use an infinite loop here. */

  ret.errors = 0;
  ret.warnings = 0;

  while (1) {
    repaint_empty();

    /* First check that the datfile exists.  If it doesn't, check if
       the endfile was found. */

    sprintf (ext, "%d", ++disknum);
    _makepath (datfile, "", fromdir, diskset, ext);

    if (!isfile (datfile)) {
      /* Does the endfile exist? */

      if (isfile (endfile)) {
	s = catgets (cat, 3, 5, "Done installing this disk series.");
	gotoxy (2, 10);
	cputs (s);

	s = catgets (cat, 3, 10, "If you are installing other disk series, please insert");
	gotoxy (2, 15);
	cputs (s);

	s = catgets (cat, 3, 11, "disk #1 of the next series in the drive now.");
	gotoxy (2, 16);
	cputs (s);

	s = catgets (cat, 1, 0, "Press any key to continue");
	gotoxy (2, 25);
	cputs (s);

	getch();
	return (ret);
      }

      /* The endfile was not found, so we know there is at least one
         more disk left to do.  Keep asking the user to insert the
         next disk. */

      do {
	s = catgets (cat, 3, 4, "Can't find data file for this install disk!");
	gotoxy (2, 10);
	cputs (s);

	gotoxy (2, 11);
	cputs (datfile);

	s = catgets (cat, 3, 8, "You may not have the right install floppy in the drive.");
	gotoxy (2, 15);
	cputs (s);

	s = catgets (cat, 3, 9, "Double check that you have the right disk and try again.");
	gotoxy (2, 16);
	cputs (s);

	s = catgets (cat, 2, 3, "Continue installing this disk? [yn]");
	ch = select_yn (s, "Yes", "No");

	if (ch)
	  {
	    /* user has decided to quit this series */
	    return (ret);
	  }

      } while (!isfile (datfile));
    } /* if no datfile */

    /* Install files from this disk */

    this = disk_install (datfile, descfile, fromdir, destdir);
    ret.errors += this.errors;
    ret.warnings += this.warnings;
  } /* while (1) */
}

inst_t
disk_install(const char *datfile, const char *descfile,
	     char *fromdir, char *destdir)
{
  char lsmfile[_MAX_PATH];		/* Linux software map file */
  char *s;

  int dat_size = 30;			/* malloc size of the dat array */
  int dat_count;			/* size of the dat array */
  int ret;
  int ch;
  int i;

  dat_t *dat_ary;			/* the DAT array */
  inst_t this;				/* return: no. of errors,warnings */

  /* Initialize variables */

  this.errors = 0;
  this.warnings = 0;

  /* Read dat file */

  dat_ary = malloc (sizeof (dat_t) * dat_size);
  if (dat_ary == NULL)
    {
      fprintf (stderr, "Error!\n");
      fprintf (stderr, "Unable to allocate enough memory for install floppy data file!\n");

      s = catgets (cat, 1, 0, "Press any key to continue");
      gotoxy (2, 25);
      cputs (s);

      getch();
      return (this);
    }

  dat_count = dat_read (datfile, dat_ary, dat_size);
  if (dat_count < 1)
    {
      fprintf (stderr, "Error!\n");
      fprintf (stderr, "The install floppy data file is empty!\n");

      s = catgets (cat, 1, 0, "Press any key to continue");
      gotoxy (2, 25);
      cputs (s);

      getch();
      free (dat_ary);
      return (this);
    }

  /* Run the install */

  for (i = 0; i < dat_count; i++) {
    /* Print the screen and progress bargraph */

    repaint_empty();

    box (14, 17, 66, 19);
    gotoxy (15, 18);
    bargraph (i, dat_count, 50 /* width */);

    /* Print the package name */

    gotoxy (2, 5);
    s = catgets (cat, 3, 3, "Package: ");
    cputs (s);

    cputs (dat_ary[i].name);

    /* Show the package description */

    /* Generate the lsmfile name */

    _makepath (lsmfile, "", fromdir, dat_ary[i].name, "LSM");

    if (isfile (lsmfile))
      {
	lsm_description (8, 2, 10, lsmfile);
      }
    else
      {
	/* no lsm file. try it again with a plain txt file */

	_makepath (lsmfile, "", fromdir, dat_ary[i].name, "TXT");

	if (isfile (lsmfile))
	  {
	    gotoxy (2, 8);
	    cat_file (lsmfile, 10 /* no. lines */);
	  }
      }

    /* Find out which ones the user wants to install */

    gotoxy (2, 25);
    switch (dat_ary[i].rank) {
    case 'n':
    case 'N':
      /* Do not install */

      s = catgets (cat, 4, 2, "SKIPPED");
      cputs (s);
      break;

    case 'y':
    case 'Y':
      /* Always install */

      s = catgets (cat, 4, 1, "REQUIRED");
      cputs (s);

      ret = unzip_file (dat_ary[i].name, fromdir, destdir);

      if (ret != 0) {
	/* Print an error message */

	s = catgets (cat, 3, 6, "ERROR!  Failed to install REQUIRED package.");
	cputs (s);

	/* Return an error */

	this.errors++;

	/* Does the user want to continue anyway? */

	s = catgets (cat, 2, 3, "Continue installing this disk? [yn]");
	ch = select_yn(s, "Yes", "No");

	if (ch)
	  {
	    return (this);
	  }
      }
      break;

    default:
      /* Optional */

      s = catgets (cat, 4, 0, "OPTIONAL");
      cputs (s);

      /* Ask the user if you want to install it */

      s = catgets (cat, 2, 4, "Install this package? [yn]");
      ch = select_yn (s, "Yes", "No");

      if (ch)
	{
	  ret = unzip_file (dat_ary[i].name, fromdir, destdir);

	  if (ret != 0)
	    {
	      /* Print a warning message */

	      gotoxy (2, 23);
	      s = catgets (cat, 3, 7, "WARNING!  Failed to install OPTIONAL package.");
	      cputs (s);

	      gotoxy (2, 25);
	      s = catgets (cat, 1, 0, "Press any key to continue");
	      cputs (s);

	      getch();
	      this.warnings++;
	    }
	}
      break;

    } /* switch */
  } /* for */

  /* Free memory for this disk */

  free (dat_ary);
  return (this);
}
