/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "ifsim.h"
#include "util.h"
#include "iferrmsg.h"
#include "cpdefs.h"
#include "fteext.h"
#include "suffix.h"

int
INPapName(ckt,type,analPtr,parmname,value)
    GENERIC *ckt;
    int type;
    GENERIC *analPtr;
    char * parmname;
    IFvalue *value;
{
    int i;

    if (parmname && ft_sim->analyses[type]) {
	for(i=0;i<ft_sim->analyses[type]->numParms;i++)
	    if (strcmp(parmname,
		ft_sim->analyses[type]->analysisParms[i].keyword) == 0)
	    {
		return (*(ft_sim->setAnalysisParm))(ckt, analPtr,
			ft_sim->analyses[type]->analysisParms[i].id,
			value, (IFvalue*)NULL);
	    }
    }
    return(E_BADPARM);
}
