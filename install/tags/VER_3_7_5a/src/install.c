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

#define INSTALL_VERSION "3.7.5"


#include <stdio.h>
#include <stdlib.h>                 /* for malloc */
#include <string.h>                 /* for strcmp */
#include <conio.h>                  /* DOS conio */

#include "globals.h"			/* nl_catd cat language catalog */
                                    /* char *yes localized version  */
                                    /* char *no  of yes and no      */
                                    /* int nopauseflag, 0=pause,    */
                                    /* 1=don't pause ie autoinstall */
                                    /* and #include "catgets.h"     */
/* #include "catgets.h"	            /* DOS catopen(), catgets() */
#include "cat.h"                    /* for cat() */
#include "dat.h"                    /* for dat_read() */
#include "dir.h"
#include "inst.h"                   /* for inst_t */
#include "repaint.h"                /* repaint_empty() */
#include "sel_list.h"               /* select_yn() */
#include "pause.h"                  /* for pause() */
#include "cdp.h"                    /* createdestpath() */
#include "log.h"                    /* openlog(), log(), closelog() */
#include "cchndlr.h"                /* SIGINT (ctrl-c) handler */

#include "text.h"                   /* All strings displayed */


/* Functions */
inst_t install_top (dat_t *dat_ary, int dat_count);
void getLocalizedYesNo(void);


/* Globals local to this file only */
char fromdir[MAXDIR];               /* path to install from */
char destdir[MAXDIR];               /* path to install to */
int fromdirflag = 0;                /* 0=prompt user,    */
int destdirflag = 0;                /* 1=on command line */
int wantlog = 1;                    /* user wants to log activity */


/* program starts here */

int
main (int argc, char **argv)
{
  int dat_count;                    /* size of the dat array */
  int i;
  int mono = 0;                     /* running on monochrome monitor */

  dat_t *dat_ary;                   /* the dat file array */
  inst_t ret;                       /* no. of errors, warnings */

  struct text_info ti;              /* (borland) for gettextinfo */
 

  /* Open the language catalog */
  cat = catopen ("install", 0);

  /* Check command line */

  for (i = 1; i < argc; i++)
  {
    if (strcmpi(argv[i], "/mono") == 0)
      mono = 1;
    else if (strcmpi(argv[i], "/nopause") == 0)
      nopauseflag = 1;
    else if (strcmpi(argv[i], "/nolog") == 0)
      wantlog = 0;
    else if ( (strcmpi(argv[i], "/src") == 0) && (i+1 < argc))
    {
      fromdirflag = 1;
      i++;
      strcpy(fromdir+2, argv[i]);
    }
    else if ( (strcmpi(argv[i], "/dst") == 0) && (i+1 < argc))
    {
      destdirflag = 1;
      i++;
      strcpy(destdir+2, argv[i]);
    }
    else
    {
      fprintf (stderr, catgets (cat, SET_USAGE, MSG_USAGE, MSG_USAGE_STR));
      exit (1);
    }
  }

  /* unzip overwrites screen with warning if TZ not set, so check    */
  /* and if not set, then set for us to GMT0, which means no offsets */
  if (getenv("TZ") == NULL) putenv("TZ=GMT0");


  /* Read dat file */

  dat_ary = dat_read ("INSTALL.DAT", &dat_count);
  if (dat_ary == NULL)
  {
    if (dat_count > 0)
    {
      fprintf (stderr, catgets (cat, SET_ERRORS, MSG_ERRALLOCMEMDF, MSG_ERRALLOCMEMDF_STR));
      exit (2);
    }
    else /* Either error reading file, eg no file, or file has no entries */
    {
      fprintf (stderr, catgets (cat, SET_ERRORS, MSG_ERREMPTYDATAFILE, MSG_ERREMPTYDATAFILE_STR));
      exit (3);
    }
  }

  /* Get localized "Yes" and "No" strings */
  getLocalizedYesNo();

  /* register our SIGINT handler (Ctrl-C) */
  registerSIGINTHandler();

  /* Start the install */

  /* save current setting so we can restore them */
  gettextinfo (&ti);

  /* setup screen colors then draw the screen */
  if (mono)
  {
    textbackground (BLACK);
    textcolor (LIGHTGRAY);
  }
  else
  {
    textbackground (BLUE);
    textcolor (WHITE);
  }

  repaint_empty();
  gotoxy (2, 3);
  cat_file ("COPYR", 18);
  pause();

  repaint_empty();
  gotoxy (2, 3);
  cat_file ("OEM", 18);
  pause();

  ret.errors = 0;
  ret.warnings = 0;

  ret = install_top (dat_ary, dat_count);

  /* Finished with install */

  textattr (ti.attribute); /* restore user's screen */
  clrscr();

  if ((ret.errors == 0) && (ret.warnings == 0))
      printf (catgets (cat, SET_GENERAL, MSG_INSTALLOK, MSG_INSTALLOK_STR));
  else
      printf (catgets (cat, SET_GENERAL, MSG_INSTALLERRORS, MSG_INSTALLERRORS_STR), ret.errors, ret.warnings);

  /* Done */

  free (dat_ary);
  free (yes);
  free (no);
  catclose (cat);

  /* restore original SIGINT handler */
  unregisterSIGINTHandler();

  return (0);
}

inst_t
install_top (dat_t *dat_ary, int dat_count)
{
  /* Top-level piece for the install program.  Determines what disk
     sets the user wants to install, then installs them. */

  char *s;                          /* used for retrieving localized text */
  char txtfile[MAXPATH];            /* name of text descr file */
  int ch;
  int i;
  inst_t ret;                       /* return: no. of errors,warnings */
  inst_t this;                      /* no. of errors,warnings */

  /* Where to install from, to */

  if (!fromdirflag || !destdirflag) /* only prompt if one or both are missing */
  do {

    /* get directories from user */

    repaint_empty();

    fromdir[0] = MAXDIR;		/* max length of the string */
    destdir[0] = MAXDIR;		/* max length of the string */

    s = catgets (cat, SET_PROMPT_LOC, MSG_INSTALLFROM, MSG_INSTALLFROM_STR);
    gotoxy (5, 10);
    cputs (s);

    gotoxy (5, 12);
    if (fromdirflag)
      cputs(&(fromdir[2]));
    else
      cgets (fromdir);

    s = catgets (cat, SET_PROMPT_LOC, MSG_INSTALLTO, MSG_INSTALLTO_STR);
    gotoxy (5, 16);
    cputs (s);
      
    gotoxy (5, 18);
    if (destdirflag)
      cputs(&(destdir[2]));
    else
      cgets (destdir);

    /* let user verify */

    repaint_empty();
    gotoxy (5, 10);
    cputs (catgets (cat, SET_PROMPT_LOC, MSG_WILLINSTALLFROM, MSG_WILLINSTALLFROM_STR));
    gotoxy (5, 11);
    cputs (&fromdir[2]);
    gotoxy (5, 13);
    cputs (catgets (cat, SET_PROMPT_LOC, MSG_WILLINSTALLTO, MSG_WILLINSTALLTO_STR));
    gotoxy (5, 14);
    cputs (&destdir[2]);
      
    gotoxy (5, 16);
    s = catgets (cat, SET_PROMPT_LOC, MSG_INSTALLDIROK, MSG_INSTALLDIROK_STR);
    ch = select_yn (s, yes, no, NULL, NULL);
  } while (ch);

  /* Make sure destination directory exists (or create if not) */
  createdestpath(&destdir[2]);

  /* open the log file, if user wants it */
  if (wantlog)
  {
    if ((i = openlog(&destdir[2], "INSTALL.LOG")) != 0)
    {
      repaint_empty();
      gotoxy (5, 10);
      cputs (catgets (cat, SET_PROMPT_LOC, MSG_WILLINSTALLTO, MSG_WILLINSTALLTO_STR));
      gotoxy (5, 11);
      cputs (&destdir[2]);
      gotoxy (5, 13);
      fprintf(stderr, catgets(cat, SET_ERRORS, MSG_ERRCREATELOG, MSG_ERRCREATELOG_STR), i);
      pause();
    }
    /* note: if log is not opened, then calls to log immediately 
       return, so if (wantlog) log(...) wrapper is not needed for calls to log() */
    log("<install ver=\"%s\" logformat=\"1.0\" >\n", INSTALL_VERSION);
    log("<source path=\"%s\" />\n", &fromdir[2]);
    log("<dest path=\"%s\" />\n\n", &destdir[2]);
  }

  /* Ask to install every disk set */

  log("<installsets>\n");
  for (i = 0; i < dat_count; i++)
    {
      repaint_empty();
      gotoxy (2, 5);
      s = catgets (cat, SET_PKG_GENERAL, MSG_DISKSET, MSG_DISKSET_STR);
      cputs (s);
      cputs (dat_ary[i].name);
      log("<diskset name=\"%s\" ", dat_ary[i].name);

      /* create the txt file name */

      gotoxy (2, 10);
      cat_file (dat_ary[i].name, 10 /* no. lines */);

      gotoxy (2, 6);
      switch (dat_ary[i].rank)
	{
	case 'Y':
	case 'y':
	  s = catgets (cat, SET_PKG_NEED, MSG_REQUIRED, MSG_REQUIRED_STR);
	  cputs (s);
        log("choice=\"y\" />\n");
	  pause();
	  break;

	case 'N':
	case 'n':
	  s = catgets (cat, SET_PKG_NEED, MSG_SKIPPED, MSG_SKIPPED_STR);
	  cputs (s);
        log("choice=\"n\" />\n");
	  /* don't need to pause for this */
	  break;

	default:
	  s = catgets (cat, SET_PKG_NEED, MSG_OPTIONAL, MSG_OPTIONAL_STR);
	  cputs (s);

	  s = catgets (cat, SET_PROMPT_YN, MSG_INSTALLSETYN, MSG_INSTALLSETYN_STR);
	  ch = select_yn (s, yes, no, NULL, NULL);

	  switch (ch)
	    {
	    case 0:
	      dat_ary[i].rank = 'y';
            log("choice=\"y\" />\n");
	      break;

	    default:
	      dat_ary[i].rank = 'n';
            log("choice=\"n\" />\n");
	      break;
	    } /* switch ch */
	  break;
	} /* switch rank */
    } /* for i */
  log("</installsets>\n\n");

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


  /* close the log file, if user wanted it */
  log("</install>\n");
  closelog();

  /* Done */
  return (ret);
}


/* Get localized "Yes" and "No" strings for yes/no prompts */
void getLocalizedYesNo(void)
{
  char *s;

  s = catgets (cat, SET_PROMPT_YN, MSG_YES, MSG_YES_STR);
  yes = (char *)malloc((strlen(s)+1)*sizeof(char));
  if (yes == NULL)
  {
    s = catgets(cat, SET_ERRORS, MSG_ERRORALLOCMEM, MSG_ERRORALLOCMEM_STR);
    fprintf(stderr, s);
    exit (4);
  }
  strcpy(yes, s);

  s = catgets (cat, SET_PROMPT_YN, MSG_NO, MSG_NO_STR);
  no = (char *)malloc((strlen(s)+1)*sizeof(char));
  if (no == NULL)
  {
    s = catgets(cat, SET_ERRORS, MSG_ERRORALLOCMEM, MSG_ERRORALLOCMEM_STR);
    fprintf(stderr, s);
    exit (5);
  }
  strcpy(no, s);
}
