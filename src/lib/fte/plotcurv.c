/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group 
**********/

/*
 * Curve plotting routines and general (non-graphics) plotting things.
 */

#include "spice.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "ftedata.h"
#include "ftegraph.h"
#include "ftedbgra.h"
#include "suffix.h"

static void plotinterval();

/* Plot the vector v, with scale xs.  If we are doing curve-fitting, then
 * do some tricky stuff.
 */

void
ft_graf(v, xs, nostart)
    struct dvec *v, *xs;
    bool nostart;
{
    int degree, gridsize, length;
    register int i, j, l;
    double *scratch, *result, *gridbuf, *mm;
    register double *xdata, *ydata;
    bool rot, increasing;
    double dx, dy, lx = 0, ly = 0;
    int	dir;

    /* if already started, use saved degree */
    if (nostart) {
      degree = currentgraph->degree;
    } else {
        if (!cp_getvar("polydegree", VT_NUM, (char *) &degree))
            degree = 1;
        currentgraph->degree = degree;
    }
    if (degree > v->v_length)
        degree = v->v_length;
    if (degree < 1) {
        fprintf(cp_err, "Error: polydegree is %d, can't plot...\n",
                degree);
        return;
    }

    if (!cp_getvar("gridsize", VT_NUM, (char *) &gridsize))
        gridsize = 0;
    if ((gridsize < 0) || (gridsize > 10000)) {
        fprintf(cp_err, "Error: bad grid size %d\n", gridsize);
        return;
    }
    if (gridsize && xs) {
      if( isreal(xs) ) {
        increasing = (xs->v_realdata[0] < xs->v_realdata[1]);
        for (i = 0; i < xs->v_length - 1; i++)
            if (increasing != (xs->v_realdata[i] < 
                    xs->v_realdata[i + 1])) {
                fprintf(cp_err, 
        "Warning: scale not monotonic, gridsize not relevant.\n");
                gridsize = 0;
                break;
            }
       } else {
        increasing = (realpart (&xs->v_compdata[0]) < 
				realpart( &xs->v_compdata[1]));
        for (i = 0; i < xs->v_length - 1; i++)
            if (increasing != (realpart( &xs->v_compdata[i]) < 
                    		realpart( &xs->v_compdata[i + 1]))) {
                fprintf(cp_err, 
        "Warning: scale not monotonic, gridsize not relevant.\n");
                gridsize = 0;
                break;
            }
       }
    }

    if (!nostart)
        gr_start(v);

    /* Do the one value case */

    if (!xs) {
        for (i = 0; i < v->v_length; i++) {

/* We should do the one - point case too!
 *	Important for pole-zero for example
 */
	    if( v->v_length == 1 ) {
		j = 0;
	    } else {
		j = i-1;
		if( i == 0 )
			continue;
	   }

            if (isreal(v)) {
                /* This isn't good but we may as well do
                 * something useful.
                 */
                gr_point(v, v->v_realdata[i],
                    v->v_realdata[i],
                    v->v_realdata[j],
                    v->v_realdata[j], (j==i ? 1 : i));
            } else {
                gr_point(v, realpart(&v->v_compdata[i]),
                    imagpart(&v->v_compdata[i]),
                    realpart(&v->v_compdata[j]),
                    imagpart(&v->v_compdata[j]), (j==i ? 1 : i));
            }
	}
        gr_end(v);
        return;
    }

    xs->v_flags |= VF_PERMANENT;

    /* First check the simple case, where we don't have to do any 
     * interpolation.
     */
    if ((degree == 1) && (gridsize == 0)) {
	dir = 0;
        for (i = 0, j = v->v_length; i < j; i++) {
            dx = isreal(xs) ? xs->v_realdata[i] : 
                    realpart(&xs->v_compdata[i]);
            dy = isreal(v) ? v->v_realdata[i] : 
                    realpart(&v->v_compdata[i]);
	    if (i == 0 || (dir > 0 ? lx > dx : dir < 0 ? lx < dx : 0)
		&& xs->v_plot->pl_scale == xs)
	    {
		gr_point(v, dx, dy, lx, ly, 0);
	    } else {
		gr_point(v, dx, dy, lx, ly, i);
		if (!dir)
		    dir = lx > dx ? -1 : lx < dx ? 1 : 0;
	    }
            lx = dx;
            ly = dy;
        }
        if (v->v_length == 1)
            gr_point(v, dx, dy, lx, ly, 1);
        gr_end(v);
        return;
    }

    if (gridsize < degree + 1)
        gridsize = 0;

    if (gridsize) {
        /* This is done quite differently from what we do below... */
        gridbuf = (double *) tmalloc(gridsize * sizeof (double));
        result = (double *) tmalloc(gridsize * sizeof (double));
        if (isreal(v))
            ydata = v->v_realdata;
        else {
            ydata = (double *) tmalloc(v->v_length * 
                    sizeof (double));
            for (i = 0; i < v->v_length; i++)
                ydata[i] = realpart(&v->v_compdata[i]);
        }
        if (isreal(xs))
            xdata = xs->v_realdata;
        else {
            xdata = (double *) tmalloc(xs->v_length * 
                    sizeof (double));
            for (i = 0; i < xs->v_length; i++)
                xdata[i] = realpart(&xs->v_compdata[i]);
        }
        
        mm = ft_minmax(xs, true);
        dx = (mm[1] - mm[0]) / gridsize;
        if (increasing)
            for (i = 0, dy = mm[0]; i < gridsize; i++, dy += dx)
                gridbuf[i] = dy;
        else
            for (i = 0, dy = mm[1]; i < gridsize; i++, dy -= dx)
                gridbuf[i] = dy;
        if (!ft_interpolate(ydata, result, xdata, v->v_length, gridbuf,
                gridsize, degree)) {
            fprintf(cp_err, "Error: can't put %s on gridsize %d\n",
                    v->v_name, gridsize);
            return;
        }
        /* Now this is a problem.  There's no way that we can
         * figure out where to put the tic marks to correspond with
         * the actual data...
         */
        for (i = 0; i < gridsize; i++)
            gr_point(v, gridbuf[i], result[i], gridbuf[i ? (i - 1)
                    : i], result[i ? (i - 1) : i], -1);
        gr_end(v);
        tfree(gridbuf);
        tfree(result);
        if (!isreal(v))
            tfree(ydata);
        if (!isreal(xs))
            tfree(xdata);
        return;
    }

    /* We need to do curve fitting now. First get some scratch 
     * space
     */
    scratch = (double *) tmalloc((degree + 1) * (degree + 2) * 
            sizeof (double));
    result = (double *) tmalloc((degree + 1) * sizeof (double));
    xdata = (double *) tmalloc((degree + 1) * sizeof (double));
    ydata = (double *) tmalloc((degree + 1) * sizeof (double));


    /* Plot the first degree segments... */
    if (isreal(v))
        bcopy((char *) v->v_realdata, (char *) ydata, 
                (degree + 1) * sizeof (double));
    else
        for (i = 0; i <= degree; i++)
            ydata[i] = realpart(&v->v_compdata[i]);
    if (isreal(xs))
        bcopy((char *) xs->v_realdata, (char *) xdata, 
                (degree + 1) * sizeof (double));
    else
        for (i = 0; i <= degree; i++)
            xdata[i] = realpart(&xs->v_compdata[i]);

    rot = false;
    while (!ft_polyfit(xdata, ydata, result, degree, scratch)) {
        /* Rotate the coordinate system 90 degrees and try again.
         * If it doesn't work this time, bump the interpolation
         * degree down by one...
         */
        if (ft_polyfit(ydata, xdata, result, degree, scratch)) {
            rot = true;
            break;
        }
        if (--degree == 0) {
            fprintf(cp_err, "plotcurve: Internal Error: ack...\n");
            return;
        }
    }

    /* Plot this part of the curve... */
    for (i = 0; i < degree; i++)
        if (rot)
            plotinterval(v, ydata[i], ydata[i + 1], result, degree,
                    true);
        else
            plotinterval(v, xdata[i], xdata[i + 1], result, degree,
                    false);

    /* Now plot the rest, piece by piece... l is the 
     * last element under consideration.
     */
    length = v->v_length;
    for (l = degree + 1; l < length; l++) {

        /* Shift the old stuff by one and get another value. */
        for (i = 0; i < degree; i++) {
            xdata[i] = xdata[i + 1];
            ydata[i] = ydata[i + 1];
        }
        if (isreal(v))
            ydata[i] = v->v_realdata[l];
        else
            ydata[i] = realpart(&v->v_compdata[l]);
        if (isreal(xs))
            xdata[i] = xs->v_realdata[l];
        else
            xdata[i] = realpart(&xs->v_compdata[l]);

        rot = false;
        while (!ft_polyfit(xdata, ydata, result, degree, scratch)) {
            if (ft_polyfit(ydata, xdata, result, degree, scratch)) {
                rot = true;
                break;
            }
            if (--degree == 0) {
                fprintf(cp_err, 
                    "plotcurve: Internal Error: ack...\n");
                return;
            }
        }
        if (rot)
            plotinterval(v, ydata[degree - 1], ydata[degree], 
                    result, degree, true);
        else
            plotinterval(v, xdata[degree - 1], xdata[degree],
                    result, degree, false);
    }
    tfree(scratch);
    tfree(xdata);
    tfree(ydata);
    tfree(result);
    gr_end(v);
    return;
}

#define GRANULARITY 10

static void
plotinterval(v, lo, hi, coeffs, degree, rotated)
    struct dvec *v;
    double lo, hi;
    register double *coeffs;
    bool rotated;
{
    double incr, dx, dy, lx, ly;
    register int i;
    int steps;

    /*
    fprintf(cp_err, "plotinterval(%s, %G, %G, [ ", v->v_name, lo, hi);
    for (i = 0; i <= degree; i++)
        fprintf(cp_err, "%G ", coeffs[i]);
    fprintf(cp_err, "], %d, %s)\n\r", degree, rotated ? "true" : "false");
    */

    /* This is a problem -- how do we know what granularity to use?  If
     * the guy cares about this he will use gridsize.
     */
    if (!cp_getvar("polysteps", VT_NUM, (char *) &steps))
        steps = GRANULARITY;

    incr = (hi - lo) / (double) (steps + 1);
    dx = lo + incr;
    lx = lo;
    ly = ft_peval(lo, coeffs, degree);
    for (i = 0; i <= steps; i++, dx += incr) {
        dy = ft_peval(dx, coeffs, degree);
        if (rotated)
            gr_point(v, dy, dx, ly, lx, -1);
        else
            gr_point(v, dx, dy, lx, ly, -1);
        lx = dx;
        ly = dy;
        /* fprintf(cp_err, "plot (%G, %G)\n\r", dx, dy); */
    }
    return;
}

#ifdef notdef

static void
printmat(name, mat, m, n)
    char *name;
    double *mat;
{
    int i, j;

    printf("\n\r=== Matrix: %s ===\n\r", name);
    for (i = 0; i < m; i++) {
        printf(" | ");
        for (j = 0; j < n; j++)
            printf("%G ", mat[i * n + j]);
        printf("|\n\r");
    }
    printf("===\n\r");
    return;
}

#endif

