/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

    /*
     *CKTnodName(ckt)
     *  output information on all circuit nodes/equations
     *
     */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "suffix.h"


IFuid
CKTnodName(ckt,nodenum)
    CKTcircuit *ckt;
    register int nodenum;
{
    register CKTnode *here;

    for(here = ckt->CKTnodes;here; here = here->next) {
        if(here->number == nodenum) { 
            /* found it */
            return(here->name);
        }
    }
    /* doesn't exist - do something */
    return("UNKNOWN NODE");
}
