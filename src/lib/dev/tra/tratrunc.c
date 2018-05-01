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
#include "tradefs.h"
#include "sperror.h"
#include "suffix.h"


int
TRAtrunc(inModel,ckt,timeStep)
    GENmodel *inModel;
    register CKTcircuit *ckt;
    double *timeStep;

{
    register TRAmodel *model = (TRAmodel *)inModel;
    register TRAinstance *here;
    double v1,v2,v3,v4;
    double v5,v6,d1,d2,d3,d4;
    double tmp;

    /*  loop through all the transmission line models */
    for( ; model != NULL; model = model->TRAnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->TRAinstances; here != NULL ;
                here=here->TRAnextInstance) {
            v1 = ( *(ckt->CKTrhsOld + here->TRAposNode2)
                - *(ckt->CKTrhsOld + here->TRAnegNode2))
                + *(ckt->CKTrhsOld + here->TRAbrEq2) *
                    here->TRAimped;
            v2 = *(here->TRAdelays+1+3*(here->TRAsizeDelay));
            v3 = *(here->TRAdelays+1+3*(here->TRAsizeDelay-1));
            v4 = ( *(ckt->CKTrhsOld + here->TRAposNode1)
                - *(ckt->CKTrhsOld + here->TRAnegNode1))
                + *(ckt->CKTrhsOld + here->TRAbrEq1) *
                    here->TRAimped;
            v5 = *(here->TRAdelays+2+3*(here->TRAsizeDelay));
            v6 = *(here->TRAdelays+2+3*(here->TRAsizeDelay-1));
            d1 = (v1-v2)/ckt->CKTdeltaOld[1];
            d2 = (v2-v3)/ckt->CKTdeltaOld[2];
            d3 = (v4-v5)/ckt->CKTdeltaOld[1];
            d4 = (v5-v6)/ckt->CKTdeltaOld[2];
            /*printf("%s: at %g derivs are %g, %g and %g, %g\n",
                    here->TRAname,ckt->CKTtime,d1,d2,d3,d4);*/
            if( (FABS(d1-d2) >= here->TRAreltol*MAX(FABS(d1),FABS(d2))+
                    here->TRAabstol) ||
                    (FABS(d3-d4) >= here->TRAreltol*MAX(FABS(d3),FABS(d4))+
                    here->TRAabstol) ) {
                /* derivitive changing - need to schedule after delay */
                /*printf("%s:  at %g set for %g\n",here->TRAname,
                    ckt->CKTtime,
                    *(here->TRAdelays+3*here->TRAsizeDelay-3)+here->TRAtd
                    );*/
                /*printf("%g, %g, %g -> %g, %g \n",v1,v2,v3,d1,d2);*/
                /*printf("%g, %g, %g -> %g, %g \n",v4,v5,v6,d3,d4);*/
                /* also set for break after PREVIOUS point */
                /*printf("setting break\n");*/
                /* will need to set a breakpoint at 
                    *(here->TRAdelays+3*(here->TRAsizeDelay)) + here->TRAtd
                    so we need to make sure we don't step past it 
                */
                /* the previous timepoint plus the delay */
                tmp = *(here->TRAdelays+3*here->TRAsizeDelay) + here->TRAtd;
                /* minus current time */
                tmp -= ckt->CKTtime;
                *timeStep = MIN(*timeStep,tmp);
            }
        }
    }
    return(OK);
}

