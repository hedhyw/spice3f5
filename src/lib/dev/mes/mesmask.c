/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1987 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "devdefs.h"
#include "ifsim.h"
#include "mesdefs.h"
#include "sperror.h"
#include "suffix.h"


/* ARGSUSED */
int
MESmAsk(ckt,inst,which,value)
    CKTcircuit *ckt;
    GENmodel *inst;
    int which;
    IFvalue *value;
{
    MESmodel *here = (MESmodel*)inst;
    switch(which) {
        case MES_MOD_VTO:
            value->rValue = here->MESthreshold;
            return (OK);
        case MES_MOD_ALPHA:
            value->rValue = here->MESalpha;
            return (OK);
        case MES_MOD_BETA:
            value->rValue = here->MESbeta;
            return (OK);
        case MES_MOD_LAMBDA:
            value->rValue = here->MESlModulation;
            return (OK);
        case MES_MOD_B:
            value->rValue = here->MESb;
            return (OK);
        case MES_MOD_RD:
            value->rValue = here->MESdrainResist;
            return (OK);
        case MES_MOD_RS:
            value->rValue = here->MESsourceResist;
            return (OK);
        case MES_MOD_CGS:
            value->rValue = here->MEScapGS;
            return (OK);
        case MES_MOD_CGD:
            value->rValue = here->MEScapGD;
            return (OK);
        case MES_MOD_PB:
            value->rValue = here->MESgatePotential;
            return (OK);
        case MES_MOD_IS:
            value->rValue = here->MESgateSatCurrent;
            return (OK);
        case MES_MOD_FC:
            value->rValue = here->MESdepletionCapCoeff;
            return (OK);
        case MES_MOD_DRAINCONDUCT:
            value->rValue = here->MESdrainConduct;
            return (OK);
        case MES_MOD_SOURCECONDUCT:
            value->rValue = here->MESsourceConduct;
            return (OK);
        case MES_MOD_DEPLETIONCAP:
            value->rValue = here->MESdepletionCap;
            return (OK);
        case MES_MOD_VCRIT:
            value->rValue = here->MESvcrit;
            return (OK);
	case MES_MOD_TYPE:
	    if (here->MEStype == NMF)
                value->sValue = "nmf";
	    else
                value->sValue = "pmf";
        default:
            return (E_BADPARM);
    }
    /* NOTREACHED */
}
