/* io.c */

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
#include "conio.h"


/* Globals */

extern struct text_info conio_ti;		/* from conio.c */


/* Functions to implement */

/* INPUT/OUTPUT ROUTINES */

/* getch */
/* not needed.  curses already has a getch function */

/* getche */

int
getche (void)
{
  /* Read a key, then print it */

  int ch;

  ch = getch();
  putch (ch);

  /* As a hack, we won't refresh the screen unless we really need to.
     So we'll refresh the screen with getche, but not with putch */

  refresh();					/* curses */
  return (ch);
}

/* kbhit */

/* putch */

int
putch (int ch)
{
  /* Add a character.  Remember to wrap if we move beyond the edge of
     the window */

#if 0 /* disabled code */
  /* I think I have a serious bug here, so I'm disabling this block to
     see if the problem is how I handle putch */

  /* yup!  by disabling this code block, my regular conio code seems
     to work!  it used to stop printing characters after a certain
     number of characters were displayed.  I'll leave this turned off
     for now and fix it later.  -jh 1/6/00 */

  if (conio_ti.curx > conio_ti.winright)
    {
      conio_ti.cury++;
      conio_ti.curx = 0;
      gotoxy (conio_ti.curx, conio_ti.cury);
    }

  if (conio_ti.cury > conio_ti.winbottom)
    {
      return (EOF);
    }
#endif /* disabled code */

  addch (ch);					/* curses */

  /* As a hack, we won't refresh the screen unless we really need to.
     So we'll refresh the screen with getche, but not with putch */

  /* refresh();					/* curses */

  /* Increment the position */

  conio_ti.curx++;
  return (ch);
}

/* _setcursortype */

/* ungetch */
