/* Copyright (C) 1998,2000 Jim Hall <jhall@freedos.org> */

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


#include <stdlib.h>			/* getenv(), NULL */
#include <stdio.h>
#include <string.h>			/* strcat(), strlen(), strcpy() */
#include <conio.h>
#include "dir.h"			/* DIR_CHAR == '\\' or '/' */

#include "cat.h"

/* Symbolic constants */

#define COLS 80				/* 80 columns on the screen */


/* similar to cputs, except expands tabs and converts \n to \r\n */
/* I need to use a hack here instead of cputs(s) because we
   appear to be printing a circle (ASCII 09h) instead of blank
   space. It's not a perfect hack, but it works. */
void cat_cputs(const char *msg)
{
  /* Use putch to add characters from the string */

  while (*msg)
  {
    switch (*msg)
    {
#ifndef unix
      case '\n':
             cputs("\r\n");
             break;
#endif 
      case '\t':
            cputs ("        ");		/* print spaces instead */
            break;
      default:
            putch (*msg);
    } /* switch ch */

    msg++;

  } /* while (*msg), ie while not end of string */
}


/* displays maxlines of file given by filename, if maxlines is -1 prints whole file
   trys (in this order)
   %LANG%\filename.TXT
   %LANG%\filename
   filename.%LANG%
   if %LANG% in format "aa-BB", such as "en-UK" then try
   {
     aa\filename.TXT   (ie .\en\filename.TXT)
     aa\filename
     filename.aa       (ie .\filename.en )
   }
   filename.TXT
   filename
   repeat above search, except instead of current dir, use each path in %NLSPATH%
   if still not found then return failure (-1)
   if found then returns number of lines printed
*/
int
cat_file (const char *filename, int maxlines)
{
  /* Display a file using conio.  Returns 0 if failed, or non-zero if
     successful.  Only shows up to maxlines of the file. */

  /* The programmer is responsible for positioning the cursor (usually
     at 1,1) before making a call to cat_file.  Be careful that the
     text from the file does not overrun the right boundary of the
     screen. */

  FILE *stream;				/* file to read */
  char s[COLS+1];				/* string read from the file */
  int start_x, start_y;			/* starting coordinates */
  int nlines;				/* number of lines printed so far */
  int i;

  /* Get the starting coordinates */

  start_x = wherex();
  start_y = wherey();

  /* open the file */
  if ((stream = langfopen(filename, "rt")) == NULL)
  {
    /* failed to open the file */
    return (-1);
  }

  /* Display the contents to the screen */

  nlines = 0;

  while ((fgets (s, COLS, stream) != NULL) && ((maxlines < 0) || (nlines < maxlines)))
    {
      gotoxy (start_x, start_y + nlines);

      /* ensure null terminated string */
      s[COLS] = '\0';

      /* initially cputs, then changed to handle tabs, now use cat_cputs */
      cat_cputs(s);

      nlines++;
    } /* while */

  /* Done */

  fclose (stream);
  return (nlines);
}


FILE * trypath(const char *flags, const char *path, const char *name, const char *ext)
{
  char buffer[MAXPATH];			/* full path to try */

  fnmerge (buffer, NULL, path, name, ext);
  return fopen(buffer, flags);
}

char *strcatpath(char *buffer, const char *initialpath, const char *morepath)
{
  register int len;
  if (initialpath == NULL || !*initialpath)
  {
    if (morepath == NULL || !*morepath)
    {
      strcpy(buffer, "");
    }
    else
    {
      strcpy(buffer, morepath);
    }
  }
  else
  {
    if (morepath == NULL || !*morepath)
    {
      strcpy(buffer, initialpath);
    }
    else
    {
      strcpy(buffer, initialpath);
      len = strlen(buffer); /* guarenteed to be >= 1 by above tests */
      if (initialpath[len-1] != DIR_CHAR)
      {
        buffer[len] = DIR_CHAR;
        buffer[len+1] = '\0';
      }
      strcat(buffer, morepath);
    }
  }

  return buffer;
}

FILE * tryvariations(const char *flags, const char *basepath, const char *name, const char *lang, const char *lang_2)
{
  FILE *stream;
  char buffer[MAXPATH];			/* full path to try */

  
  /* basepath\%LANG%\filename.TXT */
  strcatpath(buffer, basepath, lang);
  if ((stream = trypath(flags, buffer, name, ".TXT")) != NULL) return stream;

  /* basepath\%LANG%\filename */
  if ((stream = trypath(flags, buffer, name, NULL)) != NULL) return stream;

  /* basepath\filename.%LANG% */
  if ((stream = trypath(flags, basepath, name, lang)) != NULL) return stream;
  
  /*
   if %LANG% in format "aa-BB", such as "en-UK" then try
   {
     basepath\aa\filename.TXT   (ie .\en\filename.TXT)
     basepath\aa\filename       (ie .\en\filename)
     basepath\filename.aa       (ie .\filename.en )
   }
  */
  if (strlen(lang) > 2)
  {
    strcatpath(buffer, basepath, lang_2);
    if ((stream = trypath(flags, buffer, name, ".TXT")) != NULL) return stream;
    if ((stream = trypath(flags, buffer, name, NULL)) != NULL) return stream;
    if ((stream = trypath(flags, basepath, name, lang_2)) != NULL) return stream;  
  }

  /* .\filename.TXT */
  if ((stream = trypath(flags, basepath, name, ".TXT")) != NULL) return stream;  

  /* .\filename */
  if ((stream = trypath(flags, basepath, name, NULL)) != NULL) return stream;  

  /* Failed to open any of the variants, return failure */
  return NULL;
}


/* This function based on catopen from Jim Hall's catgets */
FILE *langfopen(const char *name, const char *flags)
{
  FILE *stream;
  char nlspath[MAXPATH];		/* value of %NLSPATH% */
  char *nlsptr;				/* ptr to NLSPATH */
  char *lang;                           /* ptr to LANG */
  char lang_2[3];                       /* 2-char version of %LANG% */
  char *tok;                            /* pointer when using strtok */

#ifdef PATHSPECIFIED
  if (strchr (name, DIR_CHAR))
    {
      /* first approximation: 'name' is a filename */
      return fopen(name, flags);
    }
#endif


  /* We will need the value of LANG, and may need a 2-letter abbrev of
     LANG later on, so get it now. */

  lang = getenv ("LANG");

  /* If LANG env var is not set, default to english */
  if (lang == NULL)
    lang = "EN";

  strncpy (lang_2, lang, 2);
  lang_2[2] = '\0';


  /* first search relative to current directory (or root if name starts with DIR_CHAR) */
  if ((stream = tryvariations(flags, "", name, lang, lang_2)) != NULL) return stream;

  /* repeat above search, except instead of current dir based, use each path in %NLSPATH% */
  /* step through NLSPATH */

  nlsptr = getenv ("NLSPATH");

  if (nlsptr == NULL)
    {
      /* Return failure - we won't be able to locate the cat file */
      return NULL;
    }

  strcpy (nlspath, nlsptr);

  tok = strtok (nlspath, ";");
  while (tok != NULL)
    {
      if ((stream = tryvariations(flags, tok, name, lang, lang_2)) != NULL) return stream;

      /* Grab next tok for the next while iteration */
      tok = strtok (NULL, ";");
    } /* while tok */

  /* We could not find it.  Return failure. */
  return NULL;
}


#ifdef CAT_FILE_PROGRAM
int main(int argc, char *argv[])
{
  int nlines = -1;

  if (argc > 3 || argc < 2)
  {
    printf("USAGE: cat_file <filename> [<nlines>]\n"
           "where <filename> usually does not specify an extension\n"
           "      <nlines> is number of lines from top to display\n"
           "Tries to open the file using the following search pattern:\n"
           "   %LANG%\\<filename>.TXT\n"
           "   %LANG%\\<filename>\n"
           "   <filename>.%LANG%\n"
           "   if %LANG% in format \"aa-BB\", such as \"en-UK\" then try\n"
           "   {\n"
           "     aa\\<filename>.TXT   (ie en\\filename.TXT)\n"
           "     aa\\<filename>       (ie en\\filename\n"
           "     <filename>.aa       (ie filename.en )\n"
           "   }\n"
           "   <filename>.TXT\n"
           "   <filename>\n"
           "   \n"
           "   Repeat above search, except instead of .,\n"
           "     use each ; separated path in %NLSPATH%\n"
           "   If still not found then prints an error message indicating not found.\n"
    );
    return 1;
  }

  if (argc > 2)
    nlines = atoi(argv[2]);

  if (cat_file(argv[1], nlines) == -1)
  {
    printf("ERROR: unable to open %s\n", argv[1]);
    return 1;
  }
  return 0;
}

#endif /* CAT_FILE_PROGRAM */
