/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

/* Pretty print the sensitivity info for all 
 * the capacitors in the circuit.
 */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "cktdefs.h"
#include "capdefs.h"
#include "trandefs.h"
#include "sperror.h"
#include "suffix.h"


void
CAPsPrint(inModel,ckt)
    GENmodel *inModel;
    register CKTcircuit *ckt;
{
    register CAPmodel *model = (CAPmodel*)inModel;
    register CAPinstance *here;

    printf("CAPACITORS-----------------\n");
    /*  loop through all the capacitor models */
    for( ; model != NULL; model = model->CAPnextModel ) {

        printf("Model name:%s\n",model->CAPmodName);

        /* loop through all the instances of the model */
        for (here = model->CAPinstances; here != NULL ;
                here=here->CAPnextInstance) {

            printf("    Instance name:%s\n",here->CAPname);
            printf("      Positive, negative nodes: %s, %s\n",
            CKTnodName(ckt,here->CAPposNode),CKTnodName(ckt,here->CAPnegNode));
            printf("      Capacitance: %e",here->CAPcapac);
            printf(here->CAPcapGiven ? "(specified)\n" : "(default)\n");
            printf("    CAPsenParmNo:%d\n",here->CAPsenParmNo);

        }
    }
}

