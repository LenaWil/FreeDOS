/* Copyright (C) 1998,1999,2000,2001 Jim Hall <jhall@freedos.org> */

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

#include <string.h> /* strchr() */
#include "dir.h"    /* DIR_CHAR and mkdir() */

/* Make sure destination directory exists (or create if not)       */
/* This fixes bug, where if path does not exist then install fails */
void createdestpath(char *destdir)
{
  char *p = strchr(destdir, DIR_CHAR); /* search for 1st dir separator */
  while (p)
  {
    *p = '\0';       /* mark as temp end of string */
    mkdir(destdir);  /* create, ignore errors, as dir may exist */
    *p = DIR_CHAR;   /* restore dir separator */
    p = strchr(p+1, DIR_CHAR); /* look for next dir separator */
  }
  mkdir(destdir);    /* create last portion of dir if didn't end in slash */
}
