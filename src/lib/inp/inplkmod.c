/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "inpdefs.h"
#include "util.h"
#include "strext.h"
#include "suffix.h"


extern INPmodel *modtab;

/*ARGSUSED*/
int
INPlookMod(name)
    char *name;
{
    register INPmodel **i;

    for (i = &modtab;*i != (INPmodel *)NULL;i = &((*i)->INPnextModel)) {
        if (strcmp((*i)->INPmodName,name) == 0) {
            /* found the model in question - return true */
            return(1);
        }
    }
    /* didn't find model - return false */
    return(0);
}

