/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

/* update the  charge sensitivities and their derivatives */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "cktdefs.h"
#include "capdefs.h"
#include "trandefs.h"
#include "sperror.h"
#include "suffix.h"


int
CAPsUpdate(inModel,ckt)
    GENmodel *inModel;
    register CKTcircuit *ckt;
{
    register CAPmodel *model = (CAPmodel*)inModel;
    register CAPinstance *here;
    int      iparmno; 
    double   s1; 
    double   s2; 
    double   sxp; 
    double   vcap;
    double   dummy1; 
    double   dummy2; 
    SENstruct *info;

    info = ckt->CKTsenInfo;
    if((info->SENmode == TRANSEN) && (ckt->CKTmode & MODEINITTRAN))
        return(OK);

#ifdef SENSDEBUG
    printf("CAPsenUpdate\n");
    printf("CKTtime = %.5e\n",ckt->CKTtime);
#endif /* SENSDEBUG */

    /*  loop through all the capacitor models */
    for( ; model != NULL; model = model->CAPnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->CAPinstances; here != NULL ;
                here=here->CAPnextInstance) {

            vcap = *(ckt->CKTrhsOld+here->CAPposNode) - 
                *(ckt->CKTrhsOld+here->CAPnegNode) ;   


            for(iparmno=1;iparmno<=info->SENparms;iparmno++){

                s1 = *(info->SEN_Sap[here->CAPposNode] + iparmno);
                s2 = *(info->SEN_Sap[here->CAPnegNode] + iparmno);
                sxp = here->CAPcapac * (s1 - s2);

                if(iparmno == here->CAPsenParmNo) sxp += vcap;

                *(ckt->CKTstate0 + here->CAPsensxp + 2*(iparmno - 1)) = sxp; 

                if(ckt->CKTtime == 0){
                    *(ckt->CKTstate0 + here->CAPsensxp + 2*(iparmno - 1) + 1)=0;
                }
                else{
                    NIintegrate(ckt,&dummy1,&dummy2,here->CAPcapac,
                            (here->CAPsensxp + 2 * (iparmno -1 )));
                }
#ifdef SENSDEBUG
                printf("after loading\n");
                printf("iparmno = %d\n",iparmno);
                printf("s1 = %.7e,s2 = %.7e\n",s1,s2);
                printf("sxp = %.7e,sdotxp = %.7e\n",
                        sxp,*(ckt->CKTstate0 + here->CAPsensxp + 
                        2*(iparmno - 1) + 1));
                printf("\n");
#endif /* SENSDEBUG */

            }
        }
    }
    return(OK);
}

