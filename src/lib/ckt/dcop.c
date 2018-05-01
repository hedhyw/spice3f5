/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "util.h"
#include "sperror.h"
#include "ifsim.h"
#include "suffix.h"

int
DCop(ckt) 
    CKTcircuit *ckt;
{
    int i;
    int CKTload();
    int converged;
    int error;
    int size;
    long save;
    int senmode;
    IFuid *nameList;
    int numNames;
    GENERIC *plot;
    
    error = CKTnames(ckt,&numNames,&nameList);
    if(error) return(error);
    error = (*(SPfrontEnd->OUTpBeginPlot))((GENERIC *)ckt,
	(GENERIC*)ckt->CKTcurJob, ckt->CKTcurJob->JOBname,
	(IFuid)NULL,IF_REAL,numNames,nameList, IF_REAL,&plot);
    if(error) return(error);

    converged = CKTop(ckt,
            (ckt->CKTmode & MODEUIC) | MODEDCOP | MODEINITJCT,
            (ckt->CKTmode & MODEUIC) | MODEDCOP | MODEINITFLOAT,
            ckt->CKTdcMaxIter);
    if(converged != 0) return(converged);

    ckt->CKTmode = (ckt->CKTmode & MODEUIC) | MODEDCOP | MODEINITSMSIG;


#ifdef HAS_SENSE2
    if(ckt->CKTsenInfo && ((ckt->CKTsenInfo->SENmode&DCSEN) || 
            (ckt->CKTsenInfo->SENmode&ACSEN)) ){
#ifdef SENSDEBUG
         printf("\nDC Operating Point Sensitivity Results\n\n");
         CKTsenPrint(ckt);
#endif /* SENSDEBUG */
         senmode = ckt->CKTsenInfo->SENmode;
         save = ckt->CKTmode;
         ckt->CKTsenInfo->SENmode = DCSEN;
         size = SMPmatSize(ckt->CKTmatrix);
         for(i = 1; i<=size ; i++){
             *(ckt->CKTrhsOp + i) = *(ckt->CKTrhsOld + i);
         }
         if(error = CKTsenDCtran(ckt)) return(error);
         ckt->CKTmode = save;
         ckt->CKTsenInfo->SENmode = senmode;

    }
#endif
    converged = CKTload(ckt);
    CKTdump(ckt,(double)0,plot);
    (*(SPfrontEnd->OUTendPlot))(plot);
    return(converged);
}
