/* $Id$ */

/* Functions for reading and dealing with the DAT file. */

/* Copyright (C) 1998-1999 Jim Hall, jhall1@isd.net */

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
#include <string.h>				/* for strtok */
#include "dat.h"

/* Symbolic constants */

#define STRLEN 80				/* size of a string */


int
dat_fread (FILE *stream, dat_t *dat_ary, int dat_size)
{
  /* reads a dat file from a stream, into an array pointed to by dat_ary,
     of max size dat_size.  Returns the number of elements in the
     array (0 may indicate an error.) */

  char str[STRLEN];				/* for reading the line */
  char *s;					/* temporary pointer */
  int count;					/* counter for ary size */

  /* Read the file until end of file */

  count = 0;

  while (fgets (str, STRLEN, stream) != NULL) {
    /* Break up the string into tokens */

    s = strtok (str, " :");

    if (s == NULL)
      {
	strncpy (dat_ary[count].name, "", 1);
      }

    else
      {
	strncpy (dat_ary[count].name, s, DAT_NAME_LEN);
      }

    s = strtok (NULL, " :");

    if (s == NULL)
      {
	dat_ary[count].rank = '?';
      }

    else
      {
	dat_ary[count].rank = s[0];
      }
    
    /* Check on the length of the array */

    count++;

    if (count >= dat_size) {
      /* The array is maxed out - give up. */

      return (dat_size);
    }
  } /* while */

  /* Return */

  return (count);
}

int
dat_read (const char *filename, dat_t *dat_ary, int dat_size)
{
  /* reads a dat file, into an array pointed to by dat_ary, of max
     size dat_size.  Returns the number of elements in the array (0
     may indicate an error.) */

  FILE *stream;
  int ret;

  /* Open the file */

  stream = fopen (filename, "r");
  if (stream == NULL) {
    /* Failed */

    return (0);
  }

  /* Read the file */

  ret = dat_fread (stream, dat_ary, dat_size);

  /* Close the file, and quit */

  fclose (stream);
  return (ret);
}
