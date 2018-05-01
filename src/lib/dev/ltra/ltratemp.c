/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1990 Jaijeet S. Roychowdhury
**********/

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "smpdefs.h"
#include "cktdefs.h"
#include "ltradefs.h"
#include "sperror.h"
#include "suffix.h"

/* ARGSUSED */
int
LTRAtemp(inModel,ckt)
    GENmodel *inModel;
    CKTcircuit *ckt;
        /*
         * pre-process parameters for later use
         */
{
    register LTRAmodel *model = (LTRAmodel *)inModel;
    register LTRAinstance *here;

    /*  loop through all the transmission line models */
    for( ; model != NULL; model = model->LTRAnextModel ) {

            
	    /*
            if(!model->LTRAtdGiven) {
                model->LTRAtd = model->LTRAnl/model->LTRAf;
            }
	    */
switch (model->LTRAspecialCase) {

case LTRA_MOD_LC:
	    model->LTRAimped =
	    	sqrt(model->LTRAinduct/model->LTRAcapac);
        model->LTRAadmit = 1/model->LTRAimped;
	    model->LTRAtd =
	    	sqrt(model->LTRAinduct*model->LTRAcapac)*model->LTRAlength;
		model->LTRAattenuation = 1.0;
		break;

case LTRA_MOD_RLC:
	    model->LTRAimped =
	    	sqrt(model->LTRAinduct/model->LTRAcapac);
        model->LTRAadmit = 1/model->LTRAimped;
	    model->LTRAtd =
	    	sqrt(model->LTRAinduct*model->LTRAcapac)*model->LTRAlength;
	    model->LTRAalpha =
	    	0.5*(model->LTRAresist/model->LTRAinduct
			/* - model->LTRAconduct/model->LTRAcapac */);
	    model->LTRAbeta = model->LTRAalpha;
		/*
	    	0.5*(model->LTRAresist/model->LTRAinduct + 
			model->LTRAconduct/model->LTRAcapac);
		*/
	    model->LTRAattenuation = exp(- model->LTRAbeta *
	    	model->LTRAtd);
		if (model->LTRAalpha > 0.0) {
			model->LTRAintH1dash =
				/*sqrt(model->LTRAconduct/model->LTRAresist)/
					model->LTRAadmit*/ - 1.0;
			model->LTRAintH2 =
				/*exp(-model->LTRAlength*sqrt(model->LTRAconduct*
				model->LTRAresist))*/1.0 - model->LTRAattenuation;
			model->LTRAintH3dash = /*(model->LTRAintH1dash+1.0)*
				(model->LTRAintH2+model->LTRAattenuation)*/ - 
				model->LTRAattenuation;
		} else if (model->LTRAalpha == 0.0) {
			model->LTRAintH1dash = model->LTRAintH2 = 
				model->LTRAintH3dash = 0.0;
		} else {
#ifdef LTRADEBUG
			fprintf(stdout,"LTRAtemp: error: alpha < 0.0\n");
#endif
		}

		/*
	    model->LTRAh1dashValues = (double *) NULL;
	    model->LTRAh2Values = (double *) NULL;
	    model->LTRAh3dashValues = (double *) NULL;

	    model->LTRAh1dashOthVals = (double *) NULL;
	    model->LTRAh2OthVals = (double *) NULL;
	    model->LTRAh3dashOthVals = (double *) NULL;
		*/

	    model->LTRAh1dashCoeffs = (double *) NULL;
	    model->LTRAh2Coeffs = (double *) NULL;
	    model->LTRAh3dashCoeffs = (double *) NULL;

		if (!model->LTRAtruncDontCut) {

		double xbig, xsmall, xmid, y1big, y1small, y1mid;
		double y2big, y2small, y2mid;
		int done=0, maxiter=50, iters=0;

		xbig = model->LTRAtd + /*ckt->CKTmaxStep*/ 9*model->LTRAtd;
		/* hack! ckt is not yet initialised... */
		xsmall = model->LTRAtd;
		xmid = 0.5*(xbig+xsmall);
		y1small = LTRArlcH2Func(xsmall,model->LTRAtd,model->LTRAalpha,model->LTRAbeta);
		y2small = LTRArlcH3dashFunc(xsmall,model->LTRAtd,model->LTRAbeta,model->LTRAbeta);
		iters = 0;
		while (1) {
			
			iters++;
			y1big = LTRArlcH2Func(xbig,model->LTRAtd,model->LTRAalpha,model->LTRAbeta);
			y1mid = LTRArlcH2Func(xmid,model->LTRAtd,model->LTRAalpha,model->LTRAbeta);
			y2big = LTRArlcH3dashFunc(xbig,model->LTRAtd,model->LTRAbeta,model->LTRAbeta);
			y2mid = LTRArlcH3dashFunc(xmid,model->LTRAtd,model->LTRAbeta,model->LTRAbeta);
			done =
				LTRAstraightLineCheck(xbig,y1big,xmid,y1mid,xsmall,
				y1small,model->LTRAstLineReltol,
				model->LTRAstLineAbstol) + 
				LTRAstraightLineCheck(xbig,y1big,xmid,y1mid,xsmall,
				y1small,model->LTRAstLineReltol,
				model->LTRAstLineAbstol);
			if ((done == 2) || (iters > maxiter)) break;
			xbig = xmid;
			xmid = 0.5*(xbig+xsmall);
		}
		model->LTRAmaxSafeStep = xbig - model->LTRAtd;
		}

		break;

case LTRA_MOD_RC:
		model->LTRAcByR = model->LTRAcapac/model->LTRAresist;
		model->LTRArclsqr = model->LTRAresist*model->LTRAcapac
			*model->LTRAlength*model->LTRAlength;
		model->LTRAintH1dash = 0.0;
		model->LTRAintH2 = 1.0;
		model->LTRAintH3dash = 0.0;

	    model->LTRAh1dashCoeffs = (double *) NULL;
	    model->LTRAh2Coeffs = (double *) NULL;
	    model->LTRAh3dashCoeffs = (double *) NULL;

		break;

case LTRA_MOD_RG:
		break;

default: return(E_BADPARM);
}

        /* loop through all the instances of the model */
        for (here = model->LTRAinstances; here != NULL ;
                here=here->LTRAnextInstance) {
	
		here->LTRAv1 = (double *) NULL;
		here->LTRAi1 = (double *) NULL;
		here->LTRAv2 = (double *) NULL;
		here->LTRAi2 = (double *) NULL;
        }
    }
    return(OK);
}
