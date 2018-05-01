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
#include "smpdefs.h"
#include "cktdefs.h"
#include "diodefs.h"
#include "sperror.h"
#include "suffix.h"


int
DIOsUpdate(inModel,ckt)
    GENmodel *inModel;
    register CKTcircuit *ckt;
{
    register DIOmodel *model = (DIOmodel*)inModel;
    register DIOinstance *here;
    int      iparmno;
    double   sposprm;
    double   sneg;
    double   sxp;
    double   dummy1;
    double   dummy2;
    SENstruct *info;

    info = ckt->CKTsenInfo;
    if(ckt->CKTtime == 0) return(OK);
    dummy1=0;
    dummy2=0;

#ifdef SENSDEBUG
    printf("DIOsenUpdate\n");
    printf("CKTtime = %.5e\n",ckt->CKTtime);
    printf("CKTorder = %.5e\n",ckt->CKTorder);
#endif /* SENSDEBUG */

    /*  loop through all the diode models */
    for( ; model != NULL; model = model->DIOnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->DIOinstances; here != NULL ;
                here=here->DIOnextInstance) {

#ifdef SENSDEBUG
            printf("capd = %.7e \n",here->DIOcap);
#endif /* SENSDEBUG */

            for(iparmno = 1;iparmno<=info->SENparms;iparmno++){

                sposprm = *(info->SEN_Sap[here->DIOposPrimeNode] + iparmno);
                sneg = *(info->SEN_Sap[here->DIOnegNode] + iparmno);
                sxp = (sposprm - sneg) * here->DIOcap;
                if(iparmno == here->DIOsenParmNo) sxp += *(here->DIOdphidp);
                *(ckt->CKTstate0 + here->DIOsensxp + 2 * (iparmno - 1)) = sxp;
                NIintegrate(ckt,&dummy1,&dummy2,here->DIOcap,
                        (here->DIOsensxp + 2 * (iparmno -1 )));

                if(ckt->CKTmode & MODEINITTRAN){
                    *(ckt->CKTstate1 + here->DIOsensxp + 2*(iparmno - 1)) = sxp;
                    *(ckt->CKTstate1 + here->DIOsensxp + 2*(iparmno - 1)+1) = 0;
                }
#ifdef SENSDEBUG
                printf("iparmno = %d\n",iparmno);
                printf("sposprm = %.7e,sneg = %.7e\n",sposprm,sneg);
                printf("sxp = %.7e,sdotxp = %.7e\n",
                        sxp,*(ckt->CKTstate0 + here->DIOsensxp + 
                        2*(iparmno - 1) + 1));
#endif /* SENSDEBUG */
            }
        }
    }
    return(OK);
}

