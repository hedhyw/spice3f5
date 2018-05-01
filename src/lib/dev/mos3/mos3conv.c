/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "mos3defs.h"
#include "sperror.h"
#include "util.h"
#include "suffix.h"

int
MOS3convTest(inModel,ckt)
    GENmodel *inModel;
    register CKTcircuit *ckt;
{
    register MOS3model *model = (MOS3model *)inModel;
    register MOS3instance *here;
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

    for( ; model != NULL; model = model->MOS3nextModel) {
        for(here = model->MOS3instances; here!= NULL;
                here = here->MOS3nextInstance) {
        
            vbs = model->MOS3type * ( 
                *(ckt->CKTrhs+here->MOS3bNode) -
                *(ckt->CKTrhs+here->MOS3sNodePrime));
            vgs = model->MOS3type * ( 
                *(ckt->CKTrhs+here->MOS3gNode) -
                *(ckt->CKTrhs+here->MOS3sNodePrime));
            vds = model->MOS3type * ( 
                *(ckt->CKTrhs+here->MOS3dNodePrime) -
                *(ckt->CKTrhs+here->MOS3sNodePrime));
            vbd=vbs-vds;
            vgd=vgs-vds;
            vgdo = *(ckt->CKTstate0 + here->MOS3vgs) -
                *(ckt->CKTstate0 + here->MOS3vds);
            delvbs = vbs - *(ckt->CKTstate0 + here->MOS3vbs);
            delvbd = vbd - *(ckt->CKTstate0 + here->MOS3vbd);
            delvgs = vgs - *(ckt->CKTstate0 + here->MOS3vgs);
            delvds = vds - *(ckt->CKTstate0 + here->MOS3vds);
            delvgd = vgd-vgdo;

            /* these are needed for convergence testing */

            if (here->MOS3mode >= 0) {
                cdhat=
                    here->MOS3cd-
                    here->MOS3gbd * delvbd +
                    here->MOS3gmbs * delvbs +
                    here->MOS3gm * delvgs + 
                    here->MOS3gds * delvds ;
            } else {
                cdhat=
                    here->MOS3cd -
                    ( here->MOS3gbd -
                    here->MOS3gmbs) * delvbd -
                    here->MOS3gm * delvgd + 
                    here->MOS3gds * delvds ;
            }
            cbhat=
                here->MOS3cbs +
                here->MOS3cbd +
                here->MOS3gbd * delvbd +
                here->MOS3gbs * delvbs ;
            /*
             *  check convergence
             */
            tol=ckt->CKTreltol*MAX(FABS(cdhat),FABS(here->MOS3cd))+
                    ckt->CKTabstol;
            if (FABS(cdhat-here->MOS3cd) >= tol) { 
                ckt->CKTnoncon++;
		ckt->CKTtroubleElt = (GENinstance *) here;
                return(OK); /* no reason to continue, we haven't converged */
            } else {
                tol=ckt->CKTreltol*
                        MAX(FABS(cbhat),FABS(here->MOS3cbs+here->MOS3cbd))
                        + ckt->CKTabstol;
                if (FABS(cbhat-(here->MOS3cbs+here->MOS3cbd)) > tol) {
                    ckt->CKTnoncon++;
		    ckt->CKTtroubleElt = (GENinstance *) here;
                    return(OK); /* no reason to continue, we haven't converged*/
                }
            }
        }
    }
    return(OK);
}
