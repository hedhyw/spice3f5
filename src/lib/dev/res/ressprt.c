/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

/* Pretty print the sensitivity info for all 
 * the resistors in the circuit.
 */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "resdefs.h"
#include "sperror.h"
#include "suffix.h"


void
RESsPrint(inModel,ckt)
    GENmodel *inModel;
    register CKTcircuit *ckt;
{
    register RESmodel *model = (RESmodel *)inModel;
    register RESinstance *here;
    printf("RESISTORS-----------------\n");

    /*  loop through all the resistor models */
    for( ; model != NULL; model = model->RESnextModel ) {

        printf("Model name:%s\n",model->RESmodName);

        /* loop through all the instances of the model */
        for (here = model->RESinstances; here != NULL ;
                here=here->RESnextInstance) {

            printf("    Instance name:%s\n",here->RESname);
            printf("      Positive, negative nodes: %s, %s\n",
            CKTnodName(ckt,here->RESposNode),CKTnodName(ckt,here->RESnegNode));
            printf("      Resistance: %f ",here->RESresist);
            printf(here->RESresGiven ? "(specified)\n" : "(default)\n");
            printf("    RESsenParmNo:%d\n",here->RESsenParmNo);

        }
    }
}
