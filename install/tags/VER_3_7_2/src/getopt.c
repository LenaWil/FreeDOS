/*
  I found this code on simtel, I think.  There was no copyright and no
  author's name on the source file.  So I am assuming it was placed
  under the public domain.  To protect this code, I am placing it
  under the GNU GPL.  -jhall1
*/

/*
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to:
    The Free Software Foundation, Inc.,
    59 Temple Place - Suite 330,
    Boston, MA  02111-1307,
    USA.  */

/*
 * getopt - get option letter from argv
 */

#include <stdio.h>
#include <string.h>			/* for strcmp(), strchr() */

char    *optarg;        /* Global argument pointer. */
int     optind = 0;     /* Global argv index. */

static char     *scan = NULL;   /* Private scan pointer. */

#if 0 /* disabled code */
extern char     *index();
#endif /* disabled code */


int
getopt(argc, argv, optstring)
int argc;
char *argv[];
char *optstring;
{
        register char c;
        register char *place;

        optarg = NULL;

        if (scan == NULL || *scan == '\0') {
                if (optind == 0)
                        optind++;
        
                if (optind >= argc || argv[optind][0] != '-' || argv[optind][1] == '\0')
                        return(EOF);
                if (strcmp(argv[optind], "--")==0) {
                        optind++;
                        return(EOF);
                }
        
                scan = argv[optind]+1;
                optind++;
        }

        c = *scan++;
        place = strchr(optstring, c);

        if (place == NULL || c == ':') {
                fprintf(stderr, "%s: unknown option -%c\n", argv[0], c);
                return('?');
        }

        place++;
        if (*place == ':') {
                if (*scan != '\0') {
                        optarg = scan;
                        scan = NULL;
                } else if (optind < argc) {
                        optarg = argv[optind];
                        optind++;
                } else {
                        fprintf(stderr, "%s: -%c argument missing\n", argv[0], c);
                        return('?');
                }
        }

        return(c);
}
