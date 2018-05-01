/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

    /* CKTcrtElement(ckt,type,inModPtr,inInstPtr,name,subname)
     *  Create a device of the specified type, with the given name, using
     *  the specified model in the named circuit.
     */

#include "spice.h"
#include <stdio.h>
#include "ifsim.h"
#include "cktdefs.h"
#include "devdefs.h"
#include "sperror.h"
#include "util.h"
#include "suffix.h"


/*ARGSUSED*/
int
CKTcrtElt(ckt,inModPtr,inInstPtr,name)
    GENERIC *ckt;
    GENERIC *inModPtr;
    GENERIC **inInstPtr;
    IFuid name;
{
    GENinstance *instPtr = NULL;
    GENmodel *modPtr=(GENmodel*)inModPtr;
    extern SPICEdev *DEVices[];
    int error;
    int type;

    if((GENmodel *)modPtr==(GENmodel*)NULL) return(E_NOMOD);
    type = ((GENmodel*)modPtr)->GENmodType;
    error =CKTfndDev(ckt,&type,(GENERIC**)&instPtr,name,inModPtr,(char *)NULL );
    if (error== OK) { 
        if(inInstPtr) *inInstPtr=(GENERIC *)instPtr;
        return(E_EXISTS);
    } else if (error != E_NODEV) return(error);
    instPtr = (GENinstance *)MALLOC(*DEVices[type]->DEVinstSize);
    if(instPtr == (GENinstance *)NULL) return(E_NOMEM);
    instPtr->GENname = name;
    instPtr->GENmodPtr = modPtr;
    instPtr->GENnextInstance = modPtr->GENinstances;
    modPtr->GENinstances = instPtr;
    if(inInstPtr != NULL) *inInstPtr = (GENERIC *)instPtr;
    return(OK);
}
