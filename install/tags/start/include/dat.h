/* dat.h - For reading and dealing with the DAT file. */

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


#if !defined (_DAT_H)
#define _DAT_H

#include "dat_t.h"


/* Functions */

int
dat_fread (FILE *stream, dat_t *dat_ary, int dat_size);
int
dat_read (const char *filename, dat_t *dat_ary, int dat_size);

#endif /* _DAT_H */
