/* $Id$ */

/* Copyright (C) 1998,2000 Jim Hall <jhall@freedos.org> */

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
#include <conio.h>

/* Symbolic constants */

#define MAXLEN 80


int
cat_file (const char *filename, int y0, int maxlines)
{
  /* Display a file using conio.  Returns 0 if failed, or non-zero if
     successful.  Only shows up to maxlines of the file. */

  FILE *stream;
  char s[MAXLEN];
  int nlines;
  int i;

  /* open the file */

  stream = fopen (filename, "r");

  if (stream == NULL)
    {
      /* failed to open the file */
      return (0);
    }

  /* Display the contents to the screen */

  nlines = 0;

  while ((fgets (s, MAXLEN, stream) != NULL) && (nlines < maxlines))
    {
      gotoxy (1, y0 + nlines);
      /* Use putch to add characters from the string */

      /* I need to use a hack here instead of cputs(s) because we
         appear to be printing a circle (ASCII 09h) instead of blank
         space. It's not a perfect hack, but it works. */

      /* The following should be replaced by cputs(s); */

      for (i = 0; s[i] != '\0'; i++)
	{
 	  switch (s[i])
 	    {
 	    case '\t':
 	      cputs ("        ");		/* print spaces instead */
 	      break;
 	    default:
 	      putch (s[i]);
 	    } /* switch ch */
	} /* for i */

      nlines++;
    } /* while */

  /* Done */

  fclose (stream);
  return (nlines);
}
