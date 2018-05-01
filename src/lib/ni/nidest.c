/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

    /* NIdestroy(ckt)
     * delete the data structures allocated for numeric integration.
     */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "cktdefs.h"
#include "suffix.h"


void
NIdestroy(ckt)
    register CKTcircuit *ckt;

{
    if (ckt->CKTmatrix)
	SMPdestroy(ckt->CKTmatrix);
    ckt->CKTmatrix = 0;
    if(ckt->CKTrhs)         FREE(ckt->CKTrhs);
    if(ckt->CKTrhsOld)      FREE(ckt->CKTrhsOld);
    if(ckt->CKTrhsSpare)    FREE(ckt->CKTrhsSpare);
    if(ckt->CKTirhs)        FREE(ckt->CKTirhs);
    if(ckt->CKTirhsOld)     FREE(ckt->CKTirhsOld);
    if(ckt->CKTirhsSpare)   FREE(ckt->CKTirhsSpare);
#ifdef HAS_SENSE2
    if(ckt->CKTsenInfo){
        if(ckt->CKTrhsOp) FREE(((CKTcircuit *)ckt)->CKTrhsOp);
        if(ckt->CKTsenRhs) FREE(((CKTcircuit *)ckt)->CKTsenRhs);
        if(ckt->CKTseniRhs) FREE(((CKTcircuit *)ckt)->CKTseniRhs);
        SENdestroy(ckt->CKTsenInfo);
    }
#endif
}
