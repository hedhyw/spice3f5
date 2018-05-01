/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

    /*
     *CKTaskNodQst
     *
     *   ask about a parameter on a node.
     */

#include "spice.h"
#include <stdio.h>
#include "ifsim.h"
#include "iferrmsg.h"
#include "cktdefs.h"
#include "util.h"
#include "suffix.h"


/* ARGSUSED */
int
CKTaskNodQst(ckt,node,parm,value,selector)
    GENERIC *ckt;
    GENERIC *node;
    int parm;
    IFvalue *value;
    IFvalue *selector;

{
    if(!node) return(E_BADPARM);
    switch(parm) {

    case PARM_NS:
        value->rValue = ((CKTnode *)node)->nodeset;
        break;

    case PARM_IC:
        value->rValue = ((CKTnode *)node)->ic;
        break;

    case PARM_NODETYPE:
        value->iValue = ((CKTnode *)node)->type;
        break;

    default:
        return(E_BADPARM);
    }
    return(OK);
}
