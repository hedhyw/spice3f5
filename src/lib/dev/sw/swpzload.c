/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Gordon Jacobs
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "swdefs.h"
#include "sperror.h"
#include "complex.h"
#include "suffix.h"


/* ARGSUSED */
int
SWpzLoad(inModel,ckt,s)
    GENmodel *inModel;
    register CKTcircuit *ckt;
    SPcomplex *s;
        /* load the current values into the 
         * sparse matrix previously provided 
         * during AC analysis.
         */
{
    register SWmodel *model = (SWmodel *)inModel;
    register SWinstance *here;
    double g_now;
    int current_state;

    /*  loop through all the switch models */
    for( ; model != NULL; model = model->SWnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->SWinstances; here != NULL ;
                here=here->SWnextInstance) {

            /* In AC analysis, just propogate the state... */

            current_state = (int)*(ckt->CKTstate0 + here->SWstate);

            g_now = current_state?(model->SWonConduct):(model->SWoffConduct);

            *(here->SWposPosptr) += g_now;
            *(here->SWposNegptr) -= g_now;
            *(here->SWnegPosptr) -= g_now;
            *(here->SWnegNegptr) += g_now;
        }
    }
    return(OK);
}
