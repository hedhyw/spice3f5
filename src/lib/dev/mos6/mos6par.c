/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1989 Takayasu Sakurai
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "const.h"
#include "util.h"
#include "ifsim.h"
#include "mos6defs.h"
#include "sperror.h"
#include "suffix.h"


/* ARGSUSED */
int
MOS6param(param,value,inst,select)
    int param;
    IFvalue *value;
    GENinstance *inst;
    IFvalue *select;
{
    MOS6instance *here = (MOS6instance *)inst;
    switch(param) {
        case MOS6_TEMP:
            here->MOS6temp = value->rValue+CONSTCtoK;
            here->MOS6tempGiven = TRUE;
            break;
        case MOS6_W:
            here->MOS6w = value->rValue;
            here->MOS6wGiven = TRUE;
            break;
        case MOS6_L:
            here->MOS6l = value->rValue;
            here->MOS6lGiven = TRUE;
            break;
        case MOS6_AS:
            here->MOS6sourceArea = value->rValue;
            here->MOS6sourceAreaGiven = TRUE;
            break;
        case MOS6_AD:
            here->MOS6drainArea = value->rValue;
            here->MOS6drainAreaGiven = TRUE;
            break;
        case MOS6_PS:
            here->MOS6sourcePerimiter = value->rValue;
            here->MOS6sourcePerimiterGiven = TRUE;
            break;
        case MOS6_PD:
            here->MOS6drainPerimiter = value->rValue;
            here->MOS6drainPerimiterGiven = TRUE;
            break;
        case MOS6_NRS:
            here->MOS6sourceSquares = value->rValue;
            here->MOS6sourceSquaresGiven = TRUE;
            break;
        case MOS6_NRD:
            here->MOS6drainSquares = value->rValue;
            here->MOS6drainSquaresGiven = TRUE;
            break;
        case MOS6_OFF:
            here->MOS6off = value->iValue;
            break;
        case MOS6_IC_VBS:
            here->MOS6icVBS = value->rValue;
            here->MOS6icVBSGiven = TRUE;
            break;
        case MOS6_IC_VDS:
            here->MOS6icVDS = value->rValue;
            here->MOS6icVDSGiven = TRUE;
            break;
        case MOS6_IC_VGS:
            here->MOS6icVGS = value->rValue;
            here->MOS6icVGSGiven = TRUE;
            break;
        case MOS6_IC:
            switch(value->v.numValue){
                case 3:
                    here->MOS6icVBS = *(value->v.vec.rVec+2);
                    here->MOS6icVBSGiven = TRUE;
                case 2:
                    here->MOS6icVGS = *(value->v.vec.rVec+1);
                    here->MOS6icVGSGiven = TRUE;
                case 1:
                    here->MOS6icVDS = *(value->v.vec.rVec);
                    here->MOS6icVDSGiven = TRUE;
                    break;
                default:
                    return(E_BADPARM);
            }
            break;
        case MOS6_L_SENS:
            if(value->iValue) {
                here->MOS6senParmNo = 1;
                here->MOS6sens_l = 1;
            }
            break;
        case MOS6_W_SENS:
            if(value->iValue) {
                here->MOS6senParmNo = 1;
                here->MOS6sens_w = 1;
            }
            break;
        default:
            return(E_BADPARM);
    }
    return(OK);
}
