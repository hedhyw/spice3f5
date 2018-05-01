/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "ifsim.h"
#include "util.h"
#include "ccvsdefs.h"
#include "sperror.h"
#include "suffix.h"


/* ARGSUSED */
int
CCVSparam(param,value,inst,select)
    int param;
    IFvalue *value;
    GENinstance *inst;
    IFvalue *select;
{
    CCVSinstance *here = (CCVSinstance*)inst;
    switch(param) {
        case CCVS_TRANS:
            here->CCVScoeff = value->rValue;
            here->CCVScoeffGiven = TRUE;
            break;
        case CCVS_CONTROL:
            here->CCVScontName = value->uValue;
            break;
        case CCVS_TRANS_SENS:
            here->CCVSsenParmNo = value->iValue;
            break;
        default:
            return(E_BADPARM);
    }
    return(OK);
}
