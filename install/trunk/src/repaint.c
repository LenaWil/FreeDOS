/* $Id$ */

/* Copyright (C) 1999,2000 Jim Hall <jhall@freedos.org> */

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

#include "box.h"

void
repaint_empty (void)
{
  /* Clear the screen and repaint an empty screen for the installer */

  clrscr();

  gotoxy (2, 1);
  cputs ("FreeDOS Install");

  box (1, 2, 80, 24);
}
