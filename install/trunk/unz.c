/* Wrapper to the info-unzip API */

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

#ifdef __WATCOMC__
# include <direct.h>
# include <process.h>
# define FA_HIDDEN _A_HIDDEN
# define fnsplit _splitpath
# define fnmerge _makepath
# define MAXPATH _MAX_PATH
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>

int UzpMain( int argc, char **argv );	/* from InfoZip's Unzip */

#include <fcntl.h>
#include <dos.h>
#include <sys/stat.h>
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>

static FILE *pkglst;

int
unzip_file (const char *zipfile, const char *fromdir, char *destdir)
{
  char full_zipfile[MAXPATH],
       full_ziplist[MAXPATH],
       buffer[MAXPATH],
       buffer2[MAXPATH];  /* path to a zipfile */
  int ret;

#ifdef DISKFREE
  unsigned long pkgsize = 0L, dfree = 0L;
#ifndef __WATCOMC__
  struct dfree freesp;
#else
  struct diskfree_t freesp;
#endif /* __WATCOMC__ */
#endif /* DISKFREE */

  /* default argv command (0-3) */

  char *argv[4] = {"unz.c", "-qq", "-o", "full_zipfile"};

  /* create the zip file name */

  fnsplit (zipfile, NULL, NULL, buffer2, NULL);
  fnmerge (full_zipfile, "", fromdir, zipfile, ".ZIP");

  /* set 3=full_zipfile */

  argv[3] = full_zipfile;    /* pointer assignment */

#ifdef ZIPLIST
  sprintf(buffer,"%s%sPACKAGES",destdir,(destdir[strlen(destdir)-1]=='\\')?"":"\\");
  if(access(buffer, 0) != 0) {
      mkdir(buffer);
#ifdef __WATCOMC__
      _dos_setfileattr (buffer, FA_HIDDEN);
#else
      _chmod(buffer, 1, FA_HIDDEN);
#endif
  }
  fnmerge (full_ziplist, "", buffer, buffer2, ".LST");
#endif /* ZIPLIST */

#ifdef DISKFREE
  /* TODO: add test if unzipped package will exceed free disk space */

#ifdef __WATCOMC__
  _dos_getdiskfree (0, &freesp);
#else
  getdfree (0, &freesp);
#endif

  dfree = freesp.avail_clusters;
  dfree *= freesp.sectors_per_cluster;
  dfree *= freesp.bytes_per_sector;
  dfree -= 4096L; /* allow a little leeway */

  /* if (pkgsize > dfree) return 3; */
#endif /* DISKFREE */

  ret = UzpMain (4, argv);      /* the Unzip code */

  /* Done */

  return (ret);
}
