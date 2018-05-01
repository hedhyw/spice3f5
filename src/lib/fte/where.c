/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group 
**********/

#include "spice.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "ftedata.h"
#include "ftehelp.h"
#include "hlpdefs.h"
#include "suffix.h"


void
com_where( )
{
	char	*msg;

	msg = (*ft_sim->nonconvErr)((GENERIC *) (ft_curckt->ci_ckt), 0);

	printf("%s", msg);
}
