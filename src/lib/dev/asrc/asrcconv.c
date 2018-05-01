/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1988 Kanwar Jit Singh
**********/

#include "spice.h"
#include <stdio.h>
#include "misc.h"
#include "cktdefs.h"
#include "asrcdefs.h"
#include "sperror.h"
#include "util.h"
#include "suffix.h"

int
ASRCconvTest( inModel, ckt)
GENmodel *inModel;
CKTcircuit *ckt;
{
    register ASRCmodel *model = (ASRCmodel *)inModel;
    register ASRCinstance *here;
    int i, node_num, branch;
    double diff;
    double prev;
    double tol;
    double rhs;

    for( ; model != NULL; model = model->ASRCnextModel) {
        for( here = model->ASRCinstances; here != NULL;
                here = here->ASRCnextInstance) {
	    i = here->ASRCtree->numVars;
	    if (asrc_nvals < i) {
		if (asrc_nvals) {
			FREE(asrc_vals);
			FREE(asrc_derivs);
		}
		asrc_nvals = i;
		asrc_vals = NEWN(double, i);
		asrc_derivs = NEWN(double, i);
	    }

            for( i=0; i < here->ASRCtree->numVars; i++){
                if( here->ASRCtree->varTypes[i] == IF_INSTANCE){
                     branch = CKTfndBranch(ckt,here->ASRCtree->vars[i].uValue);
                     asrc_vals[i] = *(ckt->CKTrhsOld+branch);
                } else {
                    node_num = ((CKTnode *)(here->ASRCtree->vars[i].nValue))
                            ->number;
                    asrc_vals[i] = *(ckt->CKTrhsOld+node_num);
                }
            }

            if( (*(here->ASRCtree->IFeval))(here->ASRCtree, ckt->CKTgmin, &rhs,
                    asrc_vals,asrc_derivs) == OK){

                prev = here->ASRCprev_value;
                diff = FABS( prev - rhs);
                if ( here->ASRCtype == ASRC_VOLTAGE){
                    tol = ckt->CKTreltol * 
                            MAX(FABS(rhs),FABS(prev)) + ckt->CKTvoltTol;
                } else {
                    tol = ckt->CKTreltol * 
                            MAX(FABS(rhs),FABS(prev)) + ckt->CKTabstol;
                }

                if ( diff > tol) {
                    ckt->CKTnoncon++;
		    ckt->CKTtroubleElt = (GENinstance *) here;
                    return(OK);
                }
            } else {

                return(E_BADPARM);
            }
        }
    }
    return(OK);
}
