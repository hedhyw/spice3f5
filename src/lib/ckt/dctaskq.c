/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "ifsim.h"
#include "iferrmsg.h"
#include "trcvdefs.h"
#include "cktdefs.h"
#include "suffix.h"

/* ARGSUSED */
int 
DCTaskQuest(ckt,anal,which,value)
    CKTcircuit *ckt;
    GENERIC *anal;
    int which;
    IFvalue *value;
{
    switch(which) {

    default:
	break;
    }
    /* NOTREACHED */ /* TEMPORARY until cases get added */
    return(E_BADPARM);
}

