/* $Id$ */

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


#include <conio.h>
#include "getkey.h"

/* getkey() will scan a key from the keyboard using getch().  If the
 key is extended, then ret.key = 0, and the value is returned in
 ret.extended.  If the key is not extended, then ret.key returns the
 value, and ret.extended = 0. */

key_t
getkey (void)
{
  key_t ret;

  ret.key = getch();
  ret.extended = ( ret.key ? 0 : getch() );

  return (ret);
}
