/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "smpdefs.h"
#include "cktdefs.h"
#include "tradefs.h"
#include "sperror.h"
#include "suffix.h"


/* ARGSUSED */
int
TRAsetup(matrix,inModel,ckt,state)
    register SMPmatrix *matrix;
    GENmodel *inModel;
    register CKTcircuit *ckt;
    int *state;
        /* load the transmission line structure with those pointers needed later
         * for fast matrix loading 
         */
{
    register TRAmodel *model = (TRAmodel *)inModel;
    register TRAinstance *here;
    int error;
    CKTnode *tmp;

    /*  loop through all the transmission line models */
    for( ; model != NULL; model = model->TRAnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->TRAinstances; here != NULL ;
                here=here->TRAnextInstance) {
            
            if(here->TRAbrEq1==0) {
                error = CKTmkVolt(ckt,&tmp,here->TRAname,"i1");
                if(error) return(error);
                here->TRAbrEq1 = tmp->number;
            }

            if(here->TRAbrEq2==0) {
                error = CKTmkVolt(ckt,&tmp,here->TRAname,"i2");
                if(error) return(error);
                here->TRAbrEq2 = tmp->number;
            }

            if(here->TRAintNode1==0) {
                error = CKTmkVolt(ckt,&tmp,here->TRAname,"int1");
                if(error) return(error);
                here->TRAintNode1 = tmp->number;
            }

            if(here->TRAintNode2==0) {
                error = CKTmkVolt(ckt,&tmp,here->TRAname,"int2");
                if(error) return(error);
                here->TRAintNode2 = tmp->number;
            }

            /* allocate the delay table */
            here->TRAdelays = (double *)MALLOC(15*sizeof(double));
            here->TRAallocDelay = 4;

/* macro to make elements with built in test for out of memory */
#define TSTALLOC(ptr,first,second) \
if((here->ptr = SMPmakeElt(matrix,here->first,here->second))==(double *)NULL){\
    return(E_NOMEM);\
}

            TSTALLOC(TRAibr1Ibr2Ptr, TRAbrEq1, TRAbrEq2)
            TSTALLOC(TRAibr1Int1Ptr, TRAbrEq1, TRAintNode1)
            TSTALLOC(TRAibr1Neg1Ptr, TRAbrEq1, TRAnegNode1)
            TSTALLOC(TRAibr1Neg2Ptr, TRAbrEq1, TRAnegNode2)
            TSTALLOC(TRAibr1Pos2Ptr, TRAbrEq1, TRAposNode2)
            TSTALLOC(TRAibr2Ibr1Ptr, TRAbrEq2, TRAbrEq1)
            TSTALLOC(TRAibr2Int2Ptr, TRAbrEq2, TRAintNode2)
            TSTALLOC(TRAibr2Neg1Ptr, TRAbrEq2, TRAnegNode1)
            TSTALLOC(TRAibr2Neg2Ptr, TRAbrEq2, TRAnegNode2)
            TSTALLOC(TRAibr2Pos1Ptr, TRAbrEq2, TRAposNode1)
            TSTALLOC(TRAint1Ibr1Ptr, TRAintNode1, TRAbrEq1)
            TSTALLOC(TRAint1Int1Ptr, TRAintNode1, TRAintNode1)
            TSTALLOC(TRAint1Pos1Ptr, TRAintNode1, TRAposNode1)
            TSTALLOC(TRAint2Ibr2Ptr, TRAintNode2, TRAbrEq2)
            TSTALLOC(TRAint2Int2Ptr, TRAintNode2, TRAintNode2)
            TSTALLOC(TRAint2Pos2Ptr, TRAintNode2, TRAposNode2)
            TSTALLOC(TRAneg1Ibr1Ptr, TRAnegNode1, TRAbrEq1)
            TSTALLOC(TRAneg2Ibr2Ptr, TRAnegNode2, TRAbrEq2)
            TSTALLOC(TRApos1Int1Ptr, TRAposNode1, TRAintNode1)
            TSTALLOC(TRApos1Pos1Ptr, TRAposNode1, TRAposNode1)
            TSTALLOC(TRApos2Int2Ptr, TRAposNode2, TRAintNode2)
            TSTALLOC(TRApos2Pos2Ptr, TRAposNode2, TRAposNode2)

            if(!here->TRAnlGiven) {
                here->TRAnl = .25;
            }
            if(!here->TRAfGiven) {
                here->TRAf = 1e9;
            }
            if(!here->TRAreltolGiven) {
                here->TRAreltol = 1;
            }
            if(!here->TRAabstolGiven) {
                here->TRAabstol = 1;
            }
            if(!here->TRAimpedGiven) {
                (*(SPfrontEnd->IFerror))(ERR_FATAL,
                        "%s: transmission line z0 must be given",
                        &(here->TRAname));
                return(E_BADPARM);
            }
        }
    }
    return(OK);
}

int
TRAunsetup(inModel,ckt)
    GENmodel *inModel;
    CKTcircuit *ckt;
{
#ifndef HAS_BATCHSIM
    TRAmodel *model;
    TRAinstance *here;

    for (model = (TRAmodel *)inModel; model != NULL;
	    model = model->TRAnextModel)
    {
        for (here = model->TRAinstances; here != NULL;
                here=here->TRAnextInstance)
	{
	    if (here->TRAbrEq1) {
		CKTdltNNum(ckt, here->TRAbrEq1);
		here->TRAbrEq1= 0;
	    }
	    if (here->TRAbrEq2) {
		CKTdltNNum(ckt, here->TRAbrEq2);
		here->TRAbrEq2= 0;
	    }
	    if (here->TRAintNode1) {
		CKTdltNNum(ckt, here->TRAintNode1);
		here->TRAintNode1= 0;
	    }
	    if (here->TRAintNode2) {
		CKTdltNNum(ckt, here->TRAintNode2);
		here->TRAintNode2= 0;
	    }
	}
    }
#endif
    return OK;
}
