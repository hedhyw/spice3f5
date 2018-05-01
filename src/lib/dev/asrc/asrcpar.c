/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1987 Kanwar Jit Singh
**********/
/*
 * singh@ic.Berkeley.edu
 */

#include "spice.h"
#include <stdio.h>
#include "ifsim.h"
#include "util.h"
#include "asrcdefs.h"
#include "sperror.h"
#include "suffix.h"


/* ARGSUSED */
int
ASRCparam(param,value,fast,select)
    int param;
    IFvalue *value;
    GENinstance *fast;
    IFvalue *select;
{
    register ASRCinstance *here = (ASRCinstance*)fast;
    switch(param) {
        case ASRC_VOLTAGE:
            here->ASRCtype = ASRC_VOLTAGE;
        here->ASRCtree = value->tValue;
            break;
        case ASRC_CURRENT:
            here->ASRCtype = ASRC_CURRENT;
        here->ASRCtree = value->tValue;
            break;
        default:
            return(E_BADPARM);
    }
    return(OK);
}
