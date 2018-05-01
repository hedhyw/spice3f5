/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1989 Takayasu Sakurai
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "mos6defs.h"
#include "sperror.h"
#include "suffix.h"


int
MOS6trunc(inModel,ckt,timeStep)
    GENmodel *inModel;
    register CKTcircuit *ckt;
    double *timeStep;
{
    register MOS6model *model = (MOS6model *)inModel;
    register MOS6instance *here;

    for( ; model != NULL; model = model->MOS6nextModel) {
        for(here=model->MOS6instances;here!=NULL;here = here->MOS6nextInstance){
            CKTterr(here->MOS6qgs,ckt,timeStep);
            CKTterr(here->MOS6qgd,ckt,timeStep);
            CKTterr(here->MOS6qgb,ckt,timeStep);
        }
    }
    return(OK);
}
