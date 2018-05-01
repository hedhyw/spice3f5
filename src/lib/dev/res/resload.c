/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "resdefs.h"
#include "sperror.h"
#include "suffix.h"


/*ARGSUSED*/
int
RESload(inModel,ckt)
    GENmodel *inModel;
    CKTcircuit *ckt;
        /* actually load the current resistance value into the 
         * sparse matrix previously provided 
         */
{
    register RESmodel *model = (RESmodel *)inModel;
    register RESinstance *here;

    /*  loop through all the resistor models */
    for( ; model != NULL; model = model->RESnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->RESinstances; here != NULL ;
                here=here->RESnextInstance) {
            
            *(here->RESposPosptr) += here->RESconduct;
            *(here->RESnegNegptr) += here->RESconduct;
            *(here->RESposNegptr) -= here->RESconduct;
            *(here->RESnegPosptr) -= here->RESconduct;
        }
    }
    return(OK);
}
