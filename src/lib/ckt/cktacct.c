/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

    /*
     *  CKTacct
     *  get the specified accounting item into 'value' in the 
     *  given circuit 'ckt'.
     */

#include "spice.h"
#include "const.h"
#include "optdefs.h"
#include "ifsim.h"
#include <stdio.h>
#include "util.h"
#include "cktdefs.h"
#include "suffix.h"


/* ARGSUSED */
int
CKTacct(ckt,anal,which,val)
    CKTcircuit *ckt;
    GENERIC *anal;
    int which;
    IFvalue *val;
{
    switch(which) {
        
    case OPT_EQNS:
        val->iValue = ckt->CKTmaxEqNum;
        break;
    case OPT_ITERS:
        val->iValue = ckt->CKTstat->STATnumIter;
        break;
    case OPT_TRANIT:
        val->iValue = ckt->CKTstat->STATtranIter;
        break;
    case OPT_TRANCURITER:
        val->iValue = ckt->CKTstat->STATnumIter - ckt->CKTstat->STAToldIter;
        break;
    case OPT_TRANPTS:
        val->iValue = ckt->CKTstat->STATtimePts;
        break;
    case OPT_TRANACCPT:
        val->iValue = ckt->CKTstat->STATaccepted;
        break;
    case OPT_TRANRJCT:
        val->iValue = ckt->CKTstat->STATrejected;
        break;
    case OPT_TOTANALTIME:
        val->rValue = ckt->CKTstat->STATtotAnalTime;
        break;
    case OPT_TRANTIME:
        val->rValue = ckt->CKTstat->STATtranTime;
        break;
    case OPT_LOADTIME:
        val->rValue = ckt->CKTstat->STATloadTime;
        break;
    case OPT_REORDTIME:
        val->rValue = ckt->CKTstat->STATreorderTime;
        break;
    case OPT_DECOMP:
        val->rValue = ckt->CKTstat->STATdecompTime;
        break;
    case OPT_SOLVE:
        val->rValue = ckt->CKTstat->STATsolveTime;
        break;
    case OPT_TRANDECOMP:
        val->rValue = ckt->CKTstat->STATtranDecompTime;
        break;
    case OPT_TRANSOLVE:
        val->rValue = ckt->CKTstat->STATtranSolveTime;
        break;
    case OPT_TEMP:
        val->rValue = ckt->CKTtemp - CONSTCtoK;
        break;
    case OPT_TNOM:
        val->rValue = ckt->CKTnomTemp - CONSTCtoK;
        break;
    default:
        return(-1);
    }
    return(0);
}
