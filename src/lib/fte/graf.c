/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1988 Jeffrey M. Hsu
**********/

/*
 *  Most of the gr_ module resides here, in particular, gr_init
 *      and gr_point, expect for the gr_ grid routines.
 *
 */

#include "spice.h"
#ifdef HAS_MFB
#include "mfb.h"
#endif /* HAS_MFB */
#include "cpdefs.h"     /* for VT_ */
#include "cpextern.h"
#include "fteconst.h"
#include "ftedebug.h"       /* for iplot */
#include "ftedata.h"        /* for struct dvec */
#include "ftedefs.h"        /* for FTEextern.h and IPOINT{MIN,MAX} */
#include "fteinput.h"
#include "ftegraph.h"
#include "ftedbgra.h"
#include "ftedev.h"
#include "suffix.h"

extern struct dbcomm *dbs;  /* for iplot */
static void gr_resize_internal();
static void drawlegend();

/* note: let's try to get rid of these */
/* global variables */
/* Graphics mode in progress, so signal handlers know to call gr_clean */
/* bool gr_gmode = false; */

/* for legends, set in gr_start, reset in gr_iplot and gr_init */
static int plotno;
static int curcolor = 1;        /* for assigning unique colors */
static int curlst = 0;          /* for assigning line styles */

/* invariant:  currentgraph contains the current graph */

/* These are what gets plotted as points when you specify point plots */
static char pointchars[128];
#define DEFPOINTCHARS   "oxabcdefhgijklmnpqrstuvwyz"

/* Buffer for ticmarks if given a list */
static char ticbuf[1024];
static char *ticlist = ticbuf;
#define MAXTICS 100
double *readtics();

#define XFACTOR 2       /* How much to expand the X scale during iplot. */
#define YFACTOR 1.5     /* How much to expand the Y scale during iplot. */

/*
 *  Start of a new graph.
 *  Fill in the data that gets displayed.
 *  Difference from old gr_init
 *    we don't try to determine the look of the screen from here
 *    leave to lower level routines
 *
 */
/* ARGSUSED */ /* since we don't really use many of them */
gr_init(xlims, ylims, xname, plotname, hcopy, nplots, xdelta, ydelta, gridtype,
                plottype, xlabel, ylabel, xtype, ytype, pname, commandline)
        double *xlims, *ylims;          /* The size of the screen. */
        char *xname, *plotname;         /* What to label things. */
	char *hcopy;                    /* The raster file. */
        int nplots;                     /* How many plots there will be. */
        double xdelta, ydelta;          /* Line increments for the scale. */
        GRIDTYPE gridtype;              /* The grid type */
        PLOTTYPE plottype;              /*  and the plot type. */
        char *xlabel, *ylabel;          /* Labels for axes. */
        int xtype, ytype;               /* The types of the data graphed. */
        char *pname;
        char *commandline;              /* For xi_zoomdata() */
{

    GRAPH *graph;
    int b;
    wordlist *wl;
    char *comb_title;

#ifdef HAS_MFB
    if ((!strcmp(dispdev->name, "MFB")
	    || !strcmp(dispdev->name, "error")) && !hcopy)
	DevInit( );
#endif
    if (!(graph = NewGraph())) {
      return(false);
    }

    /*
       The global currentgraph will always be the current graph.
    */
    SetGraphContext(graph->graphid);

    graph->onevalue = (xname ? false : true);

    /* communicate filename to plot 5 driver */
    if (hcopy) {
      graph->devdep = hcopy;
    }

    plotno = 0;

    /* note: should do only once, maybe in gr_init_once */
    if (!cp_getvar("pointchars", VT_STRING, pointchars))
          (void) strcpy(pointchars, DEFPOINTCHARS);

    if (!cp_getvar("ticmarks", VT_NUM, (char *) &graph->ticmarks)) {
      if (cp_getvar("ticmarks", VT_BOOL, (char *) &b))
        graph->ticmarks = 10;
      else
        graph->ticmarks = 0;
    }

    if (cp_getvar("ticlist", VT_LIST, ticlist)) {
	wl = (wordlist *)vareval("ticlist");
	ticlist = (char *)wl_flatten(wl);
	graph->ticdata = (double *) readtics(ticlist);
    } else
	graph->ticdata = NULL;

    /* set slow flag to stop between each plot and prompt the
        user for a return
       This is used mainly for graphics terminals w/o windows.
    */
/*
    if (incremental)
      slow = false;
    else
      (void) cp_getvar("slowplot", VT_BOOL, (char *) &slow);
*/

    if (!xlims || !ylims) {
      internalerror("gr_init:  no range specified");
      return(false);
    }

    /* indicate some graphics going on */
/*  gr_gmode = true; */

    /* save upper and lower limits */
    graph->data.xmin = xlims[0];
    graph->data.xmax = xlims[1];
    graph->data.ymin = ylims[0];
    graph->data.ymax = ylims[1];

    /* note: have enum here or some better convention */
    if (NewViewport(graph) == 1) {
      /* note: where is the error message generated? */
      /* note: undo mallocs */
      fprintf(cp_err, "Can't open viewport for graphics.\n");
      return(false);
    }

    /* layout decisions */
    /* note: have to do before gr_fixgrid and after NewViewport */
    graph->viewportxoff = graph->fontwidth * 8; /* 8 lines on left */
    graph->viewportyoff = graph->fontheight * 4;    /* 4 on bottom */

    DevClear();

    graph->grid.gridtype = gridtype;
    graph->plottype = plottype;
    graph->grid.xdatatype = xtype;
    graph->grid.ydatatype = ytype;
    graph->grid.xdelta = xdelta;
    graph->grid.ydelta = ydelta;
    graph->grid.ysized = 0;
    graph->grid.xsized = 0;

    if (!graph->onevalue) {
        if (xlabel) {
            graph->grid.xlabel = xlabel;
        } else {
            graph->grid.xlabel = xname;
        }
        if (ylabel) {
            graph->grid.ylabel = ylabel;
        }
    } else {
        if (xlabel) {
            graph->grid.xlabel = xlabel;
        } else {
            graph->grid.xlabel = "real";
        }
        if (ylabel) {
            graph->grid.ylabel = ylabel;
        } else {
            graph->grid.ylabel = "imag";
        }
    }

    if (!pname)
	pname = "(unknown)";
    if (!plotname)
	plotname = "(unknown)";
    comb_title = tmalloc(strlen(plotname) + strlen(pname) + 3);
    sprintf(comb_title, "%s: %s", pname, plotname);
    graph->plotname = comb_title;

    gr_resize_internal(graph);
    gr_redrawgrid(graph);

    /* Set up colors and line styles. */
    if (dispdev->numlinestyles == 1)
        curlst = 0; /* Use the same one all the time. */
    else
        curlst = 1;
    /* XXX Special exception for SMITH */
    if (dispdev->numcolors > 2 && (graph->grid.gridtype == GRID_SMITH
	|| graph->grid.gridtype == GRID_SMITHGRID))
    {
        curcolor = 3;
    } else
        curcolor = 1;

    graph->commandline = copy(commandline);

    return(true);

}

/*
 *  Add a point to the curve we're currently drawing.
 *  Should be in between a gr_init() and a gr_end()
 *    expect when iplotting, very bad hack
 *  Differences from old gr_point:
 *    We save points here, instead of in lower levels.
 *    Assume we are in right context
 *  Save points in data space (not screen space).
 *  We pass two points in so we can multiplex plots.
 *
 */
void
gr_point(dv, newx, newy, oldx, oldy, np)
struct dvec *dv;
double newx, newy, oldx, oldy;
int np;
{
    int oldtox, oldtoy;     /* value before clipping */

    char pointc[2];

    int fromx, fromy, tox, toy;
    int ymin, dummy;

    DatatoScreen(currentgraph, oldx, oldy, &fromx, &fromy);
    DatatoScreen(currentgraph, newx, newy, &tox, &toy);

/* note: we do not particularly want to clip here */
    oldtox = tox; oldtoy = toy;
    if (!currentgraph->grid.circular) {
      if (clip_line(&fromx, &fromy, &tox, &toy,
        currentgraph->viewportxoff, currentgraph->viewportyoff,
        currentgraph->viewport.width + currentgraph->viewportxoff,
        currentgraph->viewport.height + currentgraph->viewportyoff))
        return;
    } else {
      if (clip_to_circle(&fromx, &fromy, &tox, &toy,
            currentgraph->grid.xaxis.circular.center,
            currentgraph->grid.yaxis.circular.center,
            currentgraph->grid.xaxis.circular.radius))
        return;
    }

    if (currentgraph->plottype != PLOT_POINT) {
      SetLinestyle(dv->v_linestyle);
    } else {
      /* if PLOT_POINT,
           don't want to plot an endpoint which have been clipped */
      if (tox != oldtox || toy != oldtoy)
    return;
    }
    SetColor(dv->v_color);

    switch (currentgraph->plottype) {
      double	*tics;
      case PLOT_LIN:

    /* If it's a linear plot, ignore first point since we don't want
        to connect with oldx and oldy. */
	if (np)
	    DrawLine(fromx, fromy, tox, toy);
	if (tics = (double *) currentgraph->ticdata) {
	    for (; *tics < HUGE; tics++) {
		if (*tics == (double) np) {
		    Text("x", (int) (tox - currentgraph->fontwidth / 2),
			(int) (toy - currentgraph->fontheight / 2));
		    SaveText(currentgraph, "x",
			(int) (tox - currentgraph->fontwidth / 2),
			(int) (toy - currentgraph->fontheight / 2));
		    break;
		}
	    }
	} else if ((currentgraph->ticmarks >0) && (np > 0)
	    && (np % currentgraph->ticmarks == 0))
	{
	    /* Draw an 'x' */
	    Text("x", (int) (tox - currentgraph->fontwidth / 2),
		(int) (toy - currentgraph->fontheight / 2));
	    SaveText(currentgraph, "x",
		(int) (tox - currentgraph->fontwidth / 2),
		(int) (toy - currentgraph->fontheight / 2));
	}
        break;
      case PLOT_COMB:
        DatatoScreen(currentgraph,
                (double) 0, currentgraph->datawindow.ymin,
                &dummy, &ymin);
        DrawLine(tox, ymin, tox, toy);
        break;
      case PLOT_POINT:
        /* Here, gi_linestyle is the character used for the point.  */
        pointc[0] = dv->v_linestyle;
        pointc[1] = '\0';
        Text(pointc, (int) (tox - currentgraph->fontwidth / 2),
            (int) (toy - currentgraph->fontheight / 2));
        /* gr_redraw will redraw this w/o our having to save it */
        /* SaveText(currentgraph, pointc,
            (int) (tox - currentgraph->fontwidth / 2),
            (int) (toy - currentgraph->fontheight / 2)); */
      default:
        break;
    }

}

static void
gr_start_internal(dv, copyvec)
struct dvec *dv;
bool copyvec;
{

    struct dveclist *link;
    char *s;

    /* Do something special with poles and zeros.  Poles are 'x's, and
     * zeros are 'o's.
     */
    s = ft_typenames(dv->v_type);
        if (eq(s, "pole")) {
                dv->v_linestyle = 'x';
                return;
        } else if (eq(s, "zero")) {
                dv->v_linestyle = 'o';
                return;
        }

    /* Find a (hopefully) new line style and color. */
    if (currentgraph->plottype == PLOT_POINT) {
	if (pointchars[curlst - 1])
	    curlst++;
	else
	    curlst = 2;
    } else if ((curlst > 0) && (++curlst == dispdev->numlinestyles))
	curlst = 2;
    if ((curcolor > 0) && (++curcolor == dispdev->numcolors))
	curcolor = (((currentgraph->grid.gridtype == GRID_SMITH
	    || currentgraph->grid.gridtype == GRID_SMITHGRID) &&
	    (dispdev->numcolors > 3)) ? 4 : 2);
    if (currentgraph->plottype == PLOT_POINT)
	dv->v_linestyle = pointchars[curlst - 2];
    else
	dv->v_linestyle = curlst;
    dv->v_color = curcolor;

/* note: XXX */
#ifdef notdef
    /* This is a minor hack -- reset the color */
    if (dv->v_defcolor)
        ReSetColor(curcolor, dv->v_defcolor);
#endif

    /* save the data so we can refresh */
    link = (struct dveclist *) calloc(1, sizeof(struct dveclist));
    link->next = currentgraph->plotdata;

    if (copyvec) {
      link->vector = vec_copy(dv);
      /* vec_copy doesn't set v_color or v_linestyle */
      link->vector->v_color = dv->v_color;
      link->vector->v_linestyle = dv->v_linestyle;
      link->vector->v_flags |= VF_PERMANENT;
    } else {
      link->vector = dv;
    }

    currentgraph->plotdata = link;

    /* Put the legend entry on the screen. */
    drawlegend(currentgraph, plotno, dv);

    plotno++;

}

/* start one plot of a graph */
void
gr_start(dv)
struct dvec *dv;
{

    gr_start_internal(dv, true);

}

/* make sure the linestyles in this graph don't exceed the number of
    linestyles available in the current display device */
gr_relinestyle(graph)
GRAPH *graph;
{

    struct dveclist *link;

    for (link = graph->plotdata; link; link = link->next) {
      if (graph->plottype == PLOT_POINT) continue;
      if (!(link->vector->v_linestyle < dispdev->numlinestyles)) {
        link->vector->v_linestyle %= dispdev->numlinestyles;
      }
      if (!(link->vector->v_color < dispdev->numcolors)) {
        link->vector->v_color %= dispdev->numcolors;
      }
    }

}

static void
drawlegend(graph, plotno, dv)
GRAPH *graph;
int plotno;
struct dvec *dv;
{

    int x, y, i;
    char buf[16];

    x = ((plotno % 2) ? graph->viewportxoff :
            ((graph->viewport.width) / 2));
    y = graph->absolute.height - graph->fontheight
            - ((plotno + 2) / 2) * (graph->fontheight);
    i = y + graph->fontheight / 2 + 1;
    SetColor(dv->v_color);
    if (graph->plottype == PLOT_POINT) {
        (void) sprintf(buf, "%c : ", dv->v_linestyle);
        Text(buf, x + graph->viewport.width / 20
                - 3 * graph->fontwidth, y);
    } else {
        SetLinestyle(dv->v_linestyle);
        DrawLine(x, i, x + graph->viewport.width / 20, i);
    }
    SetColor(1);
    Text(dv->v_name, x + graph->viewport.width / 20
            + graph->fontwidth, y);

}

/* end one plot of a graph */
void
gr_end(dv)
struct dvec *dv;
{
    Update();
#ifdef HAS_MFB
    /* hack! */
    if (!strcmp(dispdev->name, "MFB") && dv->v_link2 == NULL
	&& MFBCurrent->fileDesc == 1)
    {
      gr_pmsg("Hit return to continue");
      MFBUpdate( );	 /* XXX ??? */
    }
#endif
}

/* Print text in the bottom line. */

void
gr_pmsg(text)
    char *text;
{
    char buf[BSIZE_SP];
    buf[0] = 0;

    Update();

    if (cp_getvar("device", VT_STRING, buf)
	    && !(strcmp("/dev/tty", buf) == 0))
	fprintf(cp_err, "%s", text);
    else
	Text(text, currentgraph->viewport.width
		- (strlen(currentgraph->grid.xlabel) + 3)
		* currentgraph->fontwidth,
		currentgraph->absolute.height - currentgraph->fontheight);
    Update();
    (void) getchar();
    return;
}

void
gr_clean()
{
    Update();
    return;
}

/* call this routine after viewport size changes */
gr_resize(graph)
GRAPH *graph;
{

    double oldxratio, oldyratio;
    double scalex, scaley;
    struct _keyed *k;

    oldxratio = graph->aspectratiox;
    oldyratio = graph->aspectratioy;

    graph->grid.xsized = 0;
    graph->grid.ysized = 0;

    gr_resize_internal(graph);

    /* scale keyed text */
    scalex = oldxratio / graph->aspectratiox;
    scaley = oldyratio / graph->aspectratioy;
    for (k = graph->keyed; k; k = k->next) {
      k->x = (k->x - graph->viewportxoff) * scalex + graph->viewportxoff;
      k->y = (k->y - graph->viewportyoff) * scaley + graph->viewportyoff;
    }

    /* X also generates an expose after a resize
    This is handled in X10 by not redrawing on resizes and waiting for
    the expose event to redraw.  In X11, the expose routine tries to
    be clever and only redraws the region specified in an expose
    event, which does not cover the entire region of the plot if the
    resize was from a small window to a larger window.  So in order
    to keep the clever X11 expose event handling, we have the X11
    resize routine pull out expose events for that window, and we
    redraw on resize also.
    */
#ifndef HAS_X_
    gr_redraw(graph);
#endif

}

static void gr_resize_internal(graph)
GRAPH *graph;
{

    if (!graph->grid.xsized)
	    graph->viewport.width = graph->absolute.width -
		    1.4 * graph->viewportxoff;
    if (!graph->grid.ysized)
	    graph->viewport.height = graph->absolute.height -
		    2 * graph->viewportyoff;

    gr_fixgrid(graph, graph->grid.xdelta, graph->grid.ydelta,
            graph->grid.xdatatype, graph->grid.ydatatype);

    /* cache width and height info to make DatatoScreen go fast */
    /* note: XXX see if this is actually used anywhere */
    graph->datawindow.width = graph->datawindow.xmax -
                    graph->datawindow.xmin;
    graph->datawindow.height = graph->datawindow.ymax -
                    graph->datawindow.ymin;

    /* cache (datawindow size) / (viewport size) */
    graph->aspectratiox = graph->datawindow.width / graph->viewport.width;
    graph->aspectratioy = graph->datawindow.height / graph->viewport.height;

}

/* redraw everything in struct graph */
gr_redraw(graph)
GRAPH *graph;
{

    struct dveclist *link;

    /* establish current graph so default graphic calls will work right */
    PushGraphContext(graph);

    DevClear();

    /* redraw grid */
    gr_redrawgrid(graph);

    for (link=graph->plotdata, plotno = 0; link;
            link = link->next, plotno++) {
      /* redraw legend */
      drawlegend(graph, plotno, link->vector);

      /* replot data
        if onevalue, pass it a NULL scale
        otherwise, if vec has its own scale, pass that
            else pass vec's plot's scale
      */
      ft_graf(link->vector,
          graph->onevalue ? (struct dvec *) NULL :
                    (link->vector->v_scale ?
                    link->vector->v_scale :
                    link->vector->v_plot->pl_scale),
          true);
    }

    gr_restoretext(graph);

    PopGraphContext();

}

gr_restoretext(graph)
GRAPH *graph;
{

    struct _keyed *k;

    /* restore text */
    for (k=graph->keyed; k; k = k->next) {
      SetColor(k->colorindex);
      Text(k->text, k->x, k->y);
    }

}

/* Do some incremental plotting. 3 cases -- first, if length < IPOINTMIN, don't
 * do anything. Second, if length = IPOINTMIN, plot what we have so far. Third,
 * if length > IPOINTMIN, plot the last points and resize if needed.
 * Note we don't check for pole / zero because they are of length 1.
 */

/* note: there is a problem with multiple iplots that use the same vector,
    namely, that vector has the same color throughout.  This is another
    reason why we need to pull color and linestyle out of dvec XXX
    Or maybe even something more drastic ?? */

extern bool resumption;

static
iplot(pl, id)
    struct plot *pl;
    int id;
{
    int len = pl->pl_scale->v_length;
    struct dvec *v, *xs = pl->pl_scale;
    double *lims, dy;
    double start, stop, step;
    register int j;
    bool changed = false;
    int yt;
    char *yl = NULL;
    double xlims[2], ylims[2];
    static REQUEST reqst = { checkup_option, 0 };
    int inited = 0;
    char commandline[513];

    for (j = 0, v = pl->pl_dvecs; v; v = v->v_next)
        if (v->v_flags & VF_PLOT)
            j++;
    if (!j)
        return(0);
    if (ft_grdb)
        fprintf(cp_err, "Entering iplot, len = %d\n\r", len);

    if (len < IPOINTMIN) {
        /* Nothing yet */
        return(0);
    } else if (len == IPOINTMIN || !id
	    /* || (len > IPOINTMIN && resumption) */) {
	resumption = false;
        /* Draw the grid for the first time, and plot everything. */
        lims = ft_minmax(xs, true);
        xlims[0] = lims[0];
        xlims[1] = lims[1];
        ylims[0] = HUGE;
        ylims[1] = - ylims[0];
        for (v = pl->pl_dvecs; v; v = v->v_next)
            if (v->v_flags & VF_PLOT) {
                lims = ft_minmax(v, true);
                if (lims[0] < ylims[0])
                  ylims[0] = lims[0];
                if (lims[1] > ylims[1])
                  ylims[1] = lims[1];
                if (!yl)
                  yl = v->v_name;
            }
        if (ft_grdb)
            fprintf(cp_err,
              "iplot: after 5, xlims = %G, %G, ylims = %G, %G\n\r",
              xlims[0],
              xlims[1],
              ylims[0],
              ylims[1]);
        for (yt = pl->pl_dvecs->v_type, v = pl->pl_dvecs->v_next; v;
                v = v->v_next)
            if ((v->v_flags & VF_PLOT) && (v->v_type != yt)) {
                yt = 0;
                break;
            }
/*
        (void) gr_init((double *) NULL, (double *) NULL, xs->v_name,
            pl->pl_title, (char *) NULL, j, xdelta, ydelta,
            GRID_LIN, plottype, xs->v_name, yl, xs->v_type, yt,
            commandline, plotname);
*/
/* note: have command options for iplot to specify xdelta, etc.
    So don't need static variables hack.
    Assume default values for now.
*/
	sprintf(commandline, "iplot %s", xs->v_name);

        (void) gr_init(xlims, ylims, xs->v_name,
            pl->pl_title, (char *) NULL, j, 0.0, 0.0,
            GRID_LIN, PLOT_LIN, xs->v_name, yl, xs->v_type, yt,
            plot_cur->pl_typename, commandline);
        for (v = pl->pl_dvecs; v; v = v->v_next)
            if (v->v_flags & VF_PLOT) {
              gr_start_internal(v, false);
              ft_graf(v, xs, true);
            }
        inited = 1;
    } else {
	Input(&reqst, 0);
        /* First see if we have to make the screen bigger */
        dy = (isreal(xs) ? xs->v_realdata[len - 1] :
                realpart(&xs->v_compdata[len - 1]));
        if (ft_grdb)
            fprintf(cp_err, "x = %G\n\r", dy);
        if (!if_tranparams(ft_curckt, &start, &stop, &step) ||
                !ciprefix("tran", pl->pl_typename)) {
            stop = HUGE;
            start = - stop;
        }
        while (dy < currentgraph->data.xmin) {
            changed = true;
            if (ft_grdb)
              fprintf(cp_err, "resize: xlo %G -> %G\n\r",
                  currentgraph->data.xmin,
                  currentgraph->data.xmin -
                    (currentgraph->data.xmax -
                    currentgraph->data.xmin)
                * XFACTOR);
            currentgraph->data.xmin -=
              (currentgraph->data.xmax -
              currentgraph->data.xmin)
              * XFACTOR;
            if (currentgraph->data.xmin < start) {
                currentgraph->data.xmin = start;
                break;
            }
        }
        if (currentgraph->data.xmax <
                currentgraph->data.xmin)
            currentgraph->data.xmax =
                    currentgraph->data.xmin;
        while (dy > currentgraph->data.xmax) {
            changed = true;
            if (ft_grdb)
                fprintf(cp_err, "resize: xhi %G -> %G\n\r",
                  currentgraph->data.xmax,
                  currentgraph->data.xmax +
                    (currentgraph->data.xmax -
                    currentgraph->data.xmin) * XFACTOR);
            currentgraph->data.xmax +=
                    (currentgraph->data.xmax -
                    currentgraph->data.xmin) *
                    XFACTOR;
            if (currentgraph->data.xmax > stop) {
                currentgraph->data.xmax = stop;
                break;
            }
        }
        for (v = pl->pl_dvecs; v; v = v->v_next) {
            if (!(v->v_flags & VF_PLOT))
                continue;
            dy = (isreal(v) ? v->v_realdata[len - 1] :
                    realpart(&v->v_compdata[len - 1]));
            if (ft_grdb)
                fprintf(cp_err, "y = %G\n\r", dy);
            while (dy < currentgraph->data.ymin) {
                changed = true;
                if (ft_grdb)
                  fprintf(cp_err, "resize: ylo %G -> %G\n\r",
                    currentgraph->data.ymin,
                    currentgraph->data.ymin -
                    (currentgraph->data.ymax -
                    currentgraph->data.ymin) * YFACTOR);
                currentgraph->data.ymin -=
                  (currentgraph->data.ymax -
                  currentgraph->data.ymin) * YFACTOR;
            }
            if (currentgraph->data.ymax <
                    currentgraph->data.ymin)
                currentgraph->data.ymax =
                        currentgraph->data.ymin;
            while (dy > currentgraph->data.ymax) {
                changed = true;
                if (ft_grdb)
                  fprintf(cp_err, "resize: yhi %G -> %G\n\r",
                    currentgraph->data.ymax,
                    currentgraph->data.ymax +
                      (currentgraph->data.ymax -
                      currentgraph->data.ymin) * YFACTOR);
                currentgraph->data.ymax +=
                  (currentgraph->data.ymax -
                  currentgraph->data.ymin) * YFACTOR;
            }
        }
        if (changed) {
            /* Redraw everything. */
            gr_pmsg("Resizing screen", false);
            gr_resize(currentgraph);
            gr_redraw(currentgraph);
        } else {
            /* Just connect the last two points. This won't
             * be done with curve interpolation, so it might
             * look funny.
             */
            for (v = pl->pl_dvecs; v; v = v->v_next)
                if (v->v_flags & VF_PLOT) {
                    gr_point(v,
                    (isreal(xs) ? xs->v_realdata[len - 1] :
                    realpart(&xs->v_compdata[len - 1])),
                    (isreal(v) ? v->v_realdata[len - 1] :
                    realpart(&v->v_compdata[len - 1])),
                    (isreal(xs) ? xs->v_realdata[len - 2] :
                    realpart(&xs->v_compdata[len - 2])),
                    (isreal(v) ? v->v_realdata[len - 2] :
                    realpart(&v->v_compdata[len - 2])),
                    len - 1);
                }
        }
    }
    Update();
    return(inited);
}

static void
set(plot, db, unset, mode)
struct plot *plot;
struct dbcomm *db;
bool unset;
int mode;
{

    struct dvec *v;
    struct dbcomm *dc;

    if (db->db_type == DB_IPLOTALL || db->db_type == DB_TRACEALL) {
      for (v = plot->pl_dvecs; v; v = v->v_next) {
        if (unset)
          v->v_flags &= ~mode;
        else
          v->v_flags |= mode;
      }
      return;
    }
    for (dc = db; dc; dc = dc->db_also) {
       v = vec_fromplot(dc->db_nodename1, plot);
        if (!v || v->v_plot != plot) {
          if (!eq(dc->db_nodename1, "0") && !unset) {
              fprintf(cp_err, "Warning: node %s non-existent in %s.\n",
                  dc->db_nodename1, plot->pl_name);
        /* note: XXX remove it from dbs, so won't get further errors */
          }
          continue;
        }
        if (unset)
          v->v_flags &= ~mode;
        else
          v->v_flags |= mode;
    }
    return;
}

static char *
getitright(buf, num)
    char *buf;
    double num;
{
    char *p;
    int k;

    sprintf(buf, "    % .5g", num);
    p = index(buf, '.');

    if (p) {
	return p - 4;
    } else {
	k = strlen(buf);
	if (k > 8)
	    return buf + 4;
	else /* k >= 4 */
	    return buf + k - 4;
    }
}

static int hit, hit2;

reset_trace( )
{
	hit = -1;
	hit2 = -1;
}

void
gr_iplot(plot)
struct plot *plot;
{

    struct dbcomm *db;
    int dontpop;        /* So we don't pop w/o push. */
    char buf[30];

    hit = 0;
    for (db = dbs; db; db = db->db_next) {
      if (db->db_type == DB_IPLOT || db->db_type == DB_IPLOTALL) {

        if (db->db_graphid) PushGraphContext(FindGraph(db->db_graphid));

        set(plot, db, false, VF_PLOT);

        dontpop = 0;
        if (iplot(plot, db->db_graphid)) {
          /* graph just assigned */
          db->db_graphid = currentgraph->graphid;
          dontpop = 1;
        }

        set(plot, db, true, VF_PLOT);

        if (!dontpop && db->db_graphid) PopGraphContext();

      } else if (db->db_type == DB_TRACENODE || db->db_type == DB_TRACEALL) {

	struct dvec *v, *u;
	int len;

        set(plot, db, false, VF_PRINT);

	len = plot->pl_scale->v_length;

	dontpop = 0;
	for (v = plot->pl_dvecs; v; v = v->v_next) {
	    if (v->v_flags & VF_PRINT) {
		u = plot->pl_scale;
		if (len <= 1 || hit <= 0 || hit2 < 0) {
		    if (len <= 1 || hit2 < 0)
			term_clear( );
		    else
			term_home( );
		    hit = 1;
		    hit2 = 1;
		    printf(
		     "\tExecution trace (remove with the \"delete\" command)");
		    term_cleol( );
		    printf("\n");

		    if (u) {
			printf("%12s:", u->v_name);
			if (isreal(u)) {
			    printf("%s",
				getitright(buf, u->v_realdata[len - 1]));
			} else {
			    printf("%s",
				getitright(buf, u->v_realdata[len - 1]));
				printf(", %s",
				    getitright(buf, u->v_realdata[len - 1]));
			}
			term_cleol( );
			printf("\n");
		    }
		}
		if (v == u)
		    continue;
		printf("%12s:", v->v_name);
		if (isreal(v)) {
		    printf("%s", getitright(buf, v->v_realdata[len - 1]));
		} else {
		    printf("%s", getitright(buf, v->v_realdata[len - 1]));
		    printf(", %s", getitright(buf, v->v_realdata[len - 1]));
		}
		term_cleol( );
		printf("\n");
	    }
	}
        set(plot, db, true, VF_PRINT);

      }

    }

}

/*
 *  This gets called after iplotting is done.  We clear out the db_graphid
 *  fields.  Copy the dvecs, which we referenced by reference, so
 *  DestroyGraph gets to free its own copy.  Note:  This is a clear
 *  case for separating the linestyle and color fields from dvec.
 */

void
gr_end_iplot()
{

    struct dbcomm *db, *prev, *next;
    GRAPH *graph;
    struct dveclist *link;
    struct dvec *dv;

    prev = NULL;
    for (db = dbs; db; prev = db, db = next) {
      next = db->db_next;
      if (db->db_type == DB_DEADIPLOT) {
	    if (db->db_graphid) {
		DestroyGraph(db->db_graphid);
		if (prev)
		    prev->db_next = next;
		else
		    dbs = next;
		dbfree(db);
	    }
      } else if (db->db_type == DB_IPLOT || db->db_type == DB_IPLOTALL) {
        if (db->db_graphid) {

          /* get private copy of dvecs */
          graph = FindGraph(db->db_graphid);

          link = graph->plotdata;

          while (link) {
            dv = link->vector;
            link->vector = vec_copy(dv);
            /* vec_copy doesn't set v_color or v_linestyle */
            link->vector->v_color = dv->v_color;
            link->vector->v_linestyle = dv->v_linestyle;
            link->vector->v_flags |= VF_PERMANENT;
	    link = link->next;
          }

          db->db_graphid = 0;
        } else {
          /* warn that this wasn't plotted */
          fprintf(cp_err, "Warning: iplot %d was not executed.\n",
		  db->db_number);
        }
      }
    }
#ifdef HAS_MFB
   if (!strcmp( dispdev->name, "MFB" ))
      MFBHalt();
#endif

    return;
}

double *
readtics(string)
char *string;
{
        int i, k;
        char *words, *worde;
        double *tics, *ticsk;

        tics = (double *) tmalloc(MAXTICS * sizeof(double));
        ticsk = tics;
        words = string;

        for (i = k = 0; *words && k < MAXTICS; words = worde) {

	    while (isspace(*words))
	       words++;

	    worde = words;
	    while (isalpha(*worde) || isdigit(*worde))
	       worde++;

	    if (*worde)
		*worde++ = '\0';

	    sscanf(words, "%lf", ticsk++);

	    k++;

        }
        *ticsk = HUGE;
        return(tics);
}

