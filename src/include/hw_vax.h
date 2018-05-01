/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
**********/

/*
 *  VAX math
 */

#define HAS_VAX_FPERRORS
#define MAX_EXP_ARG	87.0		/* Approximate largest arg to exp() */

#ifndef DBL_EPSILON
# define DBL_EPSILON	6.93889e-18
#endif
#ifndef DBL_MAX
# define DBL_MAX	1.70141e+38
#endif
#ifndef DBL_MIN
# define DBL_MIN	2.938743e-39
#endif
#ifndef SHRT_MAX
# define SHRT_MAX	32766
#endif
#ifndef INT_MAX
# define INT_MAX	2147483646
#endif
#ifndef LONG_MAX
# define LONG_MAX	2147483646
#endif

#define MAXPOSINT	INT_MAX
