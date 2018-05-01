/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1987 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "const.h"
#include "ifsim.h"
#include "cktdefs.h"
#include "devdefs.h"
#include "mos3defs.h"
#include "sperror.h"
#include "suffix.h"


/*ARGSUSED*/
int
MOS3mAsk(ckt,inst,which,value)
    CKTcircuit *ckt;
    GENmodel *inst;
    int which;
    IFvalue *value;
{
    MOS3model *here = (MOS3model *)inst;
    switch(which) {
        case MOS3_MOD_TNOM:
            value->rValue = here->MOS3tnom-CONSTCtoK;
            return(OK);
        case MOS3_MOD_VTO:
            value->rValue = here->MOS3vt0;
            return(OK);
        case MOS3_MOD_KP:
            value->rValue = here->MOS3transconductance;
            return(OK);
        case MOS3_MOD_GAMMA:
            value->rValue = here->MOS3gamma;
            return(OK);
        case MOS3_MOD_PHI:
            value->rValue = here->MOS3phi;
            return(OK);
        case MOS3_MOD_RD:
            value->rValue = here->MOS3drainResistance;
            return(OK);
        case MOS3_MOD_RS:
            value->rValue = here->MOS3sourceResistance;
            return(OK);
        case MOS3_MOD_CBD:
            value->rValue = here->MOS3capBD;
            return(OK);
        case MOS3_MOD_CBS:
            value->rValue = here->MOS3capBS;
            return(OK);
        case MOS3_MOD_IS:
            value->rValue = here->MOS3jctSatCur;
            return(OK);
        case MOS3_MOD_PB:
            value->rValue = here->MOS3bulkJctPotential;
            return(OK);
        case MOS3_MOD_CGSO:
            value->rValue = here->MOS3gateSourceOverlapCapFactor;
            return(OK);
        case MOS3_MOD_CGDO:
            value->rValue = here->MOS3gateDrainOverlapCapFactor;
            return(OK);
        case MOS3_MOD_CGBO:
            value->rValue = here->MOS3gateBulkOverlapCapFactor;
            return(OK);
        case MOS3_MOD_CJ:
            value->rValue = here->MOS3bulkCapFactor;
            return(OK);
        case MOS3_MOD_MJ:
            value->rValue = here->MOS3bulkJctBotGradingCoeff;
            return(OK);
        case MOS3_MOD_CJSW:
            value->rValue = here->MOS3sideWallCapFactor;
            return(OK);
        case MOS3_MOD_MJSW:
            value->rValue = here->MOS3bulkJctSideGradingCoeff;
            return(OK);
        case MOS3_MOD_JS:
            value->rValue = here->MOS3jctSatCurDensity;
            return(OK);
        case MOS3_MOD_TOX:
            value->rValue = here->MOS3oxideThickness;
            return(OK);
        case MOS3_MOD_LD:
            value->rValue = here->MOS3latDiff;
            return(OK);
        case MOS3_MOD_RSH:
            value->rValue = here->MOS3sheetResistance;
            return(OK);
        case MOS3_MOD_U0:
            value->rValue = here->MOS3surfaceMobility;
            return(OK);
        case MOS3_MOD_FC:
            value->rValue = here->MOS3fwdCapDepCoeff;
            return(OK);
        case MOS3_MOD_NSUB:
            value->rValue = here->MOS3substrateDoping;
            return(OK);
        case MOS3_MOD_TPG:
            value->iValue = here->MOS3gateType;
            return(OK);
        case MOS3_MOD_NSS:
            value->rValue = here->MOS3surfaceStateDensity;
            return(OK);
        case MOS3_MOD_NFS:
            value->rValue = here->MOS3fastSurfaceStateDensity;
            return(OK);
        case MOS3_MOD_DELTA:
            value->rValue = here->MOS3narrowFactor;
            return(OK);
        case MOS3_MOD_VMAX:
            value->rValue = here->MOS3maxDriftVel;
            return(OK);
        case MOS3_MOD_XJ:
            value->rValue = here->MOS3junctionDepth;
            return(OK);
        case MOS3_MOD_ETA:
            value->rValue = here->MOS3eta;
            return(OK);
        case MOS3_MOD_XD:
            value->rValue = here->MOS3coeffDepLayWidth;
            return(OK);
        case MOS3_DELTA:
            value->rValue = here->MOS3delta;
            return(OK);
        case MOS3_MOD_THETA:
            value->rValue = here->MOS3theta;
            return(OK);
        case MOS3_MOD_ALPHA:
            value->rValue = here->MOS3alpha;
            return(OK);
        case MOS3_MOD_KAPPA:
            value->rValue = here->MOS3kappa;
            return(OK);
	case MOS3_MOD_TYPE:
	    if (here->MOS3type > 0)
	        value->sValue = "nmos";
	    else
	        value->sValue = "pmos";
	    break;
        default:
            return(E_BADPARM);
    }
    /* NOTREACHED */
}

