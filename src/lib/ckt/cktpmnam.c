/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/*
 * CKTpModName()
 *
 *  Take a parameter by Name and set it on the specified model
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
CKTpModName(parm,val,ckt,type,name,modfast)
    char *parm; /* the name of the parameter to set */
    IFvalue *val; /* the parameter union containing the value to set */
    CKTcircuit *ckt;/* the circuit this model is a member of */
    int type;        /* the device type code to the model being parsed */
    IFuid name;     /* the name of the model being parsed */
    GENmodel **modfast;    /* direct pointer to model being parsed */

{
    int error;  /* int to store evaluate error return codes in */
    int i;

    for(i=0;i<(*(*DEVices[type]).DEVpublic.numModelParms);i++) {
        if(strcmp(parm,((*DEVices[type]).DEVpublic.modelParms[i].keyword))==0){
            error = CKTmodParam((GENERIC *)ckt,(GENERIC*)*modfast,
                    (*DEVices[type]).DEVpublic.modelParms[i].id,val,
                    (IFvalue*)NULL);
            if(error) return(error);
            break;
        }
    }
    if(i==(*(*DEVices[type]).DEVpublic.numModelParms)) {
        return(E_BADPARM);
    }
    return(OK);
}
