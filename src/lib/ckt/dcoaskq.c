/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "ifsim.h"
#include "iferrmsg.h"
#include "opdefs.h"
#include "cktdefs.h"
#include "suffix.h"

/* ARGSUSED */
int 
DCOaskQuest(ckt,anal,which,value)
    CKTcircuit *ckt;
    GENERIC *anal;
    int which;
    IFvalue *value;
{
    return(E_BADPARM);
}

