/**********
Copyright 1991 Regents of the University of California.  All rights reserved.
**********/

#include "spice.h"
#include "cktdefs.h"
#include "trandefs.h"
#include "iferrmsg.h"
#include "suffix.h"

/*
 * this used to be in setup, but we need it here now
 * (must be done after mode is set as below)
 */

TRANinit(ckt,job)
    CKTcircuit	*ckt;
    JOB		*job;
{
    ckt->CKTfinalTime = ((TRANan*)job)->TRANfinalTime;
    ckt->CKTstep = ((TRANan*)job)->TRANstep;
    ckt->CKTinitTime = ((TRANan*)job)->TRANinitTime;
    ckt->CKTmaxStep = ((TRANan*)job)->TRANmaxStep;
    if(ckt->CKTmaxStep == 0) {
	ckt->CKTmaxStep = (ckt->CKTfinalTime-ckt->CKTinitTime)/50;
    }
    ckt->CKTdelmin = 1e-9*ckt->CKTmaxStep;	/* XXX */
    ckt->CKTmode = ((TRANan*)job)->TRANmode;

    return OK;
}
