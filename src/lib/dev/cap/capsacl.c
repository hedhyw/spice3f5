/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

/* actually load the current ac sensitivity 
 * information into the  array previously provided 
 */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "cktdefs.h"
#include "capdefs.h"
#include "trandefs.h"
#include "sperror.h"
#include "suffix.h"


int
CAPsAcLoad(inModel,ckt)

GENmodel *inModel;
register CKTcircuit *ckt;

{
    register CAPmodel *model = (CAPmodel*)inModel;
    register CAPinstance *here;
    double vcap;
    double ivcap;
    double val;
    double ival;

    /*  loop through all the capacitor models */
    for( ; model != NULL; model = model->CAPnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->CAPinstances; here != NULL ;
                    here=here->CAPnextInstance) {

            if(here->CAPsenParmNo){
                vcap = *(ckt->CKTrhsOld+here->CAPposNode) - 
                    *(ckt->CKTrhsOld+here->CAPnegNode);
                ivcap = *(ckt->CKTirhsOld+here->CAPposNode) - 
                    *(ckt->CKTirhsOld+here->CAPnegNode);   

                val = ckt->CKTomega * ivcap;
                ival = ckt->CKTomega * vcap;

                /* load the RHS matrix */

                *(ckt->CKTsenInfo->SEN_RHS[here->CAPposNode] + 
                        here->CAPsenParmNo) += val;
                *(ckt->CKTsenInfo->SEN_iRHS[here->CAPposNode] +
                        here->CAPsenParmNo) -= ival;
                *(ckt->CKTsenInfo->SEN_RHS[here->CAPnegNode] + 
                        here->CAPsenParmNo) -= val;
                *(ckt->CKTsenInfo->SEN_iRHS[here->CAPnegNode] + 
                        here->CAPsenParmNo) += ival;
            }
        }
    }
    return(OK);
}

