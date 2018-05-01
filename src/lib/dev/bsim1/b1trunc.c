/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Hong J. Park, Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "bsim1def.h"
#include "sperror.h"
#include "suffix.h"

int
B1trunc(inModel,ckt,timeStep)
    GENmodel *inModel;
    register CKTcircuit *ckt;
    double *timeStep;

{
    register B1model *model = (B1model*)inModel;
    register B1instance *here;
#ifdef STEPDEBUG
    double debugtemp;
#endif /* STEPDEBUG */

    for( ; model != NULL; model = model->B1nextModel) {
        for(here=model->B1instances;here!=NULL;here = here->B1nextInstance){
#ifdef STEPDEBUG
            debugtemp = *timeStep;
#endif /* STEPDEBUG */
            CKTterr(here->B1qb,ckt,timeStep);
            CKTterr(here->B1qg,ckt,timeStep);
            CKTterr(here->B1qd,ckt,timeStep);
#ifdef STEPDEBUG
            if(debugtemp != *timeStep) {
                printf("device %s reduces step from %g to %g\n",
                        here->B1name,debugtemp,*timeStep);
            }
#endif /* STEPDEBUG */
        }
    }
    return(OK);
}

