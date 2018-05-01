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


int
RESmParam(param,value,inModel)
    int param;
    IFvalue *value;
    GENmodel *inModel;
{
    register RESmodel *model = (RESmodel *)inModel;
    switch(param) {
        case RES_MOD_TNOM:
            model->REStnom = value->rValue+CONSTCtoK;
            model->REStnomGiven = TRUE;
            break;
        case RES_MOD_TC1:
            model->REStempCoeff1 = value->rValue;
            model->REStc1Given = TRUE;
            break;
        case RES_MOD_TC2:
            model->REStempCoeff2 = value->rValue;
            model->REStc2Given = TRUE;
            break;
        case RES_MOD_RSH:
            model->RESsheetRes = value->rValue;
            model->RESsheetResGiven = TRUE;
            break;
        case RES_MOD_DEFWIDTH:
            model->RESdefWidth = value->rValue;
            model->RESdefWidthGiven = TRUE;
            break;
        case RES_MOD_NARROW:
            model->RESnarrow = value->rValue;
            model->RESnarrowGiven = TRUE;
            break;
        case RES_MOD_R:
            /* just being reassured by user that this is a resistor model */
            /* no-op */
            break;
        default:
            return(E_BADPARM);
    }
    return(OK);
}
