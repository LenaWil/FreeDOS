/* conio.c */

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


/* Globals (only to the conio functions) */

struct text_info conio_ti;


/* Functions to start and stop conio mode under UNIX curses */

/* Not actually part of conio, but required under UNIX curses */

void
conio_init (void)
{
  /* initialize curses */

  initscr();
  cbreak();
  noecho();
  nonl();
  intrflush(stdscr, FALSE);
  keypad(stdscr, TRUE);

  /* initialize conio_ti */

  conio_ti.winleft = 1;
  conio_ti.wintop = 1;
  conio_ti.winright = COLS;
  conio_ti.winbottom = LINES;

  conio_ti.attribute = 0;		/* fake it */
  conio_ti.normattr = 0;		/* fake it */

  conio_ti.currmode = BW80;		/* fake it */

  conio_ti.screenheight = LINES;
  conio_ti.screenwidth = COLS;
  conio_ti.curx = 1;
  conio_ti.cury = 1;

  /* home the cursor */

  move (0, 0);				/* curses */
}

void
conio_end (void)
{
  endwin();
}
