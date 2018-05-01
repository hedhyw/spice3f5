/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "smpdefs.h"
#include "cktdefs.h"
#include "vsrcdefs.h"
#include "sperror.h"
#include "suffix.h"

/* ARGSUSED */
int
VSRCpzSetup(matrix,inModel,ckt,state)
    register SMPmatrix *matrix;
    GENmodel *inModel;
    register CKTcircuit *ckt;
    int *state;
        /* load the voltage source structure with those pointers needed later 
         * for fast matrix loading 
         */
{
    register VSRCmodel *model = (VSRCmodel *)inModel;
    register VSRCinstance *here;
    CKTnode *tmp;
    int error;

    /*  loop through all the voltage source models */
    for( ; model != NULL; model = model->VSRCnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->VSRCinstances; here != NULL ;
                here=here->VSRCnextInstance) {
            
            if(here->VSRCbranch == 0) {
                error = CKTmkCur(ckt,&tmp,here->VSRCname,"branch");
                if(error) return(error);
                here->VSRCbranch = tmp->number;
            }

/* macro to make elements with built in test for out of memory */
#define TSTALLOC(ptr,first,second) \
if((here->ptr = SMPmakeElt(matrix,here->first,here->second))==(double *)NULL){\
    return(E_NOMEM);\
}

            TSTALLOC(VSRCposIbrptr, VSRCposNode, VSRCbranch)
            TSTALLOC(VSRCnegIbrptr, VSRCnegNode, VSRCbranch)
            TSTALLOC(VSRCibrNegptr, VSRCbranch, VSRCnegNode)
            TSTALLOC(VSRCibrPosptr, VSRCbranch, VSRCposNode)
            TSTALLOC(VSRCibrIbrptr, VSRCbranch, VSRCbranch)
        }
    }
    return(OK);
}
