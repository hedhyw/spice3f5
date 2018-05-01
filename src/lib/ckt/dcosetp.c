/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "ifsim.h"
#include "iferrmsg.h"
#include "cktdefs.h"
#include "opdefs.h"
#include "suffix.h"

/* ARGSUSED */
int 
DCOsetParm(ckt,anal,which,value)
    CKTcircuit  *ckt;
    GENERIC *anal;
    int which;
    IFvalue *value;
{
    switch(which) {

    default:
	break;
    }
    return(E_BADPARM);
}


SPICEanalysis DCOinfo  = {
    { 
        "OP",
        "D.C. Operating point analysis",

        0,
        NULL,
    },
    sizeof(OP),
    NODOMAIN,
    1,
    DCOsetParm,
    DCOaskQuest,
    NULL,
    DCop
};
