/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

    /*  CKTbreakDump(ckt) - dump the breakpoint table associated with
     *  the given circuit 
     */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "util.h"
#include "suffix.h"


void
CKTbreakDump(ckt)
    CKTcircuit *ckt;

{
    register int i;
    for(i=0;i<ckt->CKTbreakSize;i++) {
        (void)printf("breakpoint table entry %d is %g\n",i,*(ckt->CKTbreaks+i));
    }
}
