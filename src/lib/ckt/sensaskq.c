/**********
Copyright 1991 Regents of the University of California.  All rights reserved.
**********/

#include "spice.h"
#include <stdio.h>
#include "ifsim.h"
#include "iferrmsg.h"
#include "cktdefs.h"
#include "sensdefs.h"
#include "suffix.h"

/* ARGSUSED */
int 
SENSask(ckt,anal,which,value)
    CKTcircuit *ckt;
    GENERIC *anal;
    int which;
    IFvalue *value;
{
    SENS_AN	*sinfo = (SENS_AN *) anal;

    switch (which) {

    case SENS_START:
	value->rValue = sinfo->start_freq;
        break;

    case SENS_STOP:
	value->rValue = sinfo->stop_freq;
        break;

    case SENS_STEPS:
	value->iValue = sinfo->n_freq_steps;
        break;

    case SENS_DEC:
    case SENS_OCT:
    case SENS_LIN:
    case SENS_DC:
	value->iValue = sinfo->step_type == which;
        break;

    case SENS_DEFTOL:
	sinfo->deftol = value->rValue;
	break;

    case SENS_DEFPERTURB:
	value->rValue = sinfo->defperturb;
	break;

#ifdef notdef
    case SENS_TYPE:
	value->sValue = sinfo->type;
	break;
#endif

    default:
        return(E_BADPARM);
    }
    return(OK);
}

