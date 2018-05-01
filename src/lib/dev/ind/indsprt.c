/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/* Pretty print the sensitivity info for all 
 * the inductors in the circuit.
 */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "smpdefs.h"
#include "cktdefs.h"
#include "inddefs.h"
#include "sperror.h"
#include "suffix.h"

void
INDsPrint(inModel,ckt)
GENmodel *inModel;
register CKTcircuit *ckt;
{
    register INDmodel *model = (INDmodel*)inModel;
    register INDinstance *here;

    printf("INDUCTORS----------\n");
    /*  loop through all the inductor models */
    for( ; model != NULL; model = model->INDnextModel ) {

        printf("Model name:%s\n",model->INDmodName);

        /* loop through all the instances of the model */
        for (here = model->INDinstances; here != NULL ;
                here=here->INDnextInstance) {

            printf("    Instance name:%s\n",here->INDname);
            printf("      Positive, negative nodes: %s, %s\n",
            CKTnodName(ckt,here->INDposNode),CKTnodName(ckt,here->INDnegNode));
            printf("      Branch Equation: %s\n",CKTnodName(ckt,here->INDbrEq));
            printf("      Inductance: %g ",here->INDinduct);
            printf(here->INDindGiven ? "(specified)\n" : "(default)\n");
            printf("    INDsenParmNo:%d\n",here->INDsenParmNo);
        }
    }
}
