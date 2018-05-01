/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

    /* CKTnum2nod
     *  find the given node given its name and return the node pointer
     */

#include "spice.h"
#include <stdio.h>
#include "ifsim.h"
#include "sperror.h"
#include "cktdefs.h"
#include "suffix.h"


/* ARGSUSED */
CKTnode *
CKTnum2nod(ckt,node)
    CKTcircuit *ckt;
    int node;
{
    register CKTnode *here;

    for (here = ((CKTcircuit *)ckt)->CKTnodes; here; here = here->next)  {
        if(here->number == node) {
            return(here);
        }
    }
    return((CKTnode *)NULL);
}
