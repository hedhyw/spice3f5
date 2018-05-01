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
#include "smpdefs.h"
#include "cktdefs.h"
#include "vcvsdefs.h"
#include "util.h"
#include "sperror.h"
#include "suffix.h"


int
VCVSsSetup(info,inModel)
    register SENstruct *info;
    GENmodel *inModel;
{
    register VCVSmodel *model = (VCVSmodel *)inModel;
    register VCVSinstance *here;

    /*  loop through all the voltage source models */
    for( ; model != NULL; model = model->VCVSnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->VCVSinstances; here != NULL ;
                here=here->VCVSnextInstance) {

            if(here->VCVSsenParmNo){
                here->VCVSsenParmNo = ++(info->SENparms);
            }

        }
    }
    return(OK);
}

