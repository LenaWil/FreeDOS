/****************************************************************************
   catpath.c - This file implements the functions catpath and slicepath    
   Written by Jeremy Davis <jeremyd@computer.org>                          
   February 2001                                                           

Copyright information: released to Public Domain [United States Definition]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE COPYRIGHT HOLDERS NOR AUTHORS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

****************************************************************************/


/* 
  Note: catpath and slicepath are inverses of each other,
  i.e. the result of one can be directly used for the other.
  Neither validates the path.


  char * catpath(char *path, 
                 const char *drive, 
                 const char *dir,
                 const char *fname,
                 const char *ext);

  This function will concatenate the drive, directory, file name, 
  and extension placing the results in path.  This function is a 
  replacement for the Microsoft and Borland functions makepath and fnmerge.

  The path argument should point to a buffer that is large enough
  to hold all the (non NULL) components and a terminating '\0'.
  The constant CP_MAXPATH (currently = 260) may be used when allocating
  memory for path.  If path is NULL then CP_MAXPATH memory is allocated
  using malloc and returned, otherwise path is the return value.
  If path is given as NULL, then the memory referenced by the pointer
  returned must later be released using free or memory leaks will occur.
  Note: NULL will be returned on any error (such as path given as
  NULL and not enough free memory for automatic allocation).

  The drive argument specifies the drive letter, with or without a colon (:),
  or it specifies a UNC name in the form of \\servername\share, with or
  without a terminating slash. 
  Note: When the drive refers to a UNC name, then dir always begins at the
  root path of the specified drive.  Additionally when a UNC name is used
  and dir is NULL, dir is assumed to be a single slash representing the
  root directory. 
  i.e. \\myserver\share + NULL + f + ext = \\myserver\share\f.ext
  The drive argument may be NULL, in which case no drive is specified 
  in the resulting path.

  The dir argument specifies an optional directory path.  If may be
  a relative path, include either (or both) forward (/) and backward (\)
  slashes, and may or may not end in a slash.  If dir is NULL then 
  no path is appended (except in the case of a UNC drive where the
  path will default to the root directory).
  Note: if drive is a UNC path, then even if dir does not begin with
  an initial slash (indicating absolute path from root) the resulting
  path is still relative to the root (so same as if initial slash was
  given).

  If fname is not NULL then it is appended to drive and path.
  Note: if path (or drive when path is NULL) ends in a slash and fname 
  begins with a slash, only one slash will appear between the two.
  i.e. C: + somepath\ + \file + ext = C:somepath\file.ext

  If ext is not NULL, then it is appended to the end of path.  If ext does
  not begin with a dot (.) then one is appended to path before ext is
  appended.  If drive, dir, and fname are NULL and ext is not NULL the
  result is the extension (with initial dot) i.e. NULL + ext = .ext

  If drive, dir, fname, and ext are all NULL then the path returned
  is simply an empty string ("").



  void slicepath(const char *path, 
                 char *drive, 
                 char *dir,
                 char *fname,
                 char *ext);

  This function will take a full path and split it into the corresponding
  components.  Any component not in the full path will be set to an empty
  string ("").  Any component not needed may be passed in as NULL.
  Each component must be large enough to hold the corresponding component;
  CP_MAXPATH may be used when allocating memory to ensure large enough.

  path specifies the full path.  If path is NULL then all non-NULL 
  components are set to an empty string ("").

  The drive argument is set to the drive given in path, including the
  colon (:).  If path specifies a UNC path (begins with two slashes \\),
  the drive is set to the server name (including initial two slashes)
  and share name without an ending slash.  Note: the server name is
  considered all text between the initial two slashes up to the next
  slash in path, and share name is considered all text between the
  slash marking the end of the server name up to but not including the
  following slash.  i.e. \\server\share\dir = \\server\share + \dir

  The dir argument is the directory component as is set to all text 
  begining immediately after the drive specifier up to and including the 
  last slash in path.  If there is only a single slash after the drive 
  then dir refers to the root directory (\), and if there are no slashes 
  after the drive then dir is blank ("").

  The fname argument is the base file name as is set to all text following 
  the directory (right after the last slash in path or immediately after 
  the drive) up to but not including the final dot (.) in path.  If there 
  is not a dot in path, then fname will contain the remainder of path.


  The ext argument contains the extension, beginning with and including
  the last dot up to the end of path.

*/

#include "catpath.h"  /* prototypes & CP_MAXPATH */
#include <stdlib.h>   /* malloc                  */
#include <string.h>   /* strcpy & strlen         */


/* FSLASH is the forward slash path separator */
#define FSLASH '/'
/* BSLASH is the backslash path separator */
#define BSLASH '\\'

/* SLASHCHAR is the separator used when combining components */
#define SLASHCHAR BSLASH


char * catpath(char *path, 
               const char *drive, 
               const char *dir,
               const char *fname,
               const char *ext)
{
  register char *pathptr = path;
  register int uncflag = 0;

  /* allocate memory if path is NULL */
  if (pathptr == NULL)
  {
    if ((pathptr = (char *)malloc(CP_MAXPATH * sizeof(char))) == NULL)
      return NULL;
    else
      path = pathptr;
  }

  /* initialize to empty string */
  *pathptr = '\0';

  /* add drive */
  if (drive != NULL && *drive)
  {
    /* determine is drive letter or UNC path */
    if (*drive == BSLASH || *drive == FSLASH)
    {
      strcpy(pathptr, drive);
      pathptr += strlen(drive);
      uncflag = 1;
    }
    else
    {
      *pathptr = *drive;
      pathptr++;
      *pathptr = ':';
      pathptr++;
    }
  }

  /* add dir */
  if (dir != NULL && *dir)
  {
    if (uncflag) /* make sure a slash appears between share and dir */
    {
      if ( (*dir != BSLASH && *dir != FSLASH) &&
           (*(pathptr-1) != BSLASH && *(pathptr-1) != FSLASH) )
      {
        *pathptr = SLASHCHAR;
        pathptr++;
      }
    }

    strcpy(pathptr, dir);        /* copy the dir into path    */
    pathptr += strlen(dir) - 1;  /* -1 so points to last char */

    /* make sure path ends in a slash */
    if (*pathptr != BSLASH && *pathptr != FSLASH)
    {
      pathptr++;
      *pathptr = SLASHCHAR;
    }

    pathptr++;
  }

  /* add fname */
  if (fname != NULL && *fname)
  {
    /* default to root dir if UNC and no dir given */
    if (uncflag && (dir == NULL || !*dir)) 
    {
      *pathptr = '\\';
      pathptr++;
    }

    /* if file name starts with slash and path ends in slash don't add */
    if ( (*fname == BSLASH || *fname == FSLASH) &&
         (*(pathptr-1) == BSLASH || *(pathptr-1) == FSLASH) )
      fname++;

    strcpy(pathptr, fname);
    pathptr += strlen(fname);
  }

  /* add extension */
  if (ext != NULL && *ext)
  {
    /* if extension does not begin with a dot, add one 1st */
    if (*ext != '.')
    {
      *pathptr = '.';
      pathptr++;
    }

    strcpy(pathptr, ext);
    pathptr += strlen(ext);
  }

  /* ensure properly terminated (ends with '\0') */
  *pathptr = '\0';

  return path;
}


void slicepath(const char *path, 
               char *drive, 
               char *dir,
               char *fname,
               char *ext)
{
  register const char *pathptr = path;
  register const char *endptr, *altptr;
  register int i;

  /* initialize all to empty strings */
  if (drive != NULL)
    *drive = '\0';
  if (dir != NULL)
    *dir = '\0';
  if (fname != NULL)
    *fname = '\0';
  if (ext != NULL)
    *ext = '\0';

  /* if no full path given then done */
  if (pathptr == NULL || !*pathptr)
    return;

  /* determine if drive or UNC & copy if wanted */
  if (*(pathptr+1) == ':') /* if drive letter */
  {
    if (drive != NULL)     /* if want copied  */
    {
      *drive = *pathptr;
      *(drive+1) = ':';
      *(drive+2) = '\0';
    }
    pathptr += 2;         /* skip over drive */
  }
  else if ( (*pathptr == BSLASH || *pathptr == FSLASH) &&
            (*(pathptr+1) == BSLASH || *(pathptr+1) == FSLASH) )
  {
    if (drive != NULL)  /* unc path, copy it? */
    {
      i = 0; /* copy all text until end or 4 slashes \\X\X\ */
      do
      {
        if (*pathptr == BSLASH || *pathptr == FSLASH)
          i++;
        *drive = *pathptr;
        drive++;
        pathptr++;
      }
      while (*pathptr && (i < 4));

      if (!*pathptr) /* if end of path, mark end of drive & done */
      {
        *drive = '\0';
        return; 
      }
      else                   /* setup for copying dir */
      {
        *(drive - 1) = '\0'; /* remove trailing slash & mark end */
        pathptr--;           /* move back to slash */
      }
    }
    else /* just skip drive */
    {
      pathptr += 2;
      while (*pathptr && (*pathptr != BSLASH && *pathptr != FSLASH))
        pathptr++;
      if (!*pathptr) return; /* if end of path then done */
      pathptr++;
      while (*pathptr && (*pathptr != BSLASH && *pathptr != FSLASH))
        pathptr++;
    }
  }

  /* check for end */
  if (!*pathptr) return;

  /* mark start of extenstion / end of fname */
  endptr = pathptr + strlen(pathptr) - 1;
  while ( (endptr >= pathptr) &&
          (*endptr != '.') &&
          (*endptr != BSLASH) && (*endptr != FSLASH) )
    endptr--;

  if (endptr < pathptr)     /* all left is fname */
  {
    if (fname != NULL) strcpy(fname, pathptr);
    return;
  }
  else if (*endptr != '.') /* dir and fname only */
  {
    endptr++;   /* make endptr point after slash */
    if (dir != NULL) /* copy dir, include slash  */
    {
      do
      {
        *dir = *pathptr;
        dir++;
        pathptr++;
      } while (pathptr != endptr);
      *dir = '\0';
    }
    if (fname != NULL)             /* copy fname */
      strcpy(fname, endptr);
  }
  else /* dir, fname, and ext                    */
  {
    /* copy the extension including .            */
    if (ext != NULL) strcpy(ext, endptr);

    /* path is const so can't embed '\0' in it   */
    altptr = endptr; /* mark end of file name    */

    /* find either end of dir or end of drive    */
    while ( (endptr >= pathptr) && 
            (*endptr != BSLASH) && (*endptr != FSLASH) )
      endptr--;

    if (endptr < pathptr)    /* only fname left  */
    {
      if (fname != NULL)     /* so copy it       */
      {
        while (pathptr != altptr)
        {
          *fname = *pathptr;
          fname++;
          pathptr++;
        }
        *fname = '\0';
      }
    }
    else /* dir and fname left                   */
    {
      endptr++; /* increment so just after slash */

      /* copy dir first so endptr marks its end  */
      if (dir != NULL)
      {
        while (pathptr != endptr)
        {
          *dir = *pathptr;
          dir++;
          pathptr++;
        }
        *dir = '\0';
      }

      /* now copy fname, it ends at altptr       */
      if (fname != NULL)
      {
        while (endptr != altptr)
        {
          *fname = *endptr;
          fname++;
          endptr++;
        }
        *fname = '\0';
      }
    }
  }
}


/** The rest of this file is a demonstration/test. **/

/* #define CATPATH_DEMO_TEST  */
#ifdef CATPATH_DEMO_TEST

/*#include "catpath.h" */
#include <stdlib.h>
#include <stdio.h>


void printSplitPrintCatPrint(char * fullpath)
{
  char *path;
  char drive[CP_MAXPATH],
       dir[CP_MAXPATH],
       fname[CP_MAXPATH],
       ext[CP_MAXPATH];

  printf("Full path         = %s\n", fullpath);
  slicepath(fullpath, drive, dir, fname, ext);
  printf("sliced path = [%s] [%s] [%s] [%s]\n", drive, dir, fname, ext);
  path = catpath(NULL, drive, dir, fname, ext);
  printf("concatenated path = %s\n", path);
  free(path);
  printf("\n");
}

void catWithNULLDir(char * fullpath)
{
  char path[CP_MAXPATH];
  char drive[CP_MAXPATH],
       dir[CP_MAXPATH],
       fname[CP_MAXPATH],
       ext[CP_MAXPATH];

  printf("Full path          =  %s\n", fullpath);
  slicepath(fullpath, drive, dir, fname, ext);
  printf("sliced path = [%s] [%s] [%s] [%s]\n", drive, dir, fname, ext);
  catpath(path, drive, NULL, fname, ext);
  printf("cat path minus dir = [%s]\n", path);
  printf("\n");
}

int main(void)
{
  char *p1 = "C:/some dir\\anotherdir/filename.old.ext",
       *p2 = "C:name.ext",
       *p3 = "directory\\",
       *p4 = "filename",
       *p5 = "filename.ext",
       *p6 = "C:.extension",
       *p7 = "//servername\\share",
       *p8 = "\\\\servername\\share\\dir1\\dir2\\a dir\\file.name.extension",
       *p9 = "\\\\server\\share\\fname.ext";

  printf("catpath/slicepath demo\n\n");

  printSplitPrintCatPrint(p1);
  printSplitPrintCatPrint(p2);
  printSplitPrintCatPrint(p3);
  printSplitPrintCatPrint(p4);
  printSplitPrintCatPrint(p5);
  printSplitPrintCatPrint(p6);
  printSplitPrintCatPrint(p7);
  printSplitPrintCatPrint(p8);
  printSplitPrintCatPrint(p9);

  catWithNULLDir(p9);
  catWithNULLDir(p8);
  catWithNULLDir(p7);
  catWithNULLDir(p6);
  catWithNULLDir(p5);
  catWithNULLDir(p4);
  catWithNULLDir(p3);
  catWithNULLDir(p2);
  catWithNULLDir(p1);

  printf("done.\n");

  return 0;
}

/* The output from above program,
   note that when using UNC names and dir==NULL a dir of root is assumed.

catpath/slicepath demo

Full path         = C:/some dir\anotherdir/filename.old.ext
sliced path = [C:] [/some dir\anotherdir/] [filename.old] [.ext]
concatenated path = C:/some dir\anotherdir/filename.old.ext

Full path         = C:name.ext
sliced path = [C:] [] [name] [.ext]
concatenated path = C:name.ext

Full path         = directory\
sliced path = [] [directory\] [] []
concatenated path = directory\

Full path         = filename
sliced path = [] [] [filename] []
concatenated path = filename

Full path         = filename.ext
sliced path = [] [] [filename] [.ext]
concatenated path = filename.ext

Full path         = C:.extension
sliced path = [C:] [] [] [.extension]
concatenated path = C:.extension

Full path         = //servername\share
sliced path = [//servername\share] [] [] []
concatenated path = //servername\share

Full path         = \\servername\share\dir1\dir2\a dir\file.name.extension
sliced path = [\\servername\share] [\dir1\dir2\a dir\] [file.name] [.extension]
concatenated path = \\servername\share\dir1\dir2\a dir\file.name.extension

Full path         = \\server\share\fname.ext
sliced path = [\\server\share] [\] [fname] [.ext]
concatenated path = \\server\share\fname.ext

Full path          =  \\server\share\fname.ext
sliced path = [\\server\share] [\] [fname] [.ext]
cat path minus dir = [\\server\share\fname.ext]

Full path          =  \\servername\share\dir1\dir2\a dir\file.name.extension
sliced path = [\\servername\share] [\dir1\dir2\a dir\] [file.name] [.extension]
cat path minus dir = [\\servername\share\file.name.extension]

Full path          =  //servername\share
sliced path = [//servername\share] [] [] []
cat path minus dir = [//servername\share]

Full path          =  C:.extension
sliced path = [C:] [] [] [.extension]
cat path minus dir = [C:.extension]

Full path          =  filename.ext
sliced path = [] [] [filename] [.ext]
cat path minus dir = [filename.ext]

Full path          =  filename
sliced path = [] [] [filename] []
cat path minus dir = [filename]

Full path          =  directory\
sliced path = [] [directory\] [] []
cat path minus dir = []

Full path          =  C:name.ext
sliced path = [C:] [] [name] [.ext]
cat path minus dir = [C:name.ext]

Full path          =  C:/some dir\anotherdir/filename.old.ext
sliced path = [C:] [/some dir\anotherdir/] [filename.old] [.ext]
cat path minus dir = [C:filename.old.ext]

done.
*/

#endif
