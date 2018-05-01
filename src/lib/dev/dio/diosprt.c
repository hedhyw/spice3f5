/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

/* Pretty print the sensitivity info for all 
 * the diodes in the circuit.
 */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "smpdefs.h"
#include "cktdefs.h"
#include "diodefs.h"
#include "sperror.h"
#include "suffix.h"


void
DIOsPrint(inModel,ckt)
    GENmodel *inModel;
    register CKTcircuit *ckt;
{
    register DIOmodel *model = (DIOmodel*)inModel;
    register DIOinstance *here;

    printf("DIOS-----------------\n");
    /*  loop through all the diode models */
    for( ; model != NULL; model = model->DIOnextModel ) {

        printf("Model name:%s\n",model->DIOmodName);

        /* loop through all the instances of the model */
        for (here = model->DIOinstances; here != NULL ;
                here=here->DIOnextInstance) {

            printf("    Instance name:%s\n",here->DIOname);
            printf("      Positive, negative nodes: %s, %s\n",
            CKTnodName(ckt,here->DIOposNode),CKTnodName(ckt,here->DIOnegNode));
            printf("      Area: %g ",here->DIOarea);
            printf(here->DIOareaGiven ? "(specified)\n" : "(default)\n");
            printf("    DIOsenParmNo:%d\n",here->DIOsenParmNo);

        }
    }
}
