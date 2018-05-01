/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

    /* load the current source structure with those pointers needed later 
     * for fast matrix loading 
     */

#include "spice.h"
#include <stdio.h>
#include "smpdefs.h"
#include "cktdefs.h"
#include "vccsdefs.h"
#include "sperror.h"
#include "suffix.h"


/*ARGSUSED*/
int
VCCSsetup(matrix,inModel,ckt,states)
    register SMPmatrix *matrix;
    GENmodel *inModel;
    CKTcircuit *ckt;
    int *states;
{
    register VCCSmodel *model = (VCCSmodel *)inModel;
    register VCCSinstance *here;

    /*  loop through all the current source models */
    for( ; model != NULL; model = model->VCCSnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->VCCSinstances; here != NULL ;
                here=here->VCCSnextInstance) {
            
/* macro to make elements with built in test for out of memory */
#define TSTALLOC(ptr,first,second) \
if((here->ptr = SMPmakeElt(matrix,here->first,here->second))==(double *)NULL){\
    return(E_NOMEM);\
}

            TSTALLOC(VCCSposContPosptr, VCCSposNode, VCCScontPosNode)
            TSTALLOC(VCCSposContNegptr, VCCSposNode, VCCScontNegNode)
            TSTALLOC(VCCSnegContPosptr, VCCSnegNode, VCCScontPosNode)
            TSTALLOC(VCCSnegContNegptr, VCCSnegNode, VCCScontNegNode)
        }
    }
    return(OK);
}
