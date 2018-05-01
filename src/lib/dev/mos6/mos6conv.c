/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1989 Takayasu Sakurai
**********/

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "mos6defs.h"
#include "sperror.h"
#include "util.h"
#include "suffix.h"

int
MOS6convTest(inModel,ckt)
    GENmodel *inModel;
    register CKTcircuit *ckt;
{
    register MOS6model *model = (MOS6model*)inModel;
    register MOS6instance *here;
    double delvbs;
    double delvbd;
    double delvgs;
    double delvds;
    double delvgd;
    double cbhat;
    double cdhat;
    double vbs;
    double vbd;
    double vgs;
    double vds;
    double vgd;
    double vgdo;
    double tol;

    for( ; model != NULL; model = model->MOS6nextModel) {
        for(here = model->MOS6instances; here!= NULL;
                here = here->MOS6nextInstance) {
        
            vbs = model->MOS6type * ( 
                *(ckt->CKTrhs+here->MOS6bNode) -
                *(ckt->CKTrhs+here->MOS6sNodePrime));
            vgs = model->MOS6type * ( 
                *(ckt->CKTrhs+here->MOS6gNode) -
                *(ckt->CKTrhs+here->MOS6sNodePrime));
            vds = model->MOS6type * ( 
                *(ckt->CKTrhs+here->MOS6dNodePrime) -
                *(ckt->CKTrhs+here->MOS6sNodePrime));
            vbd=vbs-vds;
            vgd=vgs-vds;
            vgdo = *(ckt->CKTstate0 + here->MOS6vgs) -
                *(ckt->CKTstate0 + here->MOS6vds);
            delvbs = vbs - *(ckt->CKTstate0 + here->MOS6vbs);
            delvbd = vbd - *(ckt->CKTstate0 + here->MOS6vbd);
            delvgs = vgs - *(ckt->CKTstate0 + here->MOS6vgs);
            delvds = vds - *(ckt->CKTstate0 + here->MOS6vds);
            delvgd = vgd-vgdo;

            /* these are needed for convergence testing */

            if (here->MOS6mode >= 0) {
                cdhat=
                    here->MOS6cd-
                    here->MOS6gbd * delvbd +
                    here->MOS6gmbs * delvbs +
                    here->MOS6gm * delvgs + 
                    here->MOS6gds * delvds ;
            } else {
                cdhat=
                    here->MOS6cd -
                    ( here->MOS6gbd -
                    here->MOS6gmbs) * delvbd -
                    here->MOS6gm * delvgd + 
                    here->MOS6gds * delvds ;
            }
            cbhat=
                here->MOS6cbs +
                here->MOS6cbd +
                here->MOS6gbd * delvbd +
                here->MOS6gbs * delvbs ;
            /*
             *  check convergence
             */
            tol=ckt->CKTreltol*MAX(FABS(cdhat),FABS(here->MOS6cd))+
                    ckt->CKTabstol;
            if (FABS(cdhat-here->MOS6cd) >= tol) { 
                ckt->CKTnoncon++;
		ckt->CKTtroubleElt = (GENinstance *) here;
                return(OK); /* no reason to continue, we haven't converged */
            } else {
                tol=ckt->CKTreltol*
                        MAX(FABS(cbhat),FABS(here->MOS6cbs+here->MOS6cbd))+
                        ckt->CKTabstol;
                if (FABS(cbhat-(here->MOS6cbs+here->MOS6cbd)) > tol) {
                    ckt->CKTnoncon++;
		    ckt->CKTtroubleElt = (GENinstance *) here;
                    return(OK); /* no reason to continue, we haven't converged*/
                }
            }
        }
    }
    return(OK);
}
