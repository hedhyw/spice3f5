/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "isrcdefs.h"
#include "sperror.h"
#include "suffix.h"


int
ISRCmDelete(inModel,modname,kill)
    GENmodel **inModel;
    IFuid modname;
    GENmodel *kill;
{
    ISRCmodel **model = (ISRCmodel**)inModel;
    ISRCmodel *modfast = (ISRCmodel*)kill;
    ISRCinstance *here;
    ISRCinstance *prev = NULL;
    ISRCmodel **oldmod;
    oldmod = model;
    for( ; *model ; model = &((*model)->ISRCnextModel)) {
        if( (*model)->ISRCmodName == modname || 
                (modfast && *model == modfast) ) goto delgot;
        oldmod = model;
    }
    return(E_NOMOD);

delgot:
    *oldmod = (*model)->ISRCnextModel; /* cut deleted device out of list */
    for(here = (*model)->ISRCinstances ; here ; here = here->ISRCnextInstance) {
        if(prev) FREE(prev);
        prev = here;
    }
    if(prev) FREE(prev);
    FREE(*model);
    return(OK);

}
