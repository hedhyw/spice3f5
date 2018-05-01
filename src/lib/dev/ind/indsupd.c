/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/* update the  charge sensitivities and their derivatives */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "smpdefs.h"
#include "cktdefs.h"
#include "inddefs.h"
#include "sperror.h"
#include "suffix.h"

int
INDsUpdate(inModel,ckt)
    GENmodel *inModel;
    register CKTcircuit *ckt;
{
    register INDmodel *model = (INDmodel*)inModel;
    register INDinstance *here;
    double   cind; 
    double sxp;
    double  s1;
    double  s2;
    double  s;
    int iparmno;
    double  dummy1;
    double  dummy2;
    SENstruct *info;
#ifdef MUTUAL
    register MUTinstance *muthere;
    register MUTmodel *mutmodel;
    double sxp1;
    double sxp2;
    double   cind1,cind2;
    double   rootl1,rootl2;
    int ktype;
    int itype;
#endif

    info = ckt->CKTsenInfo;
    if(ckt->CKTmode & MODEINITTRAN) return(OK);

    /*  loop through all the inductor models */
    for( ; model != NULL; model = model->INDnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->INDinstances; here != NULL ;
                here=here->INDnextInstance) {

            for(iparmno = 1;iparmno<=info->SENparms;iparmno++){

                cind = *(ckt->CKTrhsOld + here->INDbrEq);
                s = *(info->SEN_Sap[here->INDbrEq] + iparmno);
                sxp = here->INDinduct * s;

#ifdef SENSDEBUG
                printf("iparmno = %d,s=%.5e,cind = %.5e\n",iparmno,s,cind);
                printf("sxp(before mut) = %.5e\n",sxp);
#endif /* SENSDEBUG */

                if(iparmno == here->INDsenParmNo) sxp += cind;


                *(ckt->CKTstate0 + here->INDsensxp + 2 * (iparmno - 1)) = sxp;
            }


#ifdef MUTUAL
        }
    }
    ktype = CKTtypelook("mutual");
    mutmodel = (MUTmodel *)(ckt->CKThead[ktype]);
    /*  loop through all the mutual inductor models */
    for( ; mutmodel != NULL; mutmodel = mutmodel->MUTnextModel ) {

        /* loop through all the instances of the model */
        for (muthere = mutmodel->MUTinstances; muthere != NULL ;
                muthere=muthere->MUTnextInstance) {


            cind1 = *(ckt->CKTrhsOld + muthere->MUTind1->INDbrEq);
            cind2 = *(ckt->CKTrhsOld + muthere->MUTind2->INDbrEq);
            rootl1 = sqrt( muthere->MUTind1->INDinduct );
            rootl2 = sqrt( muthere->MUTind2->INDinduct );

            for(iparmno = 1;iparmno<=info->SENparms;iparmno++){
                s1 = *(info->SEN_Sap[muthere->MUTind1->INDbrEq] + iparmno);
                s2 = *(info->SEN_Sap[muthere->MUTind2->INDbrEq] + iparmno);
                sxp2 = muthere->MUTcoupling*rootl1*rootl2 * s1;
                sxp1 = muthere->MUTcoupling*rootl1*rootl2 * s2;
                if(iparmno == muthere->MUTsenParmNo){
                    sxp1 += cind2 * rootl1 * rootl2;
                    sxp2 += cind1 * rootl1 * rootl2;
                }
                if(iparmno == muthere->MUTind1->INDsenParmNo){
                    sxp1 += cind2 * muthere->MUTcoupling * rootl2 /(2 * rootl1);
                    sxp2 += cind1 * muthere->MUTcoupling * rootl2 /(2 * rootl1);
                }
                if(iparmno == muthere->MUTind2->INDsenParmNo){
                    sxp1 += cind2 * muthere->MUTcoupling * rootl1 /(2 * rootl2);
                    sxp2 += cind1 * muthere->MUTcoupling * rootl1 /(2 * rootl2);
                }


                *(ckt->CKTstate0 + muthere->MUTind1->INDsensxp + 2 * 
                        (iparmno - 1)) += sxp1;

                *(ckt->CKTstate0 + muthere->MUTind2->INDsensxp + 2 * 
                        (iparmno - 1)) += sxp2;

#ifdef SENSDEBUG
                printf("iparmno = %d\n",iparmno);
                printf("sxp1 = %.5e,sxp2 = %.5e\n",sxp1,sxp2);
#endif /* SENSDEBUG */
            }

        }
    }


    itype = CKTtypelook("Inductor");
    model = (INDmodel *)(ckt->CKThead[itype]);
    /*  loop through all the inductor models */
    for( ; model != NULL; model = model->INDnextModel ) {

        /* loop through all the instances of the model */
        for (here = model->INDinstances; here != NULL ;
                here=here->INDnextInstance) {
#endif /* MUTUAL */
                for(iparmno = 1;iparmno<=info->SENparms;iparmno++){

                if(ckt->CKTmode&MODETRANOP){
                    *(ckt->CKTstate0 + here->INDsensxp + 2 * 
                            (iparmno - 1) + 1) = 0;
                } else{
                    NIintegrate(ckt,&dummy1,&dummy2,here->INDinduct,
                            (here->INDsensxp + 2*(iparmno - 1)));
                }
#ifdef SENSDEBUG
                printf("sxp = %.5e,sdotxp = %.5e\n",
                        *(ckt->CKTstate0 + here->INDsensxp + 2 * 
                        (iparmno - 1)),
                        *(ckt->CKTstate0 + here->INDsensxp + 2 *
                        (iparmno - 1) + 1));
#endif /* SENSDEBUG */
            }
        }
    }
    return(OK);
}
