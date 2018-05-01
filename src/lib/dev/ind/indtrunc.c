/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "inddefs.h"
#include "sperror.h"
#include "suffix.h"


int
INDtrunc(inModel,ckt,timeStep)
    GENmodel *inModel;
    register CKTcircuit *ckt;
    register double *timeStep;
{
    register INDmodel *model = (INDmodel*)inModel;
    register INDinstance *here;
    for( ; model!= NULL; model = model->INDnextModel) {
        for(here = model->INDinstances ; here != NULL ;
                here = here->INDnextInstance) {
            CKTterr(here->INDflux,ckt,timeStep);
        }
    }
    return(OK);
}
