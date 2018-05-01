/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Gordon Jacobs
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "cswdefs.h"
#include "util.h"
#include "sperror.h"
#include "suffix.h"


int
CSWdelete(inModel,name,inst)
    GENmodel *inModel;
    IFuid name;
    GENinstance **inst;

{
    CSWmodel *model = (CSWmodel*)inModel;
    CSWinstance **fast = (CSWinstance**)inst;
    CSWinstance **prev = NULL;
    CSWinstance *here;

    for( ; model ; model = model->CSWnextModel) {
        prev = &(model->CSWinstances);
        for(here = *prev; here ; here = *prev) {
            if(here->CSWname == name || (fast && here==*fast) ) {
                *prev= here->CSWnextInstance;
                FREE(here);
                return(OK);
            }
            prev = &(here->CSWnextInstance);
        }
    }
    return(E_NODEV);
}
