/* install.c */

/* Copyright (C) 1998-1999 Jim Hall <jhall1@isd.net> */

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

#ifdef unix
#include "conio.h"			/* DOS Conio */
#else /* dos */
#include <conio.h>			/* DOS Conio */
#endif /* unix */

#ifdef unix
#include <nl_types.h>			/* UNIX catopen, catgets */
#else /* dos */
#include "catgets.h"			/* DOS catopen, catgets */
#endif

#ifdef unix
#include "makepath.h"			/* UNIX _makepath(), _splitpath() */
#endif /* unix */

#include "cat.h"			/* for cat_file */
#include "dat.h"			/* for dat_read */
#include "getch_yn.h"			/* for getch_yn */
#include "inst.h"			/* for inst_t */
#include "repaint.h"


/* Symbolic constants */

#define DAT_FILE "install.dat"		/* top-level dat file */
#define OEM_FILE "oem.txt"		/* text file for vendors */

#define VERSION "3.5"			/* the install program version */


/* Functions */

void usage (void);
inst_t install_top (dat_t *dat_ary, int dat_count);


/* Globals (what a hack!) */

nl_catd cat;				/* language catalog */


/* program starts here */

int
main (int argc, char **argv)
{
  char *s;
  int dat_count;			/* size of the dat array */
  dat_t *dat_ary;			/* the dat file array */
  inst_t ret;				/* no. of errors, warnings */

  /* CHANGE THIS VALUE AS NEEDED! */
  int dat_size = 10;			/* malloc size of the dat array */

#ifndef unix
  /* dos */
  struct text_info ti;			/* (borland) for gettextinfo */
#endif

  /* Open the language catalog */

  cat = catopen ("install", 0);

  /* Check command line */

  if (argc != 1)
    {
      usage ();
      exit (1);
    }

  /* Read dat file */

  dat_ary = malloc (sizeof (dat_t) * dat_size);
  if (dat_ary == NULL)
    {
      fprintf (stderr, "Error!\n");
      fprintf (stderr, "Unable to allocate memory for install data file!\n");
      fprintf (stderr, "The file was %s\n", DAT_FILE);
      exit (2);
    }

  dat_count = dat_read (DAT_FILE, dat_ary, dat_size);
  if (dat_count < 1)
    {
      fprintf (stderr, "Error!\n");
      fprintf (stderr, "The install data file is empty!\n");
      fprintf (stderr, "The file was %s\n", DAT_FILE);
      free (dat_ary);
      exit (3);
    }

  /* Say hello */

  s = catgets (cat, 0, 0, "Install version %s, Copyright (C) 1998-2000 Jim Hall\n");
  printf (s, VERSION);
  s = catgets (cat, 0, 0, "This is free software, and you are welcome to redistribute it\n");
  printf (s);
  s = catgets (cat, 0, 0, "under certain conditions; see the file COPYING for details.\n");
  printf (s);
  s = catgets (cat, 0, 0, "Install comes with ABSOLUTELY NO WARRANTY\n");
  printf (s);

  /* immediately start the program */

#ifdef unix
  conio_init();
#endif

#ifndef unix
  /* dos */
  gettextinfo (&ti);
  textbackground (BLUE);
  textcolor (WHITE);
#endif

  repaint_empty();

  /* show the OEM file */

  cat_file (OEM_FILE, 10 /* start line */, 10 /* no. lines */);

  gotoxy (1, 25);
  s = catgets (cat, 1, 0, "Press any key to continue");
  cputs (s);
  getch();

  /* Start the install */

  ret.errors = 0;
  ret.warnings = 0;
  ret = install_top (dat_ary, dat_count);

  /* say goodbye */

#ifndef unix
  /* dos */
  textattr (ti.attribute);
#endif

  /* Clear the screen */
  clrscr();

#ifdef unix
  conio_end();
#endif

  free (dat_ary);

  /* Print results */

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

  /* Close the catalog */

  catclose (cat);

  /* Done */

  exit (0);
}

void
usage (void)
{
  fprintf (stderr, "USAGE:  install\n\n");
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

  /* Say hello */

  repaint_empty();
  gotoxy (1, 3);
  s = catgets (cat, 3, 1, "In top-level install");
  cputs (s);

  /* Where to install from, to */

  fromdir[0] = _MAX_DIR;		/* max length of the string */
  destdir[0] = _MAX_DIR;		/* max length of the string */

  gotoxy (5, 10);
  s = catgets (cat, 1, 1, "Where are the install files? (where to install from?)");
  cputs (s);

  gotoxy (5, 12);
  cgets (fromdir);

  gotoxy (5, 16);
  s = catgets (cat, 1, 2, "Where will files be installed? (where to install to?)");
  cputs (s);
      
  gotoxy (5, 18);
  cgets (destdir);

  gotoxy (1, 25);
  s = catgets (cat, 1, 0, "Press any key to continue");
  cputs (s);
  getch();

  /* Ask to install every disk set */

  for (i = 0; i < dat_count; i++)
    {
      repaint_empty();
      gotoxy (1, 5);
      s = catgets (cat, 3, 0, "Disk set:");
      cputs (s);
      cputs (dat_ary[i].name);

      /* create the txt file name */

      /* usage is _makepath(newpath, drive, dir, name, ext) */

      _makepath (txtfile, "", "", dat_ary[i].name, "TXT");
      cat_file (txtfile, 10 /* start line */, 10 /* no. lines */);

      gotoxy (1, 6);
      switch (dat_ary[i].rank)
	{
	case 'Y':
	case 'y':
	  s = catgets (cat, 4, 1, "REQUIRED");
	  cputs (s);

	  gotoxy (1, 25);
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

	  gotoxy (1, 25);
	  s = catgets (cat, 2, 2, "Do you want to install this disk set? [yn] ");
	  cputs (s);
	  ch = getch_yn();

	  switch (ch)
	    {
	    case 'Y':
	    case 'y':
	      dat_ary[i].rank = 'y';
	      cputs ("Y");
	      break;

	    default:
	      dat_ary[i].rank = 'n';
	      cputs ("N");
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
