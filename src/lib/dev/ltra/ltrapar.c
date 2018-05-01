/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1990 Jaijeet S. Roychowdhury
**********/

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "ltradefs.h"
#include "ifsim.h"
#include "sperror.h"
#include "suffix.h"


/* ARGSUSED */
int
LTRAparam(param,value,inst,select)
    int param;
    IFvalue *value;
    GENinstance *inst;
    IFvalue *select;
{
    LTRAinstance *here = (LTRAinstance *)inst;
    switch(param) {
        case LTRA_V1:
            here->LTRAinitVolt1 = value->rValue;
            here->LTRAicV1Given = TRUE;
            break;
        case LTRA_I1:
            here->LTRAinitCur1 = value->rValue;
            here->LTRAicC1Given = TRUE;
            break;
        case LTRA_V2:
            here->LTRAinitVolt2 = value->rValue;
            here->LTRAicV2Given = TRUE;
            break;
        case LTRA_I2:
            here->LTRAinitCur2 = value->rValue;
            here->LTRAicC2Given = TRUE;
            break;
        case LTRA_IC:
            switch(value->v.numValue){
                case 4:
                    here->LTRAinitCur2 = *(value->v.vec.rVec+3);
                case 3:
                    here->LTRAinitVolt2 =  *(value->v.vec.rVec+2);
                case 2:
                    here->LTRAinitCur1 = *(value->v.vec.rVec+1);
                case 1:
                    here->LTRAinitVolt1 = *(value->v.vec.rVec);
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
