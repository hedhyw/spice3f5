/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/
/*
 */

#include "spice.h"
#include <stdio.h>
#include "strext.h"
#include "inpdefs.h"
#include "util.h"
#include "suffix.h"


char *
INPmkTemp(string)
char *string;
{
int len;
char *temp;
    
    len = strlen(string);
    temp = MALLOC(len+1);
    if(temp!=(char *)NULL) (void)strcpy(temp,string);
    return(temp);

}
