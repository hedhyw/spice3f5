/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "devdefs.h"
#include "capdefs.h"
#include "ifsim.h"
#include "sperror.h"
#include "util.h"
#include "suffix.h"

/* ARGSUSED */
int
CAPask(ckt,inst, which, value, select)
    CKTcircuit *ckt;
    GENinstance *inst;
    int which;
    IFvalue *value;  
    IFvalue *select;  
{
    CAPinstance *here = (CAPinstance *)inst;
    double vr;
    double vi;
    double sr;
    double si;
    double vm;
    static char *msg = "Current and power not available for ac analysis";

    switch(which) {
        case CAP_CAP:
        value->rValue=here->CAPcapac;
            return(OK);
        case CAP_IC:
            value->rValue = here->CAPinitCond;
            return(OK);
        case CAP_WIDTH:
            value->rValue = here->CAPwidth;
            return(OK);
        case CAP_LENGTH:
            value->rValue = here->CAPlength;
            return(OK);
        case CAP_CURRENT:
            if (ckt->CKTcurrentAnalysis & DOING_AC) {
                errMsg = MALLOC(strlen(msg)+1);
                errRtn = "CAPask";
                strcpy(errMsg,msg);
                return(E_ASKCURRENT);
            } else if (ckt->CKTcurrentAnalysis & (DOING_DCOP | DOING_TRCV)) {
                value->rValue = 0;
            } else if (ckt->CKTcurrentAnalysis & DOING_TRAN) {
                if (ckt->CKTmode & MODETRANOP) {
                    value->rValue = 0;
                } else {
                    value->rValue = *(ckt->CKTstate0 + here->CAPccap);
                }
            }
            return(OK);
        case CAP_POWER:
            if (ckt->CKTcurrentAnalysis & DOING_AC) {
                errMsg = MALLOC(strlen(msg)+1);
                errRtn = "CAPask";
                strcpy(errMsg,msg);
                return(E_ASKPOWER);
            } else if (ckt->CKTcurrentAnalysis & (DOING_DCOP | DOING_TRCV)) {
                value->rValue = 0;
            } else if (ckt->CKTcurrentAnalysis & DOING_TRAN) {
                if (ckt->CKTmode & MODETRANOP) {
                    value->rValue = 0;
                } else {
                    value->rValue = *(ckt->CKTstate0 + here->CAPccap) *
                            (*(ckt->CKTrhsOld + here->CAPposNode) - 
                            *(ckt->CKTrhsOld + here->CAPnegNode));
                }
            }
            return(OK);
        case CAP_QUEST_SENS_DC:
            if(ckt->CKTsenInfo){
               value->rValue = *(ckt->CKTsenInfo->SEN_Sap[select->iValue + 1]+
                    here->CAPsenParmNo);
            }
            return(OK);
        case CAP_QUEST_SENS_REAL:
            if(ckt->CKTsenInfo){
               value->rValue = *(ckt->CKTsenInfo->SEN_RHS[select->iValue + 1]+
                    here->CAPsenParmNo);
            }
            return(OK);
        case CAP_QUEST_SENS_IMAG:
            if(ckt->CKTsenInfo){
               value->rValue = *(ckt->CKTsenInfo->SEN_iRHS[select->iValue + 1]+
                    here->CAPsenParmNo);
            }
            return(OK);
        case CAP_QUEST_SENS_MAG:
            if(ckt->CKTsenInfo){
               vr = *(ckt->CKTrhsOld + select->iValue + 1); 
               vi = *(ckt->CKTirhsOld + select->iValue + 1); 
               vm = sqrt(vr*vr + vi*vi);
               if(vm == 0){
                 value->rValue = 0;
                 return(OK);
               }
               sr = *(ckt->CKTsenInfo->SEN_RHS[select->iValue + 1]+
                    here->CAPsenParmNo);
               si = *(ckt->CKTsenInfo->SEN_iRHS[select->iValue + 1]+
                    here->CAPsenParmNo);
                   value->rValue = (vr * sr + vi * si)/vm;
            }
            return(OK);
        case CAP_QUEST_SENS_PH:
            if(ckt->CKTsenInfo){
               vr = *(ckt->CKTrhsOld + select->iValue + 1); 
               vi = *(ckt->CKTirhsOld + select->iValue + 1); 
               vm = vr*vr + vi*vi;
               if(vm == 0){
                 value->rValue = 0;
                 return(OK);
               }
               sr = *(ckt->CKTsenInfo->SEN_RHS[select->iValue + 1]+
                    here->CAPsenParmNo);
               si = *(ckt->CKTsenInfo->SEN_iRHS[select->iValue + 1]+
                    here->CAPsenParmNo);

                   value->rValue = (vr * si - vi * sr)/vm;
            }
            return(OK);

        case CAP_QUEST_SENS_CPLX:
            if(ckt->CKTsenInfo){
                value->cValue.real= 
                        *(ckt->CKTsenInfo->SEN_RHS[select->iValue + 1]+
                        here->CAPsenParmNo);
                value->cValue.imag= 
                        *(ckt->CKTsenInfo->SEN_iRHS[select->iValue + 1]+
                        here->CAPsenParmNo);
            }
            return(OK);

        default:
            return(E_BADPARM);
        }
}

