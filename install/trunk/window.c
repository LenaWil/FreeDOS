/* window.c */

/* Copyright (c) 2011 Jim Hall <jhall@freedos.org> */

#include <stdio.h>
#include <string.h>

#include <conio.h>
#include <graph.h>

#include "colors.h"

#include "window.h"

void
titlebar (const char *title)
{
  int x;

  /* title bar is line 1, from cols 1-80 */

  _settextwindow (1,1 , 1,80);			/* y1,x1 , y2,x2 */

  _settextcolor (_BLACK_);
  _setbkcolor (_WHITE_);
  _clearscreen (_GWINDOW);

  /* center the title */

  x = (80 - strlen (title)) / 2;

  _settextposition (1, x+1);			/* y,x */

  cputs (title);
}

void
statusbar (const char *status)
{
  /* status bar is line 25, from cols 1-80 */

  _settextwindow (25,1 , 25,80);		/* y1,x1 , y2,x2 */

  _settextcolor (_BLACK_);
  _setbkcolor (_WHITE_);
  _clearscreen (_GWINDOW);

  _settextposition (1, 1);			/* y,x */

  cputs (status);
}
