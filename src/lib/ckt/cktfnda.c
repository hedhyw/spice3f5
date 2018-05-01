/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

    /* CKTfndAnal
     *  find the given Analysis given its name and return the Analysis pointer
     */

#include "spice.h"
#include <stdio.h>
#include "ifsim.h"
#include "jobdefs.h"
#include "tskdefs.h"
#include "sperror.h"
#include "cktdefs.h"
#include "suffix.h"


/* ARGSUSED */
int
CKTfndAnal(ckt,analIndex,anal,name,inTask,taskName)
    GENERIC *ckt;
    int *analIndex;
    GENERIC **anal;
    IFuid name;
    GENERIC *inTask;
    IFuid taskName;
{
    TSKtask *task = (TSKtask *)inTask;
    register JOB *here;

    for (here = ((TSKtask *)task)->jobs;here;here = here->JOBnextJob) {
        if(strcmp(here->JOBname,name)==0) {
            if(anal) *anal = (GENERIC *)here;
            return(OK);
        }
    }
    return(E_NOTFOUND);
}
