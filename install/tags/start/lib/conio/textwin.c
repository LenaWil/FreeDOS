/* textwin.c */

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

/* TEXT WINDOW ROUTINES */

/* clreol */

/* clrscr */

void
clrscr (void)
{
  /* If full screen, use curses clear screen.  Otherwise, manually erase */

  int i, j;

  /* Check if full screen, use curses */

  if ((conio_ti.winleft == 1) &&(conio_ti.winright == COLS) &&
      (conio_ti.wintop == 1) &&(conio_ti.winbottom == LINES))
    {
      erase();					/* curses */
    }

  /* Not full screen, so fake it */

  else
    {
      /* Erase the window */

      for (j = conio_ti.wintop; j < conio_ti.winbottom; j++)
	{
	  gotoxy (j, conio_ti.winleft);

	  for (i = conio_ti.winleft; i < conio_ti.winright; i++)
	    {
	      addch (' ');			/* curses */
	    }
	} /* for j */
    } /* else, not full screen */

  /* Update the curses window */

  gotoxy (conio_ti.winleft, conio_ti.wintop);
  refresh();					/* curses */
}

/* delline */

/* insline */

/* gettext */

/* gettextinfo */

void
gettextinfo (struct text_info *r)
{
  /* Copy the conio_ti structure to r */

  r->winleft = conio_ti.winleft;
  r->wintop = conio_ti.wintop;
  r->winright = conio_ti.winright;
  r->winbottom = conio_ti.winbottom;
	      
  r->attribute = conio_ti.attribute;
  r->normattr = conio_ti.normattr;
	      
  r->currmode = conio_ti.currmode;
	      
  r->screenheight = conio_ti.screenheight;
  r->screenwidth = conio_ti.screenwidth;
  r->curx = conio_ti.curx;
  r->cury = conio_ti.cury;
}

/* gotoxy */

void
gotoxy (int x, int y)
{
  /* Remember that conio counts 1...n, but curses counts 0...[n-1]
     so adjust both x and y by minus 1 */

  move (y - 1, x - 1);				/* curses */
}

/* highvideo */

/* lowvideo */

/* normvideo */

/* movetext */

/* puttext */

/* setcursortype */

/* textattr */

/* textbackground */

void
textbackground (int co)
{
  /* Fake it */
}

/* textcolor */

void
textcolor (int co)
{
  /* Fake it */
}

/* textmode */

/* wherex */

int
wherex (void)
{
  /* Return current x position. */

  /* Reposition the cursor */

  if (conio_ti.curx > conio_ti.winright)
    {
      conio_ti.cury++;
      conio_ti.curx = 0;
      gotoxy (conio_ti.curx, conio_ti.cury);
    }

  return (conio_ti.curx);
}

/* wherey */

int
wherey (void)
{
  /* Return current y position. */

  /* Reposition the cursor */

  if (conio_ti.curx > conio_ti.winright)
    {
      conio_ti.cury++;
      conio_ti.curx = 0;
      gotoxy (conio_ti.curx, conio_ti.cury);
    }

  return (conio_ti.cury);
}

/* window */

void
window (int left, int top, int right, int bottom)
{
  /* Define a new window.  First check bounds. */

  if ((left >= 1) && (right <= conio_ti.screenwidth) &&
      (top >= 1) && (bottom <= conio_ti.screenheight))
    {
      conio_ti.winleft = left;
      conio_ti.wintop = top;
      conio_ti.winright = right;
      conio_ti.winbottom = bottom;
    }

  /* If out of bounds, ignore */
}
