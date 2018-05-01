/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

    /* INPfindLev(line)
     *      find the 'level' parameter on the given line and return its
     *      value (1,2,or 3 for now, 1 default)
     */

#include "spice.h"
#include "misc.h"
#include "strext.h"
#include <stdio.h>
#include "inpdefs.h"
#include "util.h"
#include "suffix.h"

char *
INPfindLev(line,level)
    char *line;
    int *level;
{
    char *where;

    where = line;

    while(1) {
        where = index(where,'l');
        if(where == 0) {    /* no 'l' in the line => no 'level' => default */
            *level = 1;
            return((char *)NULL);
        }
        if(strncmp(where,"level",5)!=0) {   /* this l isn't in the word 
                                             * 'level', so lets try again */
            where++;    /* make sure we don't match same char again */
            continue;
        }
        /* found the word level, lets look at the rest of the line */
        where += 5;
        while( (*where == ' ') || (*where == '\t') || (*where == '=') ||
                (*where == ',') || (*where == '(') || (*where == ')') ||
                (*where == '+') ) { /* legal white space - ignore */
            where++;
        }
        /* now the magic number */
        switch(*where) {
        case '1':
            *level=1;
            return((char *)NULL);
        case '2':
            *level=2;
            return((char *)NULL);
        case '3':
            *level=3;
            return((char *)NULL);
        case '4':
            *level=4;
            return((char *)NULL);
        case '5':
            *level=5;
            return((char *)NULL);
        case '6':
            *level=6;
            return((char *)NULL);
        case '7':
            *level=7;
            return((char *)NULL);
        case '8':
            *level=8;
            return((char *)NULL);
        case '9':
            *level=9;
            return((char *)NULL);
        default:
            *level=1;
            return(INPmkTemp(
                    " illegal argument to level parameter - level=1 assumed"));
        }
    }
}
