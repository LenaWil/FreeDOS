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

/* strchar() */

/* like strchr, but returns an int (instead of a pointer) giving the
   character's location in the string */

int
strchar (char *str, int ch)
{
  int i;

  /* find a match for ch in str */

  for (i = 0; str[i]; i++)
    {
      if (str[i] == ch)
	{
	  return (i);
	}
    }

  /* failed to find a match, return -1 */

  return (-1);
}
