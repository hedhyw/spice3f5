/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

    /* load the voltage source structure with those pointers needed later 
     * for fast matrix loading 
     */

#include "spice.h"
#include <stdio.h>
#include "smpdefs.h"
#include "cktdefs.h"
#include "cccsdefs.h"
#include "sperror.h"
#include "suffix.h"


/*ARGSUSED*/
int
CCCSsetup(matrix,inModel,ckt,states)
    register SMPmatrix *matrix;
    GENmodel *inModel;
    register CKTcircuit *ckt;
    int *states;
{
    register CCCSmodel *model = (CCCSmodel*)inModel;
    register CCCSinstance *here;

    /*  loop through all the voltage source models */
    for( ; model != NULL; model = model->CCCSnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->CCCSinstances; here != NULL ;
                here=here->CCCSnextInstance) {
            
            here->CCCScontBranch = CKTfndBranch(ckt,here->CCCScontName);
            if(here->CCCScontBranch == 0) {
                IFuid namarray[2];
                namarray[0] = here->CCCSname;
                namarray[1] = here->CCCScontName;
                (*(SPfrontEnd->IFerror))(ERR_FATAL,
                        "%s: unknown controlling source %s",namarray);
                return(E_BADPARM);
            }

/* macro to make elements with built in test for out of memory */
#define TSTALLOC(ptr,first,second) \
if((here->ptr = SMPmakeElt(matrix,here->first,here->second))==(double *)NULL){\
    return(E_NOMEM);\
}

            TSTALLOC(CCCSposContBrptr,CCCSposNode,CCCScontBranch)
            TSTALLOC(CCCSnegContBrptr,CCCSnegNode,CCCScontBranch)
        }
    }
    return(OK);
}
