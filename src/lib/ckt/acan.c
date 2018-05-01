/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "acdefs.h"
#include "devdefs.h"
#include "util.h"
#include "sperror.h"
#include "suffix.h"

int
ACan(ckt,restart) 
CKTcircuit *ckt;
int restart;
{

    double freq;
    double freqTol; /* tolerence parameter for finding final frequency */
    int error;
    long save;
    int save1;
    int numNames;
    IFuid *nameList;
    IFuid freqUid;
    static GENERIC *acPlot;
    GENERIC *plot;

    if(((ACAN*)ckt->CKTcurJob)->ACsaveFreq == 0 || restart) { 
        /* start at beginning */

	if (((ACAN*)ckt->CKTcurJob)->ACnumberSteps < 1)
	    ((ACAN*)ckt->CKTcurJob)->ACnumberSteps = 1;

        switch(((ACAN*)ckt->CKTcurJob)->ACstepType) {

        case DECADE:
            ((ACAN*)ckt->CKTcurJob)->ACfreqDelta =
                    exp(log(10.0)/((ACAN*)ckt->CKTcurJob)->ACnumberSteps);
            break;
        case OCTAVE:
            ((ACAN*)ckt->CKTcurJob)->ACfreqDelta =
                    exp(log(2.0)/((ACAN*)ckt->CKTcurJob)->ACnumberSteps);
            break;
        case LINEAR:
	    if (((ACAN*)ckt->CKTcurJob)->ACnumberSteps-1 > 1)
		((ACAN*)ckt->CKTcurJob)->ACfreqDelta =
                    (((ACAN*)ckt->CKTcurJob)->ACstopFreq -
                    ((ACAN*)ckt->CKTcurJob)->ACstartFreq)/
                    (((ACAN*)ckt->CKTcurJob)->ACnumberSteps-1);
	    else
		((ACAN*)ckt->CKTcurJob)->ACfreqDelta = HUGE;
            break;
        default:
            return(E_BADPARM);
        }
        error = CKTop(ckt,
                (ckt->CKTmode & MODEUIC) | MODEDCOP | MODEINITJCT,
                (ckt->CKTmode & MODEUIC) | MODEDCOP | MODEINITFLOAT,
                ckt->CKTdcMaxIter);
        if(error) return(error);

        ckt->CKTmode = (ckt->CKTmode & MODEUIC) | MODEDCOP | MODEINITSMSIG;
        error = CKTload(ckt);
        if(error) return(error);

        error = CKTnames(ckt,&numNames,&nameList);
        if(error) return(error);

	if (ckt->CKTkeepOpInfo) {
	    /* Dump operating point. */
	    error = (*(SPfrontEnd->OUTpBeginPlot))((GENERIC *)ckt,
		(GENERIC*)ckt->CKTcurJob, "AC Operating Point",
		(IFuid)NULL,IF_REAL,numNames,nameList, IF_REAL,&plot);
	    if(error) return(error);
	    CKTdump(ckt,(double)0,plot);
	    (*(SPfrontEnd->OUTendPlot))(plot);
	}

        (*(SPfrontEnd->IFnewUid))((GENERIC *)ckt,&freqUid,(IFuid)NULL,
                "frequency", UID_OTHER,(GENERIC **)NULL);
        error = (*(SPfrontEnd->OUTpBeginPlot))((GENERIC *)ckt,
		(GENERIC*)ckt->CKTcurJob,
                ckt->CKTcurJob->JOBname,freqUid,IF_REAL,numNames,nameList,
                IF_COMPLEX,&acPlot);
	if(error) return(error);

        if (((ACAN*)ckt->CKTcurJob)->ACstepType != LINEAR) {
	    (*(SPfrontEnd->OUTattributes))((GENERIC *)acPlot,NULL,
		    OUT_SCALE_LOG, NULL);
	}
        freq = ((ACAN*)ckt->CKTcurJob)->ACstartFreq;

    } else {    /* continue previous analysis */
        freq = ((ACAN*)ckt->CKTcurJob)->ACsaveFreq;
        ((ACAN*)ckt->CKTcurJob)->ACsaveFreq = 0; /* clear the 'old' frequency */
    }
    switch(((ACAN*)ckt->CKTcurJob)->ACstepType) {
    case DECADE:
    case OCTAVE:
        freqTol = ((ACAN*)ckt->CKTcurJob)->ACfreqDelta * 
                ((ACAN*)ckt->CKTcurJob)->ACstopFreq * ckt->CKTreltol;
        break;
    case LINEAR:
        freqTol = ((ACAN*)ckt->CKTcurJob)->ACfreqDelta * ckt->CKTreltol;
        break;
    default:
        return(E_BADPARM);
    }

    while(freq <= ((ACAN*)ckt->CKTcurJob)->ACstopFreq+freqTol) {

        if( (*(SPfrontEnd->IFpauseTest))() ) { 
            /* user asked us to pause via an interrupt */
            ((ACAN*)ckt->CKTcurJob)->ACsaveFreq = freq;
            return(E_PAUSE);
        }
        ckt->CKTomega = 2.0 * M_PI *freq;
        ckt->CKTmode = (ckt->CKTmode&MODEUIC) | MODEAC;

        error = NIacIter(ckt);
        if(error) return(error);


#ifdef HAS_SENSE2
        if(ckt->CKTsenInfo && (ckt->CKTsenInfo->SENmode&ACSEN) ){

            save = ckt->CKTmode;
            ckt->CKTmode=(ckt->CKTmode&MODEUIC)|MODEDCOP|MODEINITSMSIG;
            save1 = ckt->CKTsenInfo->SENmode;
            ckt->CKTsenInfo->SENmode = ACSEN;
            if(freq == ((ACAN*)ckt->CKTcurJob)->ACstartFreq){
                ckt->CKTsenInfo->SENacpertflag = 1;
            }
            else{
                ckt->CKTsenInfo->SENacpertflag = 0;
            }
            if(error = CKTsenAC(ckt)) return (error);
            ckt->CKTmode = save;
            ckt->CKTsenInfo->SENmode = save1;
        }
#endif

        error = CKTacDump(ckt,freq,acPlot);
        if(error) return(error);

        /*  increment frequency */

        switch(((ACAN*)ckt->CKTcurJob)->ACstepType) {
        case DECADE:
        case OCTAVE:
            freq *= ((ACAN*)ckt->CKTcurJob)->ACfreqDelta;
            if(((ACAN*)ckt->CKTcurJob)->ACfreqDelta==1) goto endsweep;
            break;
        case LINEAR:
            freq += ((ACAN*)ckt->CKTcurJob)->ACfreqDelta;
            if(((ACAN*)ckt->CKTcurJob)->ACfreqDelta==0) goto endsweep;
            break;
        default:
            return(E_INTERN);
        }
    }
endsweep:
    (*(SPfrontEnd->OUTendPlot))(acPlot);
    return(0);
}


    /* CKTacLoad(ckt)
     * this is a driver program to iterate through all the various
     * ac load functions provided for the circuit elements in the
     * given circuit 
     */


int
CKTacLoad(ckt)
    register CKTcircuit *ckt;
{
    extern SPICEdev *DEVices[];
    register int i;
    register int size;
    int error;
    size = SMPmatSize(ckt->CKTmatrix);
    for (i=0;i<=size;i++) {
        *(ckt->CKTrhs+i)=0;
        *(ckt->CKTirhs+i)=0;
    }
    SMPcClear(ckt->CKTmatrix);

    for (i=0;i<DEVmaxnum;i++) {
        if ( ((*DEVices[i]).DEVacLoad != NULL) && (ckt->CKThead[i] != NULL) ){
            error = (*((*DEVices[i]).DEVacLoad))(ckt->CKThead[i],ckt);
            if(error) return(error);
        }
    }
    return(OK);
}
