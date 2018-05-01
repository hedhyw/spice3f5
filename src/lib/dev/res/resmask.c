/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "const.h"
#include "cktdefs.h"
#include "ifsim.h"
#include "resdefs.h"
#include "sperror.h"
#include "devdefs.h"
#include "suffix.h"


/* ARGSUSED */
int 
RESmodAsk(ckt,inModel,which,value)
    CKTcircuit *ckt;
    GENmodel *inModel;
    int which;
    IFvalue *value;
{
    RESmodel *model = (RESmodel *)inModel;
    switch(which) {
        case RES_MOD_TNOM:
            value->rValue = model->REStnom-CONSTCtoK;
            return(OK);
        case RES_MOD_TC1:
            value->rValue = model->REStempCoeff1;
            return(OK);
        case RES_MOD_TC2:
            value->rValue = model->REStempCoeff2;
            return(OK);
        case RES_MOD_RSH:
            value->rValue = model->RESsheetRes;
            return(OK);
        case RES_MOD_DEFWIDTH:
            value->rValue = model->RESdefWidth;
            return(OK);
        case RES_MOD_NARROW: 
            value->rValue = model->RESnarrow;
            return(OK);
        default:
            return(E_BADPARM);
    }
}

