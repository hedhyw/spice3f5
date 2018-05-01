/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

    /* CKTmodParam
     *  attach the given parameter to the specified model in the given circuit
     */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "ifsim.h"
#include "util.h"
#include "devdefs.h"
#include "sperror.h"
#include "suffix.h"

extern SPICEdev *DEVices[];

/* ARGSUSED */
int
CKTmodParam(ckt,modfast,param,val,selector)
    GENERIC *ckt;
    GENERIC *modfast;
    int param;
    IFvalue *val;
    IFvalue *selector;
{
    register int type = ((GENmodel *)modfast)->GENmodType;

    if (((*DEVices[type]).DEVmodParam)) {
        return(((*((*DEVices[type]).DEVmodParam)) (param,val,
                (GENmodel *)modfast)));
    } else {
        return(E_BADPARM);
    }
}
