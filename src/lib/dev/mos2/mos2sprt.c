/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "smpdefs.h"
#include "cktdefs.h"
#include "mos2defs.h"
#include "util.h"
#include "sperror.h"
#include "suffix.h"

void
MOS2sPrint(inModel,ckt)
    GENmodel *inModel;
    register CKTcircuit *ckt;
        /* Pretty print the sensitivity info for all the MOS2 
         * devices  in the circuit.
         */
{
    register MOS2model *model = (MOS2model *)inModel;
    register MOS2instance *here;

    printf("LEVEL 1 MOSFETS-----------------\n");
    /*  loop through all the MOS2 models */
    for( ; model != NULL; model = model->MOS2nextModel ) {

        printf("Model name:%s\n",model->MOS2modName);

        /* loop through all the instances of the model */
        for (here = model->MOS2instances; here != NULL ;
                here=here->MOS2nextInstance) {

            printf("    Instance name:%s\n",here->MOS2name);
            printf("      Drain, Gate , Source nodes: %s, %s ,%s\n",
            CKTnodName(ckt,here->MOS2dNode),CKTnodName(ckt,here->MOS2gNode),
            CKTnodName(ckt,here->MOS2sNode));

            printf("      Length: %g ",here->MOS2l);
            printf(here->MOS2lGiven ? "(specified)\n" : "(default)\n");
            printf("      Width: %g ",here->MOS2w);
            printf(here->MOS2wGiven ? "(specified)\n" : "(default)\n");
            if(here->MOS2sens_l == 1){
                printf("    MOS2senParmNo:l = %d ",here->MOS2senParmNo);
            }
            else{ 
                printf("    MOS2senParmNo:l = 0 ");
            }
            if(here->MOS2sens_w == 1){
                printf("    w = %d \n",here->MOS2senParmNo + here->MOS2sens_l);
            }
            else{ 
                printf("    w = 0 \n");
            }


        }
    }
}

