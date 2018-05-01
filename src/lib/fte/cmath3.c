/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Routines to do complex mathematical functions. These routines require
 * the -lm libraries. We sacrifice a lot of space to be able
 * to avoid having to do a seperate call for every vector element,
 * but it pays off in time savings.  These routines should never
 * allow FPE's to happen.
 *
 * Complex functions are called as follows:
 *  cx_something(data, type, length, &newlength, &newtype),
 *  and return a char * that is cast to complex or double.
 */

#include "spice.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "ftedata.h"
#include "ftecmath.h"
#include "suffix.h"

static complex *cexp();
static complex *cln();
static complex *ctimes();

char *
cx_divide(data1, data2, datatype1, datatype2, length)
    char *data1, *data2;
    short datatype1, datatype2;
{
    double *dd1 = (double *) data1;
    double *dd2 = (double *) data2;
    double *d;
    complex *cc1 = (complex *) data1;
    complex *cc2 = (complex *) data2;
    complex *c, c1, c2;
    int i;

    if ((datatype1 == VF_REAL) && (datatype2 == VF_REAL)) {
        d = alloc_d(length);
        for (i = 0; i < length; i++) {
            rcheck(dd2[i] != 0, "divide");
            d[i] = dd1[i] / dd2[i];
        }
        return ((char *) d);
    } else {
        c = alloc_c(length);
        for (i = 0; i < length; i++) {
            if (datatype1 == VF_REAL) {
                realpart(&c1) = dd1[i];
                imagpart(&c1) = 0.0;
            } else {
                realpart(&c1) = realpart(&cc1[i]);
                imagpart(&c1) = imagpart(&cc1[i]);
            }
            if (datatype2 == VF_REAL) {
                realpart(&c2) = dd2[i];
                imagpart(&c2) = 0.0;
            } else {
                realpart(&c2) = realpart(&cc2[i]);
                imagpart(&c2) = imagpart(&cc2[i]);
            }
        rcheck((realpart(&c2) != 0) || (imagpart(&c2) != 0), "divide");
#define xx5 realpart(&c1)
#define xx6 imagpart(&c1)
cdiv(xx5, xx6, realpart(&c2), imagpart(&c2), realpart(&c[i]), imagpart(&c[i]));
        }
        return ((char *) c);
    }
}

/* Should just use "j( )" */
/* The comma operator. What this does (unless it is part of the argument
 * list of a user-defined function) is arg1 + j(arg2).
 */

char *
cx_comma(data1, data2, datatype1, datatype2, length)
    char *data1, *data2;
    short datatype1, datatype2;
{
    double *dd1 = (double *) data1;
    double *dd2 = (double *) data2;
    complex *cc1 = (complex *) data1;
    complex *cc2 = (complex *) data2;
    complex *c, c1, c2;
    int i;

    c = alloc_c(length);
    for (i = 0; i < length; i++) {
        if (datatype1 == VF_REAL) {
            realpart(&c1) = dd1[i];
            imagpart(&c1) = 0.0;
        } else {
            realpart(&c1) = realpart(&cc1[i]);
            imagpart(&c1) = imagpart(&cc1[i]);
        }
        if (datatype2 == VF_REAL) {
            realpart(&c2) = dd2[i];
            imagpart(&c2) = 0.0;
        } else {
            realpart(&c2) = realpart(&cc2[i]);
            imagpart(&c2) = imagpart(&cc2[i]);
        }

        realpart(&c[i]) = realpart(&c1) + imagpart(&c2);
        imagpart(&c[i]) = imagpart(&c1) + realpart(&c2);
    }
    return ((char *) c);
}

char *
cx_power(data1, data2, datatype1, datatype2, length)
    char *data1, *data2;
    short datatype1, datatype2;
{
    double *dd1 = (double *) data1;
    double *dd2 = (double *) data2;
    double *d;
    complex *cc1 = (complex *) data1;
    complex *cc2 = (complex *) data2;
    complex *c, c1, c2, *t;
    int i;

    if ((datatype1 == VF_REAL) && (datatype2 == VF_REAL)) {
        d = alloc_d(length);
        for (i = 0; i < length; i++) {
    rcheck((dd1[i] >= 0) || (floor(dd2[i]) == ceil(dd2[i])), "power");
            d[i] = pow(dd1[i], dd2[i]);
        }
        return ((char *) d);
    } else {
        c = alloc_c(length);
        for (i = 0; i < length; i++) {
            if (datatype1 == VF_REAL) {
                realpart(&c1) = dd1[i];
                imagpart(&c1) = 0.0;
            } else {
                realpart(&c1) = realpart(&cc1[i]);
                imagpart(&c1) = imagpart(&cc1[i]);
            }
            if (datatype2 == VF_REAL) {
                realpart(&c2) = dd2[i];
                imagpart(&c2) = 0.0;
            } else {
                realpart(&c2) = realpart(&cc2[i]);
                imagpart(&c2) = imagpart(&cc2[i]);
            }

            if ((realpart(&c1) == 0.0) && (imagpart(&c1) == 0.0)) {
                realpart(&c[i]) = 0.0;
                imagpart(&c[i]) = 0.0;
            } else { /* if ((imagpart(&c1) != 0.0) && 
                        (imagpart(&c2) != 0.0)) */
                t = cexp(ctimes(&c2, cln(&c1)));
                realpart(&c[i]) = realpart(t);
                imagpart(&c[i]) = imagpart(t);
            /*
            } else {
                realpart(&c[i]) = pow(realpart(&c1), 
                                realpart(&c2)); 
                imagpart(&c[i]) = 0.0;
            */
            }
        }
        return ((char *) c);
    }
}

/* These are unnecessary... Only cx_power uses them... */

static complex *
cexp(c)
    complex *c;
{
    static complex r;
    double d;

    d = exp(realpart(c));
    realpart(&r) = d * cos(imagpart(c));
    if (imagpart(c) != 0.0)
        imagpart(&r) = d * sin(imagpart(c));
    else
        imagpart(&r) = 0.0;
    return (&r);
}

static complex *
cln(c)
    complex *c;
{
    static complex r;

    rcheck(cmag(c) != 0, "ln");
    realpart(&r) = log(cmag(c));
    if (imagpart(c) != 0.0)
        imagpart(&r) = atan2(imagpart(c), realpart(c));
    else
        imagpart(&r) = 0.0;
    return (&r);
}

static complex *
ctimes(c1, c2)
    complex *c1, *c2;
{
    static complex r;

    realpart(&r) = realpart(c1) * realpart(c2) - 
               imagpart(c1) * imagpart(c2);
    imagpart(&r) = imagpart(c1) * realpart(c2) +
               realpart(c1) * imagpart(c2);
    return (&r);
}

#ifdef HAS_SHORTMACRO

/* Some compilers get blown away by complicated macros */

void
cx_cdiv(r1, i1, r2, i2, r3, i3)
    double r1, i1, r2, i2, *r3, *i3;
{
        double r, s;

    if (FTEcabs(r2) > FTEcabs(i2)) {
        r = i2 / r2;
        s = r2 + r * i2;
        if (!s) {
            fprintf(cp_err, "Error: divide by 0\n");
            return;
        }
        *r3 = (r1 + r * i1) / s;
        *i3 = (i1 - r * r1) / s;
    } else {
        r = r2 / i2;
        s = i2 + r * r2;
        if (!s) {
            fprintf(cp_err, "Error: divide by 0\n");
            return;
        }
        *r3 = (r * r1 + i1) / s;
        *i3 = (r * i1 - r1) / s;
    }
    return;
}

#endif

/* Now come all the relational and logical functions. It's overkill to put
 * them here, but... Note that they always return a real value, with the
 * result the same length as the arguments.
 */

char *
cx_eq(data1, data2, datatype1, datatype2, length)
    char *data1, *data2;
    short datatype1, datatype2;
{
    double *dd1 = (double *) data1;
    double *dd2 = (double *) data2;
    double *d;
    complex *cc1 = (complex *) data1;
    complex *cc2 = (complex *) data2;
    complex c1, c2;
    int i;

    d = alloc_d(length);
    if ((datatype1 == VF_REAL) && (datatype2 == VF_REAL)) {
        for (i = 0; i < length; i++)
            if (dd1[i] == dd2[i])
                d[i] = 1.0;
            else
                d[i] = 0.0;
    } else {
        for (i = 0; i < length; i++) {
            if (datatype1 == VF_REAL) {
                realpart(&c1) = dd1[i];
                imagpart(&c1) = 0.0;
            } else {
                realpart(&c1) = realpart(&cc1[i]);
                imagpart(&c1) = imagpart(&cc1[i]);
            }
            if (datatype2 == VF_REAL) {
                realpart(&c2) = dd2[i];
                imagpart(&c2) = 0.0;
            } else {
                realpart(&c2) = realpart(&cc2[i]);
                imagpart(&c2) = imagpart(&cc2[i]);
            }
            d[i] = ((realpart(&c1) == realpart(&c2)) &&
                (imagpart(&c1) == imagpart(&c2)));
        }
    }
    return ((char *) d);
}

char *
cx_gt(data1, data2, datatype1, datatype2, length)
    char *data1, *data2;
    short datatype1, datatype2;
{
    double *dd1 = (double *) data1;
    double *dd2 = (double *) data2;
    double *d;
    complex *cc1 = (complex *) data1;
    complex *cc2 = (complex *) data2;
    complex c1, c2;
    int i;

    d = alloc_d(length);
    if ((datatype1 == VF_REAL) && (datatype2 == VF_REAL)) {
        for (i = 0; i < length; i++)
            if (dd1[i] > dd2[i])
                d[i] = 1.0;
            else
                d[i] = 0.0;
    } else {
        for (i = 0; i < length; i++) {
            if (datatype1 == VF_REAL) {
                realpart(&c1) = dd1[i];
                imagpart(&c1) = 0.0;
            } else {
                realpart(&c1) = realpart(&cc1[i]);
                imagpart(&c1) = imagpart(&cc1[i]);
            }
            if (datatype2 == VF_REAL) {
                realpart(&c2) = dd2[i];
                imagpart(&c2) = 0.0;
            } else {
                realpart(&c2) = realpart(&cc2[i]);
                imagpart(&c2) = imagpart(&cc2[i]);
            }
            d[i] = ((realpart(&c1) > realpart(&c2)) &&
                (imagpart(&c1) > imagpart(&c2)));
        }
    }
    return ((char *) d);
}

char *
cx_lt(data1, data2, datatype1, datatype2, length)
    char *data1, *data2;
    short datatype1, datatype2;
{
    double *dd1 = (double *) data1;
    double *dd2 = (double *) data2;
    double *d;
    complex *cc1 = (complex *) data1;
    complex *cc2 = (complex *) data2;
    complex c1, c2;
    int i;

    d = alloc_d(length);
    if ((datatype1 == VF_REAL) && (datatype2 == VF_REAL)) {
        for (i = 0; i < length; i++)
            if (dd1[i] < dd2[i])
                d[i] = 1.0;
            else
                d[i] = 0.0;
    } else {
        for (i = 0; i < length; i++) {
            if (datatype1 == VF_REAL) {
                realpart(&c1) = dd1[i];
                imagpart(&c1) = 0.0;
            } else {
                realpart(&c1) = realpart(&cc1[i]);
                imagpart(&c1) = imagpart(&cc1[i]);
            }
            if (datatype2 == VF_REAL) {
                realpart(&c2) = dd2[i];
                imagpart(&c2) = 0.0;
            } else {
                realpart(&c2) = realpart(&cc2[i]);
                imagpart(&c2) = imagpart(&cc2[i]);
            }
            d[i] = ((realpart(&c1) < realpart(&c2)) &&
                (imagpart(&c1) < imagpart(&c2)));
        }
    }
    return ((char *) d);
}

char *
cx_ge(data1, data2, datatype1, datatype2, length)
    char *data1, *data2;
    short datatype1, datatype2;
{
    double *dd1 = (double *) data1;
    double *dd2 = (double *) data2;
    double *d;
    complex *cc1 = (complex *) data1;
    complex *cc2 = (complex *) data2;
    complex c1, c2;
    int i;

    d = alloc_d(length);
    if ((datatype1 == VF_REAL) && (datatype2 == VF_REAL)) {
        for (i = 0; i < length; i++)
            if (dd1[i] >= dd2[i])
                d[i] = 1.0;
            else
                d[i] = 0.0;
    } else {
        for (i = 0; i < length; i++) {
            if (datatype1 == VF_REAL) {
                realpart(&c1) = dd1[i];
                imagpart(&c1) = 0.0;
            } else {
                realpart(&c1) = realpart(&cc1[i]);
                imagpart(&c1) = imagpart(&cc1[i]);
            }
            if (datatype2 == VF_REAL) {
                realpart(&c2) = dd2[i];
                imagpart(&c2) = 0.0;
            } else {
                realpart(&c2) = realpart(&cc2[i]);
                imagpart(&c2) = imagpart(&cc2[i]);
            }
            d[i] = ((realpart(&c1) >= realpart(&c2)) &&
                (imagpart(&c1) >= imagpart(&c2)));
        }
    }
    return ((char *) d);
}

char *
cx_le(data1, data2, datatype1, datatype2, length)
    char *data1, *data2;
    short datatype1, datatype2;
{
    double *dd1 = (double *) data1;
    double *dd2 = (double *) data2;
    double *d;
    complex *cc1 = (complex *) data1;
    complex *cc2 = (complex *) data2;
    complex c1, c2;
    int i;

    d = alloc_d(length);
    if ((datatype1 == VF_REAL) && (datatype2 == VF_REAL)) {
        for (i = 0; i < length; i++)
            if (dd1[i] <= dd2[i])
                d[i] = 1.0;
            else
                d[i] = 0.0;
    } else {
        for (i = 0; i < length; i++) {
            if (datatype1 == VF_REAL) {
                realpart(&c1) = dd1[i];
                imagpart(&c1) = 0.0;
            } else {
                realpart(&c1) = realpart(&cc1[i]);
                imagpart(&c1) = imagpart(&cc1[i]);
            }
            if (datatype2 == VF_REAL) {
                realpart(&c2) = dd2[i];
                imagpart(&c2) = 0.0;
            } else {
                realpart(&c2) = realpart(&cc2[i]);
                imagpart(&c2) = imagpart(&cc2[i]);
            }
            d[i] = ((realpart(&c1) <= realpart(&c2)) &&
                (imagpart(&c1) <= imagpart(&c2)));
        }
    }
    return ((char *) d);
}

char *
cx_ne(data1, data2, datatype1, datatype2, length)
    char *data1, *data2;
    short datatype1, datatype2;
{
    double *dd1 = (double *) data1;
    double *dd2 = (double *) data2;
    double *d;
    complex *cc1 = (complex *) data1;
    complex *cc2 = (complex *) data2;
    complex c1, c2;
    int i;

    d = alloc_d(length);
    if ((datatype1 == VF_REAL) && (datatype2 == VF_REAL)) {
        for (i = 0; i < length; i++)
            if (dd1[i] != dd2[i])
                d[i] = 1.0;
            else
                d[i] = 0.0;
    } else {
        for (i = 0; i < length; i++) {
            if (datatype1 == VF_REAL) {
                realpart(&c1) = dd1[i];
                imagpart(&c1) = 0.0;
            } else {
                realpart(&c1) = realpart(&cc1[i]);
                imagpart(&c1) = imagpart(&cc1[i]);
            }
            if (datatype2 == VF_REAL) {
                realpart(&c2) = dd2[i];
                imagpart(&c2) = 0.0;
            } else {
                realpart(&c2) = realpart(&cc2[i]);
                imagpart(&c2) = imagpart(&cc2[i]);
            }
            d[i] = ((realpart(&c1) != realpart(&c2)) &&
                (imagpart(&c1) != imagpart(&c2)));
        }
    }
    return ((char *) d);
}

