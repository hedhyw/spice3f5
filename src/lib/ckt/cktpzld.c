/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
**********/

#include "spice.h"
#include <stdio.h>
#include "pzdefs.h"
#include "smpdefs.h"
#include "cktdefs.h"
#include "complex.h"
#include "devdefs.h"
#include "sperror.h"
#include "suffix.h"

extern SPICEdev *DEVices[];

int
CKTpzLoad(ckt, s)
    CKTcircuit *ckt;
    SPcomplex *s;
{
    PZAN *pzan = (PZAN *) (ckt->CKTcurJob);
    SMPelement *elt, *telt;
    int solution_col;
    int error;
    int i;

    for (i = 0; i <= SMPmatSize(ckt->CKTmatrix); i++) {
	ckt->CKTrhs[i] = 0.0;
	ckt->CKTirhs[i] = 0.0;
    }

    SMPcClear(ckt->CKTmatrix);
    for (i = 0; i < DEVmaxnum; i++) {
        if (DEVices[i]->DEVpzLoad != NULL && ckt->CKThead[i] != NULL) {
            error = (*DEVices[i]->DEVpzLoad)(ckt->CKThead[i], ckt, s);
            if (error)
	        return(error);
        }
    }

#ifdef notdef
    printf("*** Before PZ adjustments *\n");
    SMPprint(ckt->CKTmatrix, stdout);
#endif

    if (pzan->PZbalance_col && pzan->PZsolution_col) {
	SMPcAddCol(ckt->CKTmatrix, pzan->PZbalance_col, pzan->PZsolution_col);
	/* AC sources ?? XXX */
    }

    if (pzan->PZsolution_col) {
	SMPcZeroCol(ckt->CKTmatrix, pzan->PZsolution_col);
    }

    /* Driving function (current source) */
    if (pzan->PZdrive_pptr)
	*pzan->PZdrive_pptr = 1.0;
    if (pzan->PZdrive_nptr)
	*pzan->PZdrive_nptr = -1.0;

#ifdef notdef
    printf("*** After PZ adjustments *\n");
    SMPprint(ckt->CKTmatrix, stdout);
#endif

    return(OK);
}
