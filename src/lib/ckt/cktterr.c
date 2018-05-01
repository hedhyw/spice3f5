/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "util.h"
#include "suffix.h"


#define ccap (qcap+1)

void
CKTterr(qcap,ckt,timeStep)
    register int qcap;
    register CKTcircuit *ckt;
    register double *timeStep;
{ 
    double volttol;
    double chargetol;
    double tol;
    double del;
    double diff[8];
    double deltmp[8];
    double factor;
    register int i;
    register int j;
    static double gearCoeff[] = {
        .5,
        .2222222222,
        .1363636364,
        .096,
        .07299270073,
        .05830903790
    };
    static double trapCoeff[] = {
        .5,
        .08333333333
    };

    volttol = ckt->CKTabstol + ckt->CKTreltol * 
            MAX( FABS(*(ckt->CKTstate0+ccap)), FABS(*(ckt->CKTstate1+ccap)));
            
    chargetol = MAX(FABS(*(ckt->CKTstate0 +qcap)),FABS(*(ckt->CKTstate1+qcap)));
    chargetol = ckt->CKTreltol * MAX(chargetol,ckt->CKTchgtol)/ckt->CKTdelta;
    tol = MAX(volttol,chargetol);
    /* now divided differences */
    for(i=ckt->CKTorder+1;i>=0;i--) {
        diff[i] = *(ckt->CKTstates[i] + qcap);
    }
    for(i=0 ; i <= ckt->CKTorder ; i++) {
        deltmp[i] = ckt->CKTdeltaOld[i];
    }
    j = ckt->CKTorder;
    while(1) {
        for(i=0;i <= j;i++) {
            diff[i] = (diff[i] - diff[i+1])/deltmp[i];
        }
        if (--j < 0) break;
        for(i=0;i <= j;i++) {
            deltmp[i] = deltmp[i+1] + ckt->CKTdeltaOld[i];
        }
    }
    switch(ckt->CKTintegrateMethod) {
        case GEAR:
            factor = gearCoeff[ckt->CKTorder-1];
            break;

        case TRAPEZOIDAL:
            factor = trapCoeff[ckt->CKTorder - 1] ;
            break;
    }
    del = ckt->CKTtrtol * tol/MAX(ckt->CKTabstol,factor * FABS(diff[0]));
    if(ckt->CKTorder == 2) {
        del = sqrt(del);
    } else if (ckt->CKTorder > 2) {
        del = exp(log(del)/ckt->CKTorder);
    }
    *timeStep = MIN(*timeStep,del);
    return;
}
