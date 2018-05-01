/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "vsrcdefs.h"
#include "trandefs.h"
#include "util.h"
#include "sperror.h"
#include "suffix.h"

int
VSRCaccept(ckt,inModel)
    register CKTcircuit *ckt;
    GENmodel *inModel;
        /* set up the breakpoint table.
         */
{
    register VSRCmodel *model = (VSRCmodel *)inModel;
    register VSRCinstance *here;
    int error;

    /*  loop through all the voltage source models */
    for( ; model != NULL; model = model->VSRCnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->VSRCinstances; here != NULL ;
                here=here->VSRCnextInstance) {
            
            if(!ckt->CKTmode & (MODETRAN | MODETRANOP)) {
                /* not transient, so shouldn't be here */
                return(OK);
            } else {
                /* use the transient functions */
                switch(here->VSRCfunctionType) {
                default: { /* no function specified:DC   no breakpoints */
                    break;
                }
                
                case PULSE: {
		    double	TD, TR, TF, PW, PER;
                    double time;
                    double basetime = 0;

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

/*
 #define TD ((here->VSRCfunctionOrder >=3)?(*(here->VSRCcoeffs+2)):\
    (0.0))
 #define TR ((here->VSRCfunctionOrder >=4)?(*(here->VSRCcoeffs+3)):\
    (ckt->CKTstep))
 #define TF ((here->VSRCfunctionOrder >=5)?(*(here->VSRCcoeffs+4)):\
    (ckt->CKTstep))
 #define PW ((here->VSRCfunctionOrder >=6)?(*(here->VSRCcoeffs+5)):\
    (ckt->CKTfinalTime))
 #define PER ((here->VSRCfunctionOrder>=7)?(*(here->VSRCcoeffs+6)):\
    (ckt->CKTfinalTime))
*/

#define SAMETIME(a,b) (FABS((a)-(b))<= TIMETOL * PW)
#define TIMETOL 1e-7

                    time = ckt->CKTtime - TD;
                    /* if(ckt->CKTtime >= PER) XXX was this */
                    if(time >= PER) {
                        /* repeating signal - figure out where we are */
                        /* in period */
                        basetime = PER * floor(time/PER);
                        time -= basetime;
                    }
                    if( time <= 0 || time >= TR + PW + TF) {
                        if(ckt->CKTbreak &&  SAMETIME(time,0)) {
                            /* set next breakpoint */
                            error = CKTsetBreak(ckt,basetime + TR +TD);
                            if(error) return(error);
                        } else if(ckt->CKTbreak && SAMETIME(TR+PW+TF,time) ) {
                            /* set next breakpoint */
                            error = CKTsetBreak(ckt,basetime + PER + TD);
                            if(error) return(error);
                        } else if (ckt->CKTbreak && (time == -TD) ) {
                            /* set next breakpoint */
                            error = CKTsetBreak(ckt,basetime + TD);
                            if(error) return(error);
                        } else if (ckt->CKTbreak && SAMETIME(PER,time) ) {
                            /* set next breakpoint */
                            error = CKTsetBreak(ckt,basetime + TD + TR + PER);
                            if(error) return(error);
                        }
                    } else  if ( time >= TR && time <= TR + PW) {
                        if(ckt->CKTbreak &&  SAMETIME(time,TR) ) {
                            /* set next breakpoint */
                            error = CKTsetBreak(ckt,basetime + TD+TR + PW);
                            if(error) return(error);
                        } else if(ckt->CKTbreak &&  SAMETIME(TR+PW,time) ) {
                            /* set next breakpoint */
                            error = CKTsetBreak(ckt,basetime + TD+TR + PW + TF);
                            if(error) return(error);
                        }
                    } else if (time > 0 && time < TR) {
                        if(ckt->CKTbreak && SAMETIME(time,0) ) {
                            /* set next breakpoint */
                            error = CKTsetBreak(ckt,basetime + TD+TR);
                            if(error) return(error);
                        } else if(ckt->CKTbreak && SAMETIME(time,TR)) {
                            /* set next breakpoint */
                            error = CKTsetBreak(ckt,basetime + TD+TR + PW);
                            if(error) return(error);
                        }
                    } else { /* time > TR + PW && < TR + PW + TF */
                        if(ckt->CKTbreak && SAMETIME(time,TR+PW) ) {
                            /* set next breakpoint */
                            error = CKTsetBreak(ckt,basetime + TD+TR + PW +TF);
                            if(error) return(error);
                        } else if(ckt->CKTbreak && SAMETIME(time,TR+PW+TF) ) {
                            /* set next breakpoint */
                            error = CKTsetBreak(ckt,basetime + TD+PER);
                            if(error) return(error);
                        }
                    }
                }
                break;

                case SINE: {
                    /* no  breakpoints (yet) */
                }
                break;
                case EXP: {
                    /* no  breakpoints (yet) */
                }
                break;
                case SFFM:{
                    /* no  breakpoints (yet) */
                }
                break;
                case PWL: {
                    register int i;
                    if(ckt->CKTtime < *(here->VSRCcoeffs)) {
                        if(ckt->CKTbreak) {
                            error = CKTsetBreak(ckt,*(here->VSRCcoeffs));
                            break;
                        }
                    }
                    for(i=0;i<(here->VSRCfunctionOrder/2)-1;i++) {
                        if((*(here->VSRCcoeffs+2*i)==ckt->CKTtime)) {
                            if(ckt->CKTbreak) {
                                error = CKTsetBreak(ckt,
                                        *(here->VSRCcoeffs+2*i+2));
                                if(error) return(error);
                            }
                            goto bkptset;
                        } 
                    }
                    break;
                }
                }
            }
bkptset: ;
        }
    }
    return(OK);
}
