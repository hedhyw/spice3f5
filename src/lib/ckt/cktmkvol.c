/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/* CKTmkVolt
 *  make the given name a 'node' of type current in the 
 * specified circuit
 */

#include "spice.h"
#include <stdio.h>
#include "ifsim.h"
#include "sperror.h"
#include "cktdefs.h"
#include "util.h"
#include "suffix.h"


/* ARGSUSED */
int
CKTmkVolt(ckt,node,basename,suffix)
    CKTcircuit  *ckt;
    CKTnode **node;
    IFuid basename;
    char *suffix; 
{
    IFuid uid;
    int error;
    CKTnode *mynode;
    CKTnode *checknode;

    error = CKTmkNode(ckt,&mynode);
    if(error) return(error);
    checknode = mynode;
    error = (*(SPfrontEnd->IFnewUid))((GENERIC *)ckt,&uid,basename,
            suffix,UID_SIGNAL,(GENERIC**)&checknode);
    if(error) {
        FREE(mynode);
        if(node) *node = checknode;
        return(error);
    }
    mynode->name = uid;
    mynode->type = SP_VOLTAGE;
    if(node) *node = mynode;
    error = CKTlinkEq(ckt,mynode);
    return(error);
}
