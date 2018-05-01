/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

/* loop through all the devices and 
 * allocate parameter #s to design parameters 
 */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "ccvsdefs.h"
#include "util.h"
#include "sperror.h"
#include "suffix.h"


int
CCVSsSetup(info,inModel)
    register SENstruct *info;
    GENmodel *inModel;

{
    register CCVSmodel *model = (CCVSmodel*)inModel;
    register CCVSinstance *here;

    /*  loop through all the voltage source models */
    for( ; model != NULL; model = model->CCVSnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->CCVSinstances; here != NULL ;
                here=here->CCVSnextInstance) {

            if(here->CCVSsenParmNo){
                here->CCVSsenParmNo = ++(info->SENparms);
            }
        }
    }
    return(OK);
}

