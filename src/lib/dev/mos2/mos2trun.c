/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "mos2defs.h"
#include "sperror.h"
#include "suffix.h"


int
MOS2trunc(inModel,ckt,timeStep)
    GENmodel *inModel;
    register CKTcircuit *ckt;
    double *timeStep;
{
    register MOS2model *model = (MOS2model *)inModel;
    register MOS2instance *here;

    for( ; model != NULL; model = model->MOS2nextModel) {
        for(here=model->MOS2instances;here!=NULL;here = here->MOS2nextInstance){
            CKTterr(here->MOS2qgs,ckt,timeStep);
            CKTterr(here->MOS2qgd,ckt,timeStep);
            CKTterr(here->MOS2qgb,ckt,timeStep);
        }
    }
    return(OK);
}
