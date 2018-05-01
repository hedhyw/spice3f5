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
#include "suffix.h"


extern INPmodel *modtab;

void
INPkillMods()
{
    INPmodel *modtmp;
    INPmodel *prev = NULL;

    for (modtmp = modtab;modtmp != (INPmodel *)NULL;modtmp =
            modtmp->INPnextModel) {
        if(prev) FREE(prev);
        prev = modtmp;
    }
    if(prev) FREE(prev);
    modtab = (INPmodel *)NULL;
}

