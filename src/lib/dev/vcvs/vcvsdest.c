/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "vcvsdefs.h"
#include "suffix.h"


void
VCVSdestroy(inModel)
    GENmodel **inModel;
{
    VCVSmodel **model = (VCVSmodel **)inModel;
    VCVSinstance *here;
    VCVSinstance *prev = NULL;
    VCVSmodel *mod = *model;
    VCVSmodel *oldmod = NULL;

    for( ; mod ; mod = mod->VCVSnextModel) {
        if(oldmod) FREE(oldmod);
        oldmod = mod;
        prev = (VCVSinstance *)NULL;
        for(here = mod->VCVSinstances ; here ; here = here->VCVSnextInstance) {
            if(prev) FREE(prev);
            prev = here;
        }
        if(prev) FREE(prev);
    }
    if(oldmod) FREE(oldmod);
    *model = NULL;
}
