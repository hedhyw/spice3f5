/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

/*
 * This routine performs truncation error calculations for
 * BJTs in the circuit.
 */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "bjtdefs.h"
#include "sperror.h"
#include "suffix.h"


int
BJTtrunc(inModel,ckt,timeStep)
    GENmodel *inModel;
    register CKTcircuit *ckt;
    double *timeStep;

{
    register BJTmodel *model = (BJTmodel*)inModel;
    register BJTinstance *here;

    for( ; model != NULL; model = model->BJTnextModel) {
        for(here=model->BJTinstances;here!=NULL;here = here->BJTnextInstance){
            CKTterr(here->BJTqbe,ckt,timeStep);
            CKTterr(here->BJTqbc,ckt,timeStep);
            CKTterr(here->BJTqcs,ckt,timeStep);
        }
    }
    return(OK);
}
