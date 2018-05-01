/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1987 Mathew Lew and Thomas L. Quarles
Sydney University mods Copyright(c) 1989 Anthony E. Parker, David J. Skellern
	Laboratory for Communication Science Engineering
	Sydney University Department of Electrical Engineering, Australia
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "const.h"
#include "ifsim.h"
#include "cktdefs.h"
#include "devdefs.h"
#include "jfetdefs.h"
#include "sperror.h"
#include "suffix.h"


/*ARGSUSED*/
int
JFETmAsk(ckt,inModel,which,value)
    CKTcircuit *ckt;
    GENmodel *inModel;
    int which;
    IFvalue *value;
{
    JFETmodel *model = (JFETmodel*)inModel;
    switch(which) {
        case JFET_MOD_TNOM:
            value->rValue = model->JFETtnom-CONSTCtoK;
            return(OK);
        case JFET_MOD_VTO:
            value->rValue = model->JFETthreshold;
            return(OK);
        case JFET_MOD_BETA:
            value->rValue = model->JFETbeta;
            return(OK);
        case JFET_MOD_LAMBDA:
            value->rValue = model->JFETlModulation;
            return(OK);
	/* Modification for Sydney University JFET model */
	case JFET_MOD_B:
		value->rValue = model->JFETb;
		return(OK);
	/* end Sydney University mod */
        case JFET_MOD_RD:
            value->rValue = model->JFETdrainResist;
            return(OK);
        case JFET_MOD_RS:
            value->rValue = model->JFETsourceResist;
            return(OK);
        case JFET_MOD_CGS:
            value->rValue = model->JFETcapGS;
            return(OK);
        case JFET_MOD_CGD:
            value->rValue = model->JFETcapGD;
            return(OK);
        case JFET_MOD_PB:
            value->rValue = model->JFETgatePotential;
            return(OK);
        case JFET_MOD_IS:
            value->rValue = model->JFETgateSatCurrent;
            return(OK);
        case JFET_MOD_FC:
            value->rValue = model->JFETdepletionCapCoeff;
            return(OK);
        case JFET_MOD_DRAINCONDUCT:
            value->rValue = model->JFETdrainConduct;
            return(OK);
        case JFET_MOD_SOURCECONDUCT:
            value->rValue = model->JFETsourceConduct;
            return(OK);
        case JFET_MOD_TYPE:
	    if (model->JFETtype == NJF)
                value->sValue = "njf";
	    else
                value->sValue = "pjf";
            return(OK);
        default:
            return(E_BADPARM);
    }
    /* NOTREACHED */
}

