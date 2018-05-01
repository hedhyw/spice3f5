/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Hong J. Park, Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "ifsim.h"
#include "bsim2def.h"
#include "util.h"
#include "sperror.h"
#include "suffix.h"


/* ARGSUSED */
int
B2param(param,value,inst,select)
    int param;
    IFvalue *value;
    GENinstance *inst;
    IFvalue *select;
{
    B2instance *here = (B2instance*)inst;
    switch(param) {
        case BSIM2_W:
            here->B2w = value->rValue;
            here->B2wGiven = TRUE;
            break;
        case BSIM2_L:
            here->B2l = value->rValue;
            here->B2lGiven = TRUE;
            break;
        case BSIM2_AS:
            here->B2sourceArea = value->rValue;
            here->B2sourceAreaGiven = TRUE;
            break;
        case BSIM2_AD:
            here->B2drainArea = value->rValue;
            here->B2drainAreaGiven = TRUE;
            break;
        case BSIM2_PS:
            here->B2sourcePerimeter = value->rValue;
            here->B2sourcePerimeterGiven = TRUE;
            break;
        case BSIM2_PD:
            here->B2drainPerimeter = value->rValue;
            here->B2drainPerimeterGiven = TRUE;
            break;
        case BSIM2_NRS:
            here->B2sourceSquares = value->rValue;
            here->B2sourceSquaresGiven = TRUE;
            break;
        case BSIM2_NRD:
            here->B2drainSquares = value->rValue;
            here->B2drainSquaresGiven = TRUE;
            break;
        case BSIM2_OFF:
            here->B2off = value->iValue;
            break;
        case BSIM2_IC_VBS:
            here->B2icVBS = value->rValue;
            here->B2icVBSGiven = TRUE;
            break;
        case BSIM2_IC_VDS:
            here->B2icVDS = value->rValue;
            here->B2icVDSGiven = TRUE;
            break;
        case BSIM2_IC_VGS:
            here->B2icVGS = value->rValue;
            here->B2icVGSGiven = TRUE;
            break;
        case BSIM2_IC:
            switch(value->v.numValue){
                case 3:
                    here->B2icVBS = *(value->v.vec.rVec+2);
                    here->B2icVBSGiven = TRUE;
                case 2:
                    here->B2icVGS = *(value->v.vec.rVec+1);
                    here->B2icVGSGiven = TRUE;
                case 1:
                    here->B2icVDS = *(value->v.vec.rVec);
                    here->B2icVDSGiven = TRUE;
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



