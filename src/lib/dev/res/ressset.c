/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "smpdefs.h"
#include "resdefs.h"
#include "sperror.h"
#include "suffix.h"
#include "cktdefs.h"


int
RESsSetup(info,inModel)
    register SENstruct *info;
    GENmodel *inModel;
        /* loop through all the devices and 
         * assign parameter #s to design parameters 
         */
{
    register RESmodel *model = (RESmodel *)inModel;
    register RESinstance *here;

    /*  loop through all the resistor models */
    for( ; model != NULL; model = model->RESnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->RESinstances; here != NULL ;
            here=here->RESnextInstance) {

            if(here->RESsenParmNo){
                here->RESsenParmNo = ++(info->SENparms);
            }
        }
    }
    return(OK);
}
