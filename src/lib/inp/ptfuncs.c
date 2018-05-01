/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1987 Wayne A. Christopher, U. C. Berkeley CAD Group 
**********/

/*
 * All the functions used in the parse tree.  These functions return HUGE
 * if their argument is out of range.
 */

#include "spice.h"
#include <stdio.h>
#include "ifsim.h"
#include "inpptree.h"
#include "suffix.h"

/* XXX These should be in math.h */

#ifdef HAS_ATRIGH
#  ifdef HAS_NO_ATRIGH_DECL
extern double asinh(), acosh(), atanh();
#  endif
#endif /* HAS_ATRIGH */

double PTfudge_factor;

#define	MODULUS(NUM,LIMIT)	((NUM) - ((int) ((NUM) / (LIMIT))) * (LIMIT))

double
PTabs(arg)
    double arg;
{
    return arg >= 0.0 ? arg : -arg;
}

double
PTsgn(arg)
    double arg;
{
    return arg > 0.0 ? 1.0 : arg < 0.0 ? -1.0 : 0.0;
}

double
PTplus(arg1, arg2)
    double arg1, arg2;
{
    return (arg1 + arg2);
}

double
PTminus(arg1, arg2)
    double arg1, arg2;
{
    return (arg1 - arg2);
}

double
PTtimes(arg1, arg2)
    double arg1, arg2;
{
    return (arg1 * arg2);
}

double 
PTdivide(arg1, arg2)
    double arg1, arg2;
{
    if (arg2 >= 0.0)
	arg2 += PTfudge_factor;
    else
	arg2 -= PTfudge_factor;

    if (arg2 == 0.0)
        return (HUGE);

    return (arg1 / arg2);
}

double
PTpower(arg1, arg2)
    double arg1, arg2;
{
    if (arg1 < 0.0) {
	if (fabs(arg2 - ((int) arg2)) / (arg2 + 0.001) < 0.000001) {
	    arg2 = (int) arg2;
	} else {
	    arg1 = -arg1;
	}
    }
    return (pow(arg1, arg2));
}

double
PTacos(arg)
    double arg;
{
    return (acos(arg));
}

double
PTacosh(arg)
    double arg;
{
#ifdef HAS_ATRIGH
    return (acosh(arg));
#else
    if (arg < 1.0)
	arg = 1.0;
    return (log(arg + sqrt(arg*arg-1.0)));
#endif /* HAS_ATRIGH */
}

double
PTasin(arg)
    double arg;
{
    return (asin(arg));
}

double
PTasinh(arg)
    double arg;
{
#ifdef HAS_ATRIGH
    return (asinh(arg));
#else
    return log(arg + sqrt(arg * arg + 1.0));
#endif /* HAS_ATRIGH */
}

double
PTatan(arg)
    double arg;
{
    return (atan(arg));
}

double
PTatanh(arg)
    double arg;
{
#ifdef HAS_ATRIGH
    return (atanh(arg));
#else
    if (arg < -1.0)
	arg = -1.0 + PTfudge_factor + 1e-10;
    else if (arg > 1.0)
	arg = 1.0 - PTfudge_factor - 1e-10;
    return (log((1.0 + arg) / (1.0 - arg)) / 2.0);
#endif /* HAS_ATRIGH */
}

double
PTustep(arg)
    double arg;
{
    if (arg < 0.0)
	return 0.0;
    else if (arg > 0.0)
	return 1.0;
    else
	return 0.5; /* Ick! */
}

double
PTuramp(arg)
    double arg;
{
    if (arg < 0.0)
	return 0.0;
    else
	return arg;
}

double
PTcos(arg)
    double arg;
{
    return (cos(MODULUS(arg, 2 * M_PI)));
}

double
PTcosh(arg)
    double arg;
{
    return (cosh(arg));
}

double
PTexp(arg)
    double arg;
{
    return (exp(arg));
}

double
PTln(arg)
    double arg;
{
    if (arg < 0.0)
	arg = -arg;
    return (log(arg));
}

double
PTlog(arg)
    double arg;
{
    if (arg < 0.0)
	arg = -arg;
    return (log10(arg));
}

double
PTsin(arg)
    double arg;
{
    return (sin(MODULUS(arg, 2 * M_PI)));
}

double
PTsinh(arg)
    double arg;
{
    return (sinh(arg));
}

double
PTsqrt(arg)
    double arg;
{
    if (arg < 0.0)
	arg = -arg;
    return (sqrt(arg));
}

double
PTtan(arg)
    double arg;
{
    return (tan(MODULUS(arg, M_PI)));
}

double
PTtanh(arg)
    double arg;
{
    return (tanh(arg));
}

double
PTuminus(arg)
    double arg;
{
    return (- arg);
}

