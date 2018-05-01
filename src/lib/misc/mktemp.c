/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
**********/

/*
 * A more portable version of the standard "mktemp( )" function
 */

#include "spice.h"
#include "stdio.h"
#include "misc.h"

#ifndef TEMPFORMAT
#define TEMPFORMAT "temp%s%d"
#endif

char *
smktemp(id)
    char *id;
{
    char	rbuf[513];
    char	*nbuf;
    int		num;

#ifdef HAS_GETPID
    num = getpid( );
#else
    num = 0;
#endif

    if (!id)
	id = "sp";

    sprintf(rbuf, TEMPFORMAT, id, num);
    nbuf = (char *) malloc(strlen(rbuf) + 1);
    strcpy(nbuf, rbuf);

    return nbuf;
}
