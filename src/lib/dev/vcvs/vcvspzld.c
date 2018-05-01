/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "complex.h"
#include "sperror.h"
#include "vcvsdefs.h"
#include "suffix.h"


/*ARGSUSED*/
int
VCVSpzLoad(inModel,ckt,s)
    GENmodel *inModel;
    CKTcircuit *ckt;
    SPcomplex *s;
        /* actually load the current voltage value into the 
         * sparse matrix previously provided 
         */
{
    register VCVSmodel *model = (VCVSmodel *)inModel;
    register VCVSinstance *here;

    /*  loop through all the voltage source models */
    for( ; model != NULL; model = model->VCVSnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->VCVSinstances; here != NULL ;
                here=here->VCVSnextInstance) {
            
            *(here->VCVSposIbrptr) += 1.0 ;
            *(here->VCVSnegIbrptr) -= 1.0 ;
            *(here->VCVSibrPosptr) += 1.0 ;
            *(here->VCVSibrNegptr) -= 1.0 ;
            *(here->VCVSibrContPosptr) -= here->VCVScoeff ;
            *(here->VCVSibrContNegptr) += here->VCVScoeff ;
        }
    }
    return(OK);
}
