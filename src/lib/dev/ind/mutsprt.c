/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/* Pretty print the sensitivity info for all 
 * the mutual inductors in the circuit.
 */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "smpdefs.h"
#include "cktdefs.h"
#include "inddefs.h"
#include "sperror.h"
#include "suffix.h"


#ifdef MUTUAL
/* ARGSUSED */
void
MUTsPrint(inModel,ckt)
    GENmodel *inModel;
    CKTcircuit* ckt;
{
    register MUTmodel *model = (MUTmodel*)inModel;
    register MUTinstance *here;

    printf("MUTUAL INDUCTORS-----------------\n");
    /*  loop through all the inductor models */
    for( ; model != NULL; model = model->MUTnextModel ) {

        printf("Model name:%s\n",model->MUTmodName);

        /* loop through all the instances of the model */
        for (here = model->MUTinstances; here != NULL ;
                here=here->MUTnextInstance) {

            printf("    Instance name:%s\n",here->MUTname);
            printf("      Mutual Inductance: %g ",here->MUTcoupling);
            printf(here->MUTindGiven ? "(specified)\n" : "(default)\n");
            printf("      coupling factor: %g \n",here->MUTfactor);
            printf("      inductor 1 name: %s \n",here->MUTindName1);
            printf("      inductor 2 name: %s \n",here->MUTindName2);
            printf("    MUTsenParmNo:%d\n",here->MUTsenParmNo);

        }
    }
}
#endif /* MUTUAL */
