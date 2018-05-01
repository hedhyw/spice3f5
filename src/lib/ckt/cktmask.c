/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

    /* CKTmodAsk
     *  Ask questions about a specified device.
     */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "ifsim.h"
#include "devdefs.h"
#include "sperror.h"
#include "util.h"
#include "suffix.h"

extern SPICEdev *DEVices[];

/* ARGSUSED */
int
CKTmodAsk(ckt,modfast,which,value,selector)
    GENERIC *ckt;
    GENERIC *modfast;
    int which;
    IFvalue *value;
    IFvalue *selector;
{
    int type = ((GENmodel *)modfast)->GENmodType;
    if((*DEVices[type]).DEVmodAsk) {
        return( (*((*DEVices[type]).DEVmodAsk)) ((CKTcircuit *)ckt,
                (GENmodel *)modfast,which,value) );
    }
    return(E_BADPARM);
}
