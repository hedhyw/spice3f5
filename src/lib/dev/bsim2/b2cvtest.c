/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Hong J. Park, Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "cktdefs.h"
#include "bsim2def.h"
#include "trandefs.h"
#include "const.h"
#include "devdefs.h"
#include "sperror.h"
#include "suffix.h"

int
B2convTest(inModel,ckt)

    GENmodel *inModel;
    register CKTcircuit *ckt;

        /* actually load the current value into the 
         * sparse matrix previously provided 
         */
{
    register B2model *model = (B2model*)inModel;
    register B2instance *here;
    double cbd;
    double cbhat;
    double cbs;
    double cd;
    double cdhat;
    double delvbd;
    double delvbs;
    double delvds;
    double delvgd;
    double delvgs;
    double tol;
    double vbd;
    double vbs;
    double vds;
    double vgd;
    double vgdo;
    double vgs;


    /*  loop through all the B2 device models */
    for( ; model != NULL; model = model->B2nextModel ) {

        /* loop through all the instances of the model */
        for (here = model->B2instances; here != NULL ;
                here=here->B2nextInstance) {

            vbs = model->B2type * ( 
                *(ckt->CKTrhsOld+here->B2bNode) -
                *(ckt->CKTrhsOld+here->B2sNodePrime));
            vgs = model->B2type * ( 
                *(ckt->CKTrhsOld+here->B2gNode) -
                *(ckt->CKTrhsOld+here->B2sNodePrime));
            vds = model->B2type * ( 
                *(ckt->CKTrhsOld+here->B2dNodePrime) -
                *(ckt->CKTrhsOld+here->B2sNodePrime));
            vbd=vbs-vds;
            vgd=vgs-vds;
            vgdo = *(ckt->CKTstate0 + here->B2vgs) - 
                *(ckt->CKTstate0 + here->B2vds);
            delvbs = vbs - *(ckt->CKTstate0 + here->B2vbs);
            delvbd = vbd - *(ckt->CKTstate0 + here->B2vbd);
            delvgs = vgs - *(ckt->CKTstate0 + here->B2vgs);
            delvds = vds - *(ckt->CKTstate0 + here->B2vds);
            delvgd = vgd-vgdo;

            if (here->B2mode >= 0) {
                cdhat=
                    *(ckt->CKTstate0 + here->B2cd) -
                    *(ckt->CKTstate0 + here->B2gbd) * delvbd +
                    *(ckt->CKTstate0 + here->B2gmbs) * delvbs +
                    *(ckt->CKTstate0 + here->B2gm) * delvgs + 
                    *(ckt->CKTstate0 + here->B2gds) * delvds ;
            } else {
                cdhat=
                    *(ckt->CKTstate0 + here->B2cd) -
                    ( *(ckt->CKTstate0 + here->B2gbd) -
                      *(ckt->CKTstate0 + here->B2gmbs)) * delvbd -
                    *(ckt->CKTstate0 + here->B2gm) * delvgd +
                    *(ckt->CKTstate0 + here->B2gds) * delvds;
            }
            cbhat=
                *(ckt->CKTstate0 + here->B2cbs) +
                *(ckt->CKTstate0 + here->B2cbd) +
                *(ckt->CKTstate0 + here->B2gbd) * delvbd +
                *(ckt->CKTstate0 + here->B2gbs) * delvbs ;

            cd = *(ckt->CKTstate0 + here->B2cd);
            cbs = *(ckt->CKTstate0 + here->B2cbs);
            cbd = *(ckt->CKTstate0 + here->B2cbd);
            /*
             *  check convergence
             */
            if ( (here->B2off == 0)  || (!(ckt->CKTmode & MODEINITFIX)) ){
                tol=ckt->CKTreltol*MAX(FABS(cdhat),FABS(cd))+ckt->CKTabstol;
                if (FABS(cdhat-cd) >= tol) { 
                    ckt->CKTnoncon++;
		    ckt->CKTtroubleElt = (GENinstance *) here;
                    return(OK);
                } 
                tol=ckt->CKTreltol*MAX(FABS(cbhat),FABS(cbs+cbd))+
                    ckt->CKTabstol;
                if (FABS(cbhat-(cbs+cbd)) > tol) {
                    ckt->CKTnoncon++;
		    ckt->CKTtroubleElt = (GENinstance *) here;
                    return(OK);
                }
            }
        }
    }
    return(OK);
}

