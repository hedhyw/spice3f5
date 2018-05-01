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
#include "suffix.h"


void
CSWdestroy(inModel)
    GENmodel **inModel;
{
    CSWmodel **model = (CSWmodel**)inModel;
    CSWinstance *here;
    CSWinstance *prev = NULL;
    CSWmodel *mod = *model;
    CSWmodel *oldmod = NULL;

    for( ; mod ; mod = mod->CSWnextModel) {
        if(oldmod) FREE(oldmod);
        oldmod = mod;
        prev = (CSWinstance *)NULL;
        for(here = mod->CSWinstances ; here ; here = here->CSWnextInstance) {
            if(prev) FREE(prev);
            prev = here;
        }
        if(prev) FREE(prev);
    }
    if(oldmod) FREE(oldmod);
    *model = NULL;
}
