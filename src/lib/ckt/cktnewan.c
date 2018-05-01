/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "tskdefs.h"
#include "jobdefs.h"
#include "ifsim.h"
#include "util.h"
#include "iferrmsg.h"
#include "suffix.h"

extern SPICEanalysis *analInfo[];

/* ARGSUSED */
int
CKTnewAnal(ckt,type,name,analPtr,taskPtr)
    GENERIC *ckt;
    int type;
    IFuid name;
    GENERIC **analPtr;
    GENERIC *taskPtr;
{
    if(type==0) {
        /* special case for analysis type 0 == option card */
        *analPtr=taskPtr; /* pointer to the task itself */
        (*(JOB **)analPtr)->JOBname = name;
        (*(JOB **)analPtr)->JOBtype = type;
        return(OK); /* doesn't need to be created */
    }
    *analPtr = (GENERIC *)MALLOC(analInfo[type]->size);
    if(*analPtr==NULL) return(E_NOMEM);
    (*(JOB **)analPtr)->JOBname = name;
    (*(JOB **)analPtr)->JOBtype = type;
    (*(JOB **)analPtr)->JOBnextJob = ((TSKtask *)taskPtr)->jobs;
    ((TSKtask *)taskPtr)->jobs = (JOB *)*analPtr;
    return(OK);
}
