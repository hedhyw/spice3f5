/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "complex.h"
#include "sperror.h"
#include "inddefs.h"
#include "suffix.h"


#ifdef MUTUAL
/* ARGSUSED */
int
MUTpzLoad(inModel,ckt,s)
    GENmodel *inModel;
    CKTcircuit *ckt;
    register SPcomplex *s;
{
    register MUTmodel *model = (MUTmodel*)inModel;
    double val;
    register MUTinstance *here;

    for( ; model != NULL; model = model->MUTnextModel) {
        for( here = model->MUTinstances;here != NULL; 
                here = here->MUTnextInstance) {
    
            val =  here->MUTfactor;
            *(here->MUTbr1br2 ) -= val * s->real;
            *(here->MUTbr1br2 +1) -= val * s->imag;
            *(here->MUTbr2br1 ) -= val * s->real;
            *(here->MUTbr2br1 +1) -= val * s->imag;
        }
    }
    return(OK);

}
#endif /*MUTUAL*/
