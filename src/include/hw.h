/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
**********/

/*
 *  Hardware specific machine limits
 */

#ifdef HAS_LIMITS_H
# include <limits.h>
#endif

#ifdef HAS_FLOAT_H
# include <float.h>
#endif

#ifdef vax

#  include "hw_vax.h"
#  ifndef	HUGE_VAL
#    define HUGE_VAL	1.701411834604692293e+38 
#  endif

#else
#  include "hw_ieee.h"

#define HAS_IEEE_FLOAT
#  ifndef	HUGE_VAL
#    define	HUGE_VAL	1.8e+308
#  endif

#endif

#ifndef	HUGE
#  define HUGE	HUGE_VAL
#endif

