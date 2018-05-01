/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "complex.h"
#include "sperror.h"
#include "bsim1def.h"
#include "suffix.h"


int
B1pzLoad(inModel,ckt,s)
    GENmodel *inModel;
    register CKTcircuit *ckt;
    register SPcomplex *s;
{
    register B1model *model = (B1model*)inModel;
    register B1instance *here;
    int xnrm;
    int xrev;
    double gdpr;
    double gspr;
    double gm;
    double gds;
    double gmbs;
    double gbd;
    double gbs;
    double capbd;
    double capbs;
    double xcggb;
    double xcgdb;
    double xcgsb;
    double xcbgb;
    double xcbdb;
    double xcbsb;
    double xcddb;
    double xcssb;
    double xcdgb;
    double xcsgb;
    double xcdsb;
    double xcsdb;
    double cggb;
    double cgdb;
    double cgsb;
    double cbgb;
    double cbdb;
    double cbsb;
    double cddb;
    double cdgb;
    double cdsb;

    for( ; model != NULL; model = model->B1nextModel) {
        for(here = model->B1instances; here!= NULL;
                here = here->B1nextInstance) {
        
            if (here->B1mode >= 0) {
                xnrm=1;
                xrev=0;
            } else {
                xnrm=0;
                xrev=1;
            }
            gdpr=here->B1drainConductance;
            gspr=here->B1sourceConductance;
            gm= *(ckt->CKTstate0 + here->B1gm);
            gds= *(ckt->CKTstate0 + here->B1gds);
            gmbs= *(ckt->CKTstate0 + here->B1gmbs);
            gbd= *(ckt->CKTstate0 + here->B1gbd);
            gbs= *(ckt->CKTstate0 + here->B1gbs);
            capbd= *(ckt->CKTstate0 + here->B1capbd);
            capbs= *(ckt->CKTstate0 + here->B1capbs);
            /*
             *    charge oriented model parameters
             */

            cggb = *(ckt->CKTstate0 + here->B1cggb);
            cgsb = *(ckt->CKTstate0 + here->B1cgsb);
            cgdb = *(ckt->CKTstate0 + here->B1cgdb);

            cbgb = *(ckt->CKTstate0 + here->B1cbgb);
            cbsb = *(ckt->CKTstate0 + here->B1cbsb);
            cbdb = *(ckt->CKTstate0 + here->B1cbdb);

            cdgb = *(ckt->CKTstate0 + here->B1cdgb);
            cdsb = *(ckt->CKTstate0 + here->B1cdsb);
            cddb = *(ckt->CKTstate0 + here->B1cddb);

            xcdgb = (cdgb - here->B1GDoverlapCap) ;
            xcddb = (cddb + capbd + here->B1GDoverlapCap) ;
            xcdsb = cdsb ;
            xcsgb = -(cggb + cbgb + cdgb + here->B1GSoverlapCap ) ;
            xcsdb = -(cgdb + cbdb + cddb) ;
            xcssb = (capbs + here->B1GSoverlapCap - (cgsb+cbsb+cdsb)) ;
            xcggb = (cggb + here->B1GDoverlapCap + here->B1GSoverlapCap + 
                    here->B1GBoverlapCap) ;
            xcgdb = (cgdb - here->B1GDoverlapCap ) ;
            xcgsb = (cgsb - here->B1GSoverlapCap) ;
            xcbgb = (cbgb - here->B1GBoverlapCap) ;
            xcbdb = (cbdb - capbd ) ;
            xcbsb = (cbsb - capbs ) ;


            *(here->B1GgPtr   ) += xcggb * s->real;
            *(here->B1GgPtr +1) += xcggb * s->imag;
            *(here->B1BbPtr   ) += (-xcbgb-xcbdb-xcbsb) * s->real;
            *(here->B1BbPtr +1) += (-xcbgb-xcbdb-xcbsb) * s->imag;
            *(here->B1DPdpPtr   ) += xcddb * s->real;
            *(here->B1DPdpPtr +1) += xcddb * s->imag;
            *(here->B1SPspPtr   ) += xcssb * s->real;
            *(here->B1SPspPtr +1) += xcssb * s->imag;
            *(here->B1GbPtr   ) += (-xcggb-xcgdb-xcgsb) * s->real;
            *(here->B1GbPtr +1) += (-xcggb-xcgdb-xcgsb) * s->imag;
            *(here->B1GdpPtr   ) += xcgdb * s->real;
            *(here->B1GdpPtr +1) += xcgdb * s->imag;
            *(here->B1GspPtr   ) += xcgsb * s->real;
            *(here->B1GspPtr +1) += xcgsb * s->imag;
            *(here->B1BgPtr   ) += xcbgb * s->real;
            *(here->B1BgPtr +1) += xcbgb * s->imag;
            *(here->B1BdpPtr   ) += xcbdb * s->real;
            *(here->B1BdpPtr +1) += xcbdb * s->imag;
            *(here->B1BspPtr   ) += xcbsb * s->real;
            *(here->B1BspPtr +1) += xcbsb * s->imag;
            *(here->B1DPgPtr   ) += xcdgb * s->real;
            *(here->B1DPgPtr +1) += xcdgb * s->imag;
            *(here->B1DPbPtr   ) += (-xcdgb-xcddb-xcdsb) * s->real;
            *(here->B1DPbPtr +1) += (-xcdgb-xcddb-xcdsb) * s->imag;
            *(here->B1DPspPtr   ) += xcdsb * s->real;
            *(here->B1DPspPtr +1) += xcdsb * s->imag;
            *(here->B1SPgPtr   ) += xcsgb * s->real;
            *(here->B1SPgPtr +1) += xcsgb * s->imag;
            *(here->B1SPbPtr   ) += (-xcsgb-xcsdb-xcssb) * s->real;
            *(here->B1SPbPtr +1) += (-xcsgb-xcsdb-xcssb) * s->imag;
            *(here->B1SPdpPtr   ) += xcsdb * s->real;
            *(here->B1SPdpPtr +1) += xcsdb * s->imag;
            *(here->B1DdPtr) += gdpr;
            *(here->B1SsPtr) += gspr;
            *(here->B1BbPtr) += gbd+gbs;
            *(here->B1DPdpPtr) += gdpr+gds+gbd+xrev*(gm+gmbs);
            *(here->B1SPspPtr) += gspr+gds+gbs+xnrm*(gm+gmbs);
            *(here->B1DdpPtr) -= gdpr;
            *(here->B1SspPtr) -= gspr;
            *(here->B1BdpPtr) -= gbd;
            *(here->B1BspPtr) -= gbs;
            *(here->B1DPdPtr) -= gdpr;
            *(here->B1DPgPtr) += (xnrm-xrev)*gm;
            *(here->B1DPbPtr) += -gbd+(xnrm-xrev)*gmbs;
            *(here->B1DPspPtr) += -gds-xnrm*(gm+gmbs);
            *(here->B1SPgPtr) += -(xnrm-xrev)*gm;
            *(here->B1SPsPtr) -= gspr;
            *(here->B1SPbPtr) += -gbs-(xnrm-xrev)*gmbs;
            *(here->B1SPdpPtr) += -gds-xrev*(gm+gmbs);

        }
    }
    return(OK);
}
