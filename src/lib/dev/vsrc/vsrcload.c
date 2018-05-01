/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "cktdefs.h"
#include "vsrcdefs.h"
#include "trandefs.h"
#include "sperror.h"
#include "suffix.h"

int
VSRCload(inModel,ckt)
    GENmodel *inModel;
    register CKTcircuit *ckt;
        /* actually load the current voltage value into the 
         * sparse matrix previously provided 
         */
{
    register VSRCmodel *model = (VSRCmodel *)inModel;
    register VSRCinstance *here;
    double time;

    /*  loop through all the voltage source models */
    for( ; model != NULL; model = model->VSRCnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->VSRCinstances; here != NULL ;
                here=here->VSRCnextInstance) {
            
            *(here->VSRCposIbrptr) += 1.0 ;
            *(here->VSRCnegIbrptr) -= 1.0 ;
            *(here->VSRCibrPosptr) += 1.0 ;
            *(here->VSRCibrNegptr) -= 1.0 ;
            if( (ckt->CKTmode & (MODEDCOP | MODEDCTRANCURVE)) &&
                    here->VSRCdcGiven ) {
                /* grab dc value */
                *(ckt->CKTrhs + (here->VSRCbranch)) += ckt->CKTsrcFact * 
                        here->VSRCdcValue;
            } else {
                if(ckt->CKTmode & (MODEDC)) {
                    time = 0;
                } else {
                    time = ckt->CKTtime;
                }
                /* use the transient functions */
                switch(here->VSRCfunctionType) {
                default: { /* no function specified:   use the DC value */
                    *(ckt->CKTrhs + (here->VSRCbranch)) += here->VSRCdcValue;
                    break;
                }
                
                case PULSE: {
		    double	V1, V2, TD, TR, TF, PW, PER;
                    double	basetime = 0;

		    V1 = here->VSRCcoeffs[0];
		    V2 = here->VSRCcoeffs[1];
		    TD = here->VSRCfunctionOrder > 2
			? here->VSRCcoeffs[2] : 0.0;
		    TR = here->VSRCfunctionOrder > 3
			&& here->VSRCcoeffs[3] != 0.0
			? here->VSRCcoeffs[3] : ckt->CKTstep;
		    TF = here->VSRCfunctionOrder > 4
			&& here->VSRCcoeffs[4] != 0.0
			? here->VSRCcoeffs[4] : ckt->CKTstep;
		    PW = here->VSRCfunctionOrder > 5
			&& here->VSRCcoeffs[5] != 0.0
			? here->VSRCcoeffs[5] : ckt->CKTfinalTime;
		    PER = here->VSRCfunctionOrder > 6
			&& here->VSRCcoeffs[6] != 0.0
			? here->VSRCcoeffs[6] : ckt->CKTfinalTime;

                    time -= TD;
                    if(time > PER) {
                        /* repeating signal - figure out where we are */
                        /* in period */
                        basetime = PER * floor(time/PER);
                        time -= basetime;
                    }
                    if (time <= 0 || time >= TR + PW + TF) {
                        ckt->CKTrhs[here->VSRCbranch] += V1;
                    } else  if (time >= TR && time <= TR + PW) {
                        ckt->CKTrhs[here->VSRCbranch] += V2;
                    } else if (time > 0 && time < TR) {
                        ckt->CKTrhs[here->VSRCbranch] += 
                                V1 + (V2 - V1) * (time) / TR;
                    } else { /* time > TR + PW && < TR + PW + TF */
                        ckt->CKTrhs[here->VSRCbranch] += 
                                V2 + (V1 - V2) * (time - (TR + PW)) / TF;
                    }

                }
                break;

                case SINE: {
#define VO    (*(here->VSRCcoeffs))
#define VA    (*(here->VSRCcoeffs+1))
#define FREQ  (((here->VSRCfunctionOrder >=3) && (*(here->VSRCcoeffs+2)))? \
    (*(here->VSRCcoeffs+2)):(1/ckt->CKTfinalTime))
#define TD    ((here->VSRCfunctionOrder >=4)?(*(here->VSRCcoeffs+3)):(0.0))
#define THETA ((here->VSRCfunctionOrder >=5)?(*(here->VSRCcoeffs+4)):(0.0))
                    time -= TD;
                    if (time <= 0) {
                        *(ckt->CKTrhs + (here->VSRCbranch)) += VO;
                    } else {
                        *(ckt->CKTrhs + (here->VSRCbranch)) += 
                                VO + VA * sin(FREQ * time * 2.0 * M_PI) * 
                                exp(-(time*THETA));
                                /* 2PI to convert from hz to radians/sec*/
                    }
#undef VO
#undef VA
#undef FREQ
#undef TD
#undef THETA
                }
                break;

                case EXP: {
                    double td1;
                    double td2;
#define V1 (*(here->VSRCcoeffs))
#define V2 (*(here->VSRCcoeffs+1))
#define TD1 ((here->VSRCfunctionOrder >=3)?(*(here->VSRCcoeffs+2)):\
    ckt->CKTstep)
#define TAU1 (((here->VSRCfunctionOrder >=4) && (*(here->VSRCcoeffs+3)))? \
    (*(here->VSRCcoeffs+3)):ckt->CKTstep)
#define TD2 (((here->VSRCfunctionOrder >=5) && (*(here->VSRCcoeffs+4)))? \
    (*(here->VSRCcoeffs+4)):TD1+ckt->CKTstep)
#define TAU2 (((here->VSRCfunctionOrder >=6) && (*(here->VSRCcoeffs+5)))? \
    (*(here->VSRCcoeffs+5)):ckt->CKTstep)
                    td1 = TD1;
                    td2 = TD2;
                    if(time <= td1)  {
                        *(ckt->CKTrhs + (here->VSRCbranch)) += V1;
                    } else if (time <= td2) {
                        *(ckt->CKTrhs + (here->VSRCbranch)) += 
                                V1 + (V2-V1)*(1-exp(-(time-td1)/TAU1));
                    } else {
                        *(ckt->CKTrhs + (here->VSRCbranch)) += 
                                V1 + (V2-V1)*(1-exp(-(time-td1)/TAU1)) +
                                     (V1-V2)*(1-exp(-(time-td2)/TAU2)) ;
                    }
#undef V1
#undef V2
#undef TD1
#undef TAU1
#undef TD2
#undef TAU2
                }
                break;

                case SFFM:{
#define VO (*(here->VSRCcoeffs))
#define VA (*(here->VSRCcoeffs+1))
#define FC (((here->VSRCfunctionOrder >=3) && (*(here->VSRCcoeffs+2)))? \
    (*(here->VSRCcoeffs+2)):(1/ckt->CKTfinalTime))
#define MDI ((here->VSRCfunctionOrder>=4)?(*(here->VSRCcoeffs+3)):\
    0.0)
#define FS (((here->VSRCfunctionOrder >=5) && (*(here->VSRCcoeffs+4)))? \
    (*(here->VSRCcoeffs+4)):(1/ckt->CKTfinalTime))
                    *(ckt->CKTrhs + (here->VSRCbranch)) += VO + VA * 
                                sin((2 * 3.141592654 * FC * time) +
                                MDI * sin(2 * 3.141592654 * FS * time));
#undef VO
#undef VA
#undef FC
#undef MDI
#undef FS
                }
                break;

                case PWL: {
                    register int i;
                    double foo;
                    if(time < *(here->VSRCcoeffs)) {
                        foo = *(here->VSRCcoeffs + 1) ;
                        *(ckt->CKTrhs + (here->VSRCbranch)) += foo;
                        goto loadDone;
                    }
                    for(i=0;i<(here->VSRCfunctionOrder/2)-1;i++) {
                        if((*(here->VSRCcoeffs+2*i)==time)) {
                            foo = *(here->VSRCcoeffs+2*i+1);
                            *(ckt->CKTrhs + (here->VSRCbranch)) +=
                                foo;
                            goto loadDone;
                        } else if((*(here->VSRCcoeffs+2*i)<time) &&
                                (*(here->VSRCcoeffs+2*(i+1)) >time)) {
                            foo = *(here->VSRCcoeffs+2*i+1) +
                                (((time-*(here->VSRCcoeffs+2*i))/
                                (*(here->VSRCcoeffs+2*(i+1)) - 
                                 *(here->VSRCcoeffs+2*i))) *
                                (*(here->VSRCcoeffs+2*i+3) - 
                                 *(here->VSRCcoeffs+2*i+1)));
                            *(ckt->CKTrhs + (here->VSRCbranch)) +=
                                foo;
                            goto loadDone;
                        }
                    }
                    foo = *(here->VSRCcoeffs+ here->VSRCfunctionOrder-1) ;
                    *(ckt->CKTrhs + (here->VSRCbranch)) += foo;
                    break;
                }
                }
            }
loadDone: ;
        }
    }
    return(OK);
}
