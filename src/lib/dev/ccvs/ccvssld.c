/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

/* actually load the current sensitivity information
 * into the array previously provided 
 */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "ccvsdefs.h"
#include "util.h"
#include "sperror.h"
#include "suffix.h"


int
CCVSsLoad(inModel,ckt)
    GENmodel *inModel;
    CKTcircuit *ckt;
{
    register CCVSmodel *model = (CCVSmodel*)inModel;
    register CCVSinstance *here;
    double   ic;

    /*  loop through all the voltage source models */
    for( ; model != NULL; model = model->CCVSnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->CCVSinstances; here != NULL ;
                here=here->CCVSnextInstance) {
            if(here->CCVSsenParmNo){
                ic = *(ckt->CKTrhsOld + here->CCVScontBranch);
                *(ckt->CKTsenInfo->SEN_RHS[here->CCVSbranch] + 
                        here->CCVSsenParmNo) -= ic;
            }
        }
    }
    return(OK);
}

