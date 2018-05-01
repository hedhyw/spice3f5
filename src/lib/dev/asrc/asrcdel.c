/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1987 Kanwar Jit Singh
**********/
/*
 * singh@ic.Berkeley.edu
 */

#include "spice.h"
#include <stdio.h>
#include "asrcdefs.h"
#include "util.h"
#include "sperror.h"
#include "suffix.h"


int
ASRCdelete(model,name,fast)
    GENmodel *model;
    IFuid name;
    GENinstance **fast;

{
    register ASRCinstance **instPtr = (ASRCinstance**)fast;
    ASRCmodel *modPtr = (ASRCmodel*)model;

    ASRCinstance **prev = NULL;
    ASRCinstance *here;

    for( ; modPtr ; modPtr = modPtr->ASRCnextModel) {
        prev = &(modPtr->ASRCinstances);
        for(here = *prev; here ; here = *prev) {
            if(here->ASRCname == name || (instPtr && here==*instPtr) ) {
                *prev= here->ASRCnextInstance;
                FREE(here);
                return(OK);
            }
            prev = &(here->ASRCnextInstance);
        }
    }
    return(E_NODEV);
}
