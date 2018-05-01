/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1988 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "ifsim.h"
#include "inpdefs.h"
#include "inpmacs.h"
#include "fteext.h"
#include "suffix.h"

void
INP2V(ckt,tab,current)
    GENERIC *ckt;
    INPtables *tab;
    card *current;

{

        /* Vname <node> <node> [ [DC] <val>] [AC [<val> [<val> ] ] ]
         *       [<tran function>] */

int type;   /* the type the model says it is */
char *line; /* the part of the current line left to parse */
char *name; /* the resistor's name */
char *nname1;   /* the first node's name */
char *nname2;   /* the second node's name */
GENERIC *node1; /* the first node's node pointer */
GENERIC *node2; /* the second node's node pointer */
int error;      /* error code temporary */
GENERIC *fast;  /* pointer to the actual instance */
IFvalue ptemp;  /* a value structure to package resistance into */
int waslead;    /* flag to indicate that funny unlabeled number was found */
double leadval; /* actual value of unlabeled number */
IFuid uid;      /* uid for default model */

    type = INPtypelook("Vsource");
    if(type < 0 ) {
        LITERR("Device type Vsource not supported by this binary\n")
        return;
    }
    line = current->line;
    INPgetTok(&line,&name,1);
    INPinsert(&name,tab);
    INPgetTok(&line,&nname1,1);
    INPtermInsert(ckt,&nname1,tab,&node1);
    INPgetTok(&line,&nname2,1);
    INPtermInsert(ckt,&nname2,tab,&node2);
    if(!tab->defVmod) {
        /* create default V model */
        IFnewUid(ckt,&uid,(IFuid)NULL,"V",UID_MODEL,(GENERIC**)NULL);
        IFC(newModel,(ckt,type,&(tab->defVmod),uid))
    }
    IFC(newInstance,(ckt,tab->defVmod,&fast,name))
    IFC(bindNode,(ckt,fast,1,node1))
    IFC(bindNode,(ckt,fast,2,node2))
    PARSECALL((&line,ckt,type,fast,&leadval,&waslead,tab))
    if(waslead) {
        ptemp.rValue = leadval;
        GCA(INPpName,("dc",&ptemp,ckt,type,fast))
    }
}
