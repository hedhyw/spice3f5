/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "const.h"
#include "util.h"
#include "ifsim.h"
#include "resdefs.h"
#include "sperror.h"
#include "suffix.h"


/* ARGSUSED */
int
RESparam(param,value,inst,select)
    int param;
    IFvalue *value;
    GENinstance *inst;
    IFvalue *select;
{
    RESinstance *here = (RESinstance *)inst;
    switch(param) {
        case RES_TEMP:
            here->REStemp = value->rValue+CONSTCtoK;
            here->REStempGiven = TRUE;
            break;
        case RES_RESIST:
            here->RESresist = value->rValue;
            here->RESresGiven = TRUE;
            break;
        case RES_WIDTH:
            here->RESwidth = value->rValue;
            here->RESwidthGiven = TRUE;
            break;
        case RES_LENGTH:
            here->RESlength = value->rValue;
            here->RESlengthGiven = TRUE;
            break;
        case RES_RESIST_SENS:
            here->RESsenParmNo = value->iValue;
            break;
        default:
            return(E_BADPARM);
    }
    return(OK);
}
