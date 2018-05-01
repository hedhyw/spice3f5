/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/*
 * This is the function called each iteration to evaluate the
 * BJTs in the circuit and load them into the matrix as appropriate
 */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "cktdefs.h"
#include "bjtdefs.h"
#include "const.h"
#include "trandefs.h"
#include "sperror.h"
#include "devdefs.h"
#include "suffix.h"

int
BJTload(inModel,ckt)

    GENmodel *inModel;
    register CKTcircuit *ckt;
        /* actually load the current resistance value into the
         * sparse matrix previously provided
         */
{
    register BJTmodel *model = (BJTmodel*)inModel;
    register BJTinstance *here;
    double arg1;
    double arg2;
    double arg3;
    double arg;
    double argtf;
    double c2;
    double c4;
    double capbc;
    double capbe;
    double capbx=0;
    double capcs=0;
    double cb;
    double cbc;
    double cbcn;
    double cbe;
    double cben;
    double cbhat;
    double cc;
    double cchat;
    double cdis;
    double ceq;
    double ceqbc;
    double ceqbe;
    double ceqbx;
    double ceqcs;
    double cex;
    double csat;
    double ctot;
    double czbc;
    double czbcf2;
    double czbe;
    double czbef2;
    double czbx;
    double czbxf2;
    double czcs;
    double delvbc;
    double delvbe;
    double denom;
    double dqbdvc;
    double dqbdve;
    double evbc;
    double evbcn;
    double evbe;
    double evben;
    double f1;
    double f2;
    double f3;
    double fcpc;
    double fcpe;
    double gbc;
    double gbcn;
    double gbe;
    double gben;
    double gccs;
    double gcpr;
    double gepr;
    double geq;
    double geqbx;
    double geqcb;
    double gex;
    double gm;
    double gmu;
    double go;
    double gpi;
    double gx;
    double oik;
    double oikr;
    double ovtf;
    double pc;
    double pe;
    double ps;
    double q1;
    double q2;
    double qb;
    double rbpi;
    double rbpr;
    double sarg;
    double sqarg;
    double td;
    double temp;
    double tf;
    double tol;
    double tr;
    double vbc;
    double vbe;
    double vbx;
    double vce;
    double vcs;
    double vt;
    double vtc;
    double vte;
    double vtn;
    double xfact;
    double xjrb;
    double xjtf;
    double xmc;
    double xme;
    double xms;
    double xtf;
    int icheck;
    int ichk1;
    int error;
    int SenCond=0;
    /*  loop through all the models */
    for( ; model != NULL; model = model->BJTnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->BJTinstances; here != NULL ;
                here=here->BJTnextInstance) {

            vt = here->BJTtemp * CONSTKoverQ;

            if(ckt->CKTsenInfo){
#ifdef SENSDEBUG
                printf("BJTload \n");
#endif /* SENSDEBUG */

                if((ckt->CKTsenInfo->SENstatus == PERTURBATION)&&
                    (here->BJTsenPertFlag == OFF)) continue;
                SenCond = here->BJTsenPertFlag;
            }


            gccs=0;
            ceqcs=0;
            geqbx=0;
            ceqbx=0;
            geqcb=0;
            /*
             *   dc model paramters
             */
            csat=here->BJTtSatCur*here->BJTarea;
            rbpr=model->BJTminBaseResist/here->BJTarea;
            rbpi=model->BJTbaseResist/here->BJTarea-rbpr;
            gcpr=model->BJTcollectorConduct*here->BJTarea;
            gepr=model->BJTemitterConduct*here->BJTarea;
            oik=model->BJTinvRollOffF/here->BJTarea;
            c2=here->BJTtBEleakCur*here->BJTarea;
            vte=model->BJTleakBEemissionCoeff*vt;
            oikr=model->BJTinvRollOffR/here->BJTarea;
            c4=here->BJTtBCleakCur*here->BJTarea;
            vtc=model->BJTleakBCemissionCoeff*vt;
            td=model->BJTexcessPhaseFactor;
            xjrb=model->BJTbaseCurrentHalfResist*here->BJTarea;

            if(SenCond){
#ifdef SENSDEBUG
                printf("BJTsenPertFlag = ON \n");
#endif /* SENSDEBUG */

                if((ckt->CKTsenInfo->SENmode == TRANSEN)&&
                    (ckt->CKTmode & MODEINITTRAN)) {
                    vbe = *(ckt->CKTstate1 + here->BJTvbe);
                    vbc = *(ckt->CKTstate1 + here->BJTvbc);
                    vbx=model->BJTtype*(
                        *(ckt->CKTrhsOp+here->BJTbaseNode)-
                        *(ckt->CKTrhsOp+here->BJTcolPrimeNode));
                    vcs=model->BJTtype*(
                      *(ckt->CKTrhsOp+here->BJTsubstNode)-
                      *(ckt->CKTrhsOp+here->BJTcolPrimeNode));
                }
                else{
                    vbe = *(ckt->CKTstate0 + here->BJTvbe);
                    vbc = *(ckt->CKTstate0 + here->BJTvbc);
                    if((ckt->CKTsenInfo->SENmode == DCSEN)||
                        (ckt->CKTsenInfo->SENmode == TRANSEN)){
                        vbx=model->BJTtype*(
                            *(ckt->CKTrhsOld+here->BJTbaseNode)-
                            *(ckt->CKTrhsOld+here->BJTcolPrimeNode));
                        vcs=model->BJTtype*(
                            *(ckt->CKTrhsOld+here->BJTsubstNode)-
                            *(ckt->CKTrhsOld+here->BJTcolPrimeNode));
                    }
                    if(ckt->CKTsenInfo->SENmode == ACSEN){
                        vbx=model->BJTtype*(
                            *(ckt->CKTrhsOp+here->BJTbaseNode)-
                            *(ckt->CKTrhsOp+here->BJTcolPrimeNode));
                        vcs=model->BJTtype*(
                            *(ckt->CKTrhsOp+here->BJTsubstNode)-
                            *(ckt->CKTrhsOp+here->BJTcolPrimeNode));
                    }
                }
                goto next1;
            }

            /*
             *   initialization
             */
            icheck=1;
            if(ckt->CKTmode & MODEINITSMSIG) {
                vbe= *(ckt->CKTstate0 + here->BJTvbe);
                vbc= *(ckt->CKTstate0 + here->BJTvbc);
                vbx=model->BJTtype*(
                    *(ckt->CKTrhsOld+here->BJTbaseNode)-
                    *(ckt->CKTrhsOld+here->BJTcolPrimeNode));
                vcs=model->BJTtype*(
                    *(ckt->CKTrhsOld+here->BJTsubstNode)-
                    *(ckt->CKTrhsOld+here->BJTcolPrimeNode));
            } else if(ckt->CKTmode & MODEINITTRAN) {
                vbe = *(ckt->CKTstate1 + here->BJTvbe);
                vbc = *(ckt->CKTstate1 + here->BJTvbc);
                vbx=model->BJTtype*(
                    *(ckt->CKTrhsOld+here->BJTbaseNode)-
                    *(ckt->CKTrhsOld+here->BJTcolPrimeNode));
                vcs=model->BJTtype*(
                    *(ckt->CKTrhsOld+here->BJTsubstNode)-
                    *(ckt->CKTrhsOld+here->BJTcolPrimeNode));
                if( (ckt->CKTmode & MODETRAN) && (ckt->CKTmode & MODEUIC) ) {
                    vbx=model->BJTtype*(here->BJTicVBE-here->BJTicVCE);
                    vcs=0;
                }
            } else if((ckt->CKTmode & MODEINITJCT) &&
                    (ckt->CKTmode & MODETRANOP) && (ckt->CKTmode & MODEUIC)){
                vbe=model->BJTtype*here->BJTicVBE;
                vce=model->BJTtype*here->BJTicVCE;
                vbc=vbe-vce;
                vbx=vbc;
                vcs=0;
            } else if((ckt->CKTmode & MODEINITJCT) && (here->BJToff==0)) {
                vbe=here->BJTtVcrit;
                vbc=0;
                /* ERROR:  need to initialize VCS, VBX here */
                vcs=vbx=0;
            } else if((ckt->CKTmode & MODEINITJCT) ||
                    ( (ckt->CKTmode & MODEINITFIX) && (here->BJToff!=0))) {
                vbe=0;
                vbc=0;
                /* ERROR:  need to initialize VCS, VBX here */
                vcs=vbx=0;
            } else {
#ifndef PREDICTOR
                if(ckt->CKTmode & MODEINITPRED) {
                    xfact = ckt->CKTdelta/ckt->CKTdeltaOld[1];
                    *(ckt->CKTstate0 + here->BJTvbe) =
                            *(ckt->CKTstate1 + here->BJTvbe);
                    vbe = (1+xfact)**(ckt->CKTstate1 + here->BJTvbe)-
                            xfact* *(ckt->CKTstate2 + here->BJTvbe);
                    *(ckt->CKTstate0 + here->BJTvbc) =
                            *(ckt->CKTstate1 + here->BJTvbc);
                    vbc = (1+xfact)**(ckt->CKTstate1 + here->BJTvbc)-
                            xfact* *(ckt->CKTstate2 + here->BJTvbc);
                    *(ckt->CKTstate0 + here->BJTcc) =
                            *(ckt->CKTstate1 + here->BJTcc);
                    *(ckt->CKTstate0 + here->BJTcb) =
                            *(ckt->CKTstate1 + here->BJTcb);
                    *(ckt->CKTstate0 + here->BJTgpi) =
                            *(ckt->CKTstate1 + here->BJTgpi);
                    *(ckt->CKTstate0 + here->BJTgmu) =
                            *(ckt->CKTstate1 + here->BJTgmu);
                    *(ckt->CKTstate0 + here->BJTgm) =
                            *(ckt->CKTstate1 + here->BJTgm);
                    *(ckt->CKTstate0 + here->BJTgo) =
                            *(ckt->CKTstate1 + here->BJTgo);
                    *(ckt->CKTstate0 + here->BJTgx) =
                            *(ckt->CKTstate1 + here->BJTgx);
                } else {
#endif /* PREDICTOR */
                    /*
                     *   compute new nonlinear branch voltages
                     */
                    vbe=model->BJTtype*(
                        *(ckt->CKTrhsOld+here->BJTbasePrimeNode)-
                        *(ckt->CKTrhsOld+here->BJTemitPrimeNode));
                    vbc=model->BJTtype*(
                        *(ckt->CKTrhsOld+here->BJTbasePrimeNode)-
                        *(ckt->CKTrhsOld+here->BJTcolPrimeNode));
#ifndef PREDICTOR
                }
#endif /* PREDICTOR */
                delvbe=vbe- *(ckt->CKTstate0 + here->BJTvbe);
                delvbc=vbc- *(ckt->CKTstate0 + here->BJTvbc);
                vbx=model->BJTtype*(
                    *(ckt->CKTrhsOld+here->BJTbaseNode)-
                    *(ckt->CKTrhsOld+here->BJTcolPrimeNode));
                vcs=model->BJTtype*(
                    *(ckt->CKTrhsOld+here->BJTsubstNode)-
                    *(ckt->CKTrhsOld+here->BJTcolPrimeNode));
                cchat= *(ckt->CKTstate0 + here->BJTcc)+(*(ckt->CKTstate0 +
                        here->BJTgm)+ *(ckt->CKTstate0 + here->BJTgo))*delvbe-
                        (*(ckt->CKTstate0 + here->BJTgo)+*(ckt->CKTstate0 +
                        here->BJTgmu))*delvbc;
                cbhat= *(ckt->CKTstate0 + here->BJTcb)+ *(ckt->CKTstate0 +
                        here->BJTgpi)*delvbe+ *(ckt->CKTstate0 + here->BJTgmu)*
                        delvbc;
#ifndef NOBYPASS
                /*
                 *    bypass if solution has not changed
                 */
                /* the following collections of if's would be just one
                 * if the average compiler could handle it, but many
                 * find the expression too complicated, thus the split.
                 */
                if( (ckt->CKTbypass) &&
                        (!(ckt->CKTmode & MODEINITPRED)) &&
                        (FABS(delvbe) < (ckt->CKTreltol*MAX(FABS(vbe),
                            FABS(*(ckt->CKTstate0 + here->BJTvbe)))+
                            ckt->CKTvoltTol)) )
                    if( (FABS(delvbc) < ckt->CKTreltol*MAX(FABS(vbc),
                            FABS(*(ckt->CKTstate0 + here->BJTvbc)))+
                            ckt->CKTvoltTol) )
                    if( (FABS(cchat-*(ckt->CKTstate0 + here->BJTcc)) <
                            ckt->CKTreltol* MAX(FABS(cchat),
                            FABS(*(ckt->CKTstate0 + here->BJTcc)))+
                            ckt->CKTabstol) )
                    if( (FABS(cbhat-*(ckt->CKTstate0 + here->BJTcb)) <
                            ckt->CKTreltol* MAX(FABS(cbhat),
                            FABS(*(ckt->CKTstate0 + here->BJTcb)))+
                            ckt->CKTabstol) ) {
                    /*
                     * bypassing....
                     */
                    vbe = *(ckt->CKTstate0 + here->BJTvbe);
                    vbc = *(ckt->CKTstate0 + here->BJTvbc);
                    cc = *(ckt->CKTstate0 + here->BJTcc);
                    cb = *(ckt->CKTstate0 + here->BJTcb);
                    gpi = *(ckt->CKTstate0 + here->BJTgpi);
                    gmu = *(ckt->CKTstate0 + here->BJTgmu);
                    gm = *(ckt->CKTstate0 + here->BJTgm);
                    go = *(ckt->CKTstate0 + here->BJTgo);
                    gx = *(ckt->CKTstate0 + here->BJTgx);
                    geqcb = *(ckt->CKTstate0 + here->BJTgeqcb);
                    gccs = *(ckt->CKTstate0 + here->BJTgccs);
                    geqbx = *(ckt->CKTstate0 + here->BJTgeqbx);
                    goto load;
                }
#endif /*NOBYPASS*/
                /*
                 *   limit nonlinear branch voltages
                 */
                ichk1=1;
                vbe = DEVpnjlim(vbe,*(ckt->CKTstate0 + here->BJTvbe),vt,
                        here->BJTtVcrit,&icheck);
                vbc = DEVpnjlim(vbc,*(ckt->CKTstate0 + here->BJTvbc),vt,
                        here->BJTtVcrit,&ichk1);
                if (ichk1 == 1) icheck=1;
            }
            /*
             *   determine dc current and derivitives
             */
next1:      vtn=vt*model->BJTemissionCoeffF;
            if(vbe > -5*vtn){
                evbe=exp(vbe/vtn);
                cbe=csat*(evbe-1)+ckt->CKTgmin*vbe;
                gbe=csat*evbe/vtn+ckt->CKTgmin;
                if (c2 == 0) {
                    cben=0;
                    gben=0;
                } else {
                    evben=exp(vbe/vte);
                    cben=c2*(evben-1);
                    gben=c2*evben/vte;
                }
            } else {
                gbe = -csat/vbe+ckt->CKTgmin;
                cbe=gbe*vbe;
                gben = -c2/vbe;
                cben=gben*vbe;
            }
            vtn=vt*model->BJTemissionCoeffR;
            if(vbc > -5*vtn) {
                evbc=exp(vbc/vtn);
                cbc=csat*(evbc-1)+ckt->CKTgmin*vbc;
                gbc=csat*evbc/vtn+ckt->CKTgmin;
                if (c4 == 0) {
                    cbcn=0;
                    gbcn=0;
                } else {
                    evbcn=exp(vbc/vtc);
                    cbcn=c4*(evbcn-1);
                    gbcn=c4*evbcn/vtc;
                }
            } else {
                gbc = -csat/vbc+ckt->CKTgmin;
                cbc = gbc*vbc;
                gbcn = -c4/vbc;
                cbcn=gbcn*vbc;
            }
            /*
             *   determine base charge terms
             */
            q1=1/(1-model->BJTinvEarlyVoltF*vbc-model->BJTinvEarlyVoltR*vbe);
            if(oik == 0 && oikr == 0) {
                qb=q1;
                dqbdve=q1*qb*model->BJTinvEarlyVoltR;
                dqbdvc=q1*qb*model->BJTinvEarlyVoltF;
            } else {
                q2=oik*cbe+oikr*cbc;
                arg=MAX(0,1+4*q2);
                sqarg=1;
                if(arg != 0) sqarg=sqrt(arg);
                qb=q1*(1+sqarg)/2;
                dqbdve=q1*(qb*model->BJTinvEarlyVoltR+oik*gbe/sqarg);
                dqbdvc=q1*(qb*model->BJTinvEarlyVoltF+oikr*gbc/sqarg);
            }
            /*
             *   weil's approx. for excess phase applied with backward-
             *   euler integration
             */
            cc=0;
            cex=cbe;
            gex=gbe;
            if(ckt->CKTmode & (MODETRAN | MODEAC) && td != 0) {
                arg1=ckt->CKTdelta/td;
                arg2=3*arg1;
                arg1=arg2*arg1;
                denom=1+arg1+arg2;
                arg3=arg1/denom;
                if(ckt->CKTmode & MODEINITTRAN) {
                    *(ckt->CKTstate1 + here->BJTcexbc)=cbe/qb;
                    *(ckt->CKTstate2 + here->BJTcexbc)=
                            *(ckt->CKTstate1 + here->BJTcexbc);
                }
                cc=(*(ckt->CKTstate1 + here->BJTcexbc)*(1+ckt->CKTdelta/
                        ckt->CKTdeltaOld[1]+arg2)-
                        *(ckt->CKTstate2 + here->BJTcexbc)*ckt->CKTdelta/
                        ckt->CKTdeltaOld[1])/denom;
                cex=cbe*arg3;
                gex=gbe*arg3;
                *(ckt->CKTstate0 + here->BJTcexbc)=cc+cex/qb;
            }
            /*
             *   determine dc incremental conductances
             */
            cc=cc+(cex-cbc)/qb-cbc/here->BJTtBetaR-cbcn;
            cb=cbe/here->BJTtBetaF+cben+cbc/here->BJTtBetaR+cbcn;
            gx=rbpr+rbpi/qb;
            if(xjrb != 0) {
                arg1=MAX(cb/xjrb,1e-9);
                arg2=(-1+sqrt(1+14.59025*arg1))/2.4317/sqrt(arg1);
                arg1=tan(arg2);
                gx=rbpr+3*rbpi*(arg1-arg2)/arg2/arg1/arg1;
            }
            if(gx != 0) gx=1/gx;
            gpi=gbe/here->BJTtBetaF+gben;
            gmu=gbc/here->BJTtBetaR+gbcn;
            go=(gbc+(cex-cbc)*dqbdvc/qb)/qb;
            gm=(gex-(cex-cbc)*dqbdve/qb)/qb-go;
            if( (ckt->CKTmode & (MODETRAN | MODEAC)) ||
                    ((ckt->CKTmode & MODETRANOP) && (ckt->CKTmode & MODEUIC)) ||
                    (ckt->CKTmode & MODEINITSMSIG)) {
                /*
                 *   charge storage elements
                 */
                tf=model->BJTtransitTimeF;
                tr=model->BJTtransitTimeR;
                czbe=here->BJTtBEcap*here->BJTarea;
                pe=here->BJTtBEpot;
                xme=model->BJTjunctionExpBE;
                cdis=model->BJTbaseFractionBCcap;
                ctot=here->BJTtBCcap*here->BJTarea;
                czbc=ctot*cdis;
                czbx=ctot-czbc;
                pc=here->BJTtBCpot;
                xmc=model->BJTjunctionExpBC;
                fcpe=here->BJTtDepCap;
                czcs=model->BJTcapCS*here->BJTarea;
                ps=model->BJTpotentialSubstrate;
                xms=model->BJTexponentialSubstrate;
                xtf=model->BJTtransitTimeBiasCoeffF;
                ovtf=model->BJTtransitTimeVBCFactor;
                xjtf=model->BJTtransitTimeHighCurrentF*here->BJTarea;
                if(tf != 0 && vbe >0) {
                    argtf=0;
                    arg2=0;
                    arg3=0;
                    if(xtf != 0){
                        argtf=xtf;
                        if(ovtf != 0) {
                            argtf=argtf*exp(vbc*ovtf);
                        }
                        arg2=argtf;
                        if(xjtf != 0) {
                            temp=cbe/(cbe+xjtf);
                            argtf=argtf*temp*temp;
                            arg2=argtf*(3-temp-temp);
                        }
                        arg3=cbe*argtf*ovtf;
                    }
                    cbe=cbe*(1+argtf)/qb;
                    gbe=(gbe*(1+arg2)-cbe*dqbdve)/qb;
                    geqcb=tf*(arg3-cbe*dqbdvc)/qb;
                }
                if (vbe < fcpe) {
                    arg=1-vbe/pe;
                    sarg=exp(-xme*log(arg));
                    *(ckt->CKTstate0 + here->BJTqbe)=tf*cbe+pe*czbe*
                            (1-arg*sarg)/(1-xme);
                    capbe=tf*gbe+czbe*sarg;
                } else {
                    f1=here->BJTtf1;
                    f2=model->BJTf2;
                    f3=model->BJTf3;
                    czbef2=czbe/f2;
                    *(ckt->CKTstate0 + here->BJTqbe) = tf*cbe+czbe*f1+czbef2*
                            (f3*(vbe-fcpe) +(xme/(pe+pe))*(vbe*vbe-fcpe*fcpe));
                    capbe=tf*gbe+czbef2*(f3+xme*vbe/pe);
                }
                fcpc=here->BJTtf4;
                f1=here->BJTtf5;
                f2=model->BJTf6;
                f3=model->BJTf7;
                if (vbc < fcpc) {
                    arg=1-vbc/pc;
                    sarg=exp(-xmc*log(arg));
                    *(ckt->CKTstate0 + here->BJTqbc) = tr*cbc+pc*czbc*(
                            1-arg*sarg)/(1-xmc);
                    capbc=tr*gbc+czbc*sarg;
                } else {
                    czbcf2=czbc/f2;
                    *(ckt->CKTstate0 + here->BJTqbc) = tr*cbc+czbc*f1+czbcf2*
                            (f3*(vbc-fcpc) +(xmc/(pc+pc))*(vbc*vbc-fcpc*fcpc));
                    capbc=tr*gbc+czbcf2*(f3+xmc*vbc/pc);
                }
                if(vbx < fcpc) {
                    arg=1-vbx/pc;
                    sarg=exp(-xmc*log(arg));
                    *(ckt->CKTstate0 + here->BJTqbx)=
                        pc*czbx* (1-arg*sarg)/(1-xmc);
                    capbx=czbx*sarg;
                } else {
                    czbxf2=czbx/f2;
                    *(ckt->CKTstate0 + here->BJTqbx)=czbx*f1+czbxf2*
                            (f3*(vbx-fcpc)+(xmc/(pc+pc))*(vbx*vbx-fcpc*fcpc));
                    capbx=czbxf2*(f3+xmc*vbx/pc);
                }
                if(vcs < 0){
                    arg=1-vcs/ps;
                    sarg=exp(-xms*log(arg));
                    *(ckt->CKTstate0 + here->BJTqcs) = ps*czcs*(1-arg*sarg)/
                            (1-xms);
                    capcs=czcs*sarg;
                } else {
                    *(ckt->CKTstate0 + here->BJTqcs) = vcs*czcs*(1+xms*vcs/
                            (2*ps));
                    capcs=czcs*(1+xms*vcs/ps);
		}
		here->BJTcapbe = capbe;
		here->BJTcapbc = capbc;
		here->BJTcapcs = capcs;
		here->BJTcapbx = capbx;

                /*
                 *   store small-signal parameters
                 */
                if ( (!(ckt->CKTmode & MODETRANOP))||
                        (!(ckt->CKTmode & MODEUIC)) ) {
                    if(ckt->CKTmode & MODEINITSMSIG) {
                        *(ckt->CKTstate0 + here->BJTcqbe) = capbe;
                        *(ckt->CKTstate0 + here->BJTcqbc) = capbc;
                        *(ckt->CKTstate0 + here->BJTcqcs) = capcs;
                        *(ckt->CKTstate0 + here->BJTcqbx) = capbx;
                        *(ckt->CKTstate0 + here->BJTcexbc) = geqcb;
                        if(SenCond){
                            *(ckt->CKTstate0 + here->BJTcc) = cc;
                            *(ckt->CKTstate0 + here->BJTcb) = cb;
                            *(ckt->CKTstate0 + here->BJTgpi) = gpi;
                            *(ckt->CKTstate0 + here->BJTgmu) = gmu;
                            *(ckt->CKTstate0 + here->BJTgm) = gm;
                            *(ckt->CKTstate0 + here->BJTgo) = go;
                            *(ckt->CKTstate0 + here->BJTgx) = gx;
                            *(ckt->CKTstate0 + here->BJTgccs) = gccs;
                            *(ckt->CKTstate0 + here->BJTgeqbx) = geqbx;
                        }
#ifdef SENSDEBUG
                        printf("storing small signal parameters for op\n");
                        printf("capbe = %.7e ,capbc = %.7e\n",capbe,capbc);
                        printf("capcs = %.7e ,capbx = %.7e\n",capcs,capbx);
                        printf("geqcb = %.7e ,gpi = %.7e\n",geqcb,gpi);
                        printf("gmu = %.7e ,gm = %.7e\n",gmu,gm);
                        printf("go = %.7e ,gx = %.7e\n",go,gx);
                        printf("gccs = %.7e ,geqbx = %.7e\n",gccs,geqbx);
                        printf("cc = %.7e ,cb = %.7e\n",cc,cb);
#endif /* SENSDEBUG */
                        continue; /* go to 1000 */
                    }
                    /*
                     *   transient analysis
                     */
                    if(SenCond && ckt->CKTsenInfo->SENmode == TRANSEN){
                        *(ckt->CKTstate0 + here->BJTcc) = cc;
                        *(ckt->CKTstate0 + here->BJTcb) = cb;
                        *(ckt->CKTstate0 + here->BJTgx) = gx;
                        continue;
                    }

                    if(ckt->CKTmode & MODEINITTRAN) {
                        *(ckt->CKTstate1 + here->BJTqbe) =
                                *(ckt->CKTstate0 + here->BJTqbe) ;
                        *(ckt->CKTstate1 + here->BJTqbc) =
                                *(ckt->CKTstate0 + here->BJTqbc) ;
                        *(ckt->CKTstate1 + here->BJTqbx) =
                                *(ckt->CKTstate0 + here->BJTqbx) ;
                        *(ckt->CKTstate1 + here->BJTqcs) =
                                *(ckt->CKTstate0 + here->BJTqcs) ;
                    }
                    error = NIintegrate(ckt,&geq,&ceq,capbe,here->BJTqbe);
                    if(error) return(error);
                    geqcb=geqcb*ckt->CKTag[0];
                    gpi=gpi+geq;
                    cb=cb+*(ckt->CKTstate0 + here->BJTcqbe);
                    error = NIintegrate(ckt,&geq,&ceq,capbc,here->BJTqbc);
                    if(error) return(error);
                    gmu=gmu+geq;
                    cb=cb+*(ckt->CKTstate0 + here->BJTcqbc);
                    cc=cc-*(ckt->CKTstate0 + here->BJTcqbc);
                    if(ckt->CKTmode & MODEINITTRAN) {
                        *(ckt->CKTstate1 + here->BJTcqbe) =
                                *(ckt->CKTstate0 + here->BJTcqbe);
                        *(ckt->CKTstate1 + here->BJTcqbc) =
                                *(ckt->CKTstate0 + here->BJTcqbc);
                    }
                }
            }

            if(SenCond) goto next2;

            /*
             *   check convergence
             */
            if ( (!(ckt->CKTmode & MODEINITFIX))||(!(here->BJToff))) {
                if (icheck == 1) {
                    ckt->CKTnoncon++;
		    ckt->CKTtroubleElt = (GENinstance *) here;
#ifndef NEWCONV
                } else {
                    tol=ckt->CKTreltol*MAX(FABS(cchat),FABS(cc))+ckt->CKTabstol;
                    if (FABS(cchat-cc) > tol) {
                        ckt->CKTnoncon++;
			ckt->CKTtroubleElt = (GENinstance *) here;
                    } else {
                        tol=ckt->CKTreltol*MAX(FABS(cbhat),FABS(cb))+
                            ckt->CKTabstol;
                        if (FABS(cbhat-cb) > tol) {
                            ckt->CKTnoncon++;
			    ckt->CKTtroubleElt = (GENinstance *) here;
                        }
                    }
#endif /* NEWCONV */
                }
            }

            /*
             *      charge storage for c-s and b-x junctions
             */
            if(ckt->CKTmode & (MODETRAN | MODEAC)) {
                error = NIintegrate(ckt,&gccs,&ceq,capcs,here->BJTqcs);
                if(error) return(error);
                error = NIintegrate(ckt,&geqbx,&ceq,capbx,here->BJTqbx);
                if(error) return(error);
                if(ckt->CKTmode & MODEINITTRAN) {
                    *(ckt->CKTstate1 + here->BJTcqbx) =
                            *(ckt->CKTstate0 + here->BJTcqbx);
                    *(ckt->CKTstate1 + here->BJTcqcs) =
                            *(ckt->CKTstate0 + here->BJTcqcs);
                }
            }
next2:
            *(ckt->CKTstate0 + here->BJTvbe) = vbe;
            *(ckt->CKTstate0 + here->BJTvbc) = vbc;
            *(ckt->CKTstate0 + here->BJTcc) = cc;
            *(ckt->CKTstate0 + here->BJTcb) = cb;
            *(ckt->CKTstate0 + here->BJTgpi) = gpi;
            *(ckt->CKTstate0 + here->BJTgmu) = gmu;
            *(ckt->CKTstate0 + here->BJTgm) = gm;
            *(ckt->CKTstate0 + here->BJTgo) = go;
            *(ckt->CKTstate0 + here->BJTgx) = gx;
            *(ckt->CKTstate0 + here->BJTgeqcb) = geqcb;
            *(ckt->CKTstate0 + here->BJTgccs) = gccs;
            *(ckt->CKTstate0 + here->BJTgeqbx) = geqbx;

            /* Do not load the Jacobian and the rhs if
               perturbation is being carried out */

            if(SenCond)continue;
load:
            /*
             *  load current excitation vector
             */
            ceqcs=model->BJTtype * (*(ckt->CKTstate0 + here->BJTcqcs) -
                    vcs * gccs);
            ceqbx=model->BJTtype * (*(ckt->CKTstate0 + here->BJTcqbx) -
                    vbx * geqbx);
            ceqbe=model->BJTtype * (cc + cb - vbe * (gm + go + gpi) + vbc *
                    (go - geqcb));
            ceqbc=model->BJTtype * (-cc + vbe * (gm + go) - vbc * (gmu + go));

            *(ckt->CKTrhs + here->BJTbaseNode) += (-ceqbx);
            *(ckt->CKTrhs + here->BJTcolPrimeNode) +=
                    (ceqcs+ceqbx+ceqbc);
            *(ckt->CKTrhs + here->BJTbasePrimeNode) +=
                    (-ceqbe-ceqbc);
            *(ckt->CKTrhs + here->BJTemitPrimeNode) += (ceqbe);
            *(ckt->CKTrhs + here->BJTsubstNode) += (-ceqcs);
            /*
             *  load y matrix
             */
            *(here->BJTcolColPtr) += (gcpr);
            *(here->BJTbaseBasePtr) += (gx+geqbx);
            *(here->BJTemitEmitPtr) += (gepr);
            *(here->BJTcolPrimeColPrimePtr) += (gmu+go+gcpr+gccs+geqbx);
            *(here->BJTbasePrimeBasePrimePtr) += (gx +gpi+gmu+geqcb);
            *(here->BJTemitPrimeEmitPrimePtr) += (gpi+gepr+gm+go);
            *(here->BJTcolColPrimePtr) += (-gcpr);
            *(here->BJTbaseBasePrimePtr) += (-gx);
            *(here->BJTemitEmitPrimePtr) += (-gepr);
            *(here->BJTcolPrimeColPtr) += (-gcpr);
            *(here->BJTcolPrimeBasePrimePtr) += (-gmu+gm);
            *(here->BJTcolPrimeEmitPrimePtr) += (-gm-go);
            *(here->BJTbasePrimeBasePtr) += (-gx);
            *(here->BJTbasePrimeColPrimePtr) += (-gmu-geqcb);
            *(here->BJTbasePrimeEmitPrimePtr) += (-gpi);
            *(here->BJTemitPrimeEmitPtr) += (-gepr);
            *(here->BJTemitPrimeColPrimePtr) += (-go+geqcb);
            *(here->BJTemitPrimeBasePrimePtr) += (-gpi-gm-geqcb);
            *(here->BJTsubstSubstPtr) += (gccs);
            *(here->BJTcolPrimeSubstPtr) += (-gccs);
            *(here->BJTsubstColPrimePtr) += (-gccs);
            *(here->BJTbaseColPrimePtr) += (-geqbx);
            *(here->BJTcolPrimeBasePtr) += (-geqbx);
        }
    }
    return(OK);
}
