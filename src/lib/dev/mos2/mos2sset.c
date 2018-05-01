/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "smpdefs.h"
#include "cktdefs.h"
#include "mos2defs.h"
#include "util.h"
#include "sperror.h"
#include "suffix.h"

int
MOS2sSetup(info,inModel)
register SENstruct *info;
GENmodel *inModel;
/* loop through all the devices and 
         * allocate parameter #s to design parameters 
         */
{
    register MOS2model *model = (MOS2model *)inModel;
    register MOS2instance *here;

    /*  loop through all the models */
    for( ; model != NULL; model = model->MOS2nextModel ) {

        /* loop through all the instances of the model */
        for (here = model->MOS2instances; here != NULL ;
                here=here->MOS2nextInstance) {


            if(here->MOS2senParmNo){
                if((here->MOS2sens_l)&&(here->MOS2sens_w)){
                    here->MOS2senParmNo = ++(info->SENparms);
                    ++(info->SENparms);/* MOS has two design parameters */
                }
                else{
                    here->MOS2senParmNo = ++(info->SENparms);
                }
            }
            here->MOS2senPertFlag = OFF;
            if((here->MOS2sens = (double *)MALLOC(70*sizeof(double))) == NULL) {
                return(E_NOMEM);
            }

        }
    }
    return(OK);
}


