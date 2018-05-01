/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "resdefs.h"
#include "sperror.h"
#include "suffix.h"


int
RESmDelete(inModel,modname,kill)
    GENmodel **inModel;
    IFuid modname;
    GENmodel *kill;
{
    RESmodel **model = (RESmodel **)inModel;
    RESmodel *modfast = (RESmodel *)kill;
    RESinstance *here;
    RESinstance *prev = NULL;
    RESmodel **oldmod;
    oldmod = model;
    for( ; *model ; model = &((*model)->RESnextModel)) {
        if( (*model)->RESmodName == modname || 
                (modfast && *model == modfast) ) goto delgot;
        oldmod = model;
    }
    return(E_NOMOD);

delgot:
    *oldmod = (*model)->RESnextModel; /* cut deleted device out of list */
    for(here = (*model)->RESinstances ; here ; here = here->RESnextInstance) {
        if(prev) FREE(prev);
        prev = here;
    }
    if(prev) FREE(prev);
    FREE(*model);
    return(OK);

}
