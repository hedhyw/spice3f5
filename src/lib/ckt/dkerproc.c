/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1988 Jaijeet S Roychowdhury
**********/

#include "spice.h"
#include "cktdefs.h"
#include "sperror.h"
#include "distodef.h"
#include "suffix.h"

int
DkerProc(type,rPtr,iPtr,size,job)
double *rPtr;
double *iPtr;
int size;
DISTOAN* job;
int type;

{
int i;
double temp;

switch(type) {

  case D_F1:


    for (i=1;i<=size;i++)
	{
	iPtr[i] *= 2.0; /* convert to sinusoid amplitude */
	rPtr[i] *= 2.0;
	}

    break;

  case D_F2:


    for (i=1;i<=size;i++)
	{
	rPtr[i] *= 2.0;
        iPtr[i] *= 2.0;
	}

   break;

 case D_TWOF1:

    
    for (i=1;i<=size;i++)
	{
	iPtr[i] *= 2.0;
	rPtr[i] *= 2.0;
	}

   break;

 case D_THRF1:


    for (i=1;i<=size;i++)
	{
	iPtr[i] *= 2.0;
	rPtr[i] *= 2.0;
	}

    break;

 case D_F1PF2:


    for (i=1;i<=size;i++)
	{
	iPtr[i] *= 4.0;
	rPtr[i] *= 4.0;
	}

    break;

 case D_F1MF2:

    
    for (i=1;i<=size;i++)
	{
	iPtr[i] *= 4.0;
	rPtr[i] *= 4.0;
	}

    break;

 case D_2F1MF2:

    for (i=1;i<=size;i++)
	{
	iPtr[i] *= 6.0;
	rPtr[i] *= 6.0;
	}

    break;

 default:

	return(E_BADPARM);

}

return(OK);
}
