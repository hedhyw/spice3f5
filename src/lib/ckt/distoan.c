/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1988 Jaijeet S Roychowdhury
**********/

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "distodef.h"
#include "util.h"
#include "sperror.h"
#include "suffix.h"

void
DISswap(a,b)
double **a, **b;
{
double *c;

c = *a;
*a = *b;
*b = c;
}

void
DmemAlloc(a,size)
double **a;
int size;
{
*a = (double *) MALLOC( sizeof(double) * size + 1);
}



void
DstorAlloc(header,size)
double ***header;
int size;
{
*header = (double **) MALLOC( sizeof(double *)*size);
}


#ifdef STDC
extern int CKTdisto(CKTcircuit*, int);
extern int DkerProc(int,double*,double*,int,DISTOan*);
#else
extern int CKTdisto();
extern int DkerProc();
#endif


int
DISTOan(ckt,restart) 
CKTcircuit *ckt;
int restart;
{

    double freq;
    static	double freqTol;	/* tolerence parameter for finding final frequency */
    static int NoOfPoints;
    static int size;
    static int displacement;
    int error;
    double omegadelta;
    long save;
    int save1;
    int i;
    int numNames;
    IFuid *nameList;
    IFuid freqUid;
    GENERIC *acPlot;
    register DISTOAN* job = (DISTOAN *) (ckt->CKTcurJob);
    static char *nof2src = "No source with f2 distortion input";
#ifdef DISTODEBUG
    double time,time1;
#endif


		/* start at beginning */

#ifdef D_DBG_BLOCKTIMES
time1 = (*(SPfrontEnd->IFseconds))();
#endif
        switch(job->DstepType) {

        case DECADE:
            job->DfreqDelta =
					exp(log(10.0)/job->DnumSteps);
		freqTol = job->DfreqDelta * 
				job->DstopF1 * ckt->CKTreltol;
	    NoOfPoints = 1 + floor ((job->DnumSteps) / log(10.0) * log((job->DstopF1+freqTol)/(job->DstartF1)));
            break;
        case OCTAVE:
            job->DfreqDelta =
					exp(log(2.0)/job->DnumSteps);
		freqTol = job->DfreqDelta * 
				job->DstopF1 * ckt->CKTreltol;
	    NoOfPoints = 1 + floor ((job->DnumSteps) / log(2.0) * log((job->DstopF1+freqTol)/(job->DstartF1)));
            break;
        case LINEAR:
            job->DfreqDelta =
					(job->DstopF1 -
					job->DstartF1)/
					(job->DnumSteps+1);
		freqTol = job->DfreqDelta * ckt->CKTreltol;
					NoOfPoints = job->DnumSteps+1+ floor(freqTol/(job->DfreqDelta));
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
		(GENERIC*)ckt->CKTcurJob, "Distortion Operating Point",
		(IFuid)NULL,IF_REAL,numNames,nameList, IF_REAL,&acPlot);
	    if(error) return(error);
	    CKTdump(ckt,(double)0,acPlot);
	    (*(SPfrontEnd->OUTendPlot))(acPlot);
	}

#ifdef D_DBG_BLOCKTIMES
time1 = (*(SPfrontEnd->IFseconds))() - time1;
printf("Time for initial work (including op. pt.): %g seconds \n", time1);
#endif

#ifdef D_DBG_BLOCKTIMES
time1 = (*(SPfrontEnd->IFseconds))();
#endif
	error = CKTdisto(ckt,D_SETUP);
#ifdef D_DBG_BLOCKTIMES
time1 = (*(SPfrontEnd->IFseconds))() - time1;
printf("Time outside D_SETUP: %g seconds \n", time1);
#endif
	if (error) return(error);

	displacement = 0;

#ifdef D_DBG_BLOCKTIMES
time1 = (*(SPfrontEnd->IFseconds))();
#endif
        freq = job->DstartF1;
	if (job->Df2wanted) {
	/*
	omegadelta = 2.0 * M_PI * freq *(1. - job->Df2ovrF1);
	*/
    /* keeping f2 const to be compatible with hbsim */
	job->Domega2 = 2.0 * M_PI * freq * job->Df2ovrF1;
	}
	DstorAlloc( &(job->r1H1stor),NoOfPoints+2);
	DstorAlloc( &(job->r2H11stor),NoOfPoints+2);
	DstorAlloc( &(job->i1H1stor),NoOfPoints+2);
	DstorAlloc( &(job->i2H11stor),NoOfPoints+2);
	size = SMPmatSize(ckt->CKTmatrix);
	if (! job->r1H1ptr)
		{
		DmemAlloc( &(job->r1H1ptr) , size+2);
		if (! job->r1H1ptr) return (E_NOMEM);
		}
	
	if (! job->r2H11ptr)
		{
		DmemAlloc( &(job->r2H11ptr) , size+2);
		if (! job->r2H11ptr) return (E_NOMEM);
		}
	if (! job->i1H1ptr)
		{
		DmemAlloc( &(job->i1H1ptr) , size+2);
		if (! job->i1H1ptr) return (E_NOMEM);
		}
	
	if (! job->i2H11ptr)
		{
		DmemAlloc( &(job->i2H11ptr) , size+2);
		if (! job->i2H11ptr) return (E_NOMEM);
		}

	if (! (job->Df2wanted))
		{
		DstorAlloc( &(job->r3H11stor),NoOfPoints+2);
		DstorAlloc( &(job->i3H11stor),NoOfPoints+2);
		if (! job->r3H11ptr)
			{
			DmemAlloc( &(job->r3H11ptr) , size+2);
			if (! job->r3H11ptr) return (E_NOMEM);
			}
		if (! job->i3H11ptr)
			{
			DmemAlloc( &(job->i3H11ptr) , size+2);
			if (! job->i3H11ptr) return (E_NOMEM);
			}
		} else {
		DstorAlloc ( &(job->r1H2stor),NoOfPoints+2);
		DstorAlloc ( &(job->i1H2stor),NoOfPoints+2);
		DstorAlloc ( &(job->r2H12stor),NoOfPoints+2);
		DstorAlloc ( &(job->i2H12stor),NoOfPoints+2);
		DstorAlloc ( &(job->r2H1m2stor),NoOfPoints+2);
		DstorAlloc ( &(job->i2H1m2stor),NoOfPoints+2);
		DstorAlloc ( &(job->r3H1m2stor),NoOfPoints+2);
		DstorAlloc ( &(job->i3H1m2stor),NoOfPoints+2);
		if (! job->r1H2ptr)
			{
			DmemAlloc( &(job->r1H2ptr) , size+2);
			if (! job->r1H2ptr) return (E_NOMEM);
			}
			
		if (! job->r2H12ptr)
			{
			DmemAlloc( &(job->r2H12ptr) , size+2);
			if (! job->r2H12ptr) return (E_NOMEM);
			}

		if (! job->r2H1m2ptr)
			{
			DmemAlloc( &(job->r2H1m2ptr) , size+2);
			if (! job->r2H1m2ptr) return (E_NOMEM);
			}
			
		if (! job->r3H1m2ptr)
			{
			DmemAlloc( &(job->r3H1m2ptr) , size+2);
			if (! job->r3H1m2ptr) return (E_NOMEM);
			}
		if (! job->i1H2ptr)
			{
			DmemAlloc( &(job->i1H2ptr) , size+2);
			if (! job->i1H2ptr) return (E_NOMEM);
			}
			
		if (! job->i2H12ptr)
			{
			DmemAlloc( &(job->i2H12ptr) , size+2);
			if (! job->i2H12ptr) return (E_NOMEM);
			}

		if (! job->i2H1m2ptr)
			{
			DmemAlloc( &(job->i2H1m2ptr) , size+2);
			if (! job->i2H1m2ptr) return (E_NOMEM);
			}
			
		if (! job->i3H1m2ptr)
			{
			DmemAlloc( &(job->i3H1m2ptr) , size+2);
			if (! job->i3H1m2ptr) return (E_NOMEM);
			}
}
		
#ifdef D_DBG_BLOCKTIMES
time1 = (*(SPfrontEnd->IFseconds))() - time1;
printf("Time for other setup (storage allocation etc.): %g seconds \n", time1);
#endif



#ifdef D_DBG_BLOCKTIMES
time1 = (*(SPfrontEnd->IFseconds))();
#endif
    while(freq <= job->DstopF1+freqTol) {

/*
        if( (*(SPfrontEnd->IFpauseTest))() ) { 
            job->DsaveF1 = freq;
            return(E_PAUSE);
        }
	*/
        ckt->CKTomega = 2.0 * M_PI *freq;
	job->Domega1 = ckt->CKTomega;
        ckt->CKTmode = (ckt->CKTmode&MODEUIC) | MODEAC;
#ifdef D_DBG_SMALLTIMES
time = (*(SPfrontEnd->IFseconds))();
#endif
	error = CKTacLoad(ckt);
#ifdef D_DBG_SMALLTIMES
time = (*(SPfrontEnd->IFseconds))() - time;
printf("Time for CKTacLoad: %g seconds \n", time);
#endif
	if (error) return(error);
#ifdef D_DBG_SMALLTIMES
time = (*(SPfrontEnd->IFseconds))();
#endif
	error = CKTdisto(ckt,D_RHSF1); /* sets up the RHS vector
			for all inputs corresponding to F1 */
#ifdef D_DBG_SMALLTIMES
time = (*(SPfrontEnd->IFseconds))() - time;
printf("Time outside DISTO_RHSFIX: %g seconds \n", time);
#endif
	if (error) return(error);
#ifdef D_DBG_SMALLTIMES
time = (*(SPfrontEnd->IFseconds))();
#endif
	error = NIdIter(ckt);
#ifdef D_DBG_SMALLTIMES
time = (*(SPfrontEnd->IFseconds))() - time;
printf("Time for NIdIter: %g seconds \n", time);
#endif
	if (error) return(error);
	DISswap(&(ckt->CKTrhsOld),&(job->r1H1ptr));
	DISswap(&(ckt->CKTirhsOld),&(job->i1H1ptr));

	ckt->CKTomega *= 2;
	error = CKTacLoad(ckt);
	if (error) return(error);
#ifdef D_DBG_SMALLTIMES
time = (*(SPfrontEnd->IFseconds))();
#endif
	error = CKTdisto(ckt,D_TWOF1);
#ifdef D_DBG_SMALLTIMES
time = (*(SPfrontEnd->IFseconds))() - time;
printf("Time outside D_TWOF1: %g seconds \n", time);
#endif
	if (error) return(error);
	error = NIdIter(ckt);
	if (error) return(error);
	DISswap(&(ckt->CKTrhsOld),&(job->r2H11ptr));
	DISswap(&(ckt->CKTirhsOld),&(job->i2H11ptr));

	if (! (job->Df2wanted )) 
		{


		ckt->CKTomega = 3 * job->Domega1;
		error = CKTacLoad(ckt);
		if (error) return(error);
#ifdef D_DBG_SMALLTIMES
time = (*(SPfrontEnd->IFseconds))();
#endif
		error = CKTdisto(ckt,D_THRF1);
#ifdef D_DBG_SMALLTIMES
time = (*(SPfrontEnd->IFseconds))() - time;
printf("Time outside D_THRF1: %g seconds \n", time);
#endif
		if (error) return(error);
		error = NIdIter(ckt);
		if (error) return(error);
		DISswap(&(ckt->CKTrhsOld),&(job->r3H11ptr));
		DISswap(&(ckt->CKTirhsOld),&(job->i3H11ptr));


		}
		else if (job->Df2given)
		{


		/*
		ckt->CKTomega = job->Domega1 - omegadelta;
		job->Domega2 = ckt->CKTomega;
		*/
		ckt->CKTomega = job->Domega2;
		error = CKTacLoad(ckt);
		if (error) return(error);
#ifdef D_DBG_SMALLTIMES
time = (*(SPfrontEnd->IFseconds))();
#endif
		error = CKTdisto(ckt,D_RHSF2);
#ifdef D_DBG_SMALLTIMES
time = (*(SPfrontEnd->IFseconds))() - time;
printf("Time outside DISTO_RHSFIX: %g seconds \n", time);
#endif
		if (error) return(error);
		error = NIdIter(ckt);
		if (error) return(error);
		DISswap(&(ckt->CKTrhsOld),&(job->r1H2ptr));
		DISswap(&(ckt->CKTirhsOld),&(job->i1H2ptr));


		ckt->CKTomega = job->Domega1 +
					job->Domega2;
		error = CKTacLoad(ckt);
		if (error) return(error);
#ifdef D_DBG_SMALLTIMES
time = (*(SPfrontEnd->IFseconds))();
#endif
		error = CKTdisto(ckt,D_F1PF2);
#ifdef D_DBG_SMALLTIMES
time = (*(SPfrontEnd->IFseconds))() - time;
printf("Time outside D_F1PF2: %g seconds \n", time);
#endif
		if (error) return(error);
		error = NIdIter(ckt);
		if (error) return(error);
		DISswap(&(ckt->CKTrhsOld),&(job->r2H12ptr));
		DISswap(&(ckt->CKTirhsOld),&(job->i2H12ptr));



		ckt->CKTomega = job->Domega1 -
					    job->Domega2;
		error = CKTacLoad(ckt);
		if (error) return(error);
#ifdef D_DBG_SMALLTIMES
time = (*(SPfrontEnd->IFseconds))();
#endif
		error = CKTdisto(ckt,D_F1MF2);
#ifdef D_DBG_SMALLTIMES
time = (*(SPfrontEnd->IFseconds))() - time;
printf("Time outside D_F1MF2: %g seconds \n", time);
#endif
		if (error) return(error);
		error = NIdIter(ckt);
		if (error) return(error);
		DISswap(&(ckt->CKTrhsOld),&(job->r2H1m2ptr));
		DISswap(&(ckt->CKTirhsOld),&(job->i2H1m2ptr));


		ckt->CKTomega = 2*job->Domega1 -
						job->Domega2;
		error = CKTacLoad(ckt);
		if (error) return(error);
#ifdef D_DBG_SMALLTIMES
time = (*(SPfrontEnd->IFseconds))();
#endif
		error = CKTdisto(ckt,D_2F1MF2);
#ifdef D_DBG_SMALLTIMES
time = (*(SPfrontEnd->IFseconds))() - time;
printf("Time outside D_2F1MF2: %g seconds \n", time);
#endif
		if (error) return(error);
		error = NIdIter(ckt);
		if (error) return(error);
		DISswap(&(ckt->CKTrhsOld),&(job->r3H1m2ptr));
		DISswap(&(ckt->CKTirhsOld),&(job->i3H1m2ptr));


		}
	else 
	{
        errMsg = MALLOC(strlen(nof2src)+1);
        strcpy(errMsg,nof2src);
	return(E_NOF2SRC);
	}




		DmemAlloc( &(job->r1H1stor[displacement]),size);
		DISswap(&(job->r1H1stor[displacement]),&(job->r1H1ptr));
		job->r1H1stor[displacement][0]=freq;
		DmemAlloc( &(job->r2H11stor[displacement]),size);
		DISswap(&(job->r2H11stor[displacement]),&((job->r2H11ptr)));
		job->r2H11stor[displacement][0]=freq;
		DmemAlloc( &(job->i1H1stor[displacement]),size);
		DISswap(&(job->i1H1stor[displacement]),&((job->i1H1ptr)));
		job->i1H1stor[displacement][0]=0.0;
		DmemAlloc( &(job->i2H11stor[displacement]),size);
		DISswap(&(job->i2H11stor[displacement]),&((job->i2H11ptr)));
		job->i2H11stor[displacement][0]=0.0;
	if (! (job->Df2wanted))
	{
	  DmemAlloc( &(job->r3H11stor[displacement]),size); 
	  DISswap(&(job->r3H11stor[displacement]),&((job->r3H11ptr)));
	  job->r3H11stor[displacement][0]=freq;
	  DmemAlloc( &(job->i3H11stor[displacement]),size); 
	  DISswap(&(job->i3H11stor[displacement]),&((job->i3H11ptr)));
	  job->i3H11stor[displacement][0]=0.0;
	} else {
	  DmemAlloc( &(job->r1H2stor[displacement]),size);
	  DISswap(&(job->r1H2stor[displacement]),&((job->r1H2ptr)));
	  job->r1H2stor[displacement][0]=freq;
	  DmemAlloc( &(job->r2H12stor[displacement]),size);
	  DISswap(&(job->r2H12stor[displacement]),&((job->r2H12ptr)));
	  job->r2H12stor[displacement][0]=freq;
	  DmemAlloc( &(job->r2H1m2stor[displacement]),size);
	  DISswap(&(job->r2H1m2stor[displacement]),&((job->r2H1m2ptr)));
	  job->r2H1m2stor[displacement][0]=freq;
	  DmemAlloc( &(job->r3H1m2stor[displacement]),size);
	  DISswap(&(job->r3H1m2stor[displacement]),&((job->r3H1m2ptr)));
	  job->r3H1m2stor[displacement][0]=freq;

	  DmemAlloc( &(job->i1H2stor[displacement]),size);
	  DISswap(&(job->i1H2stor[displacement]),&((job->i1H2ptr)));
	  job->i1H2stor[displacement][0]=0.0;
	  DmemAlloc( &(job->i2H12stor[displacement]),size);
	  DISswap(&(job->i2H12stor[displacement]),&((job->i2H12ptr)));
	  job->i2H12stor[displacement][0]=0.0;
	  DmemAlloc( &(job->i2H1m2stor[displacement]),size);
	  DISswap(&(job->i2H1m2stor[displacement]),&((job->i2H1m2ptr)));
	  job->i2H1m2stor[displacement][0]=0.0;
	  DmemAlloc( &(job->i3H1m2stor[displacement]),size);
	  DISswap(&(job->i3H1m2stor[displacement]),&((job->i3H1m2ptr)));
	  job->i3H1m2stor[displacement][0]=0.0;
	  }
	  displacement++;



        switch(job->DstepType) {
        case DECADE:
        case OCTAVE:
            freq *= job->DfreqDelta;
            if(job->DfreqDelta==1) goto endsweep;
            break;
        case LINEAR:
            freq += job->DfreqDelta;
            if(job->DfreqDelta==0) goto endsweep;
            break;
        default:
            return(E_INTERN);
        }
	}
#ifdef D_DBG_BLOCKTIMES
time1 = (*(SPfrontEnd->IFseconds))() - time1;
printf("Time inside frequency loop: %g seconds \n", time1);
#endif
    
endsweep:


    /* output routines to process the H's and output actual ckt variable
       values */
#ifdef D_DBG_BLOCKTIMES
time1 = (*(SPfrontEnd->IFseconds))();
#endif

       

	if (! job->Df2wanted) {
	error = CKTnames(ckt,&numNames,&nameList);
	if(error) return(error);
	(*(SPfrontEnd->IFnewUid))((GENERIC *)ckt,&freqUid,(IFuid)NULL,
		"frequency", UID_OTHER,(GENERIC **)NULL);
	(*(SPfrontEnd->OUTpBeginPlot))((GENERIC *)ckt,
	    (GENERIC*)ckt->CKTcurJob,"DISTORTION - 2nd harmonic",
	    freqUid,IF_REAL, numNames,nameList,IF_COMPLEX,&acPlot);
        if (job->DstepType != LINEAR) {
	    (*(SPfrontEnd->OUTattributes))((GENERIC *)acPlot,NULL,
		OUT_SCALE_LOG, NULL);
	}
       for (i=0; i< displacement ; i++)
       {
	DkerProc(D_TWOF1,*(job->r2H11stor + i),
			     *(job->i2H11stor + i),
				 size, job);
	ckt->CKTrhsOld = *((job->r2H11stor) + i);
	ckt->CKTirhsOld = *((job->i2H11stor) + i);
	error = CKTacDump(ckt,ckt->CKTrhsOld[0],acPlot);
        if(error) return(error);
	}
	(*(SPfrontEnd->OUTendPlot))(acPlot);

	error = CKTnames(ckt,&numNames,&nameList);
	if(error) return(error);
	(*(SPfrontEnd->IFnewUid))((GENERIC *)ckt,&freqUid,(IFuid)NULL,
		"frequency", UID_OTHER,(GENERIC **)NULL);
	(*(SPfrontEnd->OUTpBeginPlot))((GENERIC *)ckt,
	(GENERIC*)ckt->CKTcurJob,"DISTORTION - 3rd harmonic",freqUid,IF_REAL,
	numNames,nameList,IF_COMPLEX,&acPlot);
       for (i=0; i< displacement ; i++)
       {
	DkerProc(D_THRF1,*(job->r3H11stor + i),
			     *(job->i3H11stor + i),
				 size, job);
	ckt->CKTrhsOld = *((job->r3H11stor) + i);
	ckt->CKTirhsOld = *((job->i3H11stor) + i);
	error = CKTacDump(ckt,ckt->CKTrhsOld[0],acPlot);
	}
	(*(SPfrontEnd->OUTendPlot))(acPlot);
      
	} else {


	error = CKTnames(ckt,&numNames,&nameList);
	if(error) return(error);
	(*(SPfrontEnd->IFnewUid))((GENERIC *)ckt,&freqUid,(IFuid)NULL,
		"frequency", UID_OTHER,(GENERIC **)NULL);
	(*(SPfrontEnd->OUTpBeginPlot))((GENERIC *)ckt,
	(GENERIC*)ckt->CKTcurJob,"DISTORTION - IM: f1+f2",freqUid,IF_REAL,
	numNames,nameList,IF_COMPLEX,&acPlot);
	   for (i=0; i< displacement ; i++)
	   {
	DkerProc(D_F1PF2,*(job->r2H12stor + i),
			     *(job->i2H12stor + i),
				 size, job);
	ckt->CKTrhsOld = *((job->r2H12stor) + i);
	ckt->CKTirhsOld = *((job->i2H12stor) + i);
	error = CKTacDump(ckt,ckt->CKTrhsOld[0],acPlot);
	    if(error) return(error);
	    }
	(*(SPfrontEnd->OUTendPlot))(acPlot);

	error = CKTnames(ckt,&numNames,&nameList);
	if(error) return(error);
	(*(SPfrontEnd->IFnewUid))((GENERIC *)ckt,&freqUid,(IFuid)NULL,
		"frequency", UID_OTHER,(GENERIC **)NULL);
	(*(SPfrontEnd->OUTpBeginPlot))((GENERIC *)ckt,
	(GENERIC*)ckt->CKTcurJob,"DISTORTION - IM: f1-f2",freqUid,IF_REAL,
	numNames,nameList,IF_COMPLEX,&acPlot);
	   for (i=0; i< displacement ; i++)
	   {
	DkerProc(D_F1MF2,
			    *(job->r2H1m2stor + i),
			    *(job->i2H1m2stor + i),
				 size, job);
	ckt->CKTrhsOld = *((job->r2H1m2stor) + i);
	ckt->CKTirhsOld = *((job->i2H1m2stor) + i);
	error = CKTacDump(ckt,ckt->CKTrhsOld[0],acPlot);
	    if(error) return(error);
	    }
	(*(SPfrontEnd->OUTendPlot))(acPlot);

	error = CKTnames(ckt,&numNames,&nameList);
	if(error) return(error);
	(*(SPfrontEnd->IFnewUid))((GENERIC *)ckt,&freqUid,(IFuid)NULL,
		"frequency", UID_OTHER,(GENERIC **)NULL);
	(*(SPfrontEnd->OUTpBeginPlot))((GENERIC *)ckt,
	(GENERIC*)ckt->CKTcurJob,"DISTORTION - IM: 2f1-f2",freqUid,IF_REAL,
	numNames,nameList,IF_COMPLEX,&acPlot);
	   for (i=0; i< displacement ; i++)
	   {
	DkerProc(D_2F1MF2,
			*(job->r3H1m2stor + i),
			*(job->i3H1m2stor + i),
				 size, job);
	ckt->CKTrhsOld = *((job->r3H1m2stor) + i);
	ckt->CKTirhsOld = *((job->i3H1m2stor) + i);
	error = CKTacDump(ckt,ckt->CKTrhsOld[0],acPlot);
	    if(error) return(error);
	    }
	(*(SPfrontEnd->OUTendPlot))(acPlot);

    }
FREE(job->r1H1ptr);
FREE(job->i1H1ptr);
FREE(job->r2H11ptr);
FREE(job->i2H11ptr);

FREE(job->r1H1stor);
FREE(job->i1H1stor);
FREE(job->r2H11stor);
FREE(job->i2H11stor);

	if (! (job->Df2wanted))
	{
FREE(job->r3H11ptr);
FREE(job->i3H11ptr);

FREE(job->i3H11stor);
FREE(job->r3H11stor);
}
else {

FREE(job->r2H1m2ptr);
FREE(job->r3H1m2ptr);
FREE(job->r1H2ptr);
FREE(job->i1H2ptr);
FREE(job->r2H12ptr);
FREE(job->i2H12ptr);
FREE(job->i2H1m2ptr);
FREE(job->i3H1m2ptr);

FREE(job->r1H2stor);
FREE(job->r2H12stor);
FREE(job->r2H1m2stor);
FREE(job->r3H1m2stor);
FREE(job->i1H2stor);
FREE(job->i2H12stor);
FREE(job->i2H1m2stor);
FREE(job->i3H1m2stor);
    }
#ifdef D_DBG_BLOCKTIMES
time1 = (*(SPfrontEnd->IFseconds))() - time1;
printf("Time for output and deallocation: %g seconds \n", time1);
#endif
    return(OK);
}
