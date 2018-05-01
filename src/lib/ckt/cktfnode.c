/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

    /* CKTfndNode
     *  find the given node given its name and return the node pointer
     */

#include "spice.h"
#include <stdio.h>
#include "ifsim.h"
#include "sperror.h"
#include "cktdefs.h"
#include "suffix.h"


/* ARGSUSED */
int
CKTfndNode(ckt,node,name)
    GENERIC *ckt;
    GENERIC **node;
    IFuid name;
{
    register CKTnode *here;

    for (here = ((CKTcircuit *)ckt)->CKTnodes; here; here = here->next)  {
        if(here->name == name) {
            if(node) *node = (char *)here;
            return(OK);
        }
    }
    return(E_NOTFOUND);
}
