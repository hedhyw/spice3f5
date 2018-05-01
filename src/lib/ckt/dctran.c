/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/* subroutine to do DC TRANSIENT analysis    
        --- ONLY, unlike spice2 routine with the same name! */

#include "spice.h"
#include "misc.h"
#include <stdio.h>
#include "trandefs.h"
#include "cktdefs.h"
#include "util.h"
#include "sperror.h"
#include "suffix.h"

int
DCtran(ckt,restart) 
    CKTcircuit *ckt;
    int restart;    /* forced restart flag */
{
    int i;
    double olddelta;
    double delta;
    double new;
    double *temp;
    double startdTime;
    double startsTime;
    double startTime;
    int startIters;
    int converged;
    int firsttime;
    int error;
#ifdef HAS_SENSE2
#ifdef SENSDEBUG
    FILE *outsen;
#endif /* SENSDEBUG */
#endif
    int save_order;
    int save;
    int save2;
    int size;
    long save_mode;
    long save1;
    IFuid timeUid;
    IFuid *nameList;
    int numNames;
    double maxstepsize;
#ifdef HAS_SHORTMACRO
    double mt;  /* temporary so macro call won't cross line boundry */
#endif
    int ltra_num;

    if(restart || ckt->CKTtime == 0) {
        delta=MIN(ckt->CKTfinalTime/50,ckt->CKTstep)/10;

	/* begin LTRA code addition */
	if (ckt->CKTtimePoints != NULL)
	    FREE(ckt->CKTtimePoints);

	if (ckt->CKTstep >= ckt->CKTmaxStep)
	    maxstepsize = ckt->CKTstep;
	else
	    maxstepsize = ckt->CKTmaxStep;

	ckt->CKTsizeIncr = 10;
	ckt->CKTtimeIndex = -1; /* before the DC soln has been stored */
	ckt->CKTtimeListSize = ckt->CKTfinalTime / maxstepsize + 0.5;
	ltra_num = CKTtypelook("LTRA");
	if (ltra_num >= 0 && ckt->CKThead[ltra_num] != NULL)
	    ckt->CKTtimePoints = NEWN(double, ckt->CKTtimeListSize);
	/* end LTRA code addition */

        if(ckt->CKTbreaks) FREE(ckt->CKTbreaks);
        ckt->CKTbreaks=(double *)MALLOC(2*sizeof(double));
        if(ckt->CKTbreaks == (double *)NULL) return(E_NOMEM);
        *(ckt->CKTbreaks)=0;
        *(ckt->CKTbreaks+1)=ckt->CKTfinalTime;
        ckt->CKTbreakSize=2;
        if(ckt->CKTminBreak==0) ckt->CKTminBreak=ckt->CKTmaxStep*5e-5;

        error = CKTnames(ckt,&numNames,&nameList);
        if(error) return(error);
        (*(SPfrontEnd->IFnewUid))((GENERIC *)ckt,&timeUid,(IFuid)NULL,
                "time", UID_OTHER, (GENERIC **)NULL);
        error = (*(SPfrontEnd->OUTpBeginPlot))((GENERIC *)ckt,
		(GENERIC*)ckt->CKTcurJob,
                ckt->CKTcurJob->JOBname,timeUid,IF_REAL,numNames,nameList,
                IF_REAL,&(((TRANan*)ckt->CKTcurJob)->TRANplot));
        if(error) return(error);

        ckt->CKTtime = 0;
        ckt->CKTdelta = 0;
        ckt->CKTbreak=1;
        firsttime = 1;
        save_mode = (ckt->CKTmode&MODEUIC)|MODETRANOP | MODEINITJCT;
        save_order = ckt->CKTorder;
        converged = CKTop(ckt,
                (ckt->CKTmode & MODEUIC)|MODETRANOP| MODEINITJCT,
                (ckt->CKTmode & MODEUIC)|MODETRANOP| MODEINITFLOAT,
                ckt->CKTdcMaxIter);
        if(converged != 0) return(converged);
        ckt->CKTstat->STATtimePts ++;
        ckt->CKTorder=1;
        for(i=0;i<7;i++) {
            ckt->CKTdeltaOld[i]=ckt->CKTmaxStep;
        }
        ckt->CKTdelta = delta;
#ifdef STEPDEBUG
        (void)printf("delta initialized to %g\n",ckt->CKTdelta);
#endif
        ckt->CKTsaveDelta = ckt->CKTfinalTime/50;

#ifdef HAS_SENSE2
        if(ckt->CKTsenInfo && (ckt->CKTsenInfo->SENmode & TRANSEN)){
#ifdef SENSDEBUG
            printf("\nTransient Sensitivity Results\n\n");
            CKTsenPrint(ckt);
#endif /* SENSDEBUG */
            save = ckt->CKTsenInfo->SENmode;
            ckt->CKTsenInfo->SENmode = TRANSEN;
            save1 = ckt->CKTmode;
            save2 = ckt->CKTorder;
            ckt->CKTmode = save_mode;
            ckt->CKTorder = save_order;
            if(error = CKTsenDCtran(ckt)) return(error);
            ckt->CKTmode = save1;
            ckt->CKTorder = save2;
        }
#endif

        ckt->CKTmode = (ckt->CKTmode&MODEUIC)|MODETRAN | MODEINITTRAN;
		/* modeinittran set here */
        ckt->CKTag[0]=ckt->CKTag[1]=0;
        bcopy((char *)ckt->CKTstate0,(char *)ckt->CKTstate1,
                ckt->CKTnumStates*sizeof(double));

#ifdef HAS_SENSE2
        if(ckt->CKTsenInfo && (ckt->CKTsenInfo->SENmode & TRANSEN)){
            size = SMPmatSize(ckt->CKTmatrix);
            for(i = 1; i<=size ; i++)
                *(ckt->CKTrhsOp + i) = *(ckt->CKTrhsOld + i);
        }
#endif

        startTime=(*(SPfrontEnd->IFseconds))();
        startIters = ckt->CKTstat->STATnumIter;
        startdTime = ckt->CKTstat->STATdecompTime;
        startsTime = ckt->CKTstat->STATsolveTime;
    } else {
        startTime=(*(SPfrontEnd->IFseconds))();
        startIters = ckt->CKTstat->STATnumIter;
        startdTime = ckt->CKTstat->STATdecompTime;
        startsTime = ckt->CKTstat->STATsolveTime;
        if(ckt->CKTminBreak==0) ckt->CKTminBreak=ckt->CKTmaxStep*5e-5;
        firsttime=0;
        goto resume;
    }

/* 650 */
nextTime:

    /* begin LTRA code addition */
    if (ckt->CKTtimePoints) {
	ckt->CKTtimeIndex++;
	if (ckt->CKTtimeIndex >= ckt->CKTtimeListSize) {
	    /* need more space */
	    int need;
	    need = 0.5 + (ckt->CKTfinalTime - ckt->CKTtime) / maxstepsize;
	    if (need < ckt->CKTsizeIncr)
		need = ckt->CKTsizeIncr;
	    ckt->CKTtimeListSize += need;
	    ckt->CKTtimePoints = (double *) REALLOC( (char *)
		 ckt->CKTtimePoints, sizeof(double) * ckt->CKTtimeListSize);
	    ckt->CKTsizeIncr *= 1.4;
	}
	*(ckt->CKTtimePoints + ckt->CKTtimeIndex) = ckt->CKTtime;
    }
    /* end LTRA code addition */

    error = CKTaccept(ckt);
    /* check if current breakpoint is outdated; if so, clear */
    if (ckt->CKTtime > *(ckt->CKTbreaks)) CKTclrBreak(ckt);

/*
 * Breakpoint handling scheme:
 * When a timepoint t is accepted (by CKTaccept), clear all previous
 * breakpoints, because they will never be needed again.
 *
 * t may itself be a breakpoint, or indistinguishably close. DON'T
 * clear t itself; recognise it as a breakpoint and act accordingly
 *
 * if t is not a breakpoint, limit the timestep so that the next
 * breakpoint is not crossed
 */

#ifdef STEPDEBUG
    printf("accepted at %g\n",ckt->CKTtime);
#endif /* STEPDEBUG */
    ckt->CKTstat->STATaccepted ++;
    ckt->CKTbreak=0;
    if(error)  {
        ckt->CKTcurrentAnalysis = DOING_TRAN;
        ckt->CKTstat->STATtranTime += (*(SPfrontEnd->IFseconds))()-startTime;
        ckt->CKTstat->STATtranIter += ckt->CKTstat->STATnumIter - startIters;
        ckt->CKTstat->STATtranDecompTime += ckt->CKTstat->STATdecompTime -
                startdTime;
        ckt->CKTstat->STATtranSolveTime += ckt->CKTstat->STATsolveTime -
                startsTime;
        return(error);
    }
    if(ckt->CKTtime >= ckt->CKTinitTime) CKTdump(ckt,ckt->CKTtime,
            (((TRANan*)ckt->CKTcurJob)->TRANplot));
    ckt->CKTstat->STAToldIter = ckt->CKTstat->STATnumIter;
    if(FABS(ckt->CKTtime - ckt->CKTfinalTime) < ckt->CKTminBreak) {
        /*printf(" done:  time is %g, final time is %g, and tol is %g\n",*/
        /*ckt->CKTtime,ckt->CKTfinalTime,ckt->CKTminBreak);*/
        (*(SPfrontEnd->OUTendPlot))( (((TRANan*)ckt->CKTcurJob)->TRANplot));
        ckt->CKTcurrentAnalysis = 0;
        ckt->CKTstat->STATtranTime += (*(SPfrontEnd->IFseconds))()-startTime;
        ckt->CKTstat->STATtranIter += ckt->CKTstat->STATnumIter - startIters;
        ckt->CKTstat->STATtranDecompTime += ckt->CKTstat->STATdecompTime -
                startdTime;
        ckt->CKTstat->STATtranSolveTime += ckt->CKTstat->STATsolveTime -
                startsTime;
#ifdef HAS_SENSE2
        if(ckt->CKTsenInfo && (ckt->CKTsenInfo->SENmode & TRANSEN)){
            ckt->CKTsenInfo->SENmode = save;
#ifdef SENSDEBUG
            fclose(outsen);
#endif /* SENSDEBUG */
        }
#endif
        return(OK);
    }
    if( (*(SPfrontEnd->IFpauseTest))() ) {
        /* user requested pause... */
        ckt->CKTcurrentAnalysis = DOING_TRAN;
        ckt->CKTstat->STATtranTime += (*(SPfrontEnd->IFseconds))()-startTime;
        ckt->CKTstat->STATtranIter += ckt->CKTstat->STATnumIter - startIters;
        ckt->CKTstat->STATtranDecompTime += ckt->CKTstat->STATdecompTime -
                startdTime;
        ckt->CKTstat->STATtranSolveTime += ckt->CKTstat->STATsolveTime -
                startsTime;
        return(E_PAUSE);
    }
resume:
#ifdef STEPDEBUG
    if( (ckt->CKTdelta <= ckt->CKTfinalTime/50) && 
            (ckt->CKTdelta <= ckt->CKTmaxStep)) {
            ;
    } else {
        if(ckt->CKTfinalTime/50<ckt->CKTmaxStep) {
            (void)printf("limited by Tstop/50\n");
        } else {
            (void)printf("limited by Tmax\n");
        }
    }
#endif
    ckt->CKTdelta = 
            MIN(ckt->CKTdelta,ckt->CKTmaxStep);
	/* are we at a breakpoint, or indistinguishably close? */
    if ((ckt->CKTtime == *(ckt->CKTbreaks)) || (*(ckt->CKTbreaks) -
    				(ckt->CKTtime) <= ckt->CKTdelmin)) {
        /* first timepoint after a breakpoint - cut integration order */
        /* and limit timestep to .1 times minimum of time to next breakpoint,
         *  and previous timestep
         */
        ckt->CKTorder = 1;
#ifdef STEPDEBUG
        if( (ckt->CKTdelta >.1* ckt->CKTsaveDelta) ||
                (ckt->CKTdelta > .1*(*(ckt->CKTbreaks+1)-*(ckt->CKTbreaks))) ) {
            if(ckt->CKTsaveDelta < (*(ckt->CKTbreaks+1)-*(ckt->CKTbreaks)))  {
                (void)printf("limited by pre-breakpoint delta\n");
            } else {
                (void)printf("limited by next breakpoint\n");
            }
        }
#endif
#ifdef HAS_SHORTMACRO
            mt= MIN(ckt->CKTsaveDelta, *(ckt->CKTbreaks+1)-*(ckt->CKTbreaks));
            ckt->CKTdelta = MIN(ckt->CKTdelta, .1 * mt);
#else
            ckt->CKTdelta = MIN(ckt->CKTdelta, .1 * MIN(ckt->CKTsaveDelta,
                    *(ckt->CKTbreaks+1)-*(ckt->CKTbreaks)));
#endif
        if(firsttime) {
            ckt->CKTdelta /= 10;
#ifdef STEPDEBUG
            (void)printf("delta cut for initial timepoint\n");
#endif
        } 

	/* don't want to get below delmin for no reason */
	ckt->CKTdelta = MAX(ckt->CKTdelta, ckt->CKTdelmin*2.0);
    }
    else if(ckt->CKTtime + ckt->CKTdelta >= *(ckt->CKTbreaks)) {
        ckt->CKTsaveDelta = ckt->CKTdelta;
        ckt->CKTdelta = *(ckt->CKTbreaks) - ckt->CKTtime;
#ifdef STEPDEBUG
        (void)printf("delta cut to hit breakpoint\n");
#endif
        ckt->CKTbreak = 1; /* why? the current pt. is not a bkpt. */
    }
    for(i=5;i>=0;i--) {
        ckt->CKTdeltaOld[i+1]=ckt->CKTdeltaOld[i];
    }
    ckt->CKTdeltaOld[0]=ckt->CKTdelta;

    temp = ckt->CKTstates[ckt->CKTmaxOrder+1];
    for(i=ckt->CKTmaxOrder;i>=0;i--) {
        ckt->CKTstates[i+1] = ckt->CKTstates[i];
    }
    ckt->CKTstates[0] = temp;

/* 600 */
    while (1) {
        olddelta=ckt->CKTdelta;
        /* time abort? */
        ckt->CKTtime += ckt->CKTdelta;
        ckt->CKTdeltaOld[0]=ckt->CKTdelta;
        NIcomCof(ckt);
#ifdef PREDICTOR
        error = NIpred(ckt);
#endif /* PREDICTOR */
        save_mode = ckt->CKTmode;
        save_order = ckt->CKTorder;
        converged = NIiter(ckt,ckt->CKTtranMaxIter);
        ckt->CKTstat->STATtimePts ++;
        ckt->CKTmode = (ckt->CKTmode&MODEUIC)|MODETRAN | MODEINITPRED;
        if(firsttime) {
            for(i=0;i<ckt->CKTnumStates;i++) {
                *(ckt->CKTstate2+i) = *(ckt->CKTstate1+i);
                *(ckt->CKTstate3+i) = *(ckt->CKTstate1+i);
            }
        }
        if(converged != 0) {
            ckt->CKTtime = ckt->CKTtime -ckt->CKTdelta;
            ckt->CKTstat->STATrejected ++;
            ckt->CKTdelta = ckt->CKTdelta/8;
#ifdef STEPDEBUG
            (void)printf("delta cut for non-convergence\n");
#endif
            if(firsttime) {
                ckt->CKTmode = (ckt->CKTmode&MODEUIC)|MODETRAN | MODEINITTRAN;
            }
            ckt->CKTorder = 1;
        } else {
            if (firsttime) {
#ifdef HAS_SENSE2
                if(ckt->CKTsenInfo && (ckt->CKTsenInfo->SENmode & TRANSEN)){
                    save1 = ckt->CKTmode;
                    save2 = ckt->CKTorder;
                    ckt->CKTmode = save_mode;
                    ckt->CKTorder = save_order;
                    if(error = CKTsenDCtran(ckt)) return(error);
                    ckt->CKTmode = save1;
                    ckt->CKTorder = save2;
                }
#endif
                firsttime =0;
                goto nextTime;  /* no check on
                                 * first time point
                                 */
            }
            new = ckt->CKTdelta;
            error = CKTtrunc(ckt,&new);
            if(error) {
                ckt->CKTcurrentAnalysis = DOING_TRAN;
                ckt->CKTstat->STATtranTime += 
                        (*(SPfrontEnd->IFseconds))()-startTime;
                ckt->CKTstat->STATtranIter += 
                        ckt->CKTstat->STATnumIter - startIters;
                ckt->CKTstat->STATtranDecompTime += ckt->CKTstat->STATdecompTime
                        - startdTime;
                ckt->CKTstat->STATtranSolveTime += ckt->CKTstat->STATsolveTime
                        - startsTime;
                return(error);
            }
            if(new>.9 * ckt->CKTdelta) {
                if(ckt->CKTorder == 1) {
                    new = ckt->CKTdelta;
                    ckt->CKTorder = 2;
                    error = CKTtrunc(ckt,&new);
                    if(error) {
                        ckt->CKTcurrentAnalysis = DOING_TRAN;
                        ckt->CKTstat->STATtranTime += 
                                (*(SPfrontEnd->IFseconds))()-startTime;
                        ckt->CKTstat->STATtranIter += 
                                ckt->CKTstat->STATnumIter - startIters;
                        ckt->CKTstat->STATtranDecompTime += 
                                ckt->CKTstat->STATdecompTime - startdTime;
                        ckt->CKTstat->STATtranSolveTime += 
                                ckt->CKTstat->STATsolveTime - startsTime;
                        return(error);
                    }
                    if(new <= 1.05 * ckt->CKTdelta) {
                        ckt->CKTorder = 1;
                    }
                }
                /* time point OK  - 630*/
                ckt->CKTdelta = new;
#ifdef STEPDEBUG
                (void)printf(
                    "delta set to truncation error result:point accepted\n");
#endif
#ifdef HAS_SENSE2
                if(ckt->CKTsenInfo && (ckt->CKTsenInfo->SENmode & TRANSEN)){
                    save1 = ckt->CKTmode;
                    save2 = ckt->CKTorder;
                    ckt->CKTmode = save_mode;
                    ckt->CKTorder = save_order;
                    if(error = CKTsenDCtran(ckt)) return(error);
                    ckt->CKTmode = save1;
                    ckt->CKTorder = save2;
                }
#endif
                /* go to 650 - trapezoidal */
                goto nextTime;
            } else {
                ckt->CKTtime = ckt->CKTtime -ckt->CKTdelta;
                ckt->CKTstat->STATrejected ++;
                ckt->CKTdelta = new;
#ifdef STEPDEBUG
                (void)printf(
                    "delta set to truncation error result:point rejected\n");
#endif
            }
        }
        if (ckt->CKTdelta <= ckt->CKTdelmin) {
            if (olddelta > ckt->CKTdelmin) {
                ckt->CKTdelta = ckt->CKTdelmin;
#ifdef STEPDEBUG
                (void)printf("delta at delmin\n");
#endif
            } else {
                ckt->CKTcurrentAnalysis = DOING_TRAN;
                ckt->CKTstat->STATtranTime += 
                        (*(SPfrontEnd->IFseconds))()-startTime;
                ckt->CKTstat->STATtranIter += 
                        ckt->CKTstat->STATnumIter - startIters;
                ckt->CKTstat->STATtranDecompTime += 
                        ckt->CKTstat->STATdecompTime - startdTime;
                ckt->CKTstat->STATtranSolveTime += 
                        ckt->CKTstat->STATsolveTime - startsTime;
		errMsg = CKTtrouble((GENERIC *) ckt, "Timestep too small");
                return(E_TIMESTEP);
            }
        }
    }
    /* NOTREACHED */
}
