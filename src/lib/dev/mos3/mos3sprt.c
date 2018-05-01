/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

    /* Pretty print the sensitivity info for all the MOS3 
     * devices  in the circuit.
     */

#include "spice.h"
#include <stdio.h>
#include "smpdefs.h"
#include "cktdefs.h"
#include "mos3defs.h"
#include "util.h"
#include "sperror.h"
#include "suffix.h"

void
MOS3sPrint(inModel,ckt)
    GENmodel *inModel;
    register CKTcircuit *ckt;
{
    register MOS3model *model = (MOS3model *)inModel;
    register MOS3instance *here;

    printf("LEVEL 1 MOSFETS-----------------\n");
    /*  loop through all the MOS3 models */
    for( ; model != NULL; model = model->MOS3nextModel ) {

        printf("Model name:%s\n",model->MOS3modName);

        /* loop through all the instances of the model */
        for (here = model->MOS3instances; here != NULL ;
                here=here->MOS3nextInstance) {

            printf("    Instance name:%s\n",here->MOS3name);
            printf("      Drain, Gate , Source nodes: %s, %s ,%s\n",
            CKTnodName(ckt,here->MOS3dNode),CKTnodName(ckt,here->MOS3gNode),
            CKTnodName(ckt,here->MOS3sNode));

            printf("      Length: %g ",here->MOS3l);
            printf(here->MOS3lGiven ? "(specified)\n" : "(default)\n");
            printf("      Width: %g ",here->MOS3w);
            printf(here->MOS3wGiven ? "(specified)\n" : "(default)\n");
            if(here->MOS3sens_l == 1){
                printf("    MOS3senParmNo:l = %d ",here->MOS3senParmNo);
            }
            else{ 
                printf("    MOS3senParmNo:l = 0 ");
            }
            if(here->MOS3sens_w == 1){
                printf("    w = %d \n",here->MOS3senParmNo + here->MOS3sens_l);
            }
            else{ 
                printf("    w = 0 \n");
            }


        }
    }
}

