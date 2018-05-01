/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "mos3defs.h"
#include "sperror.h"
#include "suffix.h"

int
MOS3trunc(inModel,ckt,timeStep)
    GENmodel *inModel;
    register CKTcircuit *ckt;
    double *timeStep;
{
    register MOS3model *model = (MOS3model *)inModel;
    register MOS3instance *here;

    for( ; model != NULL; model = model->MOS3nextModel) {
        for(here=model->MOS3instances;here!=NULL;here = here->MOS3nextInstance){
            CKTterr(here->MOS3qgs,ckt,timeStep);
            CKTterr(here->MOS3qgd,ckt,timeStep);
            CKTterr(here->MOS3qgb,ckt,timeStep);
        }
    }
    return(OK);
}
