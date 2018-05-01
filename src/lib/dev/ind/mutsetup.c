/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

        /* load the inductor structure with those pointers needed later 
         * for fast matrix loading 
         */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "ifsim.h"
#include "smpdefs.h"
#include "cktdefs.h"
#include "inddefs.h"
#include "sperror.h"
#include "suffix.h"


#ifdef MUTUAL
/*ARGSUSED*/
int
MUTsetup(matrix,inModel,ckt,states)
    register SMPmatrix *matrix;
    GENmodel *inModel;
    CKTcircuit *ckt;
    int *states;
{
    register MUTmodel *model = (MUTmodel*)inModel;
    register MUTinstance *here;
    int ktype;
    int error;

    /*  loop through all the inductor models */
    for( ; model != NULL; model = model->MUTnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->MUTinstances; here != NULL ;
                here=here->MUTnextInstance) {
            
            ktype = CKTtypelook("Inductor");
            if(ktype <= 0) {
                (*(SPfrontEnd->IFerror))(ERR_PANIC,
                        "mutual inductor, but inductors not available!",
                        (IFuid *)NULL);
                return(E_INTERN);
            }

            error = CKTfndDev((GENERIC*)ckt,&ktype,(GENERIC**)&(here->MUTind1),
                    here->MUTindName1, (GENERIC *)NULL,(char *)NULL);
            if(error && error!= E_NODEV && error != E_NOMOD) return(error);
            if(error) {
                IFuid namarray[2];
                namarray[0]=here->MUTname;
                namarray[1]=here->MUTindName1;
                (*(SPfrontEnd->IFerror))(ERR_WARNING,
                    "%s: coupling to non-existant inductor %s.",
                    namarray);
            }
            error = CKTfndDev((GENERIC*)ckt,&ktype,(GENERIC**)&(here->MUTind2),
                    here->MUTindName2,(GENERIC *)NULL,(char *)NULL);
            if(error && error!= E_NODEV && error != E_NOMOD) return(error);
            if(error) {
                IFuid namarray[2];
                namarray[0]=here->MUTname;
                namarray[1]=here->MUTindName2;
                (*(SPfrontEnd->IFerror))(ERR_WARNING,
                    "%s: coupling to non-existant inductor %s.",
                    namarray);
            }

            here->MUTfactor = here->MUTcoupling *sqrt(here->MUTind1->INDinduct *
                    here->MUTind2->INDinduct);


/* macro to make elements with built in test for out of memory */
#define TSTALLOC(ptr,first,second) \
if((here->ptr = SMPmakeElt(matrix,here->first,here->second))==(double *)NULL){\
    return(E_NOMEM);\
}

            TSTALLOC(MUTbr1br2,MUTind1->INDbrEq,MUTind2->INDbrEq)
            TSTALLOC(MUTbr2br1,MUTind2->INDbrEq,MUTind1->INDbrEq)
        }
    }
    return(OK);
}
#endif /* MUTUAL */
