/* test.c */

/* Tests line-wrap during conio */

#include <stdio.h>
#include <stdlib.h>			/* (Borland) for exit */
#include <string.h>			/* for strlen */

#ifdef unix
#include "conio.h"
#else
#include <conio.h>
#endif

int
main (void)
{
  char str[20];
  char *p;

#ifdef unix
  conio_init();
#endif

  /* read 17 chars, and a null terminator */

  str[0] = 18;

  /* Read a string at the end of a line */

  clrscr();

  gotoxy (78, 1);
  cputs ("This text should wrap");

  gotoxy (1, 5);
  cputs ("Enter a string:");

  gotoxy (1, 6);
  p = cgets (str);

  gotoxy (1, 19);
  cputs (p);

  gotoxy (1, 20);
  cputs ("Press any key to quit");
  getch();
  clrscr();

#ifdef unix
  conio_end();
#endif

  /* Print some results */

  printf ("%s = %d chars\n", p, strlen(p));
  printf ("str[0] = %d\n", str[0]);
  printf ("str[1] = %d\n", str[1]);
  exit (0);
}
