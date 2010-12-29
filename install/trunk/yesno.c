/* Copyright (c) 2010 Jim Hall <jhall@freedos.org> */

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

#include <conio.h>			/* getch */
#include <ctype.h>			/* toupper */
#include <string.h>			/* strchr */

#include "strchar.h"

/* yesno() */

/* display a query, and only accept Y or N */

/* returns 0 for no, any other value (1) for true */

int
yesno (char *query, char *yn, char *yes, char *no)
{
  int c;
  char *pch;

  /* assumes yn is in uppercase! */

  /* query for string */

  cprintf ("%s ", query);

  c = toupper (getch());

  while ( (pch = strchr (yn, c)) == NULL)
    {
      c = toupper (getch());
    }

  if ( ((int) pch - (int) yn) == 0 )
    {
      cprintf (yes);
      return (1);
    }
  else
    {
      cprintf (no);
      return (0);
    }
}
