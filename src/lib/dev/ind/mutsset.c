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
#include "ifsim.h"
#include "smpdefs.h"
#include "cktdefs.h"
#include "inddefs.h"
#include "sperror.h"
#include "suffix.h"

#ifdef MUTUAL
/*ARGSUSED*/
int
MUTsSetup(info,inModel)
    register SENstruct *info;
    GENmodel *inModel;
{
    register MUTmodel *model = (MUTmodel*)inModel;
    register MUTinstance *here;

    /*  loop through all the inductor models */
    for( ; model != NULL; model = model->MUTnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->MUTinstances; here != NULL ;
                here=here->MUTnextInstance) {

            if(here->MUTsenParmNo){
                here->MUTsenParmNo = ++(info->SENparms);
            }


        }
    }
    return(OK);
}
#endif /* MUTUAL */

