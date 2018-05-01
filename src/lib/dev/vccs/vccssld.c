/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

/* actually load the current sensitivity information into the 
 * array  previously provided 
 */

#include "spice.h"
#include <stdio.h>
#include "smpdefs.h"
#include "cktdefs.h"
#include "vccsdefs.h"
#include "sperror.h"
#include "suffix.h"


int
VCCSsLoad(inModel,ckt)
    GENmodel *inModel;
    CKTcircuit *ckt;
{
    register VCCSmodel *model = (VCCSmodel *)inModel;
    register VCCSinstance *here;
    double  vc;

    /*  loop through all the source models */
    for( ; model != NULL; model = model->VCCSnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->VCCSinstances; here != NULL ;
                here=here->VCCSnextInstance) {
            if(here->VCCSsenParmNo){

                vc = *(ckt->CKTrhsOld + here->VCCScontPosNode)
                        -  *(ckt->CKTrhsOld + here->VCCScontNegNode);
                *(ckt->CKTsenInfo->SEN_RHS[here->VCCSposNode] + 
                        here->VCCSsenParmNo) -= vc;
                *(ckt->CKTsenInfo->SEN_RHS[here->VCCSnegNode] + 
                        here->VCCSsenParmNo) += vc;
            }
        }
    }
    return(OK);
}


