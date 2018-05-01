/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/* loop through all the devices and 
 * allocate parameter #s to design parameters 
 */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "cktdefs.h"
#include "smpdefs.h"
#include "bjtdefs.h"
#include "const.h"
#include "sperror.h"
#include "ifsim.h"
#include "suffix.h"


int
BJTsSetup(info,inModel)
    register SENstruct *info;
    GENmodel *inModel;
{
    register BJTmodel *model = (BJTmodel*)inModel;
    register BJTinstance *here;

#ifdef STEPDEBUG
    printf(" BJTsensetup \n");
#endif /* STEPDEBUG */

    /*  loop through all the diode models */
    for( ; model != NULL; model = model->BJTnextModel ) {


        /* loop through all the instances of the model */
        for (here = model->BJTinstances; here != NULL ;
                here=here->BJTnextInstance) {

            if(here->BJTsenParmNo){
                here->BJTsenParmNo = ++(info->SENparms);
                here->BJTsenPertFlag = OFF;
            }
            if((here->BJTsens = (double *)MALLOC(55*sizeof(double))) ==
                NULL) return(E_NOMEM);
        }
    }
    return(OK);
}

