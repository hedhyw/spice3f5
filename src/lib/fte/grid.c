/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
**********/

/*

    Routines to draw the various sorts of grids -- linear, log, polar.
*/

#include "spice.h"
#include "util.h"
#include "ftegraph.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "ftedata.h"
#include "fteparse.h"
#include "suffix.h"

#define RAD_TO_DEG	(180.0 / M_PI)

static double *lingrid(), *loggrid();
static void polargrid(), smithgrid();
static void drawpolargrid( );
static void drawsmithgrid( );

static void arcset();
static double cliparc();
static void adddeglabel(), addradlabel();
static void drawloggrid();
static void drawlingrid();

typedef enum { x_axis, y_axis } Axis;

/* note: scaleunits is static and never changed in this file
    ie, can get rid of it */
static bool scaleunits = true;

void
gr_fixgrid(graph, xdelta, ydelta, xtype, ytype)
    GRAPH *graph;
    double xdelta, ydelta;
    int xtype, ytype;
{
    double *dd;

    if (graph->grid.gridtype == GRID_NONE) {
        graph->grid.gridtype = GRID_LIN;
    }

    SetColor(1);
    SetLinestyle(1);

    if ((graph->data.xmin > graph->data.xmax)
            || (graph->data.ymin > graph->data.ymax)) {
      fprintf(cp_err,
        "gr_fixgrid: Internal Error - bad limits: %lg, %lg, %lg, %lg\r\n",
        graph->data.xmin, graph->data.xmax,
        graph->data.ymin, graph->data.ymax);
      return;
    }

    if (graph->grid.gridtype == GRID_POLAR) {
        graph->grid.circular = true;
        polargrid(graph);
        return;
    } else if (graph->grid.gridtype == GRID_SMITH
        || graph->grid.gridtype == GRID_SMITHGRID)
    {
        graph->grid.circular = true;
        smithgrid(graph);
        return;
    }
    graph->grid.circular = false;

    if ((graph->grid.gridtype == GRID_YLOG)
            || (graph->grid.gridtype == GRID_LOGLOG))
        dd = loggrid(graph, graph->data.ymin, graph->data.ymax,
                ytype, y_axis);
    else
        dd = lingrid(graph, graph->data.ymin, graph->data.ymax,
                ydelta, ytype, y_axis);

    graph->datawindow.ymin = dd[0];
    graph->datawindow.ymax = dd[1];

    if ((graph->grid.gridtype == GRID_XLOG)
            || (graph->grid.gridtype == GRID_LOGLOG))
        dd = loggrid(graph, graph->data.xmin, graph->data.xmax,
                xtype, x_axis);
    else
        dd = lingrid(graph, graph->data.xmin, graph->data.xmax,
                xdelta, xtype, x_axis);

    graph->datawindow.xmin = dd[0];
    graph->datawindow.xmax = dd[1];

/* do we really need this? */
/*
    SetLinestyle(0);
    DrawLine(graph->viewportxoff, graph->viewportyoff,
            graph->viewport.width + graph->viewportxoff,
            graph->viewportyoff);
    DrawLine(graph->viewportxoff, graph->viewportyoff,
            graph->viewportxoff,
            graph->viewport.height + graph->viewportyoff);
    SetLinestyle(1);
*/

    return;
}

gr_redrawgrid(graph)
GRAPH *graph;
{

    SetColor(1);
    SetLinestyle(1);
    /* draw labels */
    if (graph->grid.xlabel) {
      Text(graph->grid.xlabel,
          (int) (graph->absolute.width * 0.35),
          graph->fontheight);
    }
    if (graph->grid.ylabel) {
    	  if (graph->grid.gridtype == GRID_POLAR
    	          || graph->grid.gridtype == GRID_SMITH
    	          || graph->grid.gridtype == GRID_SMITHGRID) {
              Text(graph->grid.ylabel,
                   graph->fontwidth,
		   (graph->absolute.height * 3) / 4 );
	  } else {
              Text(graph->grid.ylabel,
                   graph->fontwidth,
		   graph->absolute.height / 2 );
	  }
    }

    switch( graph->grid.gridtype ) {
	case GRID_POLAR:
        	drawpolargrid(graph);
		break;
	case GRID_SMITH:
        	drawsmithgrid(graph);
		break;
	case GRID_SMITHGRID:
        	drawsmithgrid(graph);
		break;


    	case GRID_XLOG:
        case GRID_LOGLOG:
        	drawloggrid(graph,
               		graph->grid.xaxis.log.units,
			graph->grid.xaxis.log.hmt,
               		graph->grid.xaxis.log.lmt,
                	graph->grid.xaxis.log.decsp,
                	graph->grid.xaxis.log.subs,
                	graph->grid.xaxis.log.pp, x_axis);
		break;
	default:
        	drawlingrid(graph,
               		graph->grid.xaxis.lin.units,
               		graph->grid.xaxis.lin.spacing,
               		graph->grid.xaxis.lin.numspace,
               		graph->grid.xaxis.lin.distance,
               		graph->grid.xaxis.lin.lowlimit,
               		graph->grid.xaxis.lin.highlimit,
               		graph->grid.xaxis.lin.onedec,
               		graph->grid.xaxis.lin.mult,
               		graph->grid.xaxis.lin.tenpowmag
               		/ graph->grid.xaxis.lin.tenpowmagx,
               		graph->grid.xaxis.lin.digits,
               		x_axis);
		break;
     }

    switch( graph->grid.gridtype ) {
	case GRID_POLAR:
	case GRID_SMITH:
	case GRID_SMITHGRID:
		break;

    	case GRID_YLOG:
        case GRID_LOGLOG:
        	drawloggrid(graph,
                	graph->grid.yaxis.log.units,
			graph->grid.yaxis.log.hmt,
                	graph->grid.yaxis.log.lmt,
                	graph->grid.yaxis.log.decsp,
                	graph->grid.yaxis.log.subs,
                	graph->grid.yaxis.log.pp, y_axis);
		break;
	default:
        	drawlingrid(graph,
                	graph->grid.yaxis.lin.units,
                	graph->grid.yaxis.lin.spacing,
                	graph->grid.yaxis.lin.numspace,
                	graph->grid.yaxis.lin.distance,
                	graph->grid.yaxis.lin.lowlimit,
                	graph->grid.yaxis.lin.highlimit,
                	graph->grid.yaxis.lin.onedec,
                	graph->grid.yaxis.lin.mult,
               		graph->grid.yaxis.lin.tenpowmag
               		/ graph->grid.yaxis.lin.tenpowmagx,
               		graph->grid.yaxis.lin.digits,
                	y_axis);
		break;
     }

}

/* Plot a linear grid. Returns the new hi and lo limits. */
static double *
lingrid(graph, lo, hi, delta, type, axis)
    GRAPH *graph;
    double lo, hi;
    double delta;
    int type;
    Axis axis;
{
    int mag, mag2, mag3;
    double hmt, lmt, dst;
    int nsp;
    double tenpowmag, tenpowmag2, step, spacing;
    bool onedec = false;
    int margin;
    int max;
    static double dd[2];
    int mult = 1;
    char buf[16], *s;
    int slim, digits;

    if (axis == y_axis && graph->grid.ysized) {
	lmt = graph->grid.yaxis.lin.lowlimit;
	hmt = graph->grid.yaxis.lin.highlimit;
	tenpowmag = graph->grid.yaxis.lin.tenpowmag;
	dd[0] = lmt * tenpowmag;
	dd[1] = hmt * tenpowmag;
	return dd;
    }

    if (axis == x_axis && graph->grid.xsized) {
	lmt = graph->grid.xaxis.lin.lowlimit;
	hmt = graph->grid.xaxis.lin.highlimit;
	tenpowmag = graph->grid.xaxis.lin.tenpowmag;
	dd[0] = lmt * tenpowmag;
	dd[1] = hmt * tenpowmag;
	return dd;
    }

    if (delta < 0.0) {
        fprintf(cp_err, "Warning: %cdelta is negative -- reversed\n",
                (axis == x_axis) ? 'x' : 'y');
        delta = -delta;
    }

    mag2 = floor(log10(FABS(hi - lo)));
    tenpowmag2 = pow(10.0, (double) mag2);

    /* Round lo down, and hi up */

    /* First, round lo _up_ and hi _down_ out to the 3rd digit of accuracy */
    lmt = (ceil(1000 * lo / tenpowmag2)) / 1000.0;
    hmt = (floor(1000 * hi / tenpowmag2 + 0.9)) / 1000.0;

    lmt = floor(10.0 * lmt) / 10.0;
    hmt = ceil(10.0 * hmt) / 10.0;

    lo = lmt * tenpowmag2;
    hi = hmt * tenpowmag2;

    if (FABS(hi) > FABS(lo))
    	mag = floor(log10(FABS(hi)));
    else
    	mag = floor(log10(FABS(lo)));

    if (mag >= 0)
	mag3 = ((int) (mag / 3)) * 3;
    else
	mag3 = - ((int) ((2 - mag) / 3)) * 3;

    if (scaleunits)
	digits = mag3 - mag2;
    else {
	digits = mag - mag2;
	mag3 = mag;
    }

    if (digits < 1)
	digits = 0;

    if (axis == x_axis) {
      margin = graph->viewportxoff;
      /*max = graph->viewport.width + graph->viewportxoff;*/
      max = graph->absolute.width - graph->viewportxoff;
    } else {
      graph->viewportxoff = (digits + 5 + mag - mag3) * graph->fontwidth;
      margin = graph->viewportyoff;
      /*max = graph->viewport.height + graph->viewportyoff;*/
      max = graph->absolute.height - graph->viewportyoff;
    }

    /* Express the difference between the high and low values as
     * diff = d * 10^mag. We know diff >= 0.0.  If scaleunits is
     * set then make sure that mag is modulo 3.
     */

    dst = hmt - lmt;

    /* We have to go from lmt to hmt, so think of some useful places
     * to put grid lines. We will have a total of nsp lines, one
     * every spacing pixels, which is every dst / nsp units.
     */

    if (scaleunits) {
	static char scaleletters[ ] = "afpnum\0kMGT";
	char	*p;
	int	i, j;

        tenpowmag = pow(10.0, (double) mag3);

	*buf = 0;

	i = (mag3 + 18) / 3;

	if (i < 0)
		i = 6; /* No scale units */
	else if (i >= sizeof(scaleletters) - 1) {
		/* sizeof includes '\0' at end, which is useless */
		/* i = sizeof(scaleletters) - 2; */
		i = 6; /* No scale units */
	}

	j = mag3 - i * 3 + 18;
	if (j == 1)
		(void) sprintf(buf, "x10 ");
	else if (j == 2)
		(void) sprintf(buf, "x100 ");
	else if (j)
		(void) sprintf(buf, "x10^%d ", j);

	if (scaleletters[i]) {
		for (p = buf; *p; p++)
			;
		*p++ = scaleletters[i];
		*p++ = 0;
	}

    } else if (mag > 1) {
        tenpowmag = pow(10.0, (double) mag);
	(void) sprintf(buf, "x10^%d ", mag);
    }

    if (s = ft_typabbrev(type)) {
	(void) strcat(buf, s);
    } else {
	(void) strcat(buf, "Units");
    }

    if (delta == 0.0) {
	int	i;
	double	step;

	static struct { float div_lim, step; } div_list[ ] = {
		{ 100.0, 10.0 },
		{ 50.0, 5.0 },
		{ 20.0, 2.0 },
		{ 6.0, 1.0 },
		{ 3.0, 0.5 },
		{ 1.0, 0.2 },
		{ 0.5, 0.1 },
		{ 0.0, 0.05 },
		{ 0.0, 0.01 }
	};

	for (i = 0; i < NUMELEMS(div_list); i++) {
		if (dst > div_list[i].div_lim) {
			break;
		}
	}

	do {
		step = div_list[i].step;
		nsp = (dst + step - 0.0001) / step;
		spacing = (max - margin) / nsp;
		i += 1;
	} while (i < NUMELEMS(div_list) && spacing > 50);

	if (axis == x_axis) {
		slim = digits + 5 + mag - mag3;
	        slim = graph->fontwidth * (slim + 1);
	} else
	    slim = graph->fontheight * 3;

	while (i > 0 && spacing < slim + 3) {
		i -= 1;
		step = div_list[i].step;
		nsp = (dst + step - 0.0001) / step;
		spacing = (max - margin) / nsp;
	}

	if (lmt < 0)
		lmt = - ceil(-lmt / step) * step;
	else
		lmt = floor(lmt / step) * step;

	if (hmt < 0)
		hmt = - floor(-hmt / step) * step;
	else
		hmt = ceil(hmt / step) * step;

	dst = hmt - lmt;

	lo = lmt * tenpowmag2;
	hi = hmt * tenpowmag2;

	nsp = (dst + step - 0.0001) / step;

    } else {
	/* The user told us where to put the grid lines.  They will
	 * not be equally spaced in this case (i.e, the right edge
	 * won't be a line).
	 */
	nsp = (hi - lo) / delta;
	if (nsp > 100)
		nsp = 100;
	step = (max - margin) * delta / (hi - lo);
    }
    spacing = (max - margin) / nsp;

    dd[0] = lo;
    dd[1] = hi;

    /* Reset the max coordinate to deal with round-off error. */
    if (nsp && (delta == 0.0)) {
	if (axis == x_axis)
	    graph->viewport.width = spacing * nsp;
	else
	    graph->viewport.height = spacing * nsp;
    } else if (!nsp) {
	nsp = 1;
    }

    /* have to save non-intuitive variables left over
	from old algorithms for redraws */

    if (axis == x_axis) {
      graph->grid.xsized = 1;
      graph->grid.xaxis.lin.onedec = onedec;
      graph->grid.xaxis.lin.mult = mult;
      graph->grid.xaxis.lin.tenpowmag = tenpowmag2;
      graph->grid.xaxis.lin.tenpowmagx = tenpowmag;
      graph->grid.xaxis.lin.digits = digits;
      (void) strcpy(graph->grid.xaxis.lin.units, buf);
      graph->grid.xaxis.lin.distance = dst;
      graph->grid.xaxis.lin.lowlimit = lmt;
      graph->grid.xaxis.lin.highlimit = hmt;
      graph->grid.xaxis.lin.spacing = spacing;
      graph->grid.xaxis.lin.numspace = nsp;
    } else {
      graph->grid.ysized = 1;
      graph->grid.yaxis.lin.onedec = onedec;
      graph->grid.yaxis.lin.mult = mult;
      graph->grid.yaxis.lin.tenpowmag = tenpowmag2;
      graph->grid.yaxis.lin.tenpowmagx = tenpowmag;
      graph->grid.yaxis.lin.digits = digits;
      (void) strcpy(graph->grid.yaxis.lin.units, buf);
      graph->grid.yaxis.lin.distance = dst;
      graph->grid.yaxis.lin.lowlimit = lmt;
      graph->grid.yaxis.lin.highlimit = hmt;
      graph->grid.yaxis.lin.spacing = spacing;
      graph->grid.yaxis.lin.numspace = nsp;
    }

    return (dd);
}

static void
drawlingrid(graph, units, spacing, nsp, dst, lmt, hmt, onedec, mult, mag,
    digits, axis)
    GRAPH *graph;
    char units[16];
    bool onedec;
    int nsp, spacing, mult;
    double hmt, lmt, dst;
    double mag;
    int digits;
    Axis axis;
{

    int i, j;
    double m, step;
    char buf[16];

    /* i counts how many pixels we have drawn, and j counts which unit
     * we are at.
     */
    SetLinestyle(1);
    step = floor((double) dst / nsp * 100.0 + 0.000001);
    for (i = 0, m = lmt * 100.0; m - 0.001 <= hmt * 100.0;
      i += spacing, m += step)
    {
	j = m;
        if (j == 0)
            SetLinestyle(0);
        if (graph->grid.gridtype != GRID_NONE) {
            if (axis == x_axis)
                DrawLine(graph->viewportxoff + i,
                  graph->viewportyoff, graph->viewportxoff + i,
                  graph->viewport.height + graph->viewportyoff);
            else
                DrawLine(graph->viewportxoff,
                  graph->viewportyoff + i,
                  graph->viewport.width + graph->viewportxoff,
                  graph->viewportyoff + i);
        }
        if (j == 0)
            SetLinestyle(1);

	(void) sprintf(buf, "%.*f", digits + 1, m * mag / 100.0);

#ifdef notdef
	if (floor(step/10.0) != step/10.0)
		(void) sprintf(buf, "%.*lf", mag, m * mag / 100.0);
	else if (floor(step/100.0) != step/100.0)
		(void) sprintf(buf, "%.1lf", m * mag / 100.0);
	else
		(void) sprintf(buf, "%lg", j * mag / 100);
#endif

	if (axis == x_axis)
	    Text(buf, graph->viewportxoff + i -
		    strlen(buf) / 2 * graph->fontwidth,
		    (int) (graph->fontheight * 2.5));
	else
	    Text(buf, graph->viewportxoff -
		    graph->fontwidth * (strlen(buf)),
		    graph->viewportyoff + i -
		    graph->fontheight / 2);

        /* This is to make sure things work when delta > hi - lo. */
        if (nsp == 1)
            j += 1000;
    }
    if (axis == x_axis)
        Text(units, (int) (graph->absolute.width * 0.6),
            graph->fontheight);
    else
        Text(units, graph->fontwidth,
            (int) (graph->absolute.height - 2 * graph->fontheight));
    Update();

}

/* Plot a log grid.  Note that we pay no attention to x- and y-delta here. */
/* ARGSUSED */
static double *
loggrid(graph, lo, hi, type, axis)
    GRAPH *graph;
    double lo, hi;
    int type;
    Axis axis;
{
    static double dd[2];
    int margin;
    int max;
    int subs, pp, decsp, lmt, hmt;
    int i, j;
    double k;
    double decs;
    double mag, gain;
    char buf[32], *s;

    if (axis == x_axis && graph->grid.xsized) {
      lmt = graph->grid.xaxis.log.lmt;
      hmt = graph->grid.xaxis.log.hmt;
      dd[0] = pow(10.0, (double) lmt);
      dd[1] = pow(10.0, (double) hmt);
      return dd;
    } else if (axis == y_axis && graph->grid.ysized) {
      lmt = graph->grid.yaxis.log.lmt;
      hmt = graph->grid.yaxis.log.hmt;
      dd[0] = pow(10.0, (double) lmt);
      dd[1] = pow(10.0, (double) hmt);
      return dd;
    }

    if (axis == x_axis) {
      margin = graph->viewportxoff;
      max = graph->absolute.width - graph->viewportxoff;
    } else {
      margin = graph->viewportyoff;
      max = graph->absolute.height - graph->viewportyoff;
    }

    /* How many orders of magnitude.  We are already guaranteed that hi
     * and lo are positive.
     */

    lmt = floor(mylog10(lo));
    hmt = ceil(mylog10(hi));

    decs = hmt - lmt;

    pp = 1;
    decsp = (max - margin) / decs;

    if (decsp < 20) {
	pp = ceil(20.0 / decsp);
	decsp *= pp;
	subs = 1;
    } else if (decsp > 50) {
	static int divs[ ] = { 20, 10, 5, 4, 2, 1 };

	k = 5.0 / decsp;

	for (i = 0; i < NUMELEMS(divs) - 1; i++) {
		j = divs[i];
		if (-log10(((double) j - 1.0) / j) > k)
			break;
	}

	subs = divs[i];

    } else
	subs = 1;

    /* Start at a line */
    lmt = floor((double) lmt / pp) * pp;
    decs = hmt - lmt;
    decsp = (max - margin) / decs;

    dd[0] = pow(10.0, (double) lmt);
    dd[1] = pow(10.0, (double) hmt);

    if (s = ft_typabbrev(type)) {
	(void) strcpy(buf, s);
    } else {
	(void) strcpy(buf, "Units");
    }

    if (axis == x_axis) {
      (void) strcpy(graph->grid.xaxis.log.units, buf);
      graph->viewport.width = decs * decsp;
      graph->grid.xaxis.log.hmt = hmt;
      graph->grid.xaxis.log.lmt = lmt;
      graph->grid.xaxis.log.decsp = decsp;
      graph->grid.xaxis.log.subs = subs;
      graph->grid.xaxis.log.pp = pp;
      graph->grid.xsized = 1;
    } else {
      (void) strcpy(graph->grid.yaxis.log.units, buf);
      graph->viewport.height = decs * decsp;
      graph->grid.yaxis.log.hmt = hmt;
      graph->grid.yaxis.log.lmt = lmt;
      graph->grid.yaxis.log.decsp = decsp;
      graph->grid.yaxis.log.subs = subs;
      graph->grid.yaxis.log.pp = pp;
      graph->grid.ysized = 1;
    }

    return (dd);

}

static void
drawloggrid(graph, units, hmt, lmt, decsp, subs, pp, axis)
    GRAPH *graph;
    char *units;
    int hmt, lmt;
    int decsp, subs, pp;
    Axis axis;
{
    int i, j, k, l, m;
    double t;
    char buf[16];

    /* Now plot every pp'th decade line, with subs lines between them. */
    if (subs > 1)
	SetLinestyle(0);
    for (i = 0, j = lmt; j <= hmt; i += decsp * pp, j += pp) {
        /* Draw the decade line */
	if (graph->grid.gridtype != GRID_NONE) {
	    if (axis == x_axis)
		DrawLine(graph->viewportxoff + i,
		    graph->viewportyoff,
		    graph->viewportxoff + i,
		    graph->viewport.height
		      +graph->viewportyoff);
	    else
		DrawLine(graph->viewportxoff,
		    graph->viewportyoff + i,
		    graph->viewport.width
		      + graph->viewportxoff,
		    graph->viewportyoff + i);
	}
	if (j == -2)
	    (void) sprintf(buf, "0.01");
	else if (j == -1)
	    (void) sprintf(buf, "0.1");
	else if (j == 0)
	    (void) sprintf(buf, "1");
	else if (j == 1)
	    (void) sprintf(buf, "10");
	else if (j == 2)
	    (void) sprintf(buf, "100");
	else
	    (void) sprintf(buf, "10^%d", j);
	if (axis == x_axis)
	    Text(buf, graph->viewportxoff + i - strlen(buf) / 2,
		    (int) (graph->fontheight * 2.5));
	else
	    Text(buf, graph->viewportxoff - graph->fontwidth *
		    (strlen(buf) + 1),
		    graph->viewportyoff + i -
		    graph->fontheight / 2);

	if (j >= hmt)
	    break;

        /* Now draw the subdivision lines */
	if (subs > 1) {
            SetLinestyle(1);
	    t = 10.0 / subs;
	    for (k = ceil(subs / 10.0) + 1; k < subs; k++) {
		m = i + decsp * log10((double) t * k);
		if (graph->grid.gridtype != GRID_NONE) {
		    if (axis == x_axis)
			DrawLine(graph->viewportxoff + m,
			    graph->viewportyoff,
			    graph->viewportxoff + m,
			    graph->viewport.height
			      + graph->viewportyoff);
		    else
			DrawLine(graph->viewportxoff,
			    graph->viewportyoff + m,
			    graph->viewport.width
			      + graph->viewportxoff,
			    graph->viewportyoff + m);
		}
	    }
            SetLinestyle(0);
	}
    }
    if (axis == x_axis)
        Text(units, (int) (graph->absolute.width * 0.6),
            graph->fontheight);
    else
        Text(units, graph->fontwidth,
            (int) (graph->absolute.height - 2 * graph->fontheight));
    Update();
}

/* Polar grids */

static void
polargrid(graph)
GRAPH *graph;
{
    double d, mx, my, rad, tenpowmag;
    int hmt, lmt, mag;
    double minrad, maxrad, xx, yy;
    bool centered = false;

    /* Make sure that our area is square. */
    if (graph->viewport.width > graph->viewport.height) {
        graph->viewport.width =  graph->viewport.height;
    } else {
        graph->viewport.height = graph->viewport.width;
    }

    /* Make sure that the borders are even */
    if (graph->viewport.width & 1) {
	graph->viewport.width += 1;
	graph->viewport.height += 1;
    }
    graph->grid.xaxis.circular.center = graph->viewport.width / 2
	+ graph->viewportxoff;
    graph->grid.yaxis.circular.center = graph->viewport.height / 2
	+ graph->viewportyoff;

    graph->grid.xaxis.circular.radius = graph->viewport.width / 2;

    /* Figure out the minimum and maximum radii we're dealing with. */
    mx = (graph->data.xmin + graph->data.xmax) / 2;
    my = (graph->data.ymin + graph->data.ymax) / 2;
    d = sqrt(mx * mx + my * my);
    maxrad = d + (graph->data.xmax - graph->data.xmin) / 2;
    minrad = d - (graph->data.xmax - graph->data.xmin) / 2;

    if (maxrad == 0.0) {
        fprintf(cp_err, "Error: 0 radius in polargrid\n");
        return;
    }
    if ((graph->data.xmin < 0) && (graph->data.ymin < 0) &&
            (graph->data.xmax > 0) && (graph->data.ymax > 0))
        minrad = 0;
    if ((graph->data.xmin == - graph->data.xmax)
            && (graph->data.ymin == -graph->data.ymax)
            && (graph->data.xmin == graph->data.ymin))
        centered = true;

    mag = floor(mylog10(maxrad));
    tenpowmag = pow(10.0, (double) mag);
    hmt = maxrad / tenpowmag;
    lmt = minrad / tenpowmag;
    if (hmt * tenpowmag < maxrad)
        hmt++;
    if (lmt * tenpowmag > minrad)
        lmt--;
    maxrad = hmt * tenpowmag;
    minrad = lmt * tenpowmag;

    /* Make sure that the range is square */
    mx = graph->data.xmax - graph->data.xmin;
    my = graph->data.ymax - graph->data.ymin;
    graph->datawindow.xmin = graph->data.xmin;
    graph->datawindow.xmax = graph->data.xmax;
    graph->datawindow.ymin = graph->data.ymin;
    graph->datawindow.ymax = graph->data.ymax;
    if (mx > my) {
	graph->datawindow.ymin -= (mx - my) / 2;
	graph->datawindow.ymax += (mx - my) / 2;
    } else if (mx < my) {
	graph->datawindow.xmin -= (my - mx) / 2;
	graph->datawindow.xmax += (my - mx) / 2;
    }

    /* Range is square with upper bound maxrad */

#ifdef notdef
    xx = graph->datawindow.xmin + graph->datawindow.xmax;
    yy = graph->datawindow.ymin + graph->datawindow.ymax;
    graph->datawindow.xmin = xx - maxrad;
    graph->datawindow.xmax = xx + maxrad;
    graph->datawindow.ymin = yy - maxrad;
    graph->datawindow.ymax = yy + maxrad;
#endif


#ifdef notdef
    if (ft_grdb)
        printf("polar: maxrad = %g, center = (%g, %g)\n", maxrad, xx, yy);

    if ((minrad == 0) && ((hmt - lmt) > 5)) {
        if (!((hmt - lmt) % 2))
            step = 2;
        else if (!((hmt - lmt) % 3))
            step = 3;
        else
            step = 1;
    } else
        step = 1;

	graph->grid.xaxis.circular.lmt = lmt;
	graph->grid.yaxis.circular.lmt = step;
#endif

    graph->grid.xaxis.circular.hmt = hmt;
    graph->grid.xaxis.circular.lmt = lmt;
    graph->grid.xaxis.circular.mag = mag;

#ifdef notdef
    graph->datawindow.xmin = xx - maxrad;
    graph->datawindow.xmax = xx + maxrad;
    graph->datawindow.ymin = yy - maxrad;
    graph->datawindow.ymax = yy + maxrad;
#endif

}


static void
drawpolargrid(graph)
GRAPH *graph;
{
    double mx, my, rad, tenpowmag, theta;
    int hmt, lmt, i, step, mag;
    int relcx, relcy, relrad, dist, degs;
    int x1, y1, x2, y2;
    double minrad, maxrad, pixperunit, xx, yy;
    char buf[64];

#ifdef notdef
    step = graph->grid.yaxis.circular.lmt;
    mag = floor(mylog10(maxrad));
    tenpowmag = pow(10.0, (double) mag);
    pixperunit = graph->grid.xaxis.circular.radius / (maxrad - minrad);
#endif
    hmt = graph->grid.xaxis.circular.hmt;
    lmt = graph->grid.xaxis.circular.lmt;
    mag = graph->grid.xaxis.circular.mag;
    tenpowmag = pow(10.0, (double) mag);
    maxrad = hmt * tenpowmag;
    minrad = lmt * tenpowmag;

    if ((minrad == 0) && ((hmt - lmt) > 5)) {
	if (!((hmt - lmt) % 2))
	    step = 2;
	else if (!((hmt - lmt) % 3))
	    step = 3;
	else
	    step = 1;
	} else
	    step = 1;
    pixperunit = graph->grid.xaxis.circular.radius * 2 /
	(graph->datawindow.xmax - graph->datawindow.xmin);

    relcx = - (graph->datawindow.xmin + graph->datawindow.xmax) / 2
            * pixperunit;
    relcy = - (graph->datawindow.ymin + graph->datawindow.ymax) / 2
            * pixperunit;

    /* The distance from the center of the plotting area to the center of
     * the logical area.
     */
    dist = sqrt((double) (relcx * relcx + relcy * relcy));

    SetLinestyle(0);
    Arc(graph->grid.xaxis.circular.center,
            graph->grid.yaxis.circular.center,
            graph->grid.xaxis.circular.radius,
            (double) 0.0, (double) 0.0);
    SetLinestyle(1);

    /* Now draw the circles. */
    for (i = lmt;
	(relrad = i * tenpowmag * pixperunit)
	<= dist + graph->grid.xaxis.circular.radius;
	i += step)
    {
        cliparc((double) graph->grid.xaxis.circular.center + relcx,
                (double) graph->grid.yaxis.circular.center + relcy,
                (double) relrad, 0.0, 0.0,
                graph->grid.xaxis.circular.center,
                graph->grid.yaxis.circular.center,
                graph->grid.xaxis.circular.radius, 0);
        /* Toss on the label */
        if (relcx || relcy)
            theta = atan2((double) relcy, (double) relcx);
        else
            theta = M_PI;
        if (i && (relrad > dist - graph->grid.xaxis.circular.radius))
            addradlabel(graph, i, theta,
                (int) (graph->grid.xaxis.circular.center -
                    (relrad - dist) * cos(theta)),
                (int) (graph->grid.yaxis.circular.center
                    - (relrad - dist) * sin(theta)));
    }

    /* Now draw the spokes.  We have two possible cases -- first, the
     * origin may be inside the area -- in this case draw 12 spokes.
     * Otherwise, draw several spokes at convenient places.
     */
    if ((graph->datawindow.xmin <= 0.0)
            && (graph->datawindow.xmax >= 0.0)
            && (graph->datawindow.ymin <= 0.0)
            && (graph->datawindow.ymax >= 0.0)) {
        for (i = 0; i < 12; i++) {
            x1 = graph->grid.xaxis.circular.center + relcx;
            y1 = graph->grid.yaxis.circular.center + relcy;
            x2 = x1 + graph->grid.xaxis.circular.radius * 2
                    * cos(i * M_PI / 6);
            y2 = y1 + graph->grid.xaxis.circular.radius * 2
                    * sin(i * M_PI / 6);
            if (!clip_to_circle(&x1, &y1, &x2, &y2,
                    graph->grid.xaxis.circular.center,
                    graph->grid.yaxis.circular.center,
                    graph->grid.xaxis.circular.radius))
            {
	        DrawLine(x1, y1, x2, y2);
                /* Add a label here */
		/*XXXX*/
                adddeglabel(graph, i * 30, x2, y2, x1, y1,
		    graph->grid.xaxis.circular.center,
		    graph->grid.yaxis.circular.center);
            }
        }
    } else {
        /* Figure out the angle that we have to fill up */
        theta = 2 * asin((double) graph->grid.xaxis.circular.radius
                / dist);
        theta = theta * 180 / M_PI;   /* Convert to degrees. */

        /* See if we should put lines at 30, 15, 5, or 1 degree
         * increments.
         */
        if (theta / 30 > 3)
            degs = 30;
        else if (theta / 15 > 3)
            degs = 15;
        else if (theta / 5 > 3)
            degs = 5;
        else
            degs = 1;

        /* We'll be cheap */
        for (i = 0; i < 360; i+= degs) {
            x1 = graph->grid.xaxis.circular.center + relcx;
            y1 = graph->grid.yaxis.circular.center + relcy;
            x2 = x1 + dist * 2 * cos(i * M_PI / 180);
            y2 = y1 + dist * 2 * sin(i * M_PI / 180);
            if (!clip_to_circle(&x1, &y1, &x2, &y2,
                    graph->grid.xaxis.circular.center,
                    graph->grid.yaxis.circular.center,
                    graph->grid.xaxis.circular.radius)) {
                DrawLine(x1, y1, x2, y2);
                /* Put on the label */
                adddeglabel(graph, i, x2, y2, x1, y1,
		    graph->grid.xaxis.circular.center,
		    graph->grid.yaxis.circular.center);
            }
        }
    }

    (void) sprintf(buf, "e%d", mag);
    Text(buf, graph->grid.xaxis.circular.center
              + graph->grid.xaxis.circular.radius,
            graph->grid.yaxis.circular.center
              - graph->grid.xaxis.circular.radius);
    Update();
    return;
}

/* Put a degree label on the screen, with 'deg' as the label, near point (x, y)
 * such that the perpendicular to (cx, cy) and (x, y) doesn't overwrite the
 * label.  If the distance between the center and the point is
 * too small, don't put the label on.
 */

#define LOFF    5
#define MINDIST 10

static void
adddeglabel(graph, deg, x, y, cx, cy, lx, ly)
    GRAPH *graph;
    int deg, x, y, cx, cy, lx, ly;
{
    char buf[8];
    int d, w, h;
    double angle;

    if (sqrt((double) (x - cx) * (x - cx) + (y - cy) * (y - cy)) < MINDIST)
        return;
    (void) sprintf(buf, "%d", deg);
    w = graph->fontwidth * (strlen(buf) + 1);
    h = graph->fontheight * 1.5;
    angle = atan2((double) (y - ly), (double) (x - lx));
    d = fabs(cos(angle)) * w / 2 + fabs(sin(angle)) * h / 2 + LOFF;

    x = x + d * cos(angle) - w / 2;
    y = y + d * sin(angle) - h / 2;

    Text(buf, x, y);
    Text("o", x + strlen(buf) * graph->fontwidth,
            y + graph->fontheight / 2, 0);
    return;
}

/* This is kind of wierd. If dist = 0, then this is the normal case, where
 * the labels should go along the positive X-axis.  Otherwise, to make
 * sure that all circles drawn have labels, put the label near the circle
 * along the line from the logical center to the physical center.
 */

static void
addradlabel(graph, lab, theta, x, y)
    GRAPH *graph;
    double theta;
    int lab, x, y;
{
    char buf[32];

    (void) sprintf(buf, "%d", lab);
    if (theta == M_PI) {
        y = y - graph->fontheight - 2;
        x = x - graph->fontwidth * strlen(buf) - 3;
    } else
	x = x - graph->fontwidth * strlen(buf) - 3;
    Text(buf, x, y);
    return;
}


/* Smith charts. */

#define gr_xcenter	graph->grid.xaxis.circular.center
#define gr_ycenter	graph->grid.yaxis.circular.center
#define gr_radius	graph->grid.xaxis.circular.radius
#define gi_fntwidth	graph->fontwidth
#define gi_fntheight	graph->fontheight
#define gi_maxx		graph->viewport.width+graph->viewportxoff
#define gr_xmargin	graph->viewportxoff
#define gr_ymargin	graph->viewportyoff

static void
smithgrid(graph)
GRAPH *graph;
{
    double mx, my;
    bool centered = false;

    SetLinestyle(0);

    /* Make sure that our area is square. */
    if (graph->viewport.width > graph->viewport.height) {
        graph->viewport.width =  graph->viewport.height;
    } else {
        graph->viewport.height = graph->viewport.width;
    }

    /* Make sure that the borders are even */
    if (graph->viewport.width & 1) {
	graph->viewport.width += 1;
	graph->viewport.height += 1;
    }

    graph->grid.xaxis.circular.center = graph->viewport.width / 2
                        + graph->viewportxoff;
    graph->grid.yaxis.circular.center = graph->viewport.height / 2
                        + graph->viewportyoff;
    graph->grid.xaxis.circular.radius = graph->viewport.width / 2;


    /* We have to make sure that the range is square. */
    graph->datawindow.xmin = graph->data.xmin;
    graph->datawindow.xmax = graph->data.xmax;
    graph->datawindow.ymin = graph->data.ymin;
    graph->datawindow.ymax = graph->data.ymax;

    if (graph->datawindow.ymin > 0)
	graph->datawindow.ymin *= -1;
    if (graph->datawindow.xmin > 0)
	graph->datawindow.xmin *= -1;

    if (graph->datawindow.ymax < 0)
	graph->datawindow.ymax *= -1;
    if (graph->datawindow.xmax < 0)
	graph->datawindow.xmax *= -1;

    if (fabs(graph->datawindow.ymin) > fabs(graph->datawindow.ymax))
        graph->datawindow.ymax = - graph->datawindow.ymin;
    else
        graph->datawindow.ymin = - graph->datawindow.ymax;

    if (fabs(graph->datawindow.xmin) > fabs(graph->datawindow.xmax))
        graph->datawindow.xmax = - graph->datawindow.xmin;
    else
        graph->datawindow.xmin = - graph->datawindow.xmax;

    mx = graph->datawindow.xmax - graph->datawindow.xmin;
    my = graph->datawindow.ymax - graph->datawindow.ymin;
    if (mx > my) {
        graph->datawindow.ymin -= (mx - my) / 2;
        graph->datawindow.ymax += (mx - my) / 2;
    } else if (mx < my) {
        graph->datawindow.xmin -= (my - mx) / 2;
        graph->datawindow.xmax += (my - mx) / 2;
    }

    if ((graph->datawindow.xmin == - graph->datawindow.xmax) &&
	    (graph->datawindow.ymin == -
	    graph->datawindow.ymax) && (graph->datawindow.xmin ==
	    graph->datawindow.ymin))
	centered = true;

#ifdef notdef
    /* Figure out the minimum and maximum radii we're dealing with. */
    mx = (graph->datawindow.xmin + graph->datawindow.xmax) / 2;
    my = (graph->datawindow.ymin + graph->datawindow.ymax) / 2;
    d = sqrt(mx * mx + my * my);
    maxrad = d + (graph->datawindow.xmax - graph->datawindow.xmin) / 2;
    minrad = d - (graph->datawindow.xmax - graph->datawindow.xmin) / 2;
#endif

    /* Issue a warning if our data range is not normalized */
    if (graph->datawindow.ymax > 1.1) {
	printf("\nwarning: exceeding range for smith chart");
	printf("\nplease normalize your data to -1 < r < +1\n");
    }

}

/* maximum number of circles */
#define CMAX  50

static void
drawsmithgrid(graph)
GRAPH *graph;
{
    double mx, my, rad, tenpowmag, d, dphi[CMAX], minrad, maxrad, rnorm[CMAX];
    double pixperunit;
    int mag, i, j, k;
    double ir[CMAX], rr[CMAX], ki[CMAX], kr[CMAX], ks[CMAX];
    int xoff, yoff, zheight;
    int basemag, plen;
    char buf[64], plab[32], nlab[32];

    /* Figure out the minimum and maximum radii we're dealing with. */
    mx = (graph->datawindow.xmin + graph->datawindow.xmax) / 2;
    my = (graph->datawindow.ymin + graph->datawindow.ymax) / 2;
    d = sqrt(mx * mx + my * my);
    maxrad = d + (graph->datawindow.xmax - graph->datawindow.xmin) / 2;
    minrad = d - (graph->datawindow.xmax - graph->datawindow.xmin) / 2;

    mag = floor(mylog10(maxrad));
    tenpowmag = pow(10.0, (double) mag);

    pixperunit = graph->viewport.width / (graph->datawindow.xmax -
            graph->datawindow.xmin);

    xoff = - pixperunit * (graph->datawindow.xmin + graph->datawindow.xmax) / 2;
    yoff = - pixperunit * (graph->datawindow.ymin + graph->datawindow.ymax) / 2;

    /* Sweep the range from 10e-20 to 10e20.  If any arcs fall into the
     * picture, plot the arc set.
     */
    for (mag = -20; mag < 20; mag++) {
        i = gr_radius * pow(10.0, (double) mag) / maxrad;
        if (i > 10) {
            j = 1;
            break;
        } else if (i > 5) {
            j = 2;
            break;
        } else if (i > 2) {
            j = 5;
            break;
        }
    }
    k = 1;

    /* SetLinestyle(1); takes too long */
    /* Problems with Suns on very large radii && linestyle */
    SetLinestyle(0);

    /* Now plot all the arc sets.  Go as high as 5 times the radius that
     * will fit on the screen.  The base magnitude is one more than
     * the least magnitude that will fit...
     */
    if (i > 20)
        basemag = mag;
    else
        basemag = mag + 1;
    /* Go back one order of magnitude and have a closer look */
    mag -= 2;
    j *= 10;
    while (mag < 20) {
        i = j * pow(10.0, (double) mag) * pixperunit / 2;
        if (i / 5 > gr_radius + ((xoff > 0) ? xoff : - xoff))
            break;
	rnorm[k] = j * pow(10.0, (double) (mag - basemag));
	dphi[k] = 2.0 * atan(rnorm[k]);
	ir[k] = pixperunit * (1 + cos(dphi[k])) / sin(dphi[k]);
	rr[k] = pixperunit * 0.5 * (((1 - rnorm[k]) / (1 + rnorm[k])) + 1);
        (void) sprintf(plab, "%lg", rnorm[k]);
	plen = strlen(plab);

	/* See if the label will fit on the upper xaxis */
	/* wait for some k, so we don't get fooled */
	if (k > 6) {
	if ((int) (gr_radius - xoff - pixperunit + 2 * rr[k]) <
	     plen * gi_fntwidth + 2)
	    break;
	}
	/* See if the label will fit on the lower xaxis */
	/* First look at the leftmost circle possible*/
	if ((int) (pixperunit - 2 * rr[k] + gr_radius + xoff +
	    fabs((double) yoff)) < plen * gi_fntwidth + 4) {
	    if (j == 95) {
		j = 10;
		mag++;
	    } else {
		if (j < 20)
		  j += 1;
		else
		  j += 5;
	    }
	    continue;
	}
	/* Then look at the circles following in the viewport */
	if (k>1 && (int) 2 * (rr[k-1] - rr[k]) < plen * gi_fntwidth + 4) {
        if (j == 95) {
            j = 10;
            mag++;
        } else {
	    if (j < 20)
	      j += 1;
	    else
	      j += 5;
	}
	continue;
	}
        if (j == 95) {
            j = 10;
            mag++;
        } else {
	    if (j < 20)
	      j += 1;
	    else
	      j += 5;
	}
	ki[k-1] = ir[k];
        kr[k-1] = rr[k];
	k++;
	if (k == CMAX) {
	    printf("drawsmithgrid: grid too complex\n");
	    break;
	}
    }
    k--;

    /* Now adjust the clipping radii */
    for (i = 0; i < k; i++)
	ks[i] = ki[i];
    for (i = k-1, j = k-1; i >= 0; i -= 2, j--) {
	ki[i] = ks[j];
	if (i > 0)
	    ki[i-1] = ks[j];
    }
    for (i = 0; i < k; i++)
	ks[i] = kr[i];
    for (i = k-1, j = k-1; (i >= 0) && (dphi[i] > M_PI / 2); i -= 2, j--) {
	kr[i] = ks[j];
	if (i > 0)
	    kr[i-1] = ks[j];
    }
    for ( ; i >= 0; i--, j--)
	kr[i] = ks[j];

    if ((yoff > - gr_radius) && (yoff < gr_radius)) {
        zheight = gr_radius * cos(asin((double) yoff / gr_radius));
	zheight = (zheight > 0) ? zheight : - zheight;
    } else {
	zheight = gr_radius;
    }
    for (ki[k] = kr[k] = (double) 0; k > 0; k--) {
        (void) sprintf(plab, "%lg", rnorm[k]);
        (void) sprintf(nlab, "-%lg", rnorm[k]);
        arcset(graph, rr[k], kr[k], ir[k], ki[k], pixperunit,
		gr_radius, gr_xcenter, gr_ycenter,
                xoff, yoff, plab, nlab,
		(int) (0.5 + RAD_TO_DEG * (M_PI - dphi[k])),
		(int) (0.5 + RAD_TO_DEG * (M_PI + dphi[k])),
		gr_xcenter - zheight,
		gr_xcenter + zheight);
    }
    if (mag == 20) {
        fprintf(cp_err, "smithgrid: Internal Error: screwed up\n");
        return;
    }

    SetLinestyle(0);

    Arc(gr_xcenter, gr_ycenter, gr_radius, 0.0, 0.0);
/*
    if ((xoff > - gr_radius) && (xoff < gr_radius)) {
        zheight = gr_radius * sin(acos((double) xoff / gr_radius));
        if (zheight < 0)
            zheight = - zheight;
        DrawLine(gr_xcenter + xoff, gr_ycenter - zheight,
                gr_xcenter + xoff, gr_ycenter + zheight);
    }
 */
    if ((yoff > - gr_radius) && (yoff < gr_radius)) {
        zheight = gr_radius * cos(asin((double) yoff / gr_radius));
        if (zheight < 0)
            zheight = - zheight;
        DrawLine(gr_xcenter - zheight, gr_ycenter + yoff,
                gr_xcenter + zheight, gr_ycenter + yoff);
	Text("0", gr_xcenter + zheight + gi_fntwidth, gr_ycenter + yoff -
            gi_fntheight / 2);
	Text("o", gr_xcenter + zheight + gi_fntwidth * 2, gr_ycenter + yoff);
	Text("180", gr_xcenter - zheight - gi_fntwidth * 5, gr_ycenter
            + yoff - gi_fntheight / 2);
	Text("o", gr_xcenter - zheight - gi_fntwidth * 2, gr_ycenter + yoff);
    }

/*    (void) sprintf(buf, "e%d", basemag); */
    (void) sprintf(buf, "e%d", 0);
    Text(buf, gr_xcenter + gr_radius, gr_ycenter - gr_radius);

    Update();
    return;
}

/* Draw one arc set.  The arcs should have radius rad. The outermost circle is
 * described by (centx, centy) and maxrad, and the distance from the right side
 * of the bounding circle to the logical center of the other circles in pixels
 * is xoffset (positive brings the negative plane into the picture).
 * plab and nlab are the labels to put on the positive and negative X-arcs,
 * respectively...  If the X-axis isn't on the screen, then we have to be
 * clever...
 */

static void
arcset(graph, rad, prevrad, irad, iprevrad, radoff, maxrad, centx, centy,
       xoffset, yoffset, plab, nlab, pdeg, ndeg, pxmin, pxmax)
    GRAPH *graph;
    int ndeg, pdeg, pxmin, pxmax ;
    double rad, prevrad, irad, iprevrad, radoff;
    int maxrad, centx, centy, xoffset, yoffset;
    char *plab, *nlab;
{
    char buf[64];
    double aclip;
    double angle = atan2((double) iprevrad, (double) rad);
    double iangle = atan2((double) prevrad, (double) irad);
    int x, xlab, ylab;

    /* Let's be lazy and just draw everything -- we won't get called too
     * much and the circles get clipped anyway...
     */
    SetColor(18);

    cliparc((double) (centx + xoffset + radoff - rad),
	    (double) (centy + yoffset), rad, 2*angle,
            2 * M_PI - 2 * angle, centx, centy, maxrad, 0);


    /* These circles are not part of the smith chart
     * Let's draw them anyway
     */
    cliparc((double) (centx + xoffset + radoff + rad),
	    (double) (centy + yoffset), rad, M_PI + 2 * angle,
            M_PI - 2 * angle, centx, centy, maxrad, 0);

    /* Draw the upper and lower circles.  */
    SetColor(19);
    aclip = cliparc((double) (centx + xoffset + radoff),
	    (double) (centy + yoffset + irad), irad,
	    (double) (M_PI * 1.5 + 2 * iangle),
            (double) (M_PI * 1.5 - 2 * iangle), centx, centy, maxrad, 1);
    if ((aclip > M_PI / 180) && (pdeg > 1)) {
	xlab = centx + xoffset + radoff + irad * cos(aclip);
	ylab = centy + yoffset + irad * (1 + sin(aclip));
	if ((ylab - gr_ycenter) > graph->fontheight) {
	SetColor(1);
	adddeglabel(graph, pdeg, xlab, ylab,
	    gr_xcenter, gr_ycenter, gr_xcenter, gr_ycenter);
/*
	ylab = centy + yoffset - irad * (1 + sin(aclip));
	adddeglabel(graph, ndeg, xlab, ylab,
	    gr_xcenter, gr_ycenter, gr_xcenter, gr_ycenter);
 */
	SetColor(19);
	}
    }
    aclip = cliparc((double) (centx + xoffset + radoff),
	    (double) (centy + yoffset - irad), irad,
	    (double) (M_PI / 2 + 2 * iangle),
            (double) (M_PI / 2 - 2 * iangle), centx, centy, maxrad,
	    (iangle == 0)?2:0);
    if ((aclip >= 0 && aclip < 2*M_PI - M_PI/180) && (pdeg < 359)) {
	xlab = centx + xoffset + radoff + irad * cos(aclip);
	ylab = centy + yoffset + irad * (sin(aclip) - 1);
	SetColor(1);
	adddeglabel(graph, ndeg, xlab, ylab,
	    gr_xcenter, gr_ycenter, gr_xcenter, gr_ycenter);
	SetColor(19);
    }

    /* Now toss the labels on... */
    SetColor(1);

    x = centx + xoffset + (int)radoff - 2 * (int)rad -
	gi_fntwidth * strlen(plab) - 2;
    if ((x > pxmin) && (x < pxmax)) {
	if ((yoffset > - gr_radius) && (yoffset < gr_radius))
	    Text(plab, x, centy + yoffset - gi_fntheight - 1);
	else
	    Text(plab, x, gr_ymargin - 3 * gi_fntheight - 2);
    }
/*
    x = centx + xoffset + (int) radoff + 2 * (int)rad -
	gi_fntwidth * strlen(nlab) - 2;
    if ((x > gr_xmargin) && (x < gi_maxx))
        Text(nlab, x, centy + yoffset - gi_fntheight - 1);
 */

    return;
}

/* This routine draws an arc and clips it to a circle.  It's hard to figure
 * out how it works without looking at the piece of scratch paaper I have
 * in front of me, so let's hope it doesn't break...
 * Converted to all doubles for CRAYs
 */

static double
cliparc(cx, cy, rad, start, end, iclipx, iclipy, icliprad, flag)
    double cx, cy, rad;
    int iclipx, iclipy, icliprad, flag;
    double start, end;
{
    double clipx, clipy, cliprad;
    double x, y, tx, ty, dist;
    double alpha, theta, phi, a1, a2, d, l;
    double sclip, eclip;
    bool in;

    clipx = (double) iclipx;
    clipy = (double) iclipy;
    cliprad = (double) icliprad;
    x = cx - clipx;
    y = cy - clipy;
    dist = sqrt((double) (x * x + y * y));

    if (!rad || !cliprad)
        return(-1);
    if (dist + rad < cliprad) {
        /* The arc is entirely in the boundary. */
        Arc((int)cx, (int)cy, (int)rad, start, end);
        return(flag?start:end);
    } else if ((dist - rad >= cliprad) || (rad - dist >= cliprad)) {
        /* The arc is outside of the boundary. */
        return(-1);
    }
    /* Now let's figure out the angles at which the arc crosses the
     * circle. We know dist != 0.
     */
    if (x)
        phi = atan2((double) y, (double) x);
    else if (y > 0)
        phi = M_PI * 1.5;
    else
        phi = M_PI / 2;
    if (cx > clipx)
        theta = M_PI + phi;
    else
        theta = phi;

    alpha = (double) (dist * dist + rad * rad - cliprad * cliprad) /
            (2 * dist * rad);

    /* Sanity check */
    if (alpha > 1.0)
	alpha = 0.0;
    else if (alpha < -1.0)
	alpha = M_PI;
    else
        alpha = acos(alpha);

    a1 = theta + alpha;
    a2 = theta - alpha;
    while (a1 < 0)
        a1 += M_PI * 2;
    while (a2 < 0)
        a2 += M_PI * 2;
    while (a1 >= M_PI * 2)
        a1 -= M_PI * 2;
    while (a2 >= M_PI * 2)
        a2 -= M_PI * 2;

    tx = cos(start) * rad + x;
    ty = sin(start) * rad + y;
    d = sqrt((double) tx * tx + ty * ty);
    in = (d > cliprad) ? false : true;

    /* Now begin with start.  If the point is in, draw to either end, a1,
     * or a2, whichever comes first.
     */
    d = M_PI * 3;
    if ((end < d) && (end > start))
        d = end;
    if ((a1 < d) && (a1 > start))
        d = a1;
    if ((a2 < d) && (a2 > start))
        d = a2;
    if (d == M_PI * 3) {
        d = end;
        if (a1 < d)
            d = a1;
        if (a2 < d)
            d = a2;
    }

    if (in) {
	if (start > d) {
	    double tmp;
	    tmp = start;
	    start = d;
	    d = tmp;
	}
        Arc((int)cx, (int)cy, (int)rad, start, d);
	sclip = start;
	eclip = d;
    }
    if (d == end)
        return(flag?sclip:eclip);
    if (a1 != a2)
        in = in ? false : true;

    /* Now go from here to the next point. */
    l = d;
    d = M_PI * 3;
    if ((end < d) && (end > l))
        d = end;
    if ((a1 < d) && (a1 > l))
        d = a1;
    if ((a2 < d) && (a2 > l))
        d = a2;
    if (d == M_PI * 3) {
        d = end;
        if (a1 < d)
            d = a1;
        if (a2 < d)
            d = a2;
    }

    if (in) {
        Arc((int)cx, (int)cy, (int)rad, l, d);
	sclip = l;
	eclip = d;
    }
    if (d == end)
        return(flag?sclip:eclip);
    in = in ? false : true;

    /* And from here to the end. */
    if (in) {
        Arc((int)cx, (int)cy, (int)rad, d, end);
	/* special case */
	if (flag != 2) {
	  sclip = d;
	  eclip = end;
	}
    }
    return(flag%2?sclip:eclip);
}
