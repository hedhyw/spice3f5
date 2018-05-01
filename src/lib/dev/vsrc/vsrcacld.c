/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "vsrcdefs.h"
#include "sperror.h"
#include "suffix.h"


int
VSRCacLoad(inModel,ckt)
    GENmodel *inModel;
    register CKTcircuit *ckt;
{
    register VSRCmodel *model = (VSRCmodel *)inModel;
    register VSRCinstance *here;

    for( ; model != NULL; model = model->VSRCnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->VSRCinstances; here != NULL ;
                here=here->VSRCnextInstance) {

            *(here->VSRCposIbrptr) += 1.0 ;
            *(here->VSRCnegIbrptr) -= 1.0 ;
            *(here->VSRCibrPosptr) += 1.0 ;
            *(here->VSRCibrNegptr) -= 1.0 ;
            *(ckt->CKTrhs + (here->VSRCbranch)) += here->VSRCacReal;
            *(ckt->CKTirhs + (here->VSRCbranch)) += here->VSRCacImag;
        }
    }
    return(OK);
}
