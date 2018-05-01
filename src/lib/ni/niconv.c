/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/*
 * NIconvTest(ckt)
 *  perform the convergence test - returns 1 if any of the 
 *  values in the old and new arrays have changed by more 
 *  than absTol + relTol*(max(old,new)), otherwise returns 0
 */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "smpdefs.h"
#include "util.h"
#include "suffix.h"

int
NIconvTest(ckt)
    register CKTcircuit *ckt;

{
    register int i; /* generic loop variable */
    register int size;  /* size of the matrix */
    register CKTnode *node; /* current matrix entry */
    double old;
    double new;
    double tol;

    node = ckt->CKTnodes;
    size = SMPmatSize(ckt->CKTmatrix);
    for (i=1;i<=size;i++) {
        node = node->next;
        new =  *((ckt->CKTrhs) + i ) ;
        old =  *((ckt->CKTrhsOld) + i ) ;
        if(node->type == 3) {
            tol =  ckt->CKTreltol * (MAX(FABS(old),FABS(new))) +
                    ckt->CKTvoltTol;
            if (FABS(new-old) >tol ) {
#ifdef STEPDEBUG
                printf(" non-convergence at node %s\n",CKTnodName(ckt,i));
#endif /* STEPDEBUG */
		ckt->CKTtroubleNode = i;
		ckt->CKTtroubleElt = NULL;
                return(1);
            }
        } else {
            tol =  ckt->CKTreltol * (MAX(FABS(old),FABS(new))) +
                    ckt->CKTabstol;
            if (FABS(new-old) >tol ) {
#ifdef STEPDEBUG
                printf(" non-convergence at node %s\n",CKTnodName(ckt,i));
#endif /* STEPDEBUG */
		ckt->CKTtroubleNode = i;
		ckt->CKTtroubleElt = NULL;
                return(1);
            }
        }
    }


#ifdef NEWCONV
    i = CKTconvTest(ckt);
    if (i)
	ckt->CKTtroubleNode = 0;
    return(i);
#else /* NEWCONV */
    return(0);
#endif /* NEWCONV */
}
