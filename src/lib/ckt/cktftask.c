/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

    /* CKTfndTask
     *  find the specified task - not yet supported in spice 
     */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "ifsim.h"
#include "sperror.h"
#include "util.h"
#include "suffix.h"


/* ARGSUSED */
int
CKTfndTask(ckt,taskPtr,taskName)
    GENERIC *ckt;
    GENERIC **taskPtr;
    IFuid  taskName;
{
    return(E_UNSUPP);
}
