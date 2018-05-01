/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "cktdefs.h"
#include "vccsdefs.h"
#include "sperror.h"
#include "suffix.h"


/*ARGSUSED*/
int
VCCSload(inModel,ckt)
    GENmodel *inModel;
    CKTcircuit *ckt;
        /* actually load the current values into the 
         * sparse matrix previously provided 
         */
{
    register VCCSmodel *model = (VCCSmodel *)inModel;
    register VCCSinstance *here;

    /*  loop through all the source models */
    for( ; model != NULL; model = model->VCCSnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->VCCSinstances; here != NULL ;
                here=here->VCCSnextInstance) {
            
            *(here->VCCSposContPosptr) += here->VCCScoeff ;
            *(here->VCCSposContNegptr) -= here->VCCScoeff ;
            *(here->VCCSnegContPosptr) -= here->VCCScoeff ;
            *(here->VCCSnegContNegptr) += here->VCCScoeff ;
        }
    }
    return(OK);
}
