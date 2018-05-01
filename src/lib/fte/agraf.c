/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Line-printer (ASCII) plots.
 */

#include "spice.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "ftedata.h"
#include "fteparse.h"
#include "suffix.h"

#define FUDGE		7
#define MARGIN_BASE	11
#define LCHAR		'.'
#define MCHAR		'X'
#define PCHARS		"+*=$%!0123456789"

/* We should really deal with the xlog and delta arguments.  This routine is
 * full of magic numbers that make the formatting correct.
 */

/* ARGSUSED */
void
ft_agraf(xlims, ylims, xscale, plot, vecs, xdel, ydel, xlog, ylog, nointerp)
    double *xlims, *ylims;
    struct dvec *xscale, *vecs;
    struct plot *plot;
    double xdel, ydel;
    bool xlog, ylog, nointerp;
{
    int height;
    bool nobreakp, novalue;
    int maxx, maxy, omaxy;  /* The size of the plotting area. */
    bool /* xlogscale = false, */ ylogscale = false;
    char *field, buf[BSIZE_SP];
    char *line1, *line2, c, cb;
    double xrange[2], yrange[2], x1, x2, yy1, y2, x, y;
    int mag, hmt, lmt, dst, spacing, nsp, ypt, upper, lower, curline;
    double tenpowmag, diff, *values;
    struct dvec *v;
    int margin = MARGIN_BASE;
    int omargin;
    register int i, j, k;
    int shift;


    /* ANSI C does not specify how many digits are in an exponent for %c
     * We assumed it was 2.  If it's more, shift starting position over.
     */
    (void) sprintf(buf, "%1.1e", 0.0);		/* expect 0.0e+00 */
    shift = strlen(buf) - 7;
    margin += shift;

    /* Make sure the margin is correct */
    omargin = margin;
    if (!cp_getvar("noasciiplotvalue", VT_BOOL, (char *) &novalue) &&
            !vec_eq(xscale, vecs)) {
        margin *= 2;
    } else
        novalue = true;
    if ((xscale->v_gridtype == GRID_YLOG) ||
            (xscale->v_gridtype == GRID_LOGLOG))
        ylogscale = true;
    if (!cp_getvar("width", VT_NUM, (char *) &maxy)) {
      maxy = DEF_WIDTH;
    }
    if (!cp_getvar("height", VT_NUM, (char *) &height))
        height = DEF_HEIGHT;
    if (ft_nopage)
	nobreakp = true;
    else
   	cp_getvar("nobreak", VT_BOOL, (char *) &nobreakp);
    maxy -= (margin + FUDGE);
    maxx = xscale->v_length;
    xrange[0] = xlims[0];
    xrange[1] = xlims[1];
    yrange[0] = ylims[0];
    yrange[1] = ylims[1];

    if (maxx < 2) {
        fprintf(cp_err,
            "Error: asciiplot can't handle scale with length < 2\n");
        return;
    }

    if (maxx <= 0) {
        fprintf(cp_err, "Note: no points to plot\n");
        return;
    }

    for (v = vecs, i = 0; v; v = v->v_link2) {
        v->v_linestyle = (PCHARS[i] ? PCHARS[i++] : '#');
    }
    /* Now allocate the field and stuff. */
    field = tmalloc((maxy + 1) * (maxx + 1));
    line1 = tmalloc(maxy + margin + FUDGE + 1);
    line2 = tmalloc(maxy + margin + FUDGE + 1);
    if (!novalue)
        values = (double *) tmalloc(maxx * sizeof (double));

    /* Clear the field, put the lines in the right places, and create
     * the headers.
     */
    for (i = 0, j = (maxx + 1) * (maxy + 1); i < j; i++)
        field[i] = ' ';
    for (i = 0, j = maxy + margin + FUDGE; i < j; i++) {
        line1[i] = '-';
        line2[i] = ' ';
    }
    line1[j] = line2[j] = '\0';

    /* The following is similar to the stuff in grid.c */
    if ((xrange[0] > xrange[1]) || (yrange[0] > yrange[1])) {
        fprintf(cp_err,
    "ft_agraf: Internal Error: bad limits %lg, %lg, %lg, %lg\n",
                xrange[0], xrange[1], yrange[0], yrange[1]);
        return;
    }

    /* gcc doesn't like !double */
    if (ylims[1] == 0.0) {
        mag = (int) floor(mylog10(- ylims[0]));
        tenpowmag = pow(10.0, (double) mag);
    } else if (ylims[0] == 0.0) {
        mag = (int) floor(mylog10(ylims[1]));
        tenpowmag = pow(10.0, (double) mag);
    } else {
        diff = ylims[1] - ylims[0];
        mag = (int) floor(mylog10(diff));
        tenpowmag = pow(10.0, (double) mag);
    }

    lmt = (int) floor(ylims[0] / tenpowmag);
    yrange[0] = ylims[0] = lmt * tenpowmag;
    hmt = (int) ceil(ylims[1] / tenpowmag);
    yrange[1] = ylims[1] = hmt * tenpowmag;

    dst = hmt - lmt;

    /* This is a strange case; I don't know why it's here. */
    if (dst == 11)
        dst = 12;
    else if (dst == 1) {
        dst = 10;
        mag++;
        hmt *= 10.0;
        lmt *= 10.0;
    } else if (dst == 0) {
        dst = 2;
	lmt -= 1;
	hmt += 1;
    }

    for (nsp = 4; nsp < 8; nsp++)
        if (!(dst % nsp))
            break;
    if (nsp == 8)
        for (nsp = 2; nsp < 4; nsp++)
            if (!(dst % nsp))
                break;
    spacing = maxy / nsp;

    /* Reset the max X coordinate to deal with round-off error. */
    omaxy = maxy + 1;
    maxy = spacing * nsp;

    for (i = 0, j = lmt; j <= hmt; i += spacing, j += dst / nsp) {
        for (k = 0; k < maxx; k++)
            field[k * omaxy + i] = LCHAR;
        line1[i + margin + 2 * shift] = '|';
        (void) sprintf(buf, "%.2e", j * pow(10.0, (double) mag));
        bcopy(buf, &line2[i + margin - ((j < 0) ? 2 : 1) - shift],
                strlen(buf));
    }
    line1[i - spacing + margin + 1] = '\0';

    for (i = 1; i < omargin - 1 && xscale->v_name[i - 1]; i++)
        line2[i] = xscale->v_name[i - 1];
    if (!novalue)
        for (i = omargin + 1;
		i < margin - 2 && (vecs->v_name[i - omargin - 1]);
		i++)
            line2[i] = vecs->v_name[i - omargin - 1];

    /* Now the buffers are all set up properly. Plot points for each
     * vector using interpolation. For each point on the x-axis, find the
     * two bracketing points in xscale, and then interpolate their
     * y values for each vector.
     */

    upper = lower = 0;
    for (i = 0; i < maxx; i++) {
        if (nointerp)
            x = isreal(xscale) ? xscale->v_realdata[i] :
                    realpart(&xscale->v_compdata[i]);
        else if (xlog && xrange[0] > 0.0 && xrange[1] > 0.0)
	    x = xrange[0] * pow( 10.0, mylog10(xrange[1]/xrange[0])
			* i / (maxx - 1));
        else
            x = xrange[0] + (xrange[1] - xrange[0]) * i /
                    (maxx - 1);
        while ((isreal(xscale) ? (xscale->v_realdata[upper] < x) :
                (realpart(&xscale->v_compdata[upper]) < x)) &&
                (upper < xscale->v_length - 1))
            upper++;
        while ((isreal(xscale) ? (xscale->v_realdata[lower] < x) :
                (realpart(&xscale->v_compdata[lower]) < x)) &&
                (lower < xscale->v_length - 1))
            lower++;
        if ((isreal(xscale) ? (xscale->v_realdata[lower] > x) :
                (realpart(&xscale->v_compdata[lower]) > x)) &&
                (lower > 0))
            lower--;
        x1 = (isreal(xscale) ? xscale->v_realdata[lower] :
                realpart(&xscale->v_compdata[lower]));
        x2 = (isreal(xscale) ? xscale->v_realdata[upper] :
                realpart(&xscale->v_compdata[upper]));
        if (x1 > x2) {
            fprintf(cp_err, "Error: X scale (%s) not monotonic\n",
                    xscale->v_name);
            return;
        }
        for (v = vecs; v; v = v->v_link2) {
            yy1 = (isreal(v) ? v->v_realdata[lower] :
                    realpart(&v->v_compdata[lower]));
            y2 = (isreal(v) ? v->v_realdata[upper] :
                    realpart(&v->v_compdata[upper]));
            if (x1 == x2)
                y = yy1;
            else
                y = yy1 + (y2 - yy1) * (x - x1) / (x2 - x1);
            if (!novalue && (v == vecs))
                values[i] = y;
            ypt = ft_findpoint(y, yrange, maxy, 0, ylogscale);
            c = field[omaxy * i + ypt];
            if ((c == ' ') || (c == LCHAR))
                field[omaxy * i + ypt] = (char) v->v_linestyle;
            else
                field[omaxy * i + ypt] = MCHAR;
        }
    }

    out_init();
    for (i = 0; i < omaxy + margin; i++)
        out_send("-");
    out_send("\n");
    i = (omaxy + margin - strlen(plot->pl_title)) / 2;
    while (i-- > 0)
        out_send(" ");
    (void) strcpy(buf, plot->pl_title);
    buf[maxy + margin] = '\0';  /* Cut off if too wide */
    out_send(buf);
    out_send("\n");
    (void) sprintf(buf, "%s %s", plot->pl_name, plot->pl_date);
    buf[maxy + margin] = '\0';
    i = (omaxy + margin - strlen(buf)) / 2;
    while (i-- > 0)
        out_send(" ");
    out_send(buf);
    out_send("\n\n");
    curline = 7;
    out_send("Legend:  ");
    i = 0;
    j = (maxx + margin - 8) / 20;
    if (j == 0)
        j = 1;
    for (v = vecs; v; v = v->v_link2) {
	out_pbuf[0] = (char) v->v_linestyle;
	out_pbuf[1] = '\0';
/*        out_printf("%c = %-17s", (char) v->v_linestyle, v->v_name); */
        out_printf("%s = %-17s", out_pbuf, v->v_name);
        if (!(++i % j) && v->v_link2) {
            out_send("\n         ");
            curline++;
        }
    }
    out_send("\n");
    for (i = 0; i < omaxy + margin; i++)
        out_send("-");
    out_send("\n");
    i = 0;
    out_printf("%s\n%s\n", line2, line1);
    curline += 2;
    for (i = 0; i < maxx; i++) {
        if (nointerp)
          x = isreal(xscale) ? xscale->v_realdata[i] :
                realpart(&xscale->v_compdata[i]);
        else if (xlog && xrange[0] > 0.0 && xrange[1] > 0.0)
	    x = xrange[0] * pow( 10.0, mylog10(xrange[1]/xrange[0])
			* i / (maxx - 1));
	else
          x = xrange[0] + (xrange[1] - xrange[0]) * i / (maxx - 1);
        if (x < 0.0) {
	    sprintf(out_pbuf, "%.3e ", x);
	    out_send(out_pbuf);
/*             out_printf("%.3e ", x); */
        } else {
	    sprintf(out_pbuf, " %.3e ", x);
	    out_send(out_pbuf);
/*            out_printf(" %.3e ", x); */
	}
        if (!novalue) {
            if (values[i] < 0.0) {
	        sprintf(out_pbuf, "%.3e ", values[i]);
	        out_send(out_pbuf);
/*                out_printf("%.3e ", values[i]); */
            } else {
	        sprintf(out_pbuf, " %.3e ", values[i]);
	        out_send(out_pbuf);
/*                out_printf(" %.3e ", values[i]); */
	    }
        }
        cb = field[(i + 1) * omaxy];
        field[(i + 1) * omaxy] = '\0';
        out_send(&field[i * omaxy]);
        field[(i + 1) * omaxy] = cb;
        out_send("\n");
        if (((curline++ % height) == 0) && (i < maxx - 1) &&
                !nobreakp) {
            out_printf("%s\n%s\n\014\n%s\n%s\n", line1, line2,
                line2, line1);
            curline += 5;
        }
    }
    out_printf("%s\n%s\n", line1, line2);

    tfree(field);
    tfree(line1);
    tfree(line2);
    if (!novalue)
        tfree(values);
    return;
}

