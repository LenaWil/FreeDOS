/* getch_yn.c */

/* Copyright (C) 1999 Jim Hall <jhall1@isd.net> */

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

#ifdef unix
#include "conio.h"
#else
#include <conio.h>
#endif


int
getch_yn (void)
{
  /* Modification to the standard conio getch function, to only return
     'y' or 'n', case insensitive */

  int ret = '?';

  while ((ret != 'y') && (ret != 'n') &&
         (ret != 'Y') && (ret != 'N'))
    {
      ret = getch();
    }

  /* We have what we are looking for.  Return it */

  return (ret);
}
