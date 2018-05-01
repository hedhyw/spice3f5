/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

/*
 * This routine gets the device initial conditions for the BJTs
 * from the RHS vector
 */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "bjtdefs.h"
#include "sperror.h"
#include "suffix.h"


int
BJTgetic(inModel,ckt)
    GENmodel *inModel;
    register CKTcircuit *ckt;

{

    BJTmodel *model = (BJTmodel*)inModel;
    register BJTinstance *here;
    /*
     * grab initial conditions out of rhs array.   User specified, so use
     * external nodes to get values
     */

    for( ; model ; model = model->BJTnextModel) {
        for(here = model->BJTinstances; here ; here = here->BJTnextInstance) {
            if(!here->BJTicVBEGiven) {
                here->BJTicVBE = 
                        *(ckt->CKTrhs + here->BJTbaseNode) - 
                        *(ckt->CKTrhs + here->BJTemitNode);
            }
            if(!here->BJTicVCEGiven) {
                here->BJTicVCE = 
                        *(ckt->CKTrhs + here->BJTcolNode) - 
                        *(ckt->CKTrhs + here->BJTemitNode);
            }
        }
    }
    return(OK);
}
