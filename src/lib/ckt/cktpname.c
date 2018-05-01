/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

    /*
     * CKTpName()
     *
     *  Take a parameter by Name and set it on the specified device 
     */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "ifsim.h"
#include "devdefs.h"
#include "cktdefs.h"
#include "gendefs.h"
#include "sperror.h"
#include "suffix.h"


extern SPICEdev *DEVices[];

/* ARGSUSED */
int
CKTpName(parm,val,ckt,dev,name,fast)
    char *parm; /* the name of the parameter to set */
    IFvalue *val; /* the parameter union containing the value to set */
    CKTcircuit *ckt;/* the circuit this device is a member of */
    int dev;        /* the device type code to the device being parsed */
    char *name;     /* the name of the device being parsed */
    GENinstance **fast;    /* direct pointer to device being parsed */

{
    int error;  /* int to store evaluate error return codes in */
    int i;

    for(i=0;i<(*(*DEVices[dev]).DEVpublic.numInstanceParms);i++) {
        if(strcmp(parm,
                ((*DEVices[dev]).DEVpublic.instanceParms[i].keyword))==0) {
            error = CKTparam((GENERIC*)ckt,(GENERIC *)*fast,
                    (*DEVices[dev]).DEVpublic.instanceParms[i].id,val,
                    (IFvalue *)NULL);
            if(error) return(error);
            break;
        }
    }
    if(i==(*(*DEVices[dev]).DEVpublic.numInstanceParms)) {
        return(E_BADPARM);
    }
    return(OK);
}
