/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/* Pretty print the sensitivity info for all 
 * the MOS1 devices in the circuit.
 */

#include "spice.h"
#include <stdio.h>
#include "smpdefs.h"
#include "cktdefs.h"
#include "mos1defs.h"
#include "util.h"
#include "sperror.h"
#include "suffix.h"

void
MOS1sPrint(inModel,ckt)
GENmodel *inModel;
register CKTcircuit *ckt;
/* Pretty print the sensitivity info for all the MOS1 
         * devices  in the circuit.
         */
{
    register MOS1model *model = (MOS1model *)inModel;
    register MOS1instance *here;

    printf("LEVEL 1 MOSFETS-----------------\n");
    /*  loop through all the MOS1 models */
    for( ; model != NULL; model = model->MOS1nextModel ) {

        printf("Model name:%s\n",model->MOS1modName);

        /* loop through all the instances of the model */
        for (here = model->MOS1instances; here != NULL ;
                here=here->MOS1nextInstance) {

            printf("    Instance name:%s\n",here->MOS1name);
            printf("      Drain, Gate , Source nodes: %s, %s ,%s\n",
            CKTnodName(ckt,here->MOS1dNode),CKTnodName(ckt,here->MOS1gNode),
            CKTnodName(ckt,here->MOS1sNode));

            printf("      Length: %g ",here->MOS1l);
            printf(here->MOS1lGiven ? "(specified)\n" : "(default)\n");
            printf("      Width: %g ",here->MOS1w);
            printf(here->MOS1wGiven ? "(specified)\n" : "(default)\n");
            if(here->MOS1sens_l == 1){
                printf("    MOS1senParmNo:l = %d ",here->MOS1senParmNo);
            }
            else{ 
                printf("    MOS1senParmNo:l = 0 ");
            }
            if(here->MOS1sens_w == 1){
                printf("    w = %d \n",here->MOS1senParmNo + here->MOS1sens_l);
            }
            else{ 
                printf("    w = 0 \n");
            }


        }
    }
}

