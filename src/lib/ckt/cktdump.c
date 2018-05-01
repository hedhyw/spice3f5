/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

    /* CKTdump(ckt)
     * this is a simple program to dump the rhs vector to stdout
     */

#include "spice.h"
#include <stdio.h>
#include "smpdefs.h"
#include "cktdefs.h"
#include "suffix.h"


void
CKTdump(ckt,ref,plot)
    register CKTcircuit *ckt;
    double ref;
    GENERIC *plot;
{
    IFvalue refData;
    IFvalue valData;

    refData.rValue = ref;
    valData.v.numValue = ckt->CKTmaxEqNum-1;
    valData.v.vec.rVec = ckt->CKTrhsOld+1;
    (*(SPfrontEnd->OUTpData))(plot,&refData,&valData);
}
