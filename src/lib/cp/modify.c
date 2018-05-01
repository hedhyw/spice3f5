/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 */

#include "spice.h"
#include "cpdefs.h"
#include "suffix.h"

char cp_chars[128];

static char *singlec = "<>;&";

/* Initialize stuff. */

void
cp_init()
{
    char *s, *getenv();

    bzero(cp_chars, 128);
    for (s = singlec; *s; s++)
        cp_chars[*s] = (CPC_BRR | CPC_BRL);
    cp_vset("history", VT_NUM, (char *) &cp_maxhistlength);

    cp_curin = stdin;
    cp_curout = stdout;
    cp_curerr = stderr;

    cp_ioreset();

    return;
}

