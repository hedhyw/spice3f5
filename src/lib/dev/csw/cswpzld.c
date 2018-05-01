/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Gordon Jacobs
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "cswdefs.h"
#include "sperror.h"
#include "complex.h"
#include "suffix.h"


/* ARGSUSED */
int
CSWpzLoad(inModel,ckt,s)
    GENmodel *inModel;
    register CKTcircuit *ckt;
    SPcomplex *s;

        /* load the current values into the 
         * sparse matrix previously provided 
         * during AC analysis
         */
{
    register CSWmodel *model = (CSWmodel*)inModel;
    register CSWinstance *here;
    double g_now;
    int current_state;

    /*  loop through all the switch models */
    for( ; model != NULL; model = model->CSWnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->CSWinstances; here != NULL ;
                here=here->CSWnextInstance) {

            current_state = *(ckt->CKTstate0 + here->CSWstate);

            g_now = current_state?(model->CSWonConduct):(model->CSWoffConduct);

            *(here->CSWposPosptr) += g_now;
            *(here->CSWposNegptr) -= g_now;
            *(here->CSWnegPosptr) -= g_now;
            *(here->CSWnegNegptr) += g_now;
        }
    }
    return(OK);
}
