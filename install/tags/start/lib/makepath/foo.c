/* foo.c */

/* a test program for the _makepath() library of functions */

#include <stdio.h>

#include "makepath.h"


int
main (void)
{
  char path[_MAX_PATH];
  char drive[_MAX_DRIVE];
  char dir[_MAX_DIR];
  char name[_MAX_FNAME];
  char ext[_MAX_EXT];

  /* create a new path */

  printf ("creating new path from: C:, /home/jhall, foo, c\n");
  _makepath (path, "C:", "/home/jhall", "foo", "c");
  printf ("new path is %s\n", path);

  /* split the path */

  printf ("--\n");
  printf ("splitting path from %s\n", path);
  _splitpath (path, drive, dir, name, ext);
  printf ("components are %s, %s, %s, %s\n", drive, dir, name, ext);

  /* Create the path again */

  printf ("--\n");
  printf ("re-creating path from %s, %s, %s, %s\n", drive, dir, name, ext);
  _makepath (path, drive, dir, name, ext);
  printf ("new path is %s\n", path);

  /* Create path with name that contains ext */

  printf ("--\n");
  printf ("*creating path from C:, /home/jhall, filename.ext, \"\"\n");
  _makepath (path, "C:", "/home/jhall", "filename.ext", "");
  printf ("*new path is %s\n", path);

  /* Invalid drive test */

  printf ("--\n");
  printf ("**creating invalid drive path from DR:, /home/jhall/, file, ext\n");
  _makepath (path, "DR:", "/home/jhall", "file", "ext");
  printf ("**new path is %s\n", path);

  /* Leading slash test */

  printf ("--\n");
  printf ("creating path with no leading slash C:, home/jhall, foo, c\n");
  _makepath (path, "C:", "home/jhall", "foo", "c");
  printf ("new path is %s\n", path);
  _splitpath (path, drive, dir, name, ext);
  printf ("components are %s, %s, %s, %s\n", drive, dir, name, ext);

  exit (0);
}
