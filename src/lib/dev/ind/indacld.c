/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "inddefs.h"
#include "sperror.h"
#include "suffix.h"


int
INDacLoad(inModel,ckt)
    GENmodel *inModel;
    register CKTcircuit *ckt;
{
    register INDmodel *model = (INDmodel*)inModel;
    double val;
    register INDinstance *here;

    for( ; model != NULL; model = model->INDnextModel) {
        for( here = model->INDinstances;here != NULL; 
                here = here->INDnextInstance) {
    
            val = ckt->CKTomega * here->INDinduct;
            *(here->INDposIbrptr) += 1;
            *(here->INDnegIbrptr) -= 1;
            *(here->INDibrPosptr) += 1;
            *(here->INDibrNegptr) -= 1;
            *(here->INDibrIbrptr +1) -= val;
        }
    }
    return(OK);

}
