/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "diodefs.h"
#include "sperror.h"
#include "suffix.h"


int
DIOgetic(inModel,ckt)
    GENmodel *inModel;
    CKTcircuit *ckt;
{
    DIOmodel *model = (DIOmodel*)inModel;
    DIOinstance *here;
    /*
     * grab initial conditions out of rhs array.   User specified, so use
     * external nodes to get values
     */

    for( ; model ; model = model->DIOnextModel) {
        for(here = model->DIOinstances; here ; here = here->DIOnextInstance) {
            if(!here->DIOinitCondGiven) {
                here->DIOinitCond = 
                        *(ckt->CKTrhs + here->DIOposNode) - 
                        *(ckt->CKTrhs + here->DIOnegNode);
            }
        }
    }
    return(OK);
}
