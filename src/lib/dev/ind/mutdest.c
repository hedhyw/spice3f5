/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "inddefs.h"
#include "suffix.h"


#ifdef MUTUAL
void
MUTdestroy(inModel)
    GENmodel **inModel;
{
    MUTmodel **model = (MUTmodel**)inModel;
    MUTinstance *here;
    MUTinstance *prev = NULL;
    MUTmodel *mod = *model;
    MUTmodel *oldmod = NULL;

    for( ; mod ; mod = mod->MUTnextModel) {
        if(oldmod) FREE(oldmod);
        oldmod = mod;
        prev = (MUTinstance *)NULL;
        for(here = mod->MUTinstances ; here ; here = here->MUTnextInstance) {
            if(prev) FREE(prev);
            prev = here;
        }
        if(prev) FREE(prev);
    }
    if(oldmod) FREE(oldmod);
    *model = NULL;
}
#endif /* MUTUAL */
