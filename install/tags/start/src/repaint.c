/* $Id$ */

/* Copyright (C) 1999 Jim Hall, jhall1@isd.net */

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
#endif /* unix */


void
repaint_empty (void)
{
  /* Clear the screen and repaint an empty screen for the installer */

  clrscr();

  gotoxy (1, 1);
  cputs ("FreeDOS Install");
  gotoxy (1, 2);
  cputs ("___________________________________________________________________________");

  gotoxy (1, 24);
  cputs ("___________________________________________________________________________");
}
