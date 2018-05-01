/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "jfetdefs.h"
#include "sperror.h"
#include "suffix.h"


int
JFETdelete(inModel,name,inst)
    GENmodel *inModel;
    IFuid name;
    GENinstance **inst;
{
    JFETmodel *model = (JFETmodel*)inModel;
    JFETinstance **fast = (JFETinstance**)inst;
    JFETinstance **prev = NULL;
    JFETinstance *here;

    for( ; model ; model = model->JFETnextModel) {
        prev = &(model->JFETinstances);
        for(here = *prev; here ; here = *prev) {
            if(here->JFETname == name || (fast && here==*fast) ) {
                *prev= here->JFETnextInstance;
                FREE(here);
                return(OK);
            }
            prev = &(here->JFETnextInstance);
        }
    }
    return(E_NODEV);
}
