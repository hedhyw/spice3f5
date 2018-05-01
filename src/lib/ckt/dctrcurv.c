/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include "misc.h"
#include <stdio.h>
#ifdef HAS_FLAT_INCLUDES
#include "vsrcdefs.h"
#include "isrcdefs.h"
#else
#include "vsrc/vsrcdefs.h"
#include "isrc/isrcdefs.h"
#endif
#include "cktdefs.h"
#include "util.h"
#include "sperror.h"
#include "suffix.h"

int
DCtrCurv(ckt,restart) 
CKTcircuit *ckt;
int restart;    /* forced restart flag */
{
    register TRCV* cv = (TRCV*)ckt->CKTcurJob;
    int i;
    double *temp;
    int converged;
    int vcode;
    int icode;
    int j;
    int error;
    long save;
    IFuid varUid;
    IFuid *nameList;
    int numNames;
    int firstTime=1;
    static GENERIC *plot;

#ifdef HAS_SENSE2
#ifdef SENSDEBUG
    if(ckt->CKTsenInfo && (ckt->CKTsenInfo->SENmode&DCSEN) ){
        printf("\nDC Sensitivity Results\n\n");
        CKTsenPrint(ckt);
    }
#endif /* SENSDEBUG */
#endif



    vcode = CKTtypelook("Vsource");
    icode = CKTtypelook("Isource");
    if(!restart && cv->TRCVnestState >= 0) {
        /* continuing */
        i = cv->TRCVnestState;
        goto resume;
    }
    ckt->CKTtime = 0;
    ckt->CKTdelta = cv->TRCVvStep[0];
    ckt->CKTmode = (ckt->CKTmode & MODEUIC) | MODEDCTRANCURVE | MODEINITJCT ;
    ckt->CKTorder=1;

    for(i=0;i<7;i++) {
        ckt->CKTdeltaOld[i]=ckt->CKTdelta;
    }
    for(i=0;i<=cv->TRCVnestLevel;i++) {
        if(vcode >= 0) {
            /* voltage sources are in this version, so use them */
            register VSRCinstance *here;
            register VSRCmodel *model;
            for(model = (VSRCmodel *)ckt->CKThead[vcode];model != NULL;
                    model=model->VSRCnextModel){
                for(here=model->VSRCinstances;here!=NULL;
                        here=here->VSRCnextInstance) {
                    if(here->VSRCname == cv->TRCVvName[i]) {
                        cv->TRCVvElt[i] = (GENinstance *)here;
                        cv->TRCVvSave[i]=here->VSRCdcValue;
			cv->TRCVgSave[i] = here->VSRCdcGiven;
                        cv->TRCVvType[i]=vcode;
                        here->VSRCdcValue = cv->TRCVvStart[i];
                        here->VSRCdcGiven = 1;
                        goto found;
                    }
                }
            }
        }
        if(icode >= 0 ) {
            /* current sources are in this version, so use them */
            register ISRCinstance *here;
            register ISRCmodel *model;

            for(model= (ISRCmodel *)ckt->CKThead[icode];model != NULL;
                    model=model->ISRCnextModel){
                for(here=model->ISRCinstances;here!=NULL;
                        here=here->ISRCnextInstance) {
                    if(here->ISRCname == cv->TRCVvName[i]) {
                        cv->TRCVvElt[i]= (GENinstance *)here;
                        cv->TRCVvSave[i]=here->ISRCdcValue;
			cv->TRCVgSave[i] = here->ISRCdcGiven;
                        cv->TRCVvType[i]=icode;
                        here->ISRCdcValue = cv->TRCVvStart[i];
                        here->ISRCdcGiven = 1;
                        goto found;
                    }
                }
            }
        }
        (*(SPfrontEnd->IFerror))(ERR_FATAL, 
                "DCtrCurv: source %s not in circuit", &(cv->TRCVvName[i]));
        return(E_NODEV);
found:;
    }
    error = CKTnames(ckt,&numNames,&nameList);
    if(error) return(error);
    (*(SPfrontEnd->IFnewUid))((GENERIC *)ckt,&varUid,(IFuid )NULL,
            "sweep", UID_OTHER, (GENERIC **)NULL);
    error = (*(SPfrontEnd->OUTpBeginPlot))((GENERIC *)ckt,
	(GENERIC*)ckt->CKTcurJob, ckt->CKTcurJob->JOBname,
	varUid,IF_REAL,numNames,nameList, IF_REAL,&plot);
    if(error) return(error);
    /* now have finished the initialization - can start doing hard part */
    i = 0;
resume:
    for(;;) {

        if(cv->TRCVvType[i]==vcode) { /* voltage source */
            if((((VSRCinstance*)(cv->TRCVvElt[i]))->VSRCdcValue)*
                    SIGN(1.,cv->TRCVvStep[i]) > 
                    SIGN(1.,cv->TRCVvStep[i]) *
                    cv->TRCVvStop[i])
                { 
                    i++ ; 
                    firstTime=1;
                    ckt->CKTmode = (ckt->CKTmode & MODEUIC) | 
                            MODEDCTRANCURVE | MODEINITJCT ;
                    if (i > cv->TRCVnestLevel ) break ; 
                    goto nextstep;
                }
        } else if(cv->TRCVvType[i]==icode) { /* current source */
            if((((ISRCinstance*)(cv->TRCVvElt[i]))->ISRCdcValue)*
                    SIGN(1.,cv->TRCVvStep[i]) >
                    SIGN(1.,cv->TRCVvStep[i]) *
                    cv->TRCVvStop[i])
                { 
                    i++ ; 
                    firstTime=1;
                    ckt->CKTmode = (ckt->CKTmode & MODEUIC) | 
                            MODEDCTRANCURVE | MODEINITJCT ;
                    if (i > cv->TRCVnestLevel ) break ; 
                    goto nextstep;
                }
        } /* else  not possible */
        while (i > 0) { 
            /* init(i); */
            i--; 
            if(cv->TRCVvType[i]==vcode) { /* voltage source */
                ((VSRCinstance *)(cv->TRCVvElt[i]))->VSRCdcValue =
                        cv->TRCVvStart[i];
            } else if(cv->TRCVvType[i]==icode) { /* current source */
                ((ISRCinstance *)(cv->TRCVvElt[i]))->ISRCdcValue =
                        cv->TRCVvStart[i];
            } /* else not possible */
        }

        temp = ckt->CKTstates[ckt->CKTmaxOrder+1];
        for(j=ckt->CKTmaxOrder;j>=0;j--) {
            ckt->CKTstates[j+1] = ckt->CKTstates[j];
        }
        ckt->CKTstate0 = temp;

        /* do operation */
        converged = NIiter(ckt,ckt->CKTdcTrcvMaxIter);
        if(converged != 0) {
            converged = CKTop(ckt,
                (ckt->CKTmode&MODEUIC)|MODEDCTRANCURVE | MODEINITJCT,
                (ckt->CKTmode&MODEUIC)|MODEDCTRANCURVE | MODEINITFLOAT,
                ckt->CKTdcMaxIter);
            if(converged != 0) {
                return(converged);
            }
        }
        ckt->CKTmode = (ckt->CKTmode&MODEUIC) | MODEDCTRANCURVE | MODEINITPRED ;
        if(cv->TRCVvType[0] == vcode) {
            ckt->CKTtime = ((VSRCinstance *)(cv->TRCVvElt[i]))
                    ->VSRCdcValue ;
        } else if(cv->TRCVvType[0] == icode) {
            ckt->CKTtime = ((ISRCinstance *)(cv->TRCVvElt[i]))
                    ->ISRCdcValue ;
        }

#ifdef HAS_SENSE2
/*
        if(!ckt->CKTsenInfo) printf("sensitivity structure does not exist\n");
    */
        if(ckt->CKTsenInfo && (ckt->CKTsenInfo->SENmode&DCSEN) ){
	    int senmode;

#ifdef SENSDEBUG
            if(cv->TRCVvType[i]==vcode) { /* voltage source */
                printf("Voltage Source Value : %.5e V\n",
                        ((VSRCinstance*) (cv->TRCVvElt[i]))->VSRCdcValue);
            }
            if(cv->TRCVvType[i]==icode) { /* current source */
                printf("Current Source Value : %.5e A\n",
                        ((ISRCinstance*)(cv->TRCVvElt[i]))->ISRCdcValue);
            }
#endif /* SENSDEBUG */

            senmode = ckt->CKTsenInfo->SENmode;
            save = ckt->CKTmode;
            ckt->CKTsenInfo->SENmode = DCSEN;
            if(error = CKTsenDCtran(ckt)) return (error);
            ckt->CKTmode = save;
            ckt->CKTsenInfo->SENmode = senmode;

        }
#endif


        CKTdump(ckt,ckt->CKTtime,plot);
        if(firstTime) {
            firstTime=0;
            bcopy((char *)ckt->CKTstate0,(char *)ckt->CKTstate1,
                    ckt->CKTnumStates*sizeof(double));
        }

nextstep:;
        if(cv->TRCVvType[i]==vcode) { /* voltage source */
            ((VSRCinstance*)(cv->TRCVvElt[i]))->VSRCdcValue +=
                    cv->TRCVvStep[i];
        } else if(cv->TRCVvType[i]==icode) { /* current source */
            ((ISRCinstance*)(cv->TRCVvElt[i]))->ISRCdcValue +=
                    cv->TRCVvStep[i];
        } /* else not possible */
        if( (*(SPfrontEnd->IFpauseTest))() ) {
            /* user asked us to pause, so save state */
            cv->TRCVnestState = i;
            return(E_PAUSE);
        }
    }

    /* all done, lets put everything back */

    for(i=0;i<=cv->TRCVnestLevel;i++) {
        if(cv->TRCVvType[i] == vcode) {   /* voltage source */
            ((VSRCinstance*)(cv->TRCVvElt[i]))->VSRCdcValue = 
                    cv->TRCVvSave[i];
            ((VSRCinstance*)(cv->TRCVvElt[i]))->VSRCdcGiven = cv->TRCVgSave[i];
        } else /* if(cv->TRCVvType[i] == icode) current source */ {
            ((ISRCinstance*)(cv->TRCVvElt[i]))->ISRCdcValue = 
                    cv->TRCVvSave[i];
            ((ISRCinstance*)(cv->TRCVvElt[i]))->ISRCdcGiven = cv->TRCVgSave[i];
        } /* else not possible */
    }
    (*(SPfrontEnd->OUTendPlot))(plot);

    return(OK);
}
