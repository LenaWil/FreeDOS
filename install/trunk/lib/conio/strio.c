/* strio.c */

/* A compatibility library that mimics DOS conio library API, instead
   using UNIX curses. */

/* Copyright (C) 1998 Jim Hall, <jhall1@isd.net> */

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

#include <curses.h>
#include <string.h>				/* for strlen */
#include "conio.h"


/* Functions to implement */

/* STRING INPUT/OUTPUT ROUTINES */

/* cgets */

char *
cgets (char *str)
{
  /* This really should be capable of line wrap, but that's hard, so
     I'll just fake it with a wrapped call to getstr */

  char *p;
  int maxlen;
  int len;

  /* Set variables */

  maxlen = str[0];
  p = &str[2];

  /* Wrap a call to curses getstr */

  echo();					/* curses */
  getstr (p);					/* curses */
  noecho();					/* curses */
  len = strlen (p);

  /* Check length */

  if (len > maxlen)
    {
      /* WARNING!  THIS MIGHT INDICATE AN OVERFLOW! */
      p[maxlen] = '\0';
    }

  /* Return */

  str[1] = len;
  return (p);
}

/* cprintf */

/* cputs */

int
cputs (const char *s)
{
  int i;
  int ch;

  /* Use putch to add characters from the string */

  for (i = 0; s[i] != '\0'; i++)
    {
      ch = s[i];
      putch (ch);
    }

  /* As a hack, we won't refresh the screen unless we really need to.
     So we'll refresh the screen with cputs, but not with putch */

  refresh();					/* curses */

  /* Returns the last character printed */

  return (ch);
}

/* cscanf */

/* getpass */
