/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "complex.h"
#include "sperror.h"
#include "vsrcdefs.h"
#include "util.h"
#include "suffix.h"

/* ARGSUSED */
int
VSRCpzLoad(inModel,ckt,s)
    GENmodel *inModel;
    CKTcircuit *ckt;
    SPcomplex *s;
{
    register VSRCmodel *model = (VSRCmodel *)inModel;
    register VSRCinstance *here;

    for( ; model != NULL; model = model->VSRCnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->VSRCinstances; here != NULL ;
                here=here->VSRCnextInstance) {

            if (!(here->VSRCacGiven)) {
                /*a dc source*/
                /*the connecting nodes are shorted*/
                *(here->VSRCposIbrptr)  += 1.0 ;
                *(here->VSRCnegIbrptr)  += -1.0 ;
                *(here->VSRCibrPosptr)  += 1.0 ;
                *(here->VSRCibrNegptr)  += -1.0 ;
            } else {
                /*an ac source*/
                /*no effective contribution
                 *diagonal element made 1
                 */
                *(here->VSRCposIbrptr)  += 1.0 ;
                *(here->VSRCnegIbrptr)  += -1.0 ;
                *(here->VSRCibrIbrptr)  += 1.0 ;
            }
        }
    }
    return(OK);
}
