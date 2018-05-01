
/*
 * Dummy interface stuff, for nutmeg. This is the easiest way of
 * making sure that nutmeg doesn't try to load spice in also.
 */

#include "spice.h"
#include "ifsim.h"
#include "sperror.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "fteinp.h"
#include "util.h"
#include "suffix.h"

/* ARGSUSED */ struct variable * nutif_getparam(ckt, name, param, ind, do_model)
                char *ckt, **name, *param;
                int ind;
                int do_model;
        { return ((struct variable *) NULL); }


