/* install.c */

/* copyright (c) 2010 Jim Hall <jhall@freedos.org> */

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

#include <conio.h>			/* getch, .. */
#include <direct.h>			/* getcwd, .. */
#include <dos.h>			/* findfirst/findnext */
#include <graph.h>			/* _settextcolor, .. */
#include <string.h>			/* strncpy, strchr .. */

#include "colors.h"
#include "strchar.h"
#include "window.h"
#include "yesno.h"

#include "kitten/kitten.h"

/* function prototypes */

void install (char *fromdir, char *destdir, int all, int src);
void install_abort (char *message);

/* defs */

typedef struct
{
  int dirnum;
  char *fname;
} path_t;

#define FNAME_STRLEN 13			/* FILENAME.EXTz = 13 */
#define FILELIST_INCR 300

#define STRLEN 128			/* default string length */
#define DESTDIR "C:\\FDOS"		/* default install dir */

/* main */

int
main (int argc, char **argv)
{
  /* a program to install FreeDOS 1.1 and later */

  /* assumes that another pre-install process has ensured that drive C
     exists and has been formatted. */

  int i;
  int all = 0;
  int src = 0;

  char cwd[STRLEN];
  char fromdir[STRLEN];
  char destdir[STRLEN];

  char yn[] = "YN";
  char yes[] = "YES";
  char no[] = "NO";

  short fg;
  long bg;

  /* remember current settings */

  getcwd (&cwd, 128);

  fg = _gettextcolor ();
  bg = _getbkcolor ();

  /* check usage */

  for (i = 1; i < argc; i++)
    {
      if (strchar (argv[i], '/') != -1)
	{
	  fprintf (stderr, "Option characters not allowed [%s]\n", argv[i]);
	  usage ();
	  exit (1);
	}
    }

  if (argc > 3)
    {
      fprintf (stderr, "Too many arguments!\n");
      usage ();
      exit (1);
    }

  else if (argc == 3)
    {
      /* install {source} {dest} */
      strncpy (fromdir, argv[1], STRLEN);
      strncpy (destdir, argv[2], STRLEN);
    }

  else if (argc == 2)
    {
      /* install {dest} */
      strncpy (fromdir, cwd, STRLEN);
      strncpy (destdir, argv[1], STRLEN);
    }

  else if (argc == 1)
    {
      /* install */
      strncpy (fromdir, cwd, STRLEN);
      strncpy (destdir, DESTDIR, STRLEN);
    }

  /* does fromdir exist? does dest drive exist? */

  /* open language catalog */

  kittenopen ("install");

  /* start program */

  _settextcolor (_WHITE_);
  _setbkcolor (_BLUE_);
  _clearscreen (_GCLEARSCREEN);

  titlebar ("FREEDOS INSTALL");
  statusbar (" ");

  /* where are we installing from, to? */

  _settextwindow (2, 1, 24, 80);

  _settextcolor (_WHITE_);
  _setbkcolor (_BLUE_);
  _clearscreen (_GWINDOW);

  _settextposition (5, 5);		/* relative to window */
  cprintf ("Installing from: %s", fromdir);

  _settextposition (6, 5);		/* relative to window */
  cprintf ("Installing to: %s", destdir);

  /* install everything? */

  _settextposition (10, 15);		/* relative to window */
  all = yesno ("Do you want to install everything? (y/n)", yn, yes, no);

  /* install source code? */

  _settextposition (12, 15);		/* relative to window */
  src = yesno ("Do you want to install source code too? (y/n)", yn, yes, no);

  /* are you sure? */

  _settextposition (14, 15);		/* relative to window */
  cprintf ("Press any key to begin ...");
  getch ();

  /* do the install */

  _displaycursor (_GCURSOROFF);
  install (fromdir, destdir, all, src);
  _displaycursor (_GCURSORON);

  progressbar (1, 1);
  statusbar ("Done!");
  sleep (1);

  /* close language catalog */

  kittenclose ();

  /* reset, and quit */

  chdir (cwd);

  _settextcolor (fg);
  _setbkcolor (bg);
  _clearscreen (_GCLEARSCREEN);

  exit (0);
}


/* usage() */

void
usage (void)
{
  fprintf (stderr, "usage:\n");
  fprintf (stderr, "\tINSTALL\n");
  fprintf (stderr, "\tINSTALL {dest}\n");
  fprintf (stderr, "\tINSTALL {source} {dest}\n");
}

/* install() */

/* do-all function that performs the actuall install */

void
install (char *fromdir, char *destdir, int all, int src)
{
  int i;
  int done;
  struct find_t ffblk;

  char *dirlist[] = { "base.dir", "boot.dir", "devel.dir", "edit.dir", "gui.dir", "net.dir", "sound.dir", "util.dir" };
  int dirlist_count = 8;

  path_t *filelist;
  int filelist_size;
  int filelist_count;

  /* build install list */

  statusbar ("Building install list ...");

  /* loop through all of dirlist, and read entries into filelist */

  if (!all)
    {
      /* if not installing everything, just install base */
      dirlist_count = 1;
    }

  /* allocate memory for filelist */

  filelist_size = FILELIST_INCR;
  filelist_count = 0;

  if ( ! (filelist = (path_t *) malloc (filelist_size * sizeof (path_t))) )
    {
      install_abort ("filelist malloc");
    }

  /* read files in each dir */

  for (i = 0; i < dirlist_count; i++)
    {
      chdir (dirlist[i]);
      done = _dos_findfirst ("*.ZIP", _A_NORMAL, &ffblk);

      while (!done)
        {
          /* allocate memory for strings */

          if ( ! (filelist[filelist_count].fname = (char *) malloc (FNAME_STRLEN * sizeof (char))) )
	    {
	      install_abort ("fname malloc");
	    }

          /* copy strings */

          filelist[filelist_count].dirnum = i;
          strncpy (filelist[filelist_count].fname, ffblk.name, FNAME_STRLEN);
          filelist_count++;

          /* check size */

          if (filelist_count == filelist_size)
            {
              filelist_size += FILELIST_INCR;

              if ( ! (filelist = (path_t *) realloc (filelist, filelist_size * sizeof (path_t))) )
		{
		  install_abort ("filelist realloc");
		}
            }

          /* next */

          done = _dos_findnext (&ffblk);
        }

      chdir ("..");
    }

  /* install */

  statusbar ("Installing ...");

  for (i = 0; i < filelist_count; i++)
    {
      progressbar (i, filelist_count);

      _settextposition (14, 15);		/* relative to window */
      cprintf ("%s            ", filelist[i].fname);

      sleep (1);				/* just testing, fake the unzip */      
    }

  _settextposition (14, 15);			/* relative to window */
  cprintf ("            ");

}

void
install_abort (char *message)
{
  fprintf (stderr, "*** error allocating memory [%s]\n", message);
  fprintf (stderr, "*** INSTALL ABORTED - NO FILES INSTALLED\n");
  exit (1);
}
