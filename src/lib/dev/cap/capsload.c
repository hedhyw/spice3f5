/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

/* actually load the current sensitivity 
 * information into the  array previously provided 
 */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "cktdefs.h"
#include "capdefs.h"
#include "trandefs.h"
#include "sperror.h"
#include "suffix.h"


int
CAPsLoad(inModel,ckt)
GENmodel *inModel;
register CKTcircuit *ckt;
{
    register CAPmodel *model = (CAPmodel*)inModel;
    register CAPinstance *here;
    int      iparmno;
    double   vcap;
    double   Osxp; 
    double   tag0; 
    double   tag1; 
    SENstruct *info;

    info = ckt->CKTsenInfo;
    if((info->SENmode == DCSEN) || (ckt->CKTmode&MODETRANOP)) 
        return( OK );
    if((info->SENmode == TRANSEN) && (ckt->CKTmode & MODEINITTRAN))
        return(OK);


#ifdef SENSDEBUG
    printf("CKTtime = %.5e\n",ckt->CKTtime);
    printf("CAPsenload \n");
#endif /* SENSDEBUG */

    tag0 = ckt->CKTag[0];
    tag1 = ckt->CKTag[1];
    if(ckt->CKTorder == 1){
        /* Euler Method */
        tag1 = 0; /* we treat tag1 as beta */
    }

    /*  loop through all the capacitor models */
    for( ; model != NULL; model = model->CAPnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->CAPinstances; here != NULL ;
                here=here->CAPnextInstance) {

#ifdef SENSDEBUG
            printf("senload instance name %s\n",here->CAPname);
            printf("pos = %d , neg = %d \n",here->CAPposNode ,here->CAPnegNode);
#endif /* SENSDEBUG */

            vcap = *(ckt->CKTrhsOld+here->CAPposNode) 
                - *(ckt->CKTrhsOld+here->CAPnegNode) ;   

            for(iparmno=1;iparmno<=info->SENparms;iparmno++){
                Osxp = tag0 * *(ckt->CKTstate1 + here->CAPsensxp
                            + 2*(iparmno - 1))
                    + tag1 * *(ckt->CKTstate1 + here->CAPsensxp
                            + 2*(iparmno - 1) + 1);
#ifdef SENSDEBUG
                printf("iparmno = %d\n",iparmno);
                printf("sxp = %.5e\n"
                        ,*(ckt->CKTstate1 + here->CAPsensxp + 2*(iparmno-1)));
                printf("sdotxp = %.5e\n",
                        *(ckt->CKTstate1 + here->CAPsensxp + 2*(iparmno-1)+1));
                printf("Osxp = %.5e\n",Osxp);
#endif /* SENSDEBUG */

                if(iparmno == here->CAPsenParmNo)
                    Osxp = Osxp - tag0 * vcap;
#ifdef  SENSDEBUG 
                printf("Osxp = %.5e\n",Osxp);
#endif /* SENSDEBUG */

                *(info->SEN_RHS[here->CAPposNode] + iparmno) += Osxp;
                *(info->SEN_RHS[here->CAPnegNode] + iparmno) -= Osxp;

            }
        }
    }
    return(OK);
}

