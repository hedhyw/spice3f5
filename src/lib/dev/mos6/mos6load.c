/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1989 Takayasu Sakurai
**********/

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "devdefs.h"
#include "mos6defs.h"
#include "util.h"
#include "trandefs.h"
#include "const.h"
#include "sperror.h"
#include "suffix.h"

int
MOS6load(inModel,ckt)
    GENmodel *inModel;
    register CKTcircuit *ckt;
        /* actually load the current value into the 
         * sparse matrix previously provided 
         */
{
    register MOS6model *model = (MOS6model *) inModel;
    register MOS6instance *here;
    double betac;
    double DrainSatCur;
    double EffectiveLength;
    double GateBulkOverlapCap;
    double GateDrainOverlapCap;
    double GateSourceOverlapCap;
    double OxideCap;
    double SourceSatCur;
    double arg;
    double cbhat;
    double cdhat;
    double cdrain;
    double cdreq;
    double ceq;
    double ceqbd;
    double ceqbs;
    double ceqgb;
    double ceqgd;
    double ceqgs;
    double delvbd;
    double delvbs;
    double delvds;
    double delvgd;
    double delvgs;
    double evbd;
    double evbs;
    double gcgb;
    double gcgd;
    double gcgs;
    double geq;
    double sarg;
    double sargsw;
    double tol;
    double vbd;
    double vbs;
    double vds;
    double vdsat;
    double vgb1;
    double vgb;
    double vgd1;
    double vgd;
    double vgdo;
    double vgs1;
    double vgs;
    double von;
    double vt;
    double xfact;
    int xnrm;
    int xrev;
    double capgs;   /* total gate-source capacitance */
    double capgd;   /* total gate-drain capacitance */
    double capgb;   /* total gate-bulk capacitance */
    int Check;
#ifndef NOBYPASS
    double tempv;
#endif /*NOBYPASS*/
    int error;
#ifdef CAPBYPASS
    int senflag;
#endif /* CAPBYPASS */ 
    int SenCond;


#ifdef CAPBYPASS
    senflag = 0;
    if(ckt->CKTsenInfo && ckt->CKTsenInfo->SENstatus == PERTURBATION &&
        (ckt->CKTsenInfo->SENmode & (ACSEN | TRANSEN))) {
        senflag = 1;
    }
#endif /* CAPBYPASS */ 

    /*  loop through all the MOS6 device models */
    for( ; model != NULL; model = model->MOS6nextModel ) {

        /* loop through all the instances of the model */
        for (here = model->MOS6instances; here != NULL ;
                here=here->MOS6nextInstance) {

            vt = CONSTKoverQ * here->MOS6temp;
            Check=1;
            if(ckt->CKTsenInfo){
#ifdef SENSDEBUG
                printf("MOS6load \n");
#endif /* SENSDEBUG */

                if((ckt->CKTsenInfo->SENstatus == PERTURBATION)&&
                    (here->MOS6senPertFlag == OFF))continue;

            }
            SenCond = ckt->CKTsenInfo && here->MOS6senPertFlag;

/*

*/

#ifdef DETAILPROF
asm("   .globl mospta");
asm("mospta:");
#endif /*DETAILPROF*/

            /* first, we compute a few useful values - these could be
             * pre-computed, but for historical reasons are still done
             * here.  They may be moved at the expense of instance size
             */

            EffectiveLength=here->MOS6l - 2*model->MOS6latDiff;
            if( (here->MOS6tSatCurDens == 0) || 
                    (here->MOS6drainArea == 0) ||
                    (here->MOS6sourceArea == 0)) {
                DrainSatCur = here->MOS6tSatCur;
                SourceSatCur = here->MOS6tSatCur;
            } else {
                DrainSatCur = here->MOS6tSatCurDens * 
                        here->MOS6drainArea;
                SourceSatCur = here->MOS6tSatCurDens * 
                        here->MOS6sourceArea;
            }
            GateSourceOverlapCap = model->MOS6gateSourceOverlapCapFactor * 
                    here->MOS6w;
            GateDrainOverlapCap = model->MOS6gateDrainOverlapCapFactor * 
                    here->MOS6w;
            GateBulkOverlapCap = model->MOS6gateBulkOverlapCapFactor * 
                    EffectiveLength;
            betac = here->MOS6tKc * here->MOS6w/EffectiveLength;
            OxideCap = model->MOS6oxideCapFactor * EffectiveLength * 
                    here->MOS6w;
            /* 
             * ok - now to do the start-up operations
             *
             * we must get values for vbs, vds, and vgs from somewhere
             * so we either predict them or recover them from last iteration
             * These are the two most common cases - either a prediction
             * step or the general iteration step and they
             * share some code, so we put them first - others later on
             */

            if(SenCond){
#ifdef SENSDEBUG
                printf("MOS6senPertFlag = ON \n");
#endif /* SENSDEBUG */
                if((ckt->CKTsenInfo->SENmode == TRANSEN) &&
                (ckt->CKTmode & MODEINITTRAN)) {
                    vgs = *(ckt->CKTstate1 + here->MOS6vgs);
                    vds = *(ckt->CKTstate1 + here->MOS6vds);
                    vbs = *(ckt->CKTstate1 + here->MOS6vbs);
                    vbd = *(ckt->CKTstate1 + here->MOS6vbd);
                    vgb = vgs - vbs;
                    vgd = vgs - vds;
                }
                else if (ckt->CKTsenInfo->SENmode == ACSEN){
                    vgb = model->MOS6type * ( 
                        *(ckt->CKTrhsOp+here->MOS6gNode) -
                        *(ckt->CKTrhsOp+here->MOS6bNode));
                    vbs = *(ckt->CKTstate0 + here->MOS6vbs);
                    vbd = *(ckt->CKTstate0 + here->MOS6vbd);
                    vgd = vgb + vbd ;
                    vgs = vgb + vbs ;
                    vds = vbs - vbd ;
                }
                else{
                    vgs = *(ckt->CKTstate0 + here->MOS6vgs);
                    vds = *(ckt->CKTstate0 + here->MOS6vds);
                    vbs = *(ckt->CKTstate0 + here->MOS6vbs);
                    vbd = *(ckt->CKTstate0 + here->MOS6vbd);
                    vgb = vgs - vbs;
                    vgd = vgs - vds;
                }
#ifdef SENSDEBUG
                printf(" vbs = %.7e ,vbd = %.7e,vgb = %.7e\n",vbs,vbd,vgb);
                printf(" vgs = %.7e ,vds = %.7e,vgd = %.7e\n",vgs,vds,vgd);
#endif /* SENSDEBUG */
                goto next1;
            }


            if((ckt->CKTmode & (MODEINITFLOAT | MODEINITPRED | MODEINITSMSIG
                    | MODEINITTRAN)) ||
                    ( (ckt->CKTmode & MODEINITFIX) && (!here->MOS6off) )  ) {
#ifndef PREDICTOR
                if(ckt->CKTmode & (MODEINITPRED | MODEINITTRAN) ) {

                    /* predictor step */

                    xfact=ckt->CKTdelta/ckt->CKTdeltaOld[1];
                    *(ckt->CKTstate0 + here->MOS6vbs) = 
                            *(ckt->CKTstate1 + here->MOS6vbs);
                    vbs = (1+xfact)* (*(ckt->CKTstate1 + here->MOS6vbs))
                            -(xfact * (*(ckt->CKTstate2 + here->MOS6vbs)));
                    *(ckt->CKTstate0 + here->MOS6vgs) = 
                            *(ckt->CKTstate1 + here->MOS6vgs);
                    vgs = (1+xfact)* (*(ckt->CKTstate1 + here->MOS6vgs))
                            -(xfact * (*(ckt->CKTstate2 + here->MOS6vgs)));
                    *(ckt->CKTstate0 + here->MOS6vds) = 
                            *(ckt->CKTstate1 + here->MOS6vds);
                    vds = (1+xfact)* (*(ckt->CKTstate1 + here->MOS6vds))
                            -(xfact * (*(ckt->CKTstate2 + here->MOS6vds)));
                    *(ckt->CKTstate0 + here->MOS6vbd) = 
                            *(ckt->CKTstate0 + here->MOS6vbs)-
                            *(ckt->CKTstate0 + here->MOS6vds);
                } else {
#endif /* PREDICTOR */

                    /* general iteration */

                    vbs = model->MOS6type * ( 
                        *(ckt->CKTrhsOld+here->MOS6bNode) -
                        *(ckt->CKTrhsOld+here->MOS6sNodePrime));
                    vgs = model->MOS6type * ( 
                        *(ckt->CKTrhsOld+here->MOS6gNode) -
                        *(ckt->CKTrhsOld+here->MOS6sNodePrime));
                    vds = model->MOS6type * ( 
                        *(ckt->CKTrhsOld+here->MOS6dNodePrime) -
                        *(ckt->CKTrhsOld+here->MOS6sNodePrime));
#ifndef PREDICTOR
                }
#endif /* PREDICTOR */

                /* now some common crunching for some more useful quantities */

                vbd=vbs-vds;
                vgd=vgs-vds;
                vgdo = *(ckt->CKTstate0 + here->MOS6vgs) - 
                        *(ckt->CKTstate0 + here->MOS6vds);
                delvbs = vbs - *(ckt->CKTstate0 + here->MOS6vbs);
                delvbd = vbd - *(ckt->CKTstate0 + here->MOS6vbd);
                delvgs = vgs - *(ckt->CKTstate0 + here->MOS6vgs);
                delvds = vds - *(ckt->CKTstate0 + here->MOS6vds);
                delvgd = vgd-vgdo;

                /* these are needed for convergence testing */

                if (here->MOS6mode >= 0) {
                    cdhat=
                        here->MOS6cd-
                        here->MOS6gbd * delvbd +
                        here->MOS6gmbs * delvbs +
                        here->MOS6gm * delvgs + 
                        here->MOS6gds * delvds ;
                } else {
                    cdhat=
                        here->MOS6cd -
                        ( here->MOS6gbd -
                        here->MOS6gmbs) * delvbd -
                        here->MOS6gm * delvgd + 
                        here->MOS6gds * delvds ;
                }
                cbhat=
                    here->MOS6cbs +
                    here->MOS6cbd +
                    here->MOS6gbd * delvbd +
                    here->MOS6gbs * delvbs ;
/*

*/

#ifdef DETAILPROF
asm("   .globl mosptb");
asm("mosptb:");
#endif /*DETAILPROF*/
#ifndef NOBYPASS
                /* now lets see if we can bypass (ugh) */
                /* the following mess should be one if statement, but
                 * many compilers can't handle it all at once, so it
                 * is split into several successive if statements
                 */
                tempv = MAX(FABS(cbhat),FABS(here->MOS6cbs
                        + here->MOS6cbd))+ckt->CKTabstol;
                if((!(ckt->CKTmode & (MODEINITPRED|MODEINITTRAN|MODEINITSMSIG)
                        )) && (ckt->CKTbypass) )
                if ( (FABS(cbhat-(here->MOS6cbs + 
                        here->MOS6cbd)) < ckt->CKTreltol * 
                        tempv)) 
                if( (FABS(delvbs) < (ckt->CKTreltol * MAX(FABS(vbs),
                        FABS(*(ckt->CKTstate0+here->MOS6vbs)))+
                        ckt->CKTvoltTol)))
                if ( (FABS(delvbd) < (ckt->CKTreltol * MAX(FABS(vbd),
                        FABS(*(ckt->CKTstate0+here->MOS6vbd)))+
                        ckt->CKTvoltTol)) )
                if( (FABS(delvgs) < (ckt->CKTreltol * MAX(FABS(vgs),
                        FABS(*(ckt->CKTstate0+here->MOS6vgs)))+
                        ckt->CKTvoltTol)))
                if ( (FABS(delvds) < (ckt->CKTreltol * MAX(FABS(vds),
                        FABS(*(ckt->CKTstate0+here->MOS6vds)))+
                        ckt->CKTvoltTol)) )
                if( (FABS(cdhat- here->MOS6cd) <
                        ckt->CKTreltol * MAX(FABS(cdhat),FABS(
                        here->MOS6cd)) + ckt->CKTabstol) ) {
                    /* bypass code *
                    /* nothing interesting has changed since last
                     * iteration on this device, so we just
                     * copy all the values computed last iteration out
                     * and keep going
                     */
                    vbs = *(ckt->CKTstate0 + here->MOS6vbs);
                    vbd = *(ckt->CKTstate0 + here->MOS6vbd);
                    vgs = *(ckt->CKTstate0 + here->MOS6vgs);
                    vds = *(ckt->CKTstate0 + here->MOS6vds);
                    vgd = vgs - vds;
                    vgb = vgs - vbs;
                    cdrain = here->MOS6mode * (here->MOS6cd + here->MOS6cbd);
                    if(ckt->CKTmode & (MODETRAN | MODETRANOP)) {
                        capgs = ( *(ckt->CKTstate0+here->MOS6capgs)+ 
                                  *(ckt->CKTstate1+here->MOS6capgs) +
                                  GateSourceOverlapCap );
                        capgd = ( *(ckt->CKTstate0+here->MOS6capgd)+ 
                                  *(ckt->CKTstate1+here->MOS6capgd) +
                                  GateDrainOverlapCap );
                        capgb = ( *(ckt->CKTstate0+here->MOS6capgb)+ 
                                  *(ckt->CKTstate1+here->MOS6capgb) +
                                  GateBulkOverlapCap );

                        if(ckt->CKTsenInfo){
                            here->MOS6cgs = capgs;
                            here->MOS6cgd = capgd;
                            here->MOS6cgb = capgb;
                        }
                    }
                    goto bypass;
                }
#endif /*NOBYPASS*/
/*

*/

#ifdef DETAILPROF
asm("   .globl mosptc");
asm("mosptc:");
#endif /*DETAILPROF*/
                /* ok - bypass is out, do it the hard way */

                von = model->MOS6type * here->MOS6von;

#ifndef NODELIMITING
                /* 
                 * limiting
                 *  we want to keep device voltages from changing
                 * so fast that the exponentials churn out overflows
                 * and similar rudeness
                 */

                if(*(ckt->CKTstate0 + here->MOS6vds) >=0) {
                    vgs = DEVfetlim(vgs,*(ckt->CKTstate0 + here->MOS6vgs)
                            ,von);
                    vds = vgs - vgd;
                    vds = DEVlimvds(vds,*(ckt->CKTstate0 + here->MOS6vds));
                    vgd = vgs - vds;
                } else {
                    vgd = DEVfetlim(vgd,vgdo,von);
                    vds = vgs - vgd;
                    if(!(ckt->CKTfixLimit)) {
                        vds = -DEVlimvds(-vds,-(*(ckt->CKTstate0 + 
                                here->MOS6vds)));
                    }
                    vgs = vgd + vds;
                }
                if(vds >= 0) {
                    vbs = DEVpnjlim(vbs,*(ckt->CKTstate0 + here->MOS6vbs),
                            vt,here->MOS6sourceVcrit,&Check);
                    vbd = vbs-vds;
                } else {
                    vbd = DEVpnjlim(vbd,*(ckt->CKTstate0 + here->MOS6vbd),
                            vt,here->MOS6drainVcrit,&Check);
                    vbs = vbd + vds;
                }
#endif /*NODELIMITING*/
/*

*/

#ifdef DETAILPROF
asm("   .globl mosptd");
asm("mosptd:");
#endif /*DETAILPROF*/
            } else {

                /* ok - not one of the simple cases, so we have to
                 * look at all of the possibilities for why we were
                 * called.  We still just initialize the three voltages
                 */

                if((ckt->CKTmode & MODEINITJCT) && !here->MOS6off) {
                    vds= model->MOS6type * here->MOS6icVDS;
                    vgs= model->MOS6type * here->MOS6icVGS;
                    vbs= model->MOS6type * here->MOS6icVBS;
                    if((vds==0) && (vgs==0) && (vbs==0) && 
                            ((ckt->CKTmode & 
                                (MODETRAN|MODEDCOP|MODEDCTRANCURVE)) ||
                             (!(ckt->CKTmode & MODEUIC)))) {
                        vbs = -1;
                        vgs = model->MOS6type * here->MOS6tVto;
                        vds = 0;
                    }
                } else {
                    vbs=vgs=vds=0;
                } 
            }
/*

*/

#ifdef DETAILPROF
asm("   .globl mospte");
asm("mospte:");
#endif /*DETAILPROF*/

            /*
             * now all the preliminaries are over - we can start doing the
             * real work
             */
            vbd = vbs - vds;
            vgd = vgs - vds;
            vgb = vgs - vbs;


            /*
             * bulk-source and bulk-drain diodes
             *   here we just evaluate the ideal diode current and the
             *   corresponding derivative (conductance).
             */
next1:      if(vbs <= 0) {
                here->MOS6gbs = SourceSatCur/vt;
                here->MOS6cbs = here->MOS6gbs*vbs;
                here->MOS6gbs += ckt->CKTgmin;
            } else {
                evbs = exp(MIN(MAX_EXP_ARG,vbs/vt));
                here->MOS6gbs = SourceSatCur*evbs/vt + ckt->CKTgmin;
                here->MOS6cbs = SourceSatCur * (evbs-1);
            }
            if(vbd <= 0) {
                here->MOS6gbd = DrainSatCur/vt;
                here->MOS6cbd = here->MOS6gbd *vbd;
                here->MOS6gbd += ckt->CKTgmin;
            } else {
                evbd = exp(MIN(MAX_EXP_ARG,vbd/vt));
                here->MOS6gbd = DrainSatCur*evbd/vt +ckt->CKTgmin;
                here->MOS6cbd = DrainSatCur *(evbd-1);
            }

            /* now to determine whether the user was able to correctly
             * identify the source and drain of his device
             */
            if(vds >= 0) {
                /* normal mode */
                here->MOS6mode = 1;
            } else {
                /* inverse mode */
                here->MOS6mode = -1;
            }
/*

*/

#ifdef DETAILPROF
asm("   .globl mosptf");
asm("mosptf:");
#endif /*DETAILPROF*/
            {
            /*
             *     this block of code evaluates the drain current and its 
             *     derivatives using the n-th power MOS model and the 
             *     charges associated with the gate, channel and bulk for 
             *     mosfets
             *
             */

            /* the following 14 variables are local to this code block until 
             * it is obvious that they can be made global 
             */
            double arg;
            double sarg;
            double vgon;
            double vdshere, vbsvbd;
	    double idsat, lambda, vonbm;
	    double vdst, vdst1, vdst2, ivdst1, vdstg;

		vbsvbd = (here->MOS6mode==1?vbs:vbd);
                if (vbsvbd <= 0 ) {
                    sarg = sqrt(here->MOS6tPhi - vbsvbd);
                } else {
                    sarg = sqrt(here->MOS6tPhi);
                    sarg = sarg - vbsvbd / (sarg+sarg);
                    sarg = MAX(0,sarg);
                }
		vdshere = vds * here->MOS6mode;
                von=(here->MOS6tVbi*model->MOS6type)+model->MOS6gamma*sarg
		    - model->MOS6gamma1 * vbsvbd;
		    - model->MOS6sigma  * vdshere;
                vgon = (here->MOS6mode==1?vgs:vgd) - von;

                if (vgon <= 0) {
                    /*
                     *     cutoff region
                     */
		    vdsat = 0;
                    cdrain=0;
                    here->MOS6gm=0;
                    here->MOS6gds=0;
                    here->MOS6gmbs=0;

                } else {
                    if (sarg <= 0) {
                        arg=0;
                    } else {
                	if ((here->MOS6mode==1?vbs:vbd) <= 0 ) {
                            vonbm = model->MOS6gamma1
			          + model->MOS6gamma / (sarg + sarg);
			} else {
                            vonbm = model->MOS6gamma1
			          + model->MOS6gamma / 2 / sqrt(here->MOS6tPhi);
			}
                    }
		    sarg = log(vgon);
		    vdsat = model->MOS6kv * exp(sarg * model->MOS6nv);
		    idsat = betac * exp(sarg * model->MOS6nc);
		    lambda = model->MOS6lamda0 - model->MOS6lamda1 * vbsvbd;
                    /*
                     *     saturation region
                     */
		    cdrain = idsat * (1 + lambda * vdshere);
		    here->MOS6gm = cdrain * model->MOS6nc / vgon;
		    here->MOS6gds = here->MOS6gm * model->MOS6sigma 
				  + idsat * lambda;
		    here->MOS6gmbs = here->MOS6gm * vonbm 
				   - idsat * model->MOS6lamda1 * vdshere;
                    if (vdsat > vdshere){
                    /*
                     *     linear region
                     */
			vdst = vdshere / vdsat;
                        vdst2 = (2 - vdst) * vdst;
			vdstg = - vdst * model->MOS6nv / vgon;
			ivdst1 = cdrain * (2 - vdst - vdst);
                        cdrain = cdrain * vdst2;
                        here->MOS6gm = here->MOS6gm * vdst2 + ivdst1 * vdstg;
                        here->MOS6gds = here->MOS6gds * vdst2 + ivdst1 
				      * (1 / vdsat + vdstg * model->MOS6sigma);
                        here->MOS6gmbs = here->MOS6gmbs * vdst2 
				       + ivdst1 * vdstg * vonbm;
                    }
                }
                /*
                 *     finished
                 */
            }
/*

*/

#ifdef DETAILPROF
asm("   .globl mosptg");
asm("mosptg:");
#endif /*DETAILPROF*/

            /* now deal with n vs p polarity */

            here->MOS6von = model->MOS6type * von;
            here->MOS6vdsat = model->MOS6type * vdsat;
            /* line 490 */
            /*
             *  COMPUTE EQUIVALENT DRAIN CURRENT SOURCE
             */
            here->MOS6cd=here->MOS6mode * cdrain - here->MOS6cbd;

            if (ckt->CKTmode & (MODETRAN | MODETRANOP | MODEINITSMSIG)) {
                /* 
                 * now we do the hard part of the bulk-drain and bulk-source
                 * diode - we evaluate the non-linear capacitance and
                 * charge
                 *
                 * the basic equations are not hard, but the implementation
                 * is somewhat long in an attempt to avoid log/exponential
                 * evaluations
                 */
                /*
                 *  charge storage elements
                 *
                 *.. bulk-drain and bulk-source depletion capacitances
                 */
#ifdef CAPBYPASS
                if(((ckt->CKTmode & (MODEINITPRED | MODEINITTRAN) ) ||
                        FABS(delvbs) >= ckt->CKTreltol * MAX(FABS(vbs),
                        FABS(*(ckt->CKTstate0+here->MOS6vbs)))+
                        ckt->CKTvoltTol)|| senflag)
#endif /*CAPBYPASS*/
                {
                    /* can't bypass the diode capacitance calculations */
#ifdef CAPZEROBYPASS
                    if(here->MOS6Cbs != 0 || here->MOS6Cbssw != 0 ) {
#endif /*CAPZEROBYPASS*/
                    if (vbs < here->MOS6tDepCap){
                        arg=1-vbs/here->MOS6tBulkPot;
                        /*
                         * the following block looks somewhat long and messy,
                         * but since most users use the default grading
                         * coefficients of .5, and sqrt is MUCH faster than an
                         * exp(log()) we use this special case code to buy time.
                         * (as much as 10% of total job time!)
                         */
#ifndef NOSQRT
                        if(model->MOS6bulkJctBotGradingCoeff ==
                                model->MOS6bulkJctSideGradingCoeff) {
                            if(model->MOS6bulkJctBotGradingCoeff == .5) {
                                sarg = sargsw = 1/sqrt(arg);
                            } else {
                                sarg = sargsw =
                                        exp(-model->MOS6bulkJctBotGradingCoeff*
                                        log(arg));
                            }
                        } else {
                            if(model->MOS6bulkJctBotGradingCoeff == .5) {
                                sarg = 1/sqrt(arg);
                            } else {
#endif /*NOSQRT*/
                                sarg = exp(-model->MOS6bulkJctBotGradingCoeff*
                                        log(arg));
#ifndef NOSQRT
                            }
                            if(model->MOS6bulkJctSideGradingCoeff == .5) {
                                sargsw = 1/sqrt(arg);
                            } else {
#endif /*NOSQRT*/
                                sargsw =exp(-model->MOS6bulkJctSideGradingCoeff*
                                        log(arg));
#ifndef NOSQRT
                            }
                        }
#endif /*NOSQRT*/
                        *(ckt->CKTstate0 + here->MOS6qbs) =
                            here->MOS6tBulkPot*(here->MOS6Cbs*
                            (1-arg*sarg)/(1-model->MOS6bulkJctBotGradingCoeff)
                            +here->MOS6Cbssw*
                            (1-arg*sargsw)/
                            (1-model->MOS6bulkJctSideGradingCoeff));
                        here->MOS6capbs=here->MOS6Cbs*sarg+
                                here->MOS6Cbssw*sargsw;
                    } else {
                        *(ckt->CKTstate0 + here->MOS6qbs) = here->MOS6f4s +
                                vbs*(here->MOS6f2s+vbs*(here->MOS6f3s/2));
                        here->MOS6capbs=here->MOS6f2s+here->MOS6f3s*vbs;
                    }
#ifdef CAPZEROBYPASS
                    } else {
                        *(ckt->CKTstate0 + here->MOS6qbs) = 0;
                        here->MOS6capbs=0;
                    }
#endif /*CAPZEROBYPASS*/
                }
#ifdef CAPBYPASS
                if(((ckt->CKTmode & (MODEINITPRED | MODEINITTRAN) ) ||
                        FABS(delvbd) >= ckt->CKTreltol * MAX(FABS(vbd),
                        FABS(*(ckt->CKTstate0+here->MOS6vbd)))+
                        ckt->CKTvoltTol)|| senflag)
#endif /*CAPBYPASS*/
                    /* can't bypass the diode capacitance calculations */
                {
#ifdef CAPZEROBYPASS
                    if(here->MOS6Cbd != 0 || here->MOS6Cbdsw != 0 ) {
#endif /*CAPZEROBYPASS*/
                    if (vbd < here->MOS6tDepCap) {
                        arg=1-vbd/here->MOS6tBulkPot;
                        /*
                         * the following block looks somewhat long and messy,
                         * but since most users use the default grading
                         * coefficients of .5, and sqrt is MUCH faster than an
                         * exp(log()) we use this special case code to buy time.
                         * (as much as 10% of total job time!)
                         */
#ifndef NOSQRT
                        if(model->MOS6bulkJctBotGradingCoeff == .5 &&
                                model->MOS6bulkJctSideGradingCoeff == .5) {
                            sarg = sargsw = 1/sqrt(arg);
                        } else {
                            if(model->MOS6bulkJctBotGradingCoeff == .5) {
                                sarg = 1/sqrt(arg);
                            } else {
#endif /*NOSQRT*/
                                sarg = exp(-model->MOS6bulkJctBotGradingCoeff*
                                        log(arg));
#ifndef NOSQRT
                            }
                            if(model->MOS6bulkJctSideGradingCoeff == .5) {
                                sargsw = 1/sqrt(arg);
                            } else {
#endif /*NOSQRT*/
                                sargsw =exp(-model->MOS6bulkJctSideGradingCoeff*
                                        log(arg));
#ifndef NOSQRT
                            }
                        }
#endif /*NOSQRT*/
                        *(ckt->CKTstate0 + here->MOS6qbd) =
                            here->MOS6tBulkPot*(here->MOS6Cbd*
                            (1-arg*sarg)
                            /(1-model->MOS6bulkJctBotGradingCoeff)
                            +here->MOS6Cbdsw*
                            (1-arg*sargsw)
                            /(1-model->MOS6bulkJctSideGradingCoeff));
                        here->MOS6capbd=here->MOS6Cbd*sarg+
                                here->MOS6Cbdsw*sargsw;
                    } else {
                        *(ckt->CKTstate0 + here->MOS6qbd) = here->MOS6f4d +
                                vbd * (here->MOS6f2d + vbd * here->MOS6f3d/2);
                        here->MOS6capbd=here->MOS6f2d + vbd * here->MOS6f3d;
                    }
#ifdef CAPZEROBYPASS
                } else {
                    *(ckt->CKTstate0 + here->MOS6qbd) = 0;
                    here->MOS6capbd = 0;
                }
#endif /*CAPZEROBYPASS*/
                }
/*

*/

#ifdef DETAILPROF
asm("   .globl mospth");
asm("mospth:");
#endif /*DETAILPROF*/

                if(SenCond && (ckt->CKTsenInfo->SENmode==TRANSEN)) goto next2;

                if ( ckt->CKTmode & MODETRAN ) {
                    /* (above only excludes tranop, since we're only at this
                     * point if tran or tranop )
                     */

                    /*
                     *    calculate equivalent conductances and currents for
                     *    depletion capacitors
                     */

                    /* integrate the capacitors and save results */

                    error = NIintegrate(ckt,&geq,&ceq,here->MOS6capbd,
                            here->MOS6qbd);
                    if(error) return(error);
                    here->MOS6gbd += geq;
                    here->MOS6cbd += *(ckt->CKTstate0 + here->MOS6cqbd);
                    here->MOS6cd -= *(ckt->CKTstate0 + here->MOS6cqbd);
                    error = NIintegrate(ckt,&geq,&ceq,here->MOS6capbs,
                            here->MOS6qbs);
                    if(error) return(error);
                    here->MOS6gbs += geq;
                    here->MOS6cbs += *(ckt->CKTstate0 + here->MOS6cqbs);
                }
            }
/*

*/

#ifdef DETAILPROF
asm("   .globl mospti");
asm("mospti:");
#endif /*DETAILPROF*/

            if(SenCond) goto next2;


            /*
             *  check convergence
             */
            if ( (here->MOS6off == 0)  || 
                    (!(ckt->CKTmode & (MODEINITFIX|MODEINITSMSIG))) ){
                if (Check == 1) {
                    ckt->CKTnoncon++;
		    ckt->CKTtroubleElt = (GENinstance *) here;
#ifndef NEWCONV
                } else {
                    tol=ckt->CKTreltol*MAX(FABS(cdhat),
                            FABS(here->MOS6cd))+ckt->CKTabstol;
                    if (FABS(cdhat-here->MOS6cd) >= tol) { 
                        ckt->CKTnoncon++;
			ckt->CKTtroubleElt = (GENinstance *) here;
                    } else {
                        tol=ckt->CKTreltol*MAX(FABS(cbhat),
                                FABS(here->MOS6cbs+here->MOS6cbd))+
                                ckt->CKTabstol;
                        if (FABS(cbhat-(here->MOS6cbs+here->MOS6cbd)) > tol) {
                            ckt->CKTnoncon++;
			    ckt->CKTtroubleElt = (GENinstance *) here;
                        }
                    }
#endif /*NEWCONV*/
                }
            }
/*

*/

#ifdef DETAILPROF
asm("   .globl mosptj");
asm("mosptj:");
#endif /*DETAILPROF*/

            /* save things away for next time */

next2:      *(ckt->CKTstate0 + here->MOS6vbs) = vbs;
            *(ckt->CKTstate0 + here->MOS6vbd) = vbd;
            *(ckt->CKTstate0 + here->MOS6vgs) = vgs;
            *(ckt->CKTstate0 + here->MOS6vds) = vds;

/*

*/

#ifdef DETAILPROF
asm("   .globl mosptk");
asm("mosptk:");
#endif /*DETAILPROF*/
            /*
             *     meyer's capacitor model
             */
            if ( ckt->CKTmode & (MODETRAN | MODETRANOP | MODEINITSMSIG) ) {
                /*
                 *     calculate meyer's capacitors
                 */
                /* 
                 * new cmeyer - this just evaluates at the current time,
                 * expects you to remember values from previous time
                 * returns 1/2 of non-constant portion of capacitance
                 * you must add in the other half from previous time
                 * and the constant part
                 */
                if (here->MOS6mode > 0){
                    DEVqmeyer (vgs,vgd,vgb,von,vdsat,
                        (ckt->CKTstate0 + here->MOS6capgs),
                        (ckt->CKTstate0 + here->MOS6capgd),
                        (ckt->CKTstate0 + here->MOS6capgb),
                        here->MOS6tPhi,OxideCap);
                } else {
                    DEVqmeyer (vgd,vgs,vgb,von,vdsat,
                        (ckt->CKTstate0 + here->MOS6capgd),
                        (ckt->CKTstate0 + here->MOS6capgs),
                        (ckt->CKTstate0 + here->MOS6capgb),
                        here->MOS6tPhi,OxideCap);
                }
                vgs1 = *(ckt->CKTstate1 + here->MOS6vgs);
                vgd1 = vgs1 - *(ckt->CKTstate1 + here->MOS6vds);
                vgb1 = vgs1 - *(ckt->CKTstate1 + here->MOS6vbs);
                if(ckt->CKTmode & (MODETRANOP|MODEINITSMSIG)) {
                    capgs =  2 * *(ckt->CKTstate0+here->MOS6capgs)+ 
                              GateSourceOverlapCap ;
                    capgd =  2 * *(ckt->CKTstate0+here->MOS6capgd)+ 
                              GateDrainOverlapCap ;
                    capgb =  2 * *(ckt->CKTstate0+here->MOS6capgb)+ 
                              GateBulkOverlapCap ;
                } else {
                    capgs = ( *(ckt->CKTstate0+here->MOS6capgs)+ 
                              *(ckt->CKTstate1+here->MOS6capgs) +
                              GateSourceOverlapCap );
                    capgd = ( *(ckt->CKTstate0+here->MOS6capgd)+ 
                              *(ckt->CKTstate1+here->MOS6capgd) +
                              GateDrainOverlapCap );
                    capgb = ( *(ckt->CKTstate0+here->MOS6capgb)+ 
                              *(ckt->CKTstate1+here->MOS6capgb) +
                              GateBulkOverlapCap );
                }
                if(ckt->CKTsenInfo){
                    here->MOS6cgs = capgs;
                    here->MOS6cgd = capgd;
                    here->MOS6cgb = capgb;
                }
/*

*/

#ifdef DETAILPROF
asm("   .globl mosptl");
asm("mosptl:");
#endif /*DETAILPROF*/
                /*
                 *     store small-signal parameters (for meyer's model)
                 *  all parameters already stored, so done...
                 */
                if(SenCond){
                    if((ckt->CKTsenInfo->SENmode == DCSEN)||
                            (ckt->CKTsenInfo->SENmode == ACSEN)){
                        continue;
                    }
                }

#ifndef PREDICTOR
                if (ckt->CKTmode & (MODEINITPRED | MODEINITTRAN) ) {
                    *(ckt->CKTstate0 + here->MOS6qgs) =
                        (1+xfact) * *(ckt->CKTstate1 + here->MOS6qgs)
                        - xfact * *(ckt->CKTstate2 + here->MOS6qgs);
                    *(ckt->CKTstate0 + here->MOS6qgd) =
                        (1+xfact) * *(ckt->CKTstate1 + here->MOS6qgd)
                        - xfact * *(ckt->CKTstate2 + here->MOS6qgd);
                    *(ckt->CKTstate0 + here->MOS6qgb) =
                        (1+xfact) * *(ckt->CKTstate1 + here->MOS6qgb)
                        - xfact * *(ckt->CKTstate2 + here->MOS6qgb);
                } else {
#endif /*PREDICTOR*/
                    if(ckt->CKTmode & MODETRAN) {
                        *(ckt->CKTstate0 + here->MOS6qgs) = (vgs-vgs1)*capgs +
                            *(ckt->CKTstate1 + here->MOS6qgs) ;
                        *(ckt->CKTstate0 + here->MOS6qgd) = (vgd-vgd1)*capgd +
                            *(ckt->CKTstate1 + here->MOS6qgd) ;
                        *(ckt->CKTstate0 + here->MOS6qgb) = (vgb-vgb1)*capgb +
                            *(ckt->CKTstate1 + here->MOS6qgb) ;
                    } else {
                        /* TRANOP only */
                        *(ckt->CKTstate0 + here->MOS6qgs) = vgs*capgs;
                        *(ckt->CKTstate0 + here->MOS6qgd) = vgd*capgd;
                        *(ckt->CKTstate0 + here->MOS6qgb) = vgb*capgb;
                    }
#ifndef PREDICTOR
                }
#endif /*PREDICTOR*/
            }
bypass:
            if(SenCond) continue;

            if ( (ckt->CKTmode & (MODEINITTRAN)) || 
                    (! (ckt->CKTmode & (MODETRAN)) )  ) {
                /*
                 *  initialize to zero charge conductances 
                 *  and current
                 */
                gcgs=0;
                ceqgs=0;
                gcgd=0;
                ceqgd=0;
                gcgb=0;
                ceqgb=0;
            } else {
                if(capgs == 0) *(ckt->CKTstate0 + here->MOS6cqgs) =0;
                if(capgd == 0) *(ckt->CKTstate0 + here->MOS6cqgd) =0;
                if(capgb == 0) *(ckt->CKTstate0 + here->MOS6cqgb) =0;
                /*
                 *    calculate equivalent conductances and currents for
                 *    meyer"s capacitors
                 */
                error = NIintegrate(ckt,&gcgs,&ceqgs,capgs,here->MOS6qgs);
                if(error) return(error);
                error = NIintegrate(ckt,&gcgd,&ceqgd,capgd,here->MOS6qgd);
                if(error) return(error);
                error = NIintegrate(ckt,&gcgb,&ceqgb,capgb,here->MOS6qgb);
                if(error) return(error);
                ceqgs=ceqgs-gcgs*vgs+ckt->CKTag[0]* 
                        *(ckt->CKTstate0 + here->MOS6qgs);
                ceqgd=ceqgd-gcgd*vgd+ckt->CKTag[0]*
                        *(ckt->CKTstate0 + here->MOS6qgd);
                ceqgb=ceqgb-gcgb*vgb+ckt->CKTag[0]*
                        *(ckt->CKTstate0 + here->MOS6qgb);
            }
            /*
             *     store charge storage info for meyer's cap in lx table
             */

            /*
             *  load current vector
             */
            ceqbs = model->MOS6type * 
                    (here->MOS6cbs-(here->MOS6gbs-ckt->CKTgmin)*vbs);
            ceqbd = model->MOS6type * 
                    (here->MOS6cbd-(here->MOS6gbd-ckt->CKTgmin)*vbd);
            if (here->MOS6mode >= 0) {
                xnrm=1;
                xrev=0;
                cdreq=model->MOS6type*(cdrain-here->MOS6gds*vds-
                        here->MOS6gm*vgs-here->MOS6gmbs*vbs);
            } else {
                xnrm=0;
                xrev=1;
                cdreq = -(model->MOS6type)*(cdrain-here->MOS6gds*(-vds)-
                        here->MOS6gm*vgd-here->MOS6gmbs*vbd);
            }
            *(ckt->CKTrhs + here->MOS6gNode) -= 
                (model->MOS6type * (ceqgs + ceqgb + ceqgd));
            *(ckt->CKTrhs + here->MOS6bNode) -=
                (ceqbs + ceqbd - model->MOS6type * ceqgb);
            *(ckt->CKTrhs + here->MOS6dNodePrime) +=
                    (ceqbd - cdreq + model->MOS6type * ceqgd);
            *(ckt->CKTrhs + here->MOS6sNodePrime) += 
                    cdreq + ceqbs + model->MOS6type * ceqgs;
            /*
             *  load y matrix
             */

            *(here->MOS6DdPtr) += (here->MOS6drainConductance);
            *(here->MOS6GgPtr) += ((gcgd+gcgs+gcgb));
            *(here->MOS6SsPtr) += (here->MOS6sourceConductance);
            *(here->MOS6BbPtr) += (here->MOS6gbd+here->MOS6gbs+gcgb);
            *(here->MOS6DPdpPtr) += 
                    (here->MOS6drainConductance+here->MOS6gds+
                    here->MOS6gbd+xrev*(here->MOS6gm+here->MOS6gmbs)+gcgd);
            *(here->MOS6SPspPtr) += 
                    (here->MOS6sourceConductance+here->MOS6gds+
                    here->MOS6gbs+xnrm*(here->MOS6gm+here->MOS6gmbs)+gcgs);
            *(here->MOS6DdpPtr) += (-here->MOS6drainConductance);
            *(here->MOS6GbPtr) -= gcgb;
            *(here->MOS6GdpPtr) -= gcgd;
            *(here->MOS6GspPtr) -= gcgs;
            *(here->MOS6SspPtr) += (-here->MOS6sourceConductance);
            *(here->MOS6BgPtr) -= gcgb;
            *(here->MOS6BdpPtr) -= here->MOS6gbd;
            *(here->MOS6BspPtr) -= here->MOS6gbs;
            *(here->MOS6DPdPtr) += (-here->MOS6drainConductance);
            *(here->MOS6DPgPtr) += ((xnrm-xrev)*here->MOS6gm-gcgd);
            *(here->MOS6DPbPtr) += (-here->MOS6gbd+(xnrm-xrev)*here->MOS6gmbs);
            *(here->MOS6DPspPtr) += (-here->MOS6gds-xnrm*
                    (here->MOS6gm+here->MOS6gmbs));
            *(here->MOS6SPgPtr) += (-(xnrm-xrev)*here->MOS6gm-gcgs);
            *(here->MOS6SPsPtr) += (-here->MOS6sourceConductance);
            *(here->MOS6SPbPtr) += (-here->MOS6gbs-(xnrm-xrev)*here->MOS6gmbs);
            *(here->MOS6SPdpPtr) += (-here->MOS6gds-xrev*
                    (here->MOS6gm+here->MOS6gmbs));
        }
    }
    return(OK);
}
