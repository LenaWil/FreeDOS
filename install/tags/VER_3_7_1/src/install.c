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
#include <stdlib.h>			/* for malloc */
#include <string.h>			/* for strcmp */
#include <conio.h>			/* DOS conio */

#include "catgets.h"			/* DOS catopen(), catgets() */
#include "cat.h"			/* for cat() */
#include "dat.h"			/* for dat_read() */
#include "inst.h"			/* for inst_t */
#include "repaint.h"			/* repaint_empty() */
#include "sel_list.h"			/* select_yn() */

/* Functions */

inst_t install_top (dat_t *dat_ary, int dat_count);

/* Globals */

nl_catd cat;				/* language catalog */

/* program starts here */

int
main (int argc, char **argv)
{
  char *s;
  int dat_count;			/* size of the dat array */
  int dat_size = 10;			/* malloc size of the dat array */
  int i;

  dat_t *dat_ary;			/* the dat file array */
  inst_t ret;				/* no. of errors, warnings */

  struct text_info ti;			/* (borland) for gettextinfo */

  /* Open the language catalog */

  cat = catopen ("install", 0);

  /* Check command line */

  if (argc != 1)
    {
      fprintf (stderr, "USAGE:  install\n\n");
      exit (1);
    }

  /* Read dat file */

  dat_ary = malloc (sizeof (dat_t) * dat_size);
  if (dat_ary == NULL)
    {
      fprintf (stderr, "Unable to allocate memory for install data file!\n");
      exit (2);
    }

  dat_count = dat_read ("INSTALL.DAT", dat_ary, dat_size);
  if (dat_count < 1)
    {
      fprintf (stderr, "The install data file is empty!\n");
      free (dat_ary);
      exit (3);
    }

  /* Start the install */

  gettextinfo (&ti);
  textbackground (BLUE);
  textcolor (WHITE);

  repaint_empty();
  gotoxy (2, 3);
  cat_file ("COPYR.TXT", 10);
  s = catgets (cat, 1, 0, "Press any key to continue");
  gotoxy (2, 25);
  cputs (s);
  getch();

  repaint_empty();
  gotoxy (2, 3);
  cat_file ("OEM.TXT", 10);
  s = catgets (cat, 1, 0, "Press any key to continue");
  gotoxy (2, 25);
  cputs (s);
  getch();

  ret.errors = 0;
  ret.warnings = 0;

  ret = install_top (dat_ary, dat_count);

  /* Finished with install */

  textattr (ti.attribute);
  clrscr();

  if ((ret.errors == 0) && (ret.warnings == 0))
    {
      s = catgets (cat, 0, 4, "The Install program completed successfully.\n");
      printf (s);
    }

  else
    {
      s = catgets (cat, 0, 5, "There were %u errors and %u non-fatal warnings.\n");
      printf (s, ret.errors, ret.warnings);
    }

  /* Done */

  free (dat_ary);
  catclose (cat);
  exit (0);
}

inst_t
install_top (dat_t *dat_ary, int dat_count)
{
  /* Top-level piece for the install program.  Determines what disk
     sets the user wants to install, then installs them. */

  char *s;
  char txtfile[_MAX_PATH];		/* name of text descr file */
  char fromdir[_MAX_DIR];		/* path to install from */
  char destdir[_MAX_DIR];		/* path to install to */
  int ch;
  int i;
  inst_t ret;				/* return: no. of errors,warnings */
  inst_t this;				/* no. of errors,warnings */

  /* Where to install from, to */

  repaint_empty();

  fromdir[0] = _MAX_DIR;		/* max length of the string */
  destdir[0] = _MAX_DIR;		/* max length of the string */

  s = catgets (cat, 1, 1, "Where are the install files? (where to install from?)");
  gotoxy (5, 10);
  cputs (s);

  gotoxy (5, 12);
  cgets (fromdir);

  s = catgets (cat, 1, 2, "Where will files be installed? (where to install to?)");
  gotoxy (5, 16);
  cputs (s);
      
  gotoxy (5, 18);
  cgets (destdir);

  s = catgets (cat, 1, 0, "Press any key to continue");
  gotoxy (2, 25);
  cputs (s);

  getch();

  /* Ask to install every disk set */

  for (i = 0; i < dat_count; i++)
    {
      repaint_empty();
      gotoxy (2, 5);
      s = catgets (cat, 3, 0, "Disk set:");
      cputs (s);
      cputs (dat_ary[i].name);

      /* create the txt file name */

      /* usage is _makepath(newpath, drive, dir, name, ext) */

      _makepath (txtfile, "", "", dat_ary[i].name, "TXT");

      gotoxy (2, 10);
      cat_file (txtfile, 10 /* no. lines */);

      gotoxy (2, 6);
      switch (dat_ary[i].rank)
	{
	case 'Y':
	case 'y':
	  s = catgets (cat, 4, 1, "REQUIRED");
	  cputs (s);

	  gotoxy (2, 25);
	  s = catgets (cat, 1, 0, "Press any key to continue");
	  cputs (s);
	  getch();
	  break;

	case 'N':
	case 'n':
	  s = catgets (cat, 4, 2, "SKIPPED");
	  cputs (s);
	  /* don't need to pause for this */
	  break;

	default:
	  s = catgets (cat, 4, 0, "OPTIONAL");
	  cputs (s);

	  s = catgets (cat, 2, 2, "Do you want to install this disk set? [yn] ");
	  ch = select_yn (s, "Yes", "No");

	  switch (ch)
	    {
	    case 0:
	      dat_ary[i].rank = 'y';
	      break;

	    default:
	      dat_ary[i].rank = 'n';
	      break;
	    } /* switch ch */
	  break;
	} /* switch rank */
    } /* for i */

  /* Now install the selected disk sets */

  ret.errors = 0;
  ret.warnings = 0;

  for (i = 0; i < dat_count; i++)
    {
      switch (dat_ary[i].rank)
	{
	case 'Y':
	case 'y':
	  this = set_install (dat_ary[i].name, &fromdir[2], &destdir[2]);
	  ret.errors += this.errors;
	  ret.warnings += this.warnings;
	  break;
	} /* switch */
    } /* for i */

  /* Done */

  return (ret);
}
