/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1987 Kanwar Jit Singh
**********/

#include "spice.h"
#include <stdio.h>
#include "asrcdefs.h"
#include "util.h"
#include "suffix.h"

void
ASRCdestroy(model)
    GENmodel **model;

{
    ASRCmodel **start = (ASRCmodel**)model; /* starting model */
    ASRCinstance *here;     /* current instance */
    ASRCinstance *next;
    ASRCmodel *mod = *start;    /* current model */
    ASRCmodel *nextmod;

    for( ; mod ; mod = nextmod) {
        for(here = mod->ASRCinstances ; here ; here = next) {
            next = here->ASRCnextInstance;
	    FREE(here->ASRCacValues);
            FREE(here);
        }
        nextmod = mod->ASRCnextModel;
        FREE(mod);
    }
    *model = NULL;
}
