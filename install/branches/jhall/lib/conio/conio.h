/* conio.h */

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

#ifndef _CONIO_H
#define _CONIO_H

#include <curses.h>


/* text modes */

#define LASTMODE (-1)			/* the previous mode */
#define BW40 0				/* black and white 40 cols */
#define C40  1				/* color 40 cols */
#define BW80 2				/* black and white 80 cols */
#define C80  3				/* color 80 cols */
#define MONO 7				/* monochrome 80 cols */
#define C4350 64			/* EGA 43-lines, VGA 50-lines */


/* Define the text_info structure */

struct text_info {
  unsigned int winleft;			/* text window boundary */
  unsigned int wintop;			/* text window boundary */
  unsigned int winright;		/* text window boundary */
  unsigned int winbottom;		/* text window boundary */

  unsigned int attribute;		/* text attribute */
  unsigned int normattr;		/* normal attribute */

  unsigned int currmode;		/* the text mode */

  unsigned int screenheight;		/* no. of lines on screen */
  unsigned int screenwidth;		/* no. of cols on screen */
  unsigned int curx;			/* position in current window */
  unsigned int cury;			/* position in current window */
};


/* Functions */

/* conio.c */

void conio_init (void);
void conio_end (void);

/* interf.c */

/* io.c */

int getche (void);
int putch (int ch);

/* strio.c */

char *cgets (char *str);
int cputs (const char *s);

/* textwin.c */

void clrscr (void);
void gettextinfo (struct text_info *r);
void gotoxy (int x, int y);
void textbackground (int co);
void textcolor (int co);
int wherex (void);
int wherey (void);
void window (int left, int top, int right, int bottom);

#endif /* _CONIO_H */
