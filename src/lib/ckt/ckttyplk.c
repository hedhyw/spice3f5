/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/*  look up the 'type' in the device description struct and return the
 *  appropriate index for the device found, or -1 for not found 
 */

#include "spice.h"
#include "strext.h"
#include "cktdefs.h"
#include "devdefs.h"
#include "suffix.h"


extern SPICEdev *DEVices[];

int
CKTtypelook(type)
    char *type;
{

    int i;
    for(i=0;i<DEVmaxnum;i++) {
        if(strcmp(type,(*DEVices[i]).DEVpublic.name)==0) {
            /*found the device - return it */
            return(i);
        }
    }
    return(-1);
}

