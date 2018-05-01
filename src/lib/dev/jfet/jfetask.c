/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1987 Mathew Lew and Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "const.h"
#include "ifsim.h"
#include "cktdefs.h"
#include "devdefs.h"
#include "jfetdefs.h"
#include "sperror.h"
#include "util.h"
#include "suffix.h"


/*ARGSUSED*/
int
JFETask(ckt,inst,which,value,select)
    CKTcircuit *ckt;
    GENinstance *inst;
    int which;
    IFvalue *value;
    IFvalue *select;
{
    JFETinstance *here = (JFETinstance*)inst;
    static char *msg = "Current and power not available for ac analysis";
    switch(which) {
        case JFET_TEMP:
            value->rValue = here->JFETtemp-CONSTCtoK;
            return(OK);
        case JFET_AREA:
            value->rValue = here->JFETarea;
            return(OK);
        case JFET_IC_VDS:
            value->rValue = here->JFETicVDS;
            return(OK);
        case JFET_IC_VGS:
            value->rValue = here->JFETicVGS;
            return(OK);
        case JFET_OFF:
            value->iValue = here->JFEToff;
            return(OK);
        case JFET_DRAINNODE:
            value->iValue = here->JFETdrainNode;
            return(OK);
        case JFET_GATENODE:
            value->iValue = here->JFETgateNode;
            return(OK);
        case JFET_SOURCENODE:
            value->iValue = here->JFETsourceNode;
            return(OK);
        case JFET_DRAINPRIMENODE:
            value->iValue = here->JFETdrainPrimeNode;
            return(OK);
        case JFET_SOURCEPRIMENODE:
            value->iValue = here->JFETsourcePrimeNode;
            return(OK);
        case JFET_VGS:
            value->rValue = *(ckt->CKTstate0 + here->JFETvgs);
            return(OK);
        case JFET_VGD:
            value->rValue = *(ckt->CKTstate0 + here->JFETvgd);
            return(OK);
        case JFET_CG:
            value->rValue = *(ckt->CKTstate0 + here->JFETcg);
            return(OK);
        case JFET_CD:
            value->rValue = *(ckt->CKTstate0 + here->JFETcd);
            return(OK);
        case JFET_CGD:
            value->rValue = *(ckt->CKTstate0 + here->JFETcgd);
            return(OK);
        case JFET_GM:
            value->rValue = *(ckt->CKTstate0 + here->JFETgm);
            return(OK);
        case JFET_GDS:
            value->rValue = *(ckt->CKTstate0 + here->JFETgds);
            return(OK);
        case JFET_GGS:
            value->rValue = *(ckt->CKTstate0 + here->JFETggs);
            return(OK);
        case JFET_GGD:
            value->rValue = *(ckt->CKTstate0 + here->JFETggd);
            return(OK);
        case JFET_QGS:
            value->rValue = *(ckt->CKTstate0 + here->JFETqgs);
            return(OK);
        case JFET_CQGS:
            value->rValue = *(ckt->CKTstate0 + here->JFETcqgs);
            return(OK);
        case JFET_QGD:
            value->rValue = *(ckt->CKTstate0 + here->JFETqgd);
            return(OK);
        case JFET_CQGD:
            value->rValue = *(ckt->CKTstate0 + here->JFETcqgd);
            return(OK);
        case JFET_CS :
            if (ckt->CKTcurrentAnalysis & DOING_AC) {
                errMsg = MALLOC(strlen(msg)+1);
                errRtn = "JFETask";
                strcpy(errMsg,msg);
                return(E_ASKCURRENT);
            } else {
                value->rValue = -*(ckt->CKTstate0 + here->JFETcd);
                value->rValue -= *(ckt->CKTstate0 + here->JFETcg);
            }
            return(OK);
        case JFET_POWER :
            if (ckt->CKTcurrentAnalysis & DOING_AC) {
                errMsg = MALLOC(strlen(msg)+1);
                errRtn = "JFETask";
                strcpy(errMsg,msg);
                return(E_ASKPOWER);
            } else {
                value->rValue = *(ckt->CKTstate0 + here->JFETcd) *
                        *(ckt->CKTrhsOld + here->JFETdrainNode);
                value->rValue += *(ckt->CKTstate0 + here->JFETcg) * 
                        *(ckt->CKTrhsOld + here->JFETgateNode);
                value->rValue -= (*(ckt->CKTstate0 + here->JFETcd) +
                        *(ckt->CKTstate0 + here->JFETcg)) *
                *(ckt->CKTrhsOld + here->JFETsourceNode);
            }
            return(OK);
        default:
            return(E_BADPARM);
    }
    /* NOTREACHED */
}

