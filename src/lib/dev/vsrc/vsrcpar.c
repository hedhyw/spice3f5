/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "vsrcdefs.h"
#include "ifsim.h"
#include "util.h"
#include "sperror.h"
#include "suffix.h"


/* ARGSUSED */
int
VSRCparam(param,value,inst,select)
    int param;
    IFvalue *value;
    GENinstance *inst;
    IFvalue *select;
{
    VSRCinstance *here = (VSRCinstance *)inst;
    switch(param) {
        case VSRC_DC:
            here->VSRCdcValue = value->rValue;
            here->VSRCdcGiven = TRUE;
            break;
        case VSRC_AC_MAG:
            here->VSRCacMag = value->rValue;
            here->VSRCacMGiven = TRUE;
            here->VSRCacGiven = TRUE;
            break;
        case VSRC_AC_PHASE:
            here->VSRCacPhase = value->rValue;
            here->VSRCacPGiven = TRUE;
            here->VSRCacGiven = TRUE;
            break;
        case VSRC_AC:
            switch(value->v.numValue) {
                case 2:
                    here->VSRCacPhase = *(value->v.vec.rVec+1);
                    here->VSRCacPGiven = TRUE;
                case 1:
                    here->VSRCacMag = *(value->v.vec.rVec);
                    here->VSRCacMGiven = TRUE;
                case 0:
                    here->VSRCacGiven = TRUE;
                    break;
                default:
                    return(E_BADPARM);
            }
            break;
        case VSRC_PULSE:
            here->VSRCfunctionType = PULSE;
            here->VSRCfuncTGiven = TRUE;
            here->VSRCcoeffs = value->v.vec.rVec;
            here->VSRCfunctionOrder = value->v.numValue;
            here->VSRCcoeffsGiven = TRUE;
            break;
        case VSRC_SINE:
            here->VSRCfunctionType = SINE;
            here->VSRCfuncTGiven = TRUE;
            here->VSRCcoeffs = value->v.vec.rVec;
            here->VSRCfunctionOrder = value->v.numValue;
            here->VSRCcoeffsGiven = TRUE;
            break;
        case VSRC_EXP:
            here->VSRCfunctionType = EXP;
            here->VSRCfuncTGiven = TRUE;
            here->VSRCcoeffs = value->v.vec.rVec;
            here->VSRCfunctionOrder = value->v.numValue;
            here->VSRCcoeffsGiven = TRUE;
            break;
        case VSRC_PWL:
            here->VSRCfunctionType = PWL;
            here->VSRCfuncTGiven = TRUE;
            here->VSRCcoeffs = value->v.vec.rVec;
            here->VSRCfunctionOrder = value->v.numValue;
            here->VSRCcoeffsGiven = TRUE;
            break;
        case VSRC_SFFM:
            here->VSRCfunctionType = SFFM;
            here->VSRCfuncTGiven = TRUE;
            here->VSRCcoeffs = value->v.vec.rVec;
            here->VSRCfunctionOrder = value->v.numValue;
            here->VSRCcoeffsGiven = TRUE;
            break;
	case VSRC_D_F1:
	    here->VSRCdF1given = TRUE;
	    here->VSRCdGiven = TRUE;
	    switch(value->v.numValue) {
		case 2:
		    here->VSRCdF1phase = *(value->v.vec.rVec+1);
		    here->VSRCdF1mag = *(value->v.vec.rVec);
		    break;
		case 1:
		    here->VSRCdF1mag = *(value->v.vec.rVec);
		    here->VSRCdF1phase = 0.0;
		    break;
		case 0:
		    here->VSRCdF1mag = 1.0;
		    here->VSRCdF1phase = 0.0;
		    break;
		default:
		    return(E_BADPARM);
		    break;
	    }
	case VSRC_D_F2:
	    here->VSRCdF2given = TRUE;
	    here->VSRCdGiven = TRUE;
	    switch(value->v.numValue) {
		case 2:
		    here->VSRCdF2phase = *(value->v.vec.rVec+1);
		    here->VSRCdF2mag = *(value->v.vec.rVec);
		    break;
		case 1:
		    here->VSRCdF2mag = *(value->v.vec.rVec);
		    here->VSRCdF2phase = 0.0;
		    break;
		case 0:
		    here->VSRCdF2mag = 1.0;
		    here->VSRCdF2phase = 0.0;
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
