/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "capdefs.h"
#include "sperror.h"
#include "suffix.h"


int
CAPacLoad(inModel,ckt)
    GENmodel *inModel;
    register CKTcircuit *ckt;

{
    register CAPmodel *model = (CAPmodel*)inModel;
    double val;
    register CAPinstance *here;

    for( ; model != NULL; model = model->CAPnextModel) {
        for( here = model->CAPinstances;here != NULL; 
                here = here->CAPnextInstance) {
    
            val = ckt->CKTomega * here->CAPcapac;
            *(here->CAPposPosptr +1) += val;
            *(here->CAPnegNegptr +1) += val;
            *(here->CAPposNegptr +1) -= val;
            *(here->CAPnegPosptr +1) -= val;
        }
    }
    return(OK);

}

