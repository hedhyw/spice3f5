/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

/* actually load the current ac sensitivity information
 *  into the array previously provided 
 */

#include "spice.h"
#include <stdio.h>
#include "smpdefs.h"
#include "cktdefs.h"
#include "cccsdefs.h"
#include "sperror.h"
#include "suffix.h"


int
CCCSsAcLoad(inModel,ckt)
    GENmodel *inModel;
    CKTcircuit *ckt;
{
    register CCCSmodel *model = (CCCSmodel*)inModel;
    register CCCSinstance *here;
    double ic;
    double i_ic;

    /*  loop through all the CCCS models */
    for( ; model != NULL; model = model->CCCSnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->CCCSinstances; here != NULL ;
                here=here->CCCSnextInstance) {
            if(here->CCCSsenParmNo){

                ic = *(ckt->CKTrhsOld + here->CCCScontBranch);
                i_ic = *(ckt->CKTirhsOld + here->CCCScontBranch);
                *(ckt->CKTsenInfo->SEN_RHS[here->CCCSposNode] + 
                        here->CCCSsenParmNo) -= ic;
                *(ckt->CKTsenInfo->SEN_iRHS[here->CCCSposNode] +
                        here->CCCSsenParmNo) -= i_ic;
                *(ckt->CKTsenInfo->SEN_RHS[here->CCCSnegNode] + 
                        here->CCCSsenParmNo) += ic;
                *(ckt->CKTsenInfo->SEN_iRHS[here->CCCSnegNode] + 
                        here->CCCSsenParmNo) += i_ic;
            }
        }
    }
    return(OK);
}

