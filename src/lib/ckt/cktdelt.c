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

/* ARGSUSED */
int
CKTdelTask(ckt,task)
    GENERIC *ckt;
    GENERIC *task;
{
    JOB *job;
    JOB *old=NULL;
    for(job = ((TSKtask*)task)->jobs; job; job=job->JOBnextJob){
        if(old) FREE(old);
        old=job;
    }
    if(old)FREE(old);
    FREE(task);
    return(OK);
}
