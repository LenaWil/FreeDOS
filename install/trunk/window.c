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

#include <conio.h>
#include <graph.h>

#include "colors.h"


#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))


/* progressbar() */

/* display a simple 50-column progress bar on the screen. */

void
progressbar (int completed, int total)
{
  int cols;
  int rows;
  int pct;
  int i;
  char tmpstr[51];

  /* avoid over/underrun */

  completed = MAX (completed, 0);	/* completed should be 0-total */
  completed = MIN (completed, total);

  /* how many columns should we fill in? the progress bar will be 50
     columns wide by default. */

  pct = 100 * completed / total;

  pct = MAX (pct, 0);			/* should be 0 - 100 */
  pct = MIN (pct, 100);

  cols = 50 * completed / total;

  cols = MAX (cols, 0);			/* should be 0 - 50 */
  cols  = MIN (cols, 50);

  /* display area. the window starts at col 15 because the screen is
     80 cols, the bar will be 50 cols; 80 - 50 = 30; 30 / 2 = 15 */

  _settextwindow (10, 15, 20, 80);

  /* initialize/clear the screen only the first time, i.e. when the
     "completed" is zero */

  if (completed == 0)
    {
      _settextcolor (_YELLOW_);
      _setbkcolor (_BLUE_);
      _clearscreen (_GWINDOW);
    }

  /* draw the progress bar */

  for (i = 0; i < cols; i++)
    {
      tmpstr[i] = 219;
      /* putch (219);			/* filled square */
    }

  for (i = cols; i < 50; i++)
    {
      tmpstr[i] = 176;
      /* putch (176);			/* empty box */
    }

  tmpstr[50] = '\0';

  /* print the progress bar */

  for (rows = 1; rows <= 3; rows++)
    {
      _settextposition (rows, 1);      	/* relative to window */
      cputs (tmpstr);
    }

  _settextposition (3, 51);
  cprintf (" %d%% ", pct);
}

/* titlebar() */

/* display a message in the top title bar */

void
titlebar (char *title)
{
  int xpos;

  _settextwindow (1, 1, 1, 80);

  _settextcolor (_BLACK_);
  _setbkcolor (_WHITE_);
  _clearscreen (_GWINDOW);

  xpos = (80 - strlen (title)) / 2;

  _settextposition (1, xpos+1);		/* y,x starting at 1,1 */

  cprintf ("%s", title);
}

/* statusbar() */

/* display a message in the bottom status bar */

void
statusbar (char *status)
{
  _settextwindow (25, 1, 25, 80);

  _settextcolor (_BLACK_);
  _setbkcolor (_WHITE_);
  _clearscreen (_GWINDOW);

  _settextposition (1, 1);		/* relative to window */

  cprintf ("%s", status);
}
