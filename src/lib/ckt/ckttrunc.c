/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

    /* CKTtrunc(ckt)
     * this is a driver program to iterate through all the various
     * truncation error functions provided for the circuit elements in the
     * given circuit 
     */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "smpdefs.h"
#include "util.h"
#include "devdefs.h"
#include "sperror.h"
#include "suffix.h"


extern SPICEdev *DEVices[];

int
CKTtrunc(ckt,timeStep)
    register CKTcircuit *ckt;
    double *timeStep;
{
#ifndef NEWTRUNC
    register int i;
    double timetemp;
#ifdef STEPDEBUG
    double debugtemp;
#endif /* STEPDEBUG */
    int error;
    timetemp = HUGE;

    for (i=0;i<DEVmaxnum;i++) {
        if ((*DEVices[i]).DEVtrunc != NULL && ckt->CKThead[i] != NULL) {
#ifdef STEPDEBUG
            debugtemp = timetemp;
#endif /* STEPDEBUG */
            error = (*((*DEVices[i]).DEVtrunc))(ckt->CKThead[i],ckt,&timetemp);
            if(error) {
                return(error);
            }
#ifdef STEPDEBUG
            if(debugtemp != timetemp) {
                printf("timestep cut by device type %s from %g to %g\n",
                        (*DEVices[i]).DEVpublic.name, debugtemp,timetemp);
            }
#endif /* STEPDEBUG */
        }
    }
    *timeStep = MIN(2 * *timeStep,timetemp);
    return(OK);
#else /* NEWTRUNC */
    register int i;
    register CKTnode *node;
    double timetemp;
    double tmp;
    double diff;
    double tol;
    int size;
    timetemp = HUGE;
    size = SMPmatSize(ckt->CKTmatrix);
#ifdef STEPDEBUG
    printf("at time %g, delta %g\n",ckt->CKTtime,ckt->CKTdeltaOld[0]);
#endif STEPDEBUG
    node = ckt->CKTnodes;
    switch(ckt->CKTintegrateMethod) {

    case TRAPEZOIDAL:
        switch(ckt->CKTorder) {
        case 1:
            for(i=1;i<size;i++) {
                tol = MAX( FABS(ckt->CKTrhs[i]),FABS(ckt->CKTpred[i]))*
                        ckt->CKTlteReltol+ckt->CKTlteAbstol;
                node = node->next;
                if(node->type!= 3) continue;
                diff = ckt->CKTrhs[i]-ckt->CKTpred[i];
#ifdef STEPDEBUG
                printf("%s: cor=%g, pred=%g ",node->name,
                        ckt->CKTrhs[i],ckt->CKTpred[i]);
#endif
                if(diff != 0) {
                    tmp = ckt->CKTtrtol * tol * 2 /diff;
                    tmp = ckt->CKTdeltaOld[0]*sqrt(FABS(tmp));
                    timetemp = MIN(timetemp,tmp);
#ifdef STEPDEBUG
                    printf("tol = %g, diff = %g, h->%g\n",tol,diff,tmp);
#endif
                } else {
#ifdef STEPDEBUG
                    printf("diff is 0\n");
#endif
                }
            }
            break;
        case 2:
            for(i=1;i<size;i++) {
                tol = MAX( FABS(ckt->CKTrhs[i]),FABS(ckt->CKTpred[i]))*
                        ckt->CKTlteReltol+ckt->CKTlteAbstol;
                node = node->next;
                if(node->type!= 3) continue;
                diff = ckt->CKTrhs[i]-ckt->CKTpred[i];
#ifdef STEPDEBUG
                printf("%s: cor=%g, pred=%g ",node->name,ckt->CKTrhs[i],
                        ckt->CKTpred[i]);
#endif
                if(diff != 0) {
                    tmp = ckt->CKTdeltaOld[0]*ckt->CKTtrtol * tol * 3 * 
                            (ckt->CKTdeltaOld[0]+ckt->CKTdeltaOld[1])/diff;
                    tmp = FABS(tmp);
                    timetemp = MIN(timetemp,tmp);
#ifdef STEPDEBUG
                    printf("tol = %g, diff = %g, h->%g\n",tol,diff,tmp);
#endif
                } else {
#ifdef STEPDEBUG
                    printf("diff is 0\n");
#endif
                }
            }
            break;
        default:
            return(E_ORDER);
        break;

        }
    break;

    case GEAR: {
        double delsum=0;
        for(i=0;i<=ckt->CKTorder;i++) {
            delsum += ckt->CKTdeltaOld[i];
        }
        for(i=1;i<size;i++) {
            node = node->next;
            if(node->type!= 3) continue;
            tol = MAX( FABS(ckt->CKTrhs[i]),FABS(ckt->CKTpred[i]))*
                    ckt->CKTlteReltol+ckt->CKTlteAbstol;
            diff = (ckt->CKTrhs[i]-ckt->CKTpred[i]);
#ifdef STEPDEBUG
            printf("%s: cor=%g, pred=%g ",node->name,ckt->CKTrhs[i],
                    ckt->CKTpred[i]);
#endif
            if(diff != 0) {
                tmp = tol*ckt->CKTtrtol*delsum/(diff*ckt->CKTdelta);
                tmp = FABS(tmp);
                switch(ckt->CKTorder) {
                    case 0:
                        break;
                    case 1:
                        tmp = sqrt(tmp);
                        break;
                    default:
                        tmp = exp(log(tmp)/(ckt->CKTorder+1));
                        break;
                }
                tmp *= ckt->CKTdelta;
                timetemp = MIN(timetemp,tmp);
#ifdef STEPDEBUG
                printf("tol = %g, diff = %g, h->%g\n",tol,diff,tmp);
#endif
            } else {
#ifdef STEPDEBUG
                printf("diff is 0\n");
#endif
            }
        }
    } 
    break;

    default: 
        return(E_METHOD);

    }
    *timeStep = MIN(2 * *timeStep,timetemp);
    return(OK);
#endif /* NEWTRUNC */
}
