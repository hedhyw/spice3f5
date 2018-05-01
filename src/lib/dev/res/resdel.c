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
RESdelete(inModel,name,inst)
    GENmodel *inModel;
    IFuid name;
    GENinstance **inst;
{
    RESmodel *model = (RESmodel *)inModel;
    RESinstance **fast = (RESinstance **)inst;
    RESinstance **prev = NULL;
    RESinstance *here;

    for( ; model ; model = model->RESnextModel) {
        prev = &(model->RESinstances);
        for(here = *prev; here ; here = *prev) {
            if(here->RESname == name || (fast && here==*fast) ) {
                *prev= here->RESnextInstance;
                FREE(here);
                return(OK);
            }
            prev = &(here->RESnextInstance);
        }
    }
    return(E_NODEV);
}
