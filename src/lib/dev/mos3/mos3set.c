/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "smpdefs.h"
#include "cktdefs.h"
#include "mos3defs.h"
#include "util.h"
#include "const.h"
#include "sperror.h"
#include "suffix.h"

/* assuming silicon - make definition for epsilon of silicon */
#define EPSSIL (11.7 * 8.854214871e-12)

int
MOS3setup(matrix,inModel,ckt,states)
    register SMPmatrix *matrix;
    GENmodel *inModel;
    register CKTcircuit *ckt;
    int *states;
        /* load the MOS3 device structure with those pointers needed later 
         * for fast matrix loading 
         */

{
    register MOS3model *model = (MOS3model *)inModel;
    register MOS3instance *here;
    int error;
    CKTnode *tmp;

    /*  loop through all the MOS3 device models */
    for( ; model != NULL; model = model->MOS3nextModel ) {

        /* perform model defaulting */
        if(!model->MOS3typeGiven) {
            model->MOS3type = NMOS;
        }
        if(!model->MOS3latDiffGiven) {
            model->MOS3latDiff = 0;
        }
        if(!model->MOS3jctSatCurDensityGiven) {
            model->MOS3jctSatCurDensity = 0;
        }
        if(!model->MOS3jctSatCurGiven) {
            model->MOS3jctSatCur = 1e-14;
        }
        if(!model->MOS3drainResistanceGiven) {
            model->MOS3drainResistance = 0;
        }
        if(!model->MOS3sourceResistanceGiven) {
            model->MOS3sourceResistance = 0;
        }
        if(!model->MOS3sheetResistanceGiven) {
            model->MOS3sheetResistance = 0;
        }
        if(!model->MOS3transconductanceGiven) {
            model->MOS3transconductance = 2e-5;
        }
        if(!model->MOS3gateSourceOverlapCapFactorGiven) {
            model->MOS3gateSourceOverlapCapFactor = 0;
        }
        if(!model->MOS3gateDrainOverlapCapFactorGiven) {
            model->MOS3gateDrainOverlapCapFactor = 0;
        }
        if(!model->MOS3gateBulkOverlapCapFactorGiven) {
            model->MOS3gateBulkOverlapCapFactor = 0;
        }
        if(!model->MOS3vt0Given) {
            model->MOS3vt0 = 0;
        }
        if(!model->MOS3capBDGiven) {
            model->MOS3capBD = 0;
        }
        if(!model->MOS3capBSGiven) {
            model->MOS3capBS = 0;
        }
        if(!model->MOS3bulkCapFactorGiven) {
            model->MOS3bulkCapFactor = 0;
        }
        if(!model->MOS3sideWallCapFactorGiven) {
            model->MOS3sideWallCapFactor = 0;
        }
        if(!model->MOS3bulkJctPotentialGiven) {
            model->MOS3bulkJctPotential = .8;
        }
        if(!model->MOS3bulkJctBotGradingCoeffGiven) {
            model->MOS3bulkJctBotGradingCoeff = .5;
        }
        if(!model->MOS3bulkJctSideGradingCoeffGiven) {
            model->MOS3bulkJctSideGradingCoeff = .33;
        }
        if(!model->MOS3fwdCapDepCoeffGiven) {
            model->MOS3fwdCapDepCoeff = .5;
        }
        if(!model->MOS3phiGiven) {
            model->MOS3phi = .6;
        }
        if(!model->MOS3gammaGiven) {
            model->MOS3gamma = 0;
        }
        if(!model->MOS3deltaGiven) {
            model->MOS3delta = 0;
        }
        if(!model->MOS3maxDriftVelGiven) {
            model->MOS3maxDriftVel = 0;
        }
        if(!model->MOS3junctionDepthGiven) {
            model->MOS3junctionDepth = 0;
        }
        if(!model->MOS3fastSurfaceStateDensityGiven) {
            model->MOS3fastSurfaceStateDensity = 0;
        }
        if(!model->MOS3etaGiven) {
            model->MOS3eta = 0;
        }
        if(!model->MOS3thetaGiven) {
            model->MOS3theta = 0;
        }
        if(!model->MOS3kappaGiven) {
            model->MOS3kappa = .2;
        }
        if(!model->MOS3oxideThicknessGiven) {
            model->MOS3oxideThickness = 1e-7;
        } 
	if(!model->MOS3fNcoefGiven) {
	    model->MOS3fNcoef = 0;
	}
	if(!model->MOS3fNexpGiven) {
	    model->MOS3fNexp = 1;
	}

        /* loop through all the instances of the model */
        for (here = model->MOS3instances; here != NULL ;
                here=here->MOS3nextInstance) {

            /* allocate a chunk of the state vector */
            here->MOS3states = *states;
            *states += MOS3NUMSTATES;

            if(!here->MOS3drainAreaGiven) {
                here->MOS3drainArea = ckt->CKTdefaultMosAD;
            }
            if(!here->MOS3drainPerimiterGiven) {
                here->MOS3drainPerimiter = 0;
            }
            if(!here->MOS3drainSquaresGiven) {
                here->MOS3drainSquares = 1;
            }
            if(!here->MOS3icVBSGiven) {
                here->MOS3icVBS = 0;
            }
            if(!here->MOS3icVDSGiven) {
                here->MOS3icVDS = 0;
            }
            if(!here->MOS3icVGSGiven) {
                here->MOS3icVGS = 0;
            }
            if(!here->MOS3sourcePerimiterGiven) {
                here->MOS3sourcePerimiter = 0;
            }
            if(!here->MOS3sourceSquaresGiven) {
                here->MOS3sourceSquares = 1;
            }
            if(!here->MOS3vdsatGiven) {
                here->MOS3vdsat = 0;
            }
            if(!here->MOS3vonGiven) {
                here->MOS3von = 0;
            }
            if(!here->MOS3modeGiven) {
                here->MOS3mode = 1;
            }

            if((model->MOS3drainResistance != 0 ||
                    (model->MOS3sheetResistance != 0 &&
                     here->MOS3drainSquares != 0      ) )  && 
                    here->MOS3dNodePrime==0) {
                error = CKTmkVolt(ckt,&tmp,here->MOS3name,"internal#drain");
                if(error) return(error);
                here->MOS3dNodePrime = tmp->number;
            } else {
                here->MOS3dNodePrime = here->MOS3dNode;
            }

            if((model->MOS3sourceResistance != 0 ||
                    (model->MOS3sheetResistance != 0 && 
                     here->MOS3sourceSquares != 0     ) ) &&
                    here->MOS3sNodePrime==0) {
                error = CKTmkVolt(ckt,&tmp,here->MOS3name,"internal#source");
                if(error) return(error);
                here->MOS3sNodePrime = tmp->number;
            } else {
                here->MOS3sNodePrime = here->MOS3sNode;
            }

/* macro to make elements with built in test for out of memory */
#define TSTALLOC(ptr,first,second) \
if((here->ptr = SMPmakeElt(matrix,here->first,here->second))==(double *)NULL){\
    return(E_NOMEM);\
}

            TSTALLOC(MOS3DdPtr, MOS3dNode, MOS3dNode)
            TSTALLOC(MOS3GgPtr, MOS3gNode, MOS3gNode)
            TSTALLOC(MOS3SsPtr, MOS3sNode, MOS3sNode)
            TSTALLOC(MOS3BbPtr, MOS3bNode, MOS3bNode)
            TSTALLOC(MOS3DPdpPtr, MOS3dNodePrime, MOS3dNodePrime)
            TSTALLOC(MOS3SPspPtr, MOS3sNodePrime, MOS3sNodePrime)
            TSTALLOC(MOS3DdpPtr, MOS3dNode, MOS3dNodePrime)
            TSTALLOC(MOS3GbPtr, MOS3gNode, MOS3bNode)
            TSTALLOC(MOS3GdpPtr, MOS3gNode, MOS3dNodePrime)
            TSTALLOC(MOS3GspPtr, MOS3gNode, MOS3sNodePrime)
            TSTALLOC(MOS3SspPtr, MOS3sNode, MOS3sNodePrime)
            TSTALLOC(MOS3BdpPtr, MOS3bNode, MOS3dNodePrime)
            TSTALLOC(MOS3BspPtr, MOS3bNode, MOS3sNodePrime)
            TSTALLOC(MOS3DPspPtr, MOS3dNodePrime, MOS3sNodePrime)
            TSTALLOC(MOS3DPdPtr, MOS3dNodePrime, MOS3dNode)
            TSTALLOC(MOS3BgPtr, MOS3bNode, MOS3gNode)
            TSTALLOC(MOS3DPgPtr, MOS3dNodePrime, MOS3gNode)
            TSTALLOC(MOS3SPgPtr, MOS3sNodePrime, MOS3gNode)
            TSTALLOC(MOS3SPsPtr, MOS3sNodePrime, MOS3sNode)
            TSTALLOC(MOS3DPbPtr, MOS3dNodePrime, MOS3bNode)
            TSTALLOC(MOS3SPbPtr, MOS3sNodePrime, MOS3bNode)
            TSTALLOC(MOS3SPdpPtr, MOS3sNodePrime, MOS3dNodePrime)

        }
    }
    return(OK);
}

int
MOS3unsetup(inModel,ckt)
    GENmodel *inModel;
    CKTcircuit *ckt;
{
#ifndef HAS_BATCHSIM
    MOS3model *model;
    MOS3instance *here;

    for (model = (MOS3model *)inModel; model != NULL;
	    model = model->MOS3nextModel)
    {
        for (here = model->MOS3instances; here != NULL;
                here=here->MOS3nextInstance)
	{
	    if (here->MOS3dNodePrime
		    && here->MOS3dNodePrime != here->MOS3dNode)
	    {
		CKTdltNNum(ckt, here->MOS3dNodePrime);
		here->MOS3dNodePrime= 0;
	    }
	    if (here->MOS3sNodePrime
		    && here->MOS3sNodePrime != here->MOS3sNode)
	    {
		CKTdltNNum(ckt, here->MOS3sNodePrime);
		here->MOS3sNodePrime= 0;
	    }
	}
    }
#endif
    return OK;
}
