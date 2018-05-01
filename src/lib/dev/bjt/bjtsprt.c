/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/* Pretty print the sensitivity info for all 
 * the bjts in the circuit.
 */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "cktdefs.h"
#include "smpdefs.h"
#include "bjtdefs.h"
#include "const.h"
#include "sperror.h"
#include "ifsim.h"
#include "suffix.h"


void
BJTsPrint(inModel,ckt)

register CKTcircuit *ckt;
GENmodel *inModel;
{
    register BJTmodel *model = (BJTmodel*)inModel;
    register BJTinstance *here;

    printf("BJTS-----------------\n");
    /*  loop through all the BJT models */
    for( ; model != NULL; model = model->BJTnextModel ) {

        printf("Model name:%s\n",model->BJTmodName);

        /* loop through all the instances of the model */
        for (here = model->BJTinstances; here != NULL ;
                here=here->BJTnextInstance) {
            ckt->CKTsenInfo->SEN_parmVal[here->BJTsenParmNo] = here->BJTarea;

            printf("    Instance name:%s\n",here->BJTname);
            printf("      Collector, Base , Emitter nodes: %s, %s ,%s\n",
            CKTnodName(ckt,here->BJTcolNode),CKTnodName(ckt,here->BJTbaseNode),
            CKTnodName(ckt,here->BJTemitNode));

            printf("      Area: %g ",here->BJTarea);
            printf(here->BJTareaGiven ? "(specified)\n" : "(default)\n");
            printf("    BJTsenParmNo:%d\n",here->BJTsenParmNo);

        }
    }
}

