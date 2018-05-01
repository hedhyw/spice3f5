/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "devdefs.h"
#include "cktdefs.h"
#include "diodefs.h"
#include "const.h"
#include "trandefs.h"
#include "sperror.h"
#include "suffix.h"

int
DIOload(inModel,ckt)
    GENmodel *inModel;
    CKTcircuit *ckt;
        /* actually load the current resistance value into the 
         * sparse matrix previously provided 
         */
{
    register DIOmodel *model = (DIOmodel*)inModel;
    register DIOinstance *here;
    double arg;
    double capd;
    double cd;
    double cdeq;
    double cdhat;
    double ceq;
    double csat;    /* area-scaled saturation current */
    double czero;
    double czof2;
    double delvd;   /* change in diode voltage temporary */
    double evd;
    double evrev;
    double gd;
    double geq;
    double gspr;    /* area-scaled conductance */
    double sarg;
    double tol;     /* temporary for tolerence calculations */
    double vd;      /* current diode voltage */
    double vdtemp;
    double vt;      /* K t / Q */
    double vte;
    int Check;
    int error;
    int SenCond=0;    /* sensitivity condition */

    /*  loop through all the diode models */
    for( ; model != NULL; model = model->DIOnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->DIOinstances; here != NULL ;
                here=here->DIOnextInstance) {
            /*
             *     this routine loads diodes for dc and transient analyses.
             */

                
            if(ckt->CKTsenInfo){
                if((ckt->CKTsenInfo->SENstatus == PERTURBATION)
                        && (here->DIOsenPertFlag == OFF))continue;
                SenCond = here->DIOsenPertFlag;

#ifdef SENSDEBUG
                printf("DIOload \n");
#endif /* SENSDEBUG */

            }

            csat=here->DIOtSatCur*here->DIOarea;
            gspr=model->DIOconductance*here->DIOarea;
            vt = CONSTKoverQ * here->DIOtemp;
            vte=model->DIOemissionCoeff * vt;
            /*  
             *   initialization 
             */

            if(SenCond){

#ifdef SENSDEBUG
                printf("DIOsenPertFlag = ON \n");
#endif /* SENSDEBUG */

                if((ckt->CKTsenInfo->SENmode == TRANSEN)&&
                        (ckt->CKTmode & MODEINITTRAN)) {
                    vd = *(ckt->CKTstate1 + here->DIOvoltage);
                } else{
                    vd = *(ckt->CKTstate0 + here->DIOvoltage);
                }

#ifdef SENSDEBUG
                printf("vd = %.7e \n",vd);
#endif /* SENSDEBUG */
                goto next1;
            }

            Check=1;
            if(ckt->CKTmode & MODEINITSMSIG) {
                vd= *(ckt->CKTstate0 + here->DIOvoltage);
            } else if (ckt->CKTmode & MODEINITTRAN) {
                vd= *(ckt->CKTstate1 + here->DIOvoltage);
            } else if ( (ckt->CKTmode & MODEINITJCT) && 
                    (ckt->CKTmode & MODETRANOP) && (ckt->CKTmode & MODEUIC) ) {
                vd=here->DIOinitCond;
            } else if ( (ckt->CKTmode & MODEINITJCT) && here->DIOoff) {
                vd=0;
            } else if ( ckt->CKTmode & MODEINITJCT) {
                vd=here->DIOtVcrit;
            } else if ( ckt->CKTmode & MODEINITFIX && here->DIOoff) {
                vd=0;
            } else {
#ifndef PREDICTOR
                if (ckt->CKTmode & MODEINITPRED) {
                    *(ckt->CKTstate0 + here->DIOvoltage) = 
                            *(ckt->CKTstate1 + here->DIOvoltage);
                    vd = DEVpred(ckt,here->DIOvoltage);
                    *(ckt->CKTstate0 + here->DIOcurrent) = 
                            *(ckt->CKTstate1 + here->DIOcurrent);
                    *(ckt->CKTstate0 + here->DIOconduct) = 
                            *(ckt->CKTstate1 + here->DIOconduct);
                } else {
#endif /* PREDICTOR */
                    vd = *(ckt->CKTrhsOld+here->DIOposPrimeNode)-
                            *(ckt->CKTrhsOld + here->DIOnegNode);
#ifndef PREDICTOR
                }
#endif /* PREDICTOR */
                delvd=vd- *(ckt->CKTstate0 + here->DIOvoltage);
                cdhat= *(ckt->CKTstate0 + here->DIOcurrent) + 
                        *(ckt->CKTstate0 + here->DIOconduct) * delvd;
                /*  
                 *   bypass if solution has not changed
                 */
#ifndef NOBYPASS
                if ((!(ckt->CKTmode & MODEINITPRED)) && (ckt->CKTbypass)) {
                    tol=ckt->CKTvoltTol + ckt->CKTreltol*
                        MAX(FABS(vd),FABS(*(ckt->CKTstate0 +here->DIOvoltage)));
                    if (FABS(delvd) < tol){
                        tol=ckt->CKTreltol* MAX(FABS(cdhat),
                                FABS(*(ckt->CKTstate0 + here->DIOcurrent)))+
                                ckt->CKTabstol;
                        if (FABS(cdhat- *(ckt->CKTstate0 + here->DIOcurrent))
                                < tol) {
                            vd= *(ckt->CKTstate0 + here->DIOvoltage);
                            cd= *(ckt->CKTstate0 + here->DIOcurrent);
                            gd= *(ckt->CKTstate0 + here->DIOconduct);
                            goto load;
                        }
                    }
                }
#endif /* NOBYPASS */
                /*
                 *   limit new junction voltage
                 */
                if ( (model->DIObreakdownVoltageGiven) && 
                        (vd < MIN(0,-here->DIOtBrkdwnV+10*vte))) {
                    vdtemp = -(vd+here->DIOtBrkdwnV);
                    vdtemp = DEVpnjlim(vdtemp,
                            -(*(ckt->CKTstate0 + here->DIOvoltage) +
                            here->DIOtBrkdwnV),vte,
                            here->DIOtVcrit,&Check);
                    vd = -(vdtemp+here->DIOtBrkdwnV);
                } else {
                    vd = DEVpnjlim(vd,*(ckt->CKTstate0 + here->DIOvoltage),
                            vte,here->DIOtVcrit,&Check);
                }
            }
            /*
             *   compute dc current and derivitives
             */
next1:      if (vd >= -3*vte) {
                evd = exp(vd/vte);
                cd = csat*(evd-1)+ckt->CKTgmin*vd;
                gd = csat*evd/vte+ckt->CKTgmin;
            } else if((!(here->DIOtBrkdwnV))|| 
                    vd >= -here->DIOtBrkdwnV) {
                arg=3*vte/(vd*CONSTe);
                arg = arg * arg * arg;
                cd = -csat*(1+arg)+ckt->CKTgmin*vd;
                gd = csat*3*arg/vd+ckt->CKTgmin;
            } else {
                evrev=exp(-(here->DIOtBrkdwnV+vd)/vte);
                cd = -csat*evrev+ckt->CKTgmin*vd;
                gd=csat*evrev/vte + ckt->CKTgmin;
            }
            if( (ckt->CKTmode & (MODETRAN | MODEAC | MODEINITSMSIG)) || 
                    (ckt->CKTmode & MODETRANOP) && (ckt->CKTmode & MODEUIC) ) {
                /*
                 *   charge storage elements
                 */
                czero=here->DIOtJctCap*here->DIOarea;
                if (vd < here->DIOtDepCap){
                    arg=1-vd/model->DIOjunctionPot;
                    sarg=exp(-model->DIOgradingCoeff*log(arg));
                    *(ckt->CKTstate0 + here->DIOcapCharge) = 
                            model->DIOtransitTime*cd+model->DIOjunctionPot*
                            czero* (1-arg*sarg)/(1-model->DIOgradingCoeff);
                    capd=model->DIOtransitTime*gd+czero*sarg;
                } else {
                    czof2=czero/model->DIOf2;
                    *(ckt->CKTstate0 + here->DIOcapCharge) = 
                            model->DIOtransitTime*cd+czero*here->DIOtF1+czof2*
                            (model->DIOf3*(vd-here->DIOtDepCap)+
                            (model->DIOgradingCoeff/(model->DIOjunctionPot+
                            model->DIOjunctionPot))*(vd*vd-here->DIOtDepCap*
                            here->DIOtDepCap));
                    capd=model->DIOtransitTime*gd+czof2*(model->DIOf3+
                            model->DIOgradingCoeff*vd/model->DIOjunctionPot);
                }
	        here->DIOcap = capd;

                /*
                 *   store small-signal parameters
                 */
                if( (!(ckt->CKTmode & MODETRANOP)) || 
                        (!(ckt->CKTmode & MODEUIC)) ) {
                    if (ckt->CKTmode & MODEINITSMSIG){
                        *(ckt->CKTstate0 + here->DIOcapCurrent) = capd;

                        if(SenCond){
                            *(ckt->CKTstate0 + here->DIOcurrent) = cd;
                            *(ckt->CKTstate0 + here->DIOconduct) = gd;
#ifdef SENSDEBUG
                            printf("storing small signal parameters\n");
                            printf("cd = %.7e,vd = %.7e\n",cd,vd);
                            printf("capd = %.7e ,gd = %.7e \n",capd,gd);
#endif /* SENSDEBUG */
                        }
                        continue;
                    }

                    /*
                     *   transient analysis
                     */
                    if(SenCond && (ckt->CKTsenInfo->SENmode == TRANSEN)){
                        *(ckt->CKTstate0 + here->DIOcurrent) = cd;
#ifdef SENSDEBUG
                        printf("storing parameters for transient sensitivity\n"
                                );
                        printf("qd = %.7e, capd = %.7e,cd = %.7e\n",
                                *(ckt->CKTstate0 + here->DIOcapCharge),capd,cd);
#endif /* SENSDEBUG */
                        continue;
                    }

                    if (ckt->CKTmode & MODEINITTRAN) {
                        *(ckt->CKTstate1 + here->DIOcapCharge) = 
                                *(ckt->CKTstate0 + here->DIOcapCharge);
                    }
                    error = NIintegrate(ckt,&geq,&ceq,capd,here->DIOcapCharge);
                    if(error) return(error);
                    gd=gd+geq;
                    cd=cd+*(ckt->CKTstate0 + here->DIOcapCurrent);
                    if (ckt->CKTmode & MODEINITTRAN) {
                        *(ckt->CKTstate1 + here->DIOcapCurrent) = 
                                *(ckt->CKTstate0 + here->DIOcapCurrent);
                    }
                }
            }

            if(SenCond) goto next2;

            /*
             *   check convergence
             */
            if ( (!(ckt->CKTmode & MODEINITFIX)) || (!(here->DIOoff))  ) {
                if (Check == 1)  {
                    ckt->CKTnoncon++;
		    ckt->CKTtroubleElt = (GENinstance *) here;
#ifndef NEWCONV
                } else {
                    tol=ckt->CKTreltol*
                            MAX(FABS(cdhat),FABS(cd))+ckt->CKTabstol;
                    if (FABS(cdhat-cd) > tol) {
                        ckt->CKTnoncon++;
			ckt->CKTtroubleElt = (GENinstance *) here;
                    }
#endif /* NEWCONV */
                }
            }
next2:      *(ckt->CKTstate0 + here->DIOvoltage) = vd;
            *(ckt->CKTstate0 + here->DIOcurrent) = cd;
            *(ckt->CKTstate0 + here->DIOconduct) = gd;

            if(SenCond)  continue;


            load:

            /*
             *   load current vector
             */
            cdeq=cd-gd*vd;
            *(ckt->CKTrhs + here->DIOnegNode) += cdeq;
            *(ckt->CKTrhs + here->DIOposPrimeNode) -= cdeq;
            /*
             *   load matrix
             */
            *(here->DIOposPosPtr) += gspr;
            *(here->DIOnegNegPtr) += gd;
            *(here->DIOposPrimePosPrimePtr) += (gd + gspr);
            *(here->DIOposPosPrimePtr) -= gspr;
            *(here->DIOnegPosPrimePtr) -= gd;
            *(here->DIOposPrimePosPtr) -= gspr;
            *(here->DIOposPrimeNegPtr) -= gd;
        }
    }
    return(OK);
}
