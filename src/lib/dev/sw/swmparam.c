/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Gordon Jacobs
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "swdefs.h"
#include "util.h"
#include "ifsim.h"
#include "sperror.h"
#include "suffix.h"


int
SWmParam(param,value,inModel)
    int param;
    IFvalue *value;
    GENmodel *inModel;
{
    SWmodel *model = (SWmodel *)inModel;
    switch(param) {
        case SW_MOD_SW:
            /* just says that this is a switch */
            break;
        case SW_MOD_RON:
            model->SWonResistance = value->rValue;
            model->SWonConduct = 1.0/(value->rValue);
            model->SWonGiven = TRUE;
            break;
        case SW_MOD_ROFF:
            model->SWoffResistance = value->rValue;
            model->SWoffConduct = 1.0/(value->rValue);
            model->SWoffGiven = TRUE;
            break;
        case SW_MOD_VTH:
            /* take absolute value of hysteresis voltage */
            model->SWvThreshold = value->rValue;
            model->SWthreshGiven = TRUE;
            break;
        case SW_MOD_VHYS:
            /* take absolute value of hysteresis voltage */
            model->SWvHysteresis = (value->rValue < 0) ? -(value->rValue) :
                    value->rValue;
            model->SWhystGiven = TRUE;
            break;
        default:
            return(E_BADPARM);
    }

    return(OK);
}
