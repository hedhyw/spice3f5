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
#include "mos1defs.h"
#include "sperror.h"
#include "suffix.h"


int
MOS1pzLoad(inModel,ckt,s)
    GENmodel *inModel;
    register CKTcircuit *ckt;
    SPcomplex *s;
{
    register MOS1model *model = (MOS1model*)inModel;
    register MOS1instance *here;
    int xnrm;
    int xrev;
    double xgs;
    double xgd;
    double xgb;
    double xbd;
    double xbs;
    double capgs;
    double capgd;
    double capgb;
    double GateBulkOverlapCap;
    double GateDrainOverlapCap;
    double GateSourceOverlapCap;
    double EffectiveLength;

    for( ; model != NULL; model = model->MOS1nextModel) {
        for(here = model->MOS1instances; here!= NULL;
                here = here->MOS1nextInstance) {
        
            if (here->MOS1mode < 0) {
                xnrm=0;
                xrev=1;
            } else {
                xnrm=1;
                xrev=0;
            }
            /*
             *     meyer's model parameters
             */
            EffectiveLength=here->MOS1l - 2*model->MOS1latDiff;
            GateSourceOverlapCap = model->MOS1gateSourceOverlapCapFactor * 
                    here->MOS1w;
            GateDrainOverlapCap = model->MOS1gateDrainOverlapCapFactor * 
                    here->MOS1w;
            GateBulkOverlapCap = model->MOS1gateBulkOverlapCapFactor * 
                    EffectiveLength;
            capgs = ( 2* *(ckt->CKTstate0+here->MOS1capgs)+ 
                      GateSourceOverlapCap );
            capgd = ( 2* *(ckt->CKTstate0+here->MOS1capgd)+ 
                      GateDrainOverlapCap );
            capgb = ( 2* *(ckt->CKTstate0+here->MOS1capgb)+ 
                      GateBulkOverlapCap );
            xgs = capgs;
            xgd = capgd;
            xgb = capgb;
            xbd  = here->MOS1capbd;
            xbs  = here->MOS1capbs;
            /*printf("mos2: xgs=%g, xgd=%g, xgb=%g, xbd=%g, xbs=%g\n",
                    xgs,xgd,xgb,xbd,xbs);*/
            /*
             *    load matrix
             */

            *(here->MOS1GgPtr   ) += (xgd+xgs+xgb)*s->real;
            *(here->MOS1GgPtr +1) += (xgd+xgs+xgb)*s->imag;
            *(here->MOS1BbPtr   ) += (xgb+xbd+xbs)*s->real;
            *(here->MOS1BbPtr +1) += (xgb+xbd+xbs)*s->imag;
            *(here->MOS1DPdpPtr   ) += (xgd+xbd)*s->real;
            *(here->MOS1DPdpPtr +1) += (xgd+xbd)*s->imag;
            *(here->MOS1SPspPtr   ) += (xgs+xbs)*s->real;
            *(here->MOS1SPspPtr +1) += (xgs+xbs)*s->imag;
            *(here->MOS1GbPtr   ) -= xgb*s->real;
            *(here->MOS1GbPtr +1) -= xgb*s->imag;
            *(here->MOS1GdpPtr   ) -= xgd*s->real;
            *(here->MOS1GdpPtr +1) -= xgd*s->imag;
            *(here->MOS1GspPtr   ) -= xgs*s->real;
            *(here->MOS1GspPtr +1) -= xgs*s->imag;
            *(here->MOS1BgPtr   ) -= xgb*s->real;
            *(here->MOS1BgPtr +1) -= xgb*s->imag;
            *(here->MOS1BdpPtr   ) -= xbd*s->real;
            *(here->MOS1BdpPtr +1) -= xbd*s->imag;
            *(here->MOS1BspPtr   ) -= xbs*s->real;
            *(here->MOS1BspPtr +1) -= xbs*s->imag;
            *(here->MOS1DPgPtr   ) -= xgd*s->real;
            *(here->MOS1DPgPtr +1) -= xgd*s->imag;
            *(here->MOS1DPbPtr   ) -= xbd*s->real;
            *(here->MOS1DPbPtr +1) -= xbd*s->imag;
            *(here->MOS1SPgPtr   ) -= xgs*s->real;
            *(here->MOS1SPgPtr +1) -= xgs*s->imag;
            *(here->MOS1SPbPtr   ) -= xbs*s->real;
            *(here->MOS1SPbPtr +1) -= xbs*s->imag;
            *(here->MOS1DdPtr) += here->MOS1drainConductance;
            *(here->MOS1SsPtr) += here->MOS1sourceConductance;
            *(here->MOS1BbPtr) += here->MOS1gbd+here->MOS1gbs;
            *(here->MOS1DPdpPtr) += here->MOS1drainConductance+
                    here->MOS1gds+here->MOS1gbd+
                    xrev*(here->MOS1gm+here->MOS1gmbs);
            *(here->MOS1SPspPtr) += here->MOS1sourceConductance+
                    here->MOS1gds+here->MOS1gbs+
                    xnrm*(here->MOS1gm+here->MOS1gmbs);
            *(here->MOS1DdpPtr) -= here->MOS1drainConductance;
            *(here->MOS1SspPtr) -= here->MOS1sourceConductance;
            *(here->MOS1BdpPtr) -= here->MOS1gbd;
            *(here->MOS1BspPtr) -= here->MOS1gbs;
            *(here->MOS1DPdPtr) -= here->MOS1drainConductance;
            *(here->MOS1DPgPtr) += (xnrm-xrev)*here->MOS1gm;
            *(here->MOS1DPbPtr) += -here->MOS1gbd+(xnrm-xrev)*here->MOS1gmbs;
            *(here->MOS1DPspPtr) -= here->MOS1gds+
                    xnrm*(here->MOS1gm+here->MOS1gmbs);
            *(here->MOS1SPgPtr) -= (xnrm-xrev)*here->MOS1gm;
            *(here->MOS1SPsPtr) -= here->MOS1sourceConductance;
            *(here->MOS1SPbPtr) -= here->MOS1gbs+(xnrm-xrev)*here->MOS1gmbs;
            *(here->MOS1SPdpPtr) -= here->MOS1gds+
                    xrev*(here->MOS1gm+here->MOS1gmbs);

        }
    }
    return(OK);
}
