/* $Id$ */

/* A function to present a list to the user, and allow the user to
 select items from that list.  We use a very simple interface here,
 only allowing up/down arrow keys, and Enter.  A single char indicator
 replaces a "highlight". */

/*
   Copyright (C) 2000 Jim Hall <jhall@freedos.org>

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
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


/* Returns the index in optv chosen by the user. */

/* No screen bounds checking is done here.  It is the responsibility
 of the programmer to ensure that the list does not exceed the screen
 boundaries.  If you want to have a pretty border around this list,
 you have to do that yourself before calling this function. */

/* The programmer must position the cursor before calling select_list */

/* We will turn off the cursor here, and will set to normal cursor
 when we exit.  This may or may not affect the appearance of the rest
 of your program. */


#include <conio.h>			/* console i/o */
#include "getkey.h"


/* Additional key definitions */

#define KEY_ENTER 13

/* Additional character defs */

#define ACS_RARROW 26  /* '>' */
#define SPACE      ' '


int
select_list (int optc, char *optv[])
{
  int start_x, start_y;			/* starting coordinates */
  int i;
  key_t key;

  /* Initialize starting coordinates */

  start_x = wherex();
  start_y = wherey();

  /* Print the options, indented by 2 */

  for (i = 0; i < optc; i++)
    {
      gotoxy (start_x + 2, start_y + i);
      cputs (optv[i]);
    }

  /* Position the pointer, and allow up/down until Enter */

  _setcursortype (_NOCURSOR);

  i = 0;
  do {
    gotoxy (start_x, start_y + i);
    putch (ACS_RARROW);

    gotoxy (start_x, start_y + i);
    key = getkey();
    putch (SPACE);

    switch (key.extended)
      {
      case KEY_UP:
	i = ( i == 0 ? 0 : i-1 );
	break;

      case KEY_DOWN:
	i = ( i == optc-1 ? optc-1 : i+1 );
	break;
      }
  } while (key.key != KEY_ENTER);

  _setcursortype (_NORMALCURSOR);

  /* Return the selection */

  return (i);
}

int
select_yn (char *prompt, char *yes, char *no)
{
  int len;
  int ret;
  int x0, x1;
  char *yesno[2];

  /* Draw a box */

  len = strlen (prompt);

  x0 = 40 - (len / 2) - 1;
  if (x0 < 1)
    {
      x0 = 1;
    }

  x1 = 40 + (len / 2) + 1;
  if (x1 > 80)
    {
      x1 = 80;
    }

  box (x0, 20, x1, 25);

  /* Display the prompt, and do the y/n selection */

  gotoxy (x0 + 1, 21);
  cputs (prompt);

  yesno[0] = yes;
  yesno[1] = no;

  gotoxy (40, 22);
  ret = select_list (2, yesno);

  return (ret);
}
