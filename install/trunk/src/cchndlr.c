/*
  Control-C handler
*/

#include <stdlib.h> /* exit(), malloc()      */
#include <stdio.h>  /* fprintf()             */
#include <signal.h>

#include <bios.h>
#include <dos.h>
#include <conio.h>   /* gettext(), puttext() */
#include <catgets.h> /* catgets()            */
#include <string.h>  /* strlen(), strcpy()   */
#include "box.h"     /* box()                */
#include "globals.h" /* cat                  */
#include "text.h"


typedef void (*sighandler)(int sig);
sighandler oldSIGINThandler = NULL;

char *msgabort = NULL;
char *msgverifyabort = NULL;
char *saveblock = NULL;
int leftx, topy, rightx, bottomy;


/********************************************************
 getbioskey from int24.c

 (C) 1990 Joel Spolsky, All Rights Reserved.
 
 This code is released into the public domain by the author.
 You can do whatever you want with it. Please let me know
 if you find it useful or if you find any bugs.
 *********************************************************/
/** getbioskey
 **
 ** get one key from the BIOS
 **
 ** Like TC bioskey(0), but doesn't nab control Break. It seems
 ** that the TC bioskey was trying to block Ctrl-Breaks, which
 ** it did by changing the Ctrl-Break handler, which it called
 ** DOS to do. This made the interrupt handler reenter DOS which
 ** is illegal.
 **/
int getbioskey(void)
{
	union REGS regs;
	struct SREGS segregs;

	segread(&segregs);
	regs.h.ah = 0;
	int86x (0x16, &regs, &regs, &segregs);
	return regs.x.ax;
}


#ifdef __BORLANDC__ || __TURBOC__
#pragma argsused
#endif
void ourSIGINThandler(int sig)
{
  register int key;

  if (saveblock != NULL)
  {
    /* save current region, then print prompt */
    gettext(leftx, topy, rightx, bottomy, saveblock);
    box(leftx, topy, rightx, bottomy);
    gotoxy(leftx+2, topy+2);
    if (msgverifyabort != NULL)
      cprintf(msgverifyabort);
    else
      cprintf(MSG_SIGINTVERIFYABORT_STR);

    /* get either a 'y'es or 'n'o */
    key_loop: do {
      key=getbioskey() & 0x00FF;
      switch(key) {
        case 'n': case 'N': 
        {
          puttext(leftx, topy, rightx, bottomy, saveblock);
          signal(SIGINT, ourSIGINThandler);
          return;
        }
        case 'y': break;
        case '\x03':  /* Ctrl-C again */
        case 'Y': { key='y'; break; }
        default: break;
      }
    } while (key!='y');
  }
 
  clrscr();
  fclose(NULL);  /* close all open files */
  if (msgabort != NULL)
    fprintf(stderr, msgabort);
  else
    fprintf(stderr, MSG_SIGINTABORT_STR);
  exit(255);
}


void registerSIGINTHandler(void)
{
  register char *s;
  int len;

  if (oldSIGINThandler == NULL)
  {
    s = catgets (cat, SET_SIGINT, MSG_SIGINTABORT, MSG_SIGINTABORT_STR);
    if ((msgabort = (char *)malloc((strlen(s)+1)*sizeof(char))) != NULL)
      strcpy(msgabort, s);
    s = catgets (cat, SET_SIGINT, MSG_SIGINTVERIFYABORT, MSG_SIGINTVERIFYABORT_STR);
    if ((msgverifyabort = (char *)malloc((strlen(s)+1)*sizeof(char))) != NULL)
      strcpy(msgverifyabort, s);
    /* ( boxborder space strlen(msgverifyabort) space boxborder ) * (char + attribute) */
    len = strlen(msgverifyabort);
    saveblock = (char *)malloc((len+5)*5*2*sizeof(char));
    leftx = 40 - (len+5)/2;
    rightx = 40 + (len+5)/2;
    topy = 10;
    bottomy = 14;
    oldSIGINThandler = signal(SIGINT, ourSIGINThandler);
  }
}

/* Reregisters handler if some other process (such as unzip) de-registers it */
void reregisterSIGINTHandler(void)
{
  if (oldSIGINThandler == NULL)
    registerSIGINTHandler();
  else
    signal(SIGINT, ourSIGINThandler);
}


void unregisterSIGINTHandler(void)
{
  if (oldSIGINThandler != NULL)
  {
    signal(SIGINT, oldSIGINThandler);
    oldSIGINThandler = NULL;
    if (msgabort) free(msgabort);
    if (msgverifyabort) free(msgverifyabort);
    if (saveblock) free(saveblock);
    msgabort = NULL;
    msgverifyabort = NULL;
    saveblock = NULL;
  }
}
