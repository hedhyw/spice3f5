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
#include "cccsdefs.h"
#include "sperror.h"
#include "suffix.h"


/* ARGSUSED */
int
CCCSparam(param,value,inst,select)
    int param;
    IFvalue *value;
    GENinstance *inst;
    IFvalue *select;
{
    CCCSinstance *here = (CCCSinstance*)inst;
    switch(param) {
        case CCCS_GAIN:
            here->CCCScoeff = value->rValue;
            here->CCCScoeffGiven = TRUE;
            break;
        case CCCS_CONTROL:
            here->CCCScontName = value->uValue;
            break;
        case CCCS_GAIN_SENS:
            here->CCCSsenParmNo = value->iValue;
            break;
        default:
            return(E_BADPARM);
    }
    return(OK);
}
