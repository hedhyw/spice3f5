/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "ccvsdefs.h"
#include "util.h"
#include "sperror.h"
#include "suffix.h"


int
CCVSmDelete(inModel,modname,kill)
    GENmodel **inModel;
    IFuid modname;
    GENmodel *kill;
{
    CCVSmodel **model = (CCVSmodel**)inModel;
    CCVSmodel *modfast = (CCVSmodel*)kill;
    CCVSinstance *here;
    CCVSinstance *prev = NULL;
    CCVSmodel **oldmod;
    oldmod = model;
    for( ; *model ; model = &((*model)->CCVSnextModel)) {
        if( (*model)->CCVSmodName == modname || 
                (modfast && *model == modfast) ) goto delgot;
        oldmod = model;
    }
    return(E_NOMOD);

delgot:
    *oldmod = (*model)->CCVSnextModel; /* cut deleted device out of list */
    for(here = (*model)->CCVSinstances ; here ; here = here->CCVSnextInstance) {
        if(prev) FREE(prev);
        prev = here;
    }
    if(prev) FREE(prev);
    FREE(*model);
    return(OK);

}
