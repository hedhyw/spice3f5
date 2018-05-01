/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 S. Hwang
**********/

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "devdefs.h"
#include "cktdefs.h"
#include "mesdefs.h"
#include "const.h"
#include "trandefs.h"
#include "sperror.h"
#include "suffix.h"

/* forward declaraction of our helper function */
#ifdef __STDC__
static double qggnew(double,double,double,double,double,double,double,
    double*,double*);
#else /* stdc */
static double qggnew();
#endif /* stdc */

int
MESload(inModel,ckt)
    GENmodel *inModel;
    CKTcircuit *ckt;
        /* actually load the current resistance value into the 
         * sparse matrix previously provided 
         */
{
    register MESmodel *model = (MESmodel*)inModel;
    register MESinstance *here;
    double afact;
    double beta;
    double betap;
    double capgd;
    double capgs;
    double cd;
    double cdhat;
    double cdrain;
    double cdreq;
    double ceq;
    double ceqgd;
    double ceqgs;
    double cg;
    double cgd;
    double cgdna,cgdnb,cgdnc,cgdnd;
    double cghat;
    double cgsna,cgsnb,cgsnc,cgsnd;
    double csat;
    double czgd;
    double czgs;
    double delvds;
    double delvgd;
    double delvgs;
    double denom;
    double evgd;
    double evgs;
    double gdpr;
    double gds;
    double geq;
    double ggd;
    double ggs;
    double gm;
    double gspr;
    double invdenom;
    double lfact;
    double phib;
    double prod;
    double qgga,qggb,qggc,qggd;
    double vcap;
    double vcrit;
    double vds;
    double vgd;
    double vgd1;
    double vgdt;
    double vgs;
    double vgs1;
    double vgst;
    double vto;
    double xfact;
    int icheck;
    int ichk1;
    int error;

    /*  loop through all the models */
    for( ; model != NULL; model = model->MESnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->MESinstances; here != NULL ;
                here=here->MESnextInstance) {

            /*
             *  dc model parameters 
             */
            beta = model->MESbeta * here->MESarea;
            gdpr = model->MESdrainConduct * here->MESarea;
            gspr = model->MESsourceConduct * here->MESarea;
            csat = model->MESgateSatCurrent * here->MESarea;
            vcrit = model->MESvcrit;
            vto = model->MESthreshold;
            /*
             *    initialization
             */
            icheck = 1;
            if( ckt->CKTmode & MODEINITSMSIG) {
                vgs = *(ckt->CKTstate0 + here->MESvgs);
                vgd = *(ckt->CKTstate0 + here->MESvgd);
            } else if (ckt->CKTmode & MODEINITTRAN) {
                vgs = *(ckt->CKTstate1 + here->MESvgs);
                vgd = *(ckt->CKTstate1 + here->MESvgd);
            } else if ( (ckt->CKTmode & MODEINITJCT) &&
                    (ckt->CKTmode & MODETRANOP) &&
                    (ckt->CKTmode & MODEUIC) ) {
                vds = model->MEStype*here->MESicVDS;
                vgs = model->MEStype*here->MESicVGS;
                vgd = vgs-vds;
            } else if ( (ckt->CKTmode & MODEINITJCT) &&
                    (here->MESoff == 0)  ) {
                vgs = -1;
                vgd = -1;
            } else if( (ckt->CKTmode & MODEINITJCT) ||
                    ((ckt->CKTmode & MODEINITFIX) && (here->MESoff))) {
                vgs = 0;
                vgd = 0;
            } else {
#ifndef PREDICTOR
                if(ckt->CKTmode & MODEINITPRED) {
                    xfact = ckt->CKTdelta/ckt->CKTdeltaOld[1];
                    *(ckt->CKTstate0 + here->MESvgs) = 
                            *(ckt->CKTstate1 + here->MESvgs);
                    vgs = (1+xfact) * *(ckt->CKTstate1 + here->MESvgs) -
                           xfact * *(ckt->CKTstate2 + here->MESvgs);
                    *(ckt->CKTstate0 + here->MESvgd) = 
                            *(ckt->CKTstate1 + here->MESvgd);
                    vgd = (1+xfact)* *(ckt->CKTstate1 + here->MESvgd) -
                           xfact * *(ckt->CKTstate2 + here->MESvgd);
                    *(ckt->CKTstate0 + here->MEScg) = 
                            *(ckt->CKTstate1 + here->MEScg);
                    *(ckt->CKTstate0 + here->MEScd) = 
                            *(ckt->CKTstate1 + here->MEScd);
                    *(ckt->CKTstate0 + here->MEScgd) =
                            *(ckt->CKTstate1 + here->MEScgd);
                    *(ckt->CKTstate0 + here->MESgm) =
                            *(ckt->CKTstate1 + here->MESgm);
                    *(ckt->CKTstate0 + here->MESgds) =
                            *(ckt->CKTstate1 + here->MESgds);
                    *(ckt->CKTstate0 + here->MESggs) =
                            *(ckt->CKTstate1 + here->MESggs);
                    *(ckt->CKTstate0 + here->MESggd) =
                            *(ckt->CKTstate1 + here->MESggd);
                } else {
#endif /* PREDICTOR */
                    /*
                     *  compute new nonlinear branch voltages 
                     */
                    vgs = model->MEStype*
                        (*(ckt->CKTrhsOld+ here->MESgateNode)-
                        *(ckt->CKTrhsOld+ 
                        here->MESsourcePrimeNode));
                    vgd = model->MEStype*
                        (*(ckt->CKTrhsOld+here->MESgateNode)-
                        *(ckt->CKTrhsOld+
                        here->MESdrainPrimeNode));
#ifndef PREDICTOR
                }
#endif /* PREDICTOR */
                delvgs=vgs - *(ckt->CKTstate0 + here->MESvgs);
                delvgd=vgd - *(ckt->CKTstate0 + here->MESvgd);
                delvds=delvgs - delvgd;
                cghat= *(ckt->CKTstate0 + here->MEScg) + 
                        *(ckt->CKTstate0 + here->MESggd)*delvgd +
                        *(ckt->CKTstate0 + here->MESggs)*delvgs;
                cdhat= *(ckt->CKTstate0 + here->MEScd) +
                        *(ckt->CKTstate0 + here->MESgm)*delvgs +
                        *(ckt->CKTstate0 + here->MESgds)*delvds -
                        *(ckt->CKTstate0 + here->MESggd)*delvgd;
                /*
                 *   bypass if solution has not changed 
                 */
                if((ckt->CKTbypass) &&
                    (!(ckt->CKTmode & MODEINITPRED)) &&
                    (FABS(delvgs) < ckt->CKTreltol*MAX(FABS(vgs),
                        FABS(*(ckt->CKTstate0 + here->MESvgs)))+
                        ckt->CKTvoltTol) )
                if ( (FABS(delvgd) < ckt->CKTreltol*MAX(FABS(vgd),
                        FABS(*(ckt->CKTstate0 + here->MESvgd)))+
                        ckt->CKTvoltTol))
                if ( (FABS(cghat-*(ckt->CKTstate0 + here->MEScg)) 
                        < ckt->CKTreltol*MAX(FABS(cghat),
                        FABS(*(ckt->CKTstate0 + here->MEScg)))+
                        ckt->CKTabstol) ) if ( /* hack - expression too big */
                    (FABS(cdhat-*(ckt->CKTstate0 + here->MEScd))
                        < ckt->CKTreltol*MAX(FABS(cdhat),
                        FABS(*(ckt->CKTstate0 + here->MEScd)))+
                        ckt->CKTabstol) ) {

                    /* we can do a bypass */
                    vgs= *(ckt->CKTstate0 + here->MESvgs);
                    vgd= *(ckt->CKTstate0 + here->MESvgd);
                    vds= vgs-vgd;
                    cg= *(ckt->CKTstate0 + here->MEScg);
                    cd= *(ckt->CKTstate0 + here->MEScd);
                    cgd= *(ckt->CKTstate0 + here->MEScgd);
                    gm= *(ckt->CKTstate0 + here->MESgm);
                    gds= *(ckt->CKTstate0 + here->MESgds);
                    ggs= *(ckt->CKTstate0 + here->MESggs);
                    ggd= *(ckt->CKTstate0 + here->MESggd);
                    goto load;
                }
                /*
                 *  limit nonlinear branch voltages 
                 */
                ichk1=1;
                vgs = DEVpnjlim(vgs,*(ckt->CKTstate0 + here->MESvgs),CONSTvt0,
                        vcrit, &icheck);
                vgd = DEVpnjlim(vgd,*(ckt->CKTstate0 + here->MESvgd),CONSTvt0,
                        vcrit,&ichk1);
                if (ichk1 == 1) {
                    icheck=1;
                }
                vgs = DEVfetlim(vgs,*(ckt->CKTstate0 + here->MESvgs),
                        model->MESthreshold);
                vgd = DEVfetlim(vgd,*(ckt->CKTstate0 + here->MESvgd),
                        model->MESthreshold);
            }
            /*
             *   determine dc current and derivatives 
             */
            vds = vgs-vgd;
            if (vgs <= -5*CONSTvt0) {
                ggs = -csat/vgs+ckt->CKTgmin;
                cg = ggs*vgs;
            } else {
                evgs = exp(vgs/CONSTvt0);
                ggs = csat*evgs/CONSTvt0+ckt->CKTgmin;
                cg = csat*(evgs-1)+ckt->CKTgmin*vgs;
            }
            if (vgd <= -5*CONSTvt0) {
                ggd = -csat/vgd+ckt->CKTgmin;
                cgd = ggd*vgd;
            } else {
                evgd = exp(vgd/CONSTvt0);
                ggd = csat*evgd/CONSTvt0+ckt->CKTgmin;
                cgd = csat*(evgd-1)+ckt->CKTgmin*vgd;
            }
            cg = cg+cgd;
            /*
             *   compute drain current and derivitives for normal mode 
             */
            if (vds >= 0) {
                vgst = vgs-model->MESthreshold;
                /*
                 *   normal mode, cutoff region 
                 */
                if (vgst <= 0) {
                    cdrain = 0;
                    gm = 0;
                    gds = 0;
                } else {
                    prod = 1 + model->MESlModulation * vds;
                    betap = beta * prod;
                    denom = 1 + model->MESb * vgst;
                    invdenom = 1 / denom;
                    if (vds >= ( 3 / model->MESalpha ) ) {
                            /*
                             *   normal mode, saturation region 
                             */
                        cdrain = betap * vgst * vgst * invdenom;
                        gm = betap * vgst * (1 + denom) * invdenom * invdenom;
                        gds = model->MESlModulation * beta * vgst * vgst * 
                                invdenom;
                    } else {
                        /*
                         *   normal mode, linear region 
                         */
                        afact = 1 - model->MESalpha * vds / 3;
                        lfact = 1 - afact * afact * afact;
                        cdrain = betap * vgst * vgst * invdenom * lfact;
                        gm = betap * vgst * (1 + denom) * invdenom * invdenom *
                                lfact;
                        gds = beta * vgst * vgst * invdenom * (model->MESalpha *
                            afact * afact * prod + lfact * 
                            model->MESlModulation);
                    }
                }
            } else {
                /*
                 *   compute drain current and derivitives for inverse mode 
                 */
                vgdt = vgd - model->MESthreshold;
                if (vgdt <= 0) {
                    /*
                     *   inverse mode, cutoff region 
                     */
                    cdrain = 0;
                    gm = 0;
                    gds = 0;
                } else {
                    /*
                     *   inverse mode, saturation region 
                     */
                    prod = 1 - model->MESlModulation * vds;
                    betap = beta * prod;
                    denom = 1 + model->MESb * vgdt;
                    invdenom = 1 / denom;
                    if ( -vds >= ( 3 / model->MESalpha ) ) {
                        cdrain = -betap * vgdt * vgdt * invdenom;
                        gm = -betap * vgdt * (1 + denom) * invdenom * invdenom;
                        gds = model->MESlModulation * beta * vgdt * vgdt *
                                 invdenom-gm;
                    } else {
                        /*
                         *  inverse mode, linear region 
                         */
                        afact = 1 + model->MESalpha * vds / 3;
                        lfact = 1 - afact * afact * afact;
                        cdrain = -betap * vgdt * vgdt * invdenom * lfact;
                        gm = -betap * vgdt * (1 + denom) * invdenom * 
                                invdenom * lfact;
                        gds = beta * vgdt * vgdt * invdenom * (model->MESalpha *
                            afact * afact * prod + lfact * 
                            model->MESlModulation)-gm;
                    }
                }
            }
            /*
             *   compute equivalent drain current source 
             */
            cd = cdrain - cgd;
            if ( (ckt->CKTmode & (MODETRAN|MODEINITSMSIG)) ||
                    ((ckt->CKTmode & MODETRANOP) && (ckt->CKTmode & MODEUIC)) ){
                /* 
                 *    charge storage elements 
                 */
                czgs = model->MEScapGS * here->MESarea;
                czgd = model->MEScapGD * here->MESarea;
                phib = model->MESgatePotential;
                vgs1 = *(ckt->CKTstate1 + here->MESvgs);
                vgd1 = *(ckt->CKTstate1 + here->MESvgd);
                vcap = 1 / model->MESalpha;

                qgga = qggnew(vgs,vgd,phib,vcap,vto,czgs,czgd,&cgsna,&cgdna);
                qggb = qggnew(vgs1,vgd,phib,vcap,vto,czgs,czgd,&cgsnb,&cgdnb);
                qggc = qggnew(vgs,vgd1,phib,vcap,vto,czgs,czgd,&cgsnc,&cgdnc);
                qggd = qggnew(vgs1,vgd1,phib,vcap,vto,czgs,czgd,&cgsnd,&cgdnd);

                if(ckt->CKTmode & MODEINITTRAN) {
                    *(ckt->CKTstate1 + here->MESqgs) = qgga;
                    *(ckt->CKTstate1 + here->MESqgd) = qgga;
                }
                *(ckt->CKTstate0+here->MESqgs) = *(ckt->CKTstate1+here->MESqgs)
                        + 0.5 * (qgga-qggb + qggc-qggd);
                *(ckt->CKTstate0+here->MESqgd) = *(ckt->CKTstate1+here->MESqgd)
                        + 0.5 * (qgga-qggc + qggb-qggd);
                capgs = cgsna;
                capgd = cgdna;

                /*
                 *   store small-signal parameters 
                 */
                if( (!(ckt->CKTmode & MODETRANOP)) || 
                        (!(ckt->CKTmode & MODEUIC)) ) {
                    if(ckt->CKTmode & MODEINITSMSIG) {
                        *(ckt->CKTstate0 + here->MESqgs) = capgs;
                        *(ckt->CKTstate0 + here->MESqgd) = capgd;
                        continue; /*go to 1000*/
                    }
                    /*
                     *   transient analysis 
                     */
                    if(ckt->CKTmode & MODEINITTRAN) {
                        *(ckt->CKTstate1 + here->MESqgs) =
                                *(ckt->CKTstate0 + here->MESqgs);
                        *(ckt->CKTstate1 + here->MESqgd) =
                                *(ckt->CKTstate0 + here->MESqgd);
                    }
                    error = NIintegrate(ckt,&geq,&ceq,capgs,here->MESqgs);
                    if(error) return(error);
                    ggs = ggs + geq;
                    cg = cg + *(ckt->CKTstate0 + here->MEScqgs);
                    error = NIintegrate(ckt,&geq,&ceq,capgd,here->MESqgd);
                    if(error) return(error);
                    ggd = ggd + geq;
                    cg = cg + *(ckt->CKTstate0 + here->MEScqgd);
                    cd = cd - *(ckt->CKTstate0 + here->MEScqgd);
                    cgd = cgd + *(ckt->CKTstate0 + here->MEScqgd);
                    if (ckt->CKTmode & MODEINITTRAN) {
                        *(ckt->CKTstate1 + here->MEScqgs) =
                                *(ckt->CKTstate0 + here->MEScqgs);
                        *(ckt->CKTstate1 + here->MEScqgd) =
                                *(ckt->CKTstate0 + here->MEScqgd);
                    }
                }
            }
            /*
             *  check convergence 
             */
            if( (!(ckt->CKTmode & MODEINITFIX)) | (!(ckt->CKTmode & MODEUIC))) {
                if( (icheck == 1) 
#ifndef NEWCONV
/* XXX */
#endif /* NEWCONV */
                        || (FABS(cghat-cg) >= ckt->CKTreltol*
                            MAX(FABS(cghat),FABS(cg))+ckt->CKTabstol) ||
                        (FABS(cdhat-cd) > ckt->CKTreltol*
                            MAX(FABS(cdhat),FABS(cd))+ckt->CKTabstol) 
                        ) {
                    ckt->CKTnoncon++;
		    ckt->CKTtroubleElt = (GENinstance *) here;
                }
            }
            *(ckt->CKTstate0 + here->MESvgs) = vgs;
            *(ckt->CKTstate0 + here->MESvgd) = vgd;
            *(ckt->CKTstate0 + here->MEScg) = cg;
            *(ckt->CKTstate0 + here->MEScd) = cd;
            *(ckt->CKTstate0 + here->MEScgd) = cgd;
            *(ckt->CKTstate0 + here->MESgm) = gm;
            *(ckt->CKTstate0 + here->MESgds) = gds;
            *(ckt->CKTstate0 + here->MESggs) = ggs;
            *(ckt->CKTstate0 + here->MESggd) = ggd;
            /*
             *    load current vector
             */
load:
            ceqgd=model->MEStype*(cgd-ggd*vgd);
            ceqgs=model->MEStype*((cg-cgd)-ggs*vgs);
            cdreq=model->MEStype*((cd+cgd)-gds*vds-gm*vgs);
            *(ckt->CKTrhs + here->MESgateNode) += (-ceqgs-ceqgd);
            *(ckt->CKTrhs + here->MESdrainPrimeNode) +=
                    (-cdreq+ceqgd);
            *(ckt->CKTrhs + here->MESsourcePrimeNode) +=
                    (cdreq+ceqgs);
            /*
             *    load y matrix 
             */
            *(here->MESdrainDrainPrimePtr) += (-gdpr);
            *(here->MESgateDrainPrimePtr) += (-ggd);
            *(here->MESgateSourcePrimePtr) += (-ggs);
            *(here->MESsourceSourcePrimePtr) += (-gspr);
            *(here->MESdrainPrimeDrainPtr) += (-gdpr);
            *(here->MESdrainPrimeGatePtr) += (gm-ggd);
            *(here->MESdrainPrimeSourcePrimePtr) += (-gds-gm);
            *(here->MESsourcePrimeGatePtr) += (-ggs-gm);
            *(here->MESsourcePrimeSourcePtr) += (-gspr);
            *(here->MESsourcePrimeDrainPrimePtr) += (-gds);
            *(here->MESdrainDrainPtr) += (gdpr);
            *(here->MESgateGatePtr) += (ggd+ggs);
            *(here->MESsourceSourcePtr) += (gspr);
            *(here->MESdrainPrimeDrainPrimePtr) += (gdpr+gds+ggd);
            *(here->MESsourcePrimeSourcePrimePtr) += (gspr+gds+gm+ggs);
        }
    }
    return(OK);
}


/* function qggnew  - private, used by MESload*/
static double 
qggnew(vgs,vgd,phib,vcap,vto,cgs,cgd,cgsnew,cgdnew)
    double vgs,vgd,phib,vcap,vto,cgs,cgd,*cgsnew,*cgdnew;
{
    double veroot,veff1,veff2,del,vnroot,vnew1,vnew3,vmax,ext;
    double qroot,qggval,par1,cfact,cplus,cminus;

    veroot = sqrt( (vgs - vgd) * (vgs - vgd) + vcap*vcap );
    veff1 = 0.5 * (vgs + vgd + veroot);
    veff2 = veff1 - veroot;
    del = 0.2;
    vnroot = sqrt( (veff1 - vto)*(veff1 - vto) + del * del );
    vnew1 = 0.5 * (veff1 + vto + vnroot);
    vnew3 = vnew1;
    vmax = 0.5;
    if ( vnew1 < vmax ) {
        ext=0;
    } else {
        vnew1 = vmax;
        ext = (vnew3 - vmax)/sqrt(1 - vmax/phib);
    }

    qroot = sqrt(1 - vnew1/phib);
    qggval = cgs * (2*phib*(1-qroot) + ext) + cgd*veff2;
    par1 = 0.5 * ( 1 + (veff1-vto)/vnroot);
    cfact = (vgs- vgd)/veroot;
    cplus = 0.5 * (1 + cfact);
    cminus = cplus - cfact;
    *cgsnew = cgs/qroot*par1*cplus + cgd*cminus;
    *cgdnew = cgs/qroot*par1*cminus + cgd*cplus;
    return(qggval);
}
