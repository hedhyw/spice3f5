/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 S. Hwang
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "ifsim.h"
#include "util.h"
#include "mesdefs.h"
#include "sperror.h"
#include "suffix.h"


/* ARGSUSED */
int
MESparam(param,value,inst,select)
    int param;
    IFvalue *value;
    GENinstance *inst;
    IFvalue *select;
{
    MESinstance *here = (MESinstance*)inst;
    switch(param) {
        case MES_AREA:
            here->MESarea = value->rValue;
            here->MESareaGiven = TRUE;
            break;
        case MES_IC_VDS:
            here->MESicVDS = value->rValue;
            here->MESicVDSGiven = TRUE;
            break;
        case MES_IC_VGS:
            here->MESicVGS = value->rValue;
            here->MESicVGSGiven = TRUE;
            break;
        case MES_OFF:
            here->MESoff = value->iValue;
            break;
        case MES_IC:
            switch(value->v.numValue) {
                case 2:
                    here->MESicVGS = *(value->v.vec.rVec+1);
                    here->MESicVGSGiven = TRUE;
                case 1:
                    here->MESicVDS = *(value->v.vec.rVec);
                    here->MESicVDSGiven = TRUE;
                    break;
                default:
                    return(E_BADPARM);
            }
            break;
        default:
            return(E_BADPARM);
    }
    return(OK);
}
