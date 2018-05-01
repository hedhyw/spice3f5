/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1988 Jeffrey M. Hsu
**********/

/*
    Postscript driver
*/

#include "spice.h"
#include "util.h"
#include "cpdefs.h"
#include "ftegraph.h"
#include "ftedbgra.h"
#include "ftedev.h"
#include "fteinput.h"
#include "suffix.h"

#define RAD_TO_DEG	(180.0 / M_PI)
#define DEVDEP(g) (*((PSdevdep *) (g)->devdep))
#define	MAX_PS_LINES	1000
#define SOLID 0
#define DOTTED 1

#define gtype         graph->grid.gridtype
#define xoff          dispdev->minx
#define yoff          dispdev->miny
#define XOFF          48      /* printer left margin */
#define YOFF          48      /* printer bottom margin */
#define XTADJ         0       /* printer text adjustment x */
#define YTADJ         4       /* printer text adjustment y */

#define GRIDSIZE      420     /* printer gridsize divisible by 10, [7-2] */
#define GRIDSIZES     360     /* printer gridsize divisible by [10-8], [6-2] */

#define FONTSIZE      10      /* printer default fontsize */
#define FONTWIDTH     6       /* printer default fontwidth */
#define FONTHEIGHT    14      /* printer default fontheight */

typedef struct {
    int lastlinestyle;      /* initial invalid value */
    int	lastx, lasty, linecount;
} PSdevdep;

static char *linestyle[] = {
		"[]",           /* solid */
                "[1 2]",        /* dotted */
                "[7 7]",        /* longdashed */
                "[3 3]",        /* shortdashed */
		"[7 2 2 2]",	/* longdotdashed */
		"[3 2 1 2]",    /* shortdotdashed */
		"[8 3 2 3]",
		"[14 2]",
                "[3 5 1 5]"	/* dotdashed */
		};

static FILE *plotfile;
char psfont[128], psfontsize[32], psscale[32];
static int fontsize   = FONTSIZE;
static int fontwidth  = FONTWIDTH;
static int fontheight = FONTHEIGHT;
static int gridsize   = GRIDSIZE;
static int screenflag = 0;
static double scale;	/* Used for fine tuning */
static int xtadj;     /* text adjustment x */
static int ytadj;     /* text adjustment y */
static int hcopygraphid;

PS_Init()
{
    if (!cp_getvar("hcopyscale", VT_STRING, psscale)) {
	scale = 1.0;
    } else {
	sscanf(psscale, "%lf", &scale);
	if ((scale <= 0) || (scale > 10))
	    scale = 1.0;
    }

    dispdev->numlinestyles = NUMELEMS(linestyle);
    dispdev->numcolors = 2;

    dispdev->width = 7.75 * 72.0 * scale;       /* (8 1/2 - 3/4) * 72 */
    dispdev->height = dispdev->width;

    /* The following side effects have to be considered 
     * when the printer is called by com_hardcopy !
     * gr_init:
     * viewportxoff = 8 * fontwidth
     * viewportyoff = 4 * fontheight
     * gr_resize_internal:
     * viewport.width  = absolute.width - 2 * viewportxoff
     * viewport.height = absolute.height - 2 * viewportyoff
     */

    if (!cp_getvar("hcopyfont", VT_STRING, psfont))
	strcpy(psfont, "Helvetica");
    if (!cp_getvar("hcopyfontsize", VT_STRING, psfontsize)) {
	fontsize = 10;
	fontwidth = 6;
	fontheight = 14;
	xtadj = XTADJ * scale;
	ytadj = YTADJ * scale;
    } else {
	sscanf(psfontsize, "%d", &fontsize);
	if ((fontsize < 10) || (fontsize > 14))
	    fontsize = 10;
	fontwidth = 0.5 + 0.6 * fontsize;
	fontheight = 2.5 + 1.2 * fontsize;
	xtadj = XTADJ * scale * fontsize / 10;
	ytadj = YTADJ * scale * fontsize / 10;
    }

#ifdef notdef
    if (fontsize > 11)
	gridsize = GRIDSIZES;
    else
	gridsize = GRIDSIZE;

    dispdev->width  = gridsize+16*fontwidth;	/* was 612, p.w.h. */
    dispdev->height = gridsize+8*fontheight;	/* was 612, p.w.h. */
#endif

    screenflag = 0;
    dispdev->minx = XOFF / scale;
    dispdev->miny = YOFF / scale;

    return(0);

}

/* devdep initially contains name of output file */
PS_NewViewport(graph)
GRAPH *graph;
{
    double scaleps, scalex, scaley;

    hcopygraphid = graph->graphid;

    if (!(plotfile = fopen(graph->devdep, "w"))) {
      perror(graph->devdep);
      graph->devdep = (char *) NULL;
      return(1);
    }

    if (graph->absolute.width) {
      /* hardcopying from the screen */

      screenflag = 1;

      /* scale to fit on 8 1/2 square */
#ifdef notdef
    /* Face it, this is bogus */
#ifdef notdef
      fprintf(plotfile, "%g %g scale\n",
        (double) dispdev->width / graph->absolute.width,
        (double) dispdev->height / graph->absolute.height);
#endif

      scalex = (double) graph->absolute.width / dispdev->width;
      scaley = (double) graph->absolute.height / dispdev->width;
      /* scale left and bottom printer margin */
      scaleps = ((scalex > scaley) ? scalex : scaley) / scale;
      xoff = (int) (scaleps * (double) XOFF);
      yoff = (int) (scaleps * (double) YOFF);
      xtadj = 0;
      ytadj = 0;
      scalex = (double) dispdev->width / graph->absolute.width;
      scaley = (double) dispdev->width / graph->absolute.height;

      if (gtype == GRID_SMITH || gtype == GRID_SMITHGRID
        || gtype == GRID_POLAR)
      {
	scaleps = scale * ((scalex < scaley) ? scalex : scaley);
	fprintf(plotfile, "%g %g scale\n", scaleps, scaleps);
      } else {
	fprintf(plotfile, "%g %g scale\n", scale*scalex, scale*scaley);
      }

      /* re-scale linestyles */
      gr_relinestyle(graph);
#endif

    }

    /* reasonable values, used in gr_ for placement */
    graph->fontwidth = fontwidth * scale; /* was 12, p.w.h. */
    graph->fontheight = fontheight * scale; /* was 24, p.w.h. */

    graph->absolute.width = dispdev->width;
    graph->absolute.height = dispdev->height;
    /* Also done in gr_init, if called . . . */
    graph->viewportxoff = 8 * fontwidth;
    graph->viewportyoff = 4 * fontheight;

    xoff = scale * XOFF;
    yoff = scale * YOFF;

    /* start file off with a % */
    fprintf(plotfile, "%%!PS-Adobe-3.0 EPSF-3.0\n");
    fprintf(plotfile, "%%%%Creator: nutmeg\n");
    fprintf(plotfile, "%%%%BoundingBox: %d %d %d %d\n",
	(int) (.75 * 72), (int) (.75 * 72),
	(int) (8.5 * 72), (int) (8.5 * 72));

#ifdef notdef
    if (!screenflag)
#endif
	fprintf(plotfile, "%g %g scale\n", 1.0 / scale, 1.0 / scale);

    /* set up a reasonable font */
    fprintf(plotfile, "/%s findfont %d scalefont setfont\n",
	psfont, (int) (fontsize * scale));

    graph->devdep = tmalloc(sizeof(PSdevdep));
    DEVDEP(graph).lastlinestyle = -1;
    DEVDEP(graph).lastx = -1;
    DEVDEP(graph).lasty = -1;
    DEVDEP(graph).linecount = 0;
    graph->linestyle = -1;

    return 0;
}

PS_Close()
{

    /* in case PS_Close is called as part of an abort,
            w/o having reached PS_NewViewport */
    if (plotfile) {
      if (DEVDEP(currentgraph).lastlinestyle != -1) {
        /* haven't stroked last path */
        fprintf(plotfile, "stroke\n");
        DEVDEP(currentgraph).linecount = 0;
      }
      fprintf(plotfile, "showpage\n");
      fclose(plotfile);
      plotfile = NULL;
    }
    /* In case of hardcopy command destroy the hardcopy graph
     * and reset currentgraph to graphid 1, if possible
     */
    if (!screenflag) {
      DestroyGraph(hcopygraphid);
      currentgraph = FindGraph(1);
    }
}

PS_Clear()
{

    /* do nothing */

}

PS_DrawLine(x1, y1, x2, y2)
int x1, y1, x2, y2;
{

    /* note: this is not extendible to more than one graph
        => will have to give NewViewport a writeable graph XXX */

    if (DEVDEP(currentgraph).lastlinestyle != currentgraph->linestyle
	|| DEVDEP(currentgraph).linecount > MAX_PS_LINES)
    {
      fprintf(plotfile, "stroke\n");
      fprintf(plotfile, "newpath\n");
      DEVDEP(currentgraph).linecount = 0;
    }

    if (DEVDEP(currentgraph).linecount == 0
	    || x1 != DEVDEP(currentgraph).lastx
	    || y1 != DEVDEP(currentgraph).lasty)
    {
      fprintf(plotfile, "%d %d moveto ", x1 + xoff, y1 + yoff);
    }
    if (x1 != x2 || y1 != y2) {
	fprintf(plotfile, "%d %d lineto\n", x2 + xoff, y2 + yoff);
	DEVDEP(currentgraph).linecount += 1;
    }

    DEVDEP(currentgraph).lastx = x2;
    DEVDEP(currentgraph).lasty = y2;
    DEVDEP(currentgraph).lastlinestyle = currentgraph->linestyle;

}

/* ARGSUSED */
PS_Arc(x0, y0, r, theta1, theta2)
int x0, y0, r;
double theta1, theta2;
{
    double x1, y1;
    double angle1, angle2;

    while (theta1 >= theta2)
	theta2 += 2 * M_PI;

    angle1 = (double) (RAD_TO_DEG * theta1);
    angle2 = (double) (RAD_TO_DEG * theta2);
    x1 = (double) x0 + r * cos(theta1);
    y1 = (double) y0 + r * sin(theta1);

    fprintf(plotfile, "%lf %lf moveto ", x1+(double)xoff, y1+(double)yoff);
    fprintf(plotfile, "%d %d %d %lf %lf arc\n", x0+xoff, y0+yoff, r,
	angle1, angle2); 
    fprintf(plotfile, "stroke\n");

    DEVDEP(currentgraph).linecount = 0;
}

PS_Text(text, x, y)
char *text;
int x, y;
{

    int savedlstyle;

    /* set linestyle to solid
        or may get funny color text on some plotters */
    savedlstyle = currentgraph->linestyle;
    PS_SetLinestyle(SOLID);

    if (DEVDEP(currentgraph).linecount) {
        fprintf(plotfile, "stroke\n");
        fprintf(plotfile, "newpath\n");
        DEVDEP(currentgraph).linecount = 0;
    }
    /* move to (x, y) */
    fprintf(plotfile, "%d %d moveto\n", x + xoff + xtadj, y + yoff + ytadj);
    fprintf(plotfile, "(%s) show\n", text);

    DEVDEP(currentgraph).lastx = -1;
    DEVDEP(currentgraph).lasty = -1;

    /* restore old linestyle */
    PS_SetLinestyle(savedlstyle);

}

int
PS_SetLinestyle(linestyleid)
int linestyleid;
{

    /* special case
        get it when PS_Text restores a -1 linestyle */
    if (linestyleid == -1) {
      currentgraph->linestyle = -1;
      return 0;
    }

    if (linestyleid < 0 || linestyleid > dispdev->numlinestyles) {
      internalerror("bad linestyleid");
      return 0;
    }

    if (currentgraph->linestyle != linestyleid) {
      if (DEVDEP(currentgraph).lastlinestyle != -1) {
        fprintf(plotfile, "stroke\n");
        fprintf(plotfile, "newpath\n");
        DEVDEP(currentgraph).linecount = 0;
      }
      fprintf(plotfile, "%s 0 setdash\n", linestyle[linestyleid]);
      currentgraph->linestyle = linestyleid;
    }
    return 0;

}

/* ARGSUSED */
PS_SetColor(colorid)
{
    static int flag = 0;	/* A hack */

    /* XXXX Set line style dotted for smith grids */
    if ((colorid == 18) || (colorid == 19)) {
	PS_SetLinestyle(DOTTED);
	flag = 1;
    }
    if (flag && (colorid == 1)) {
	PS_SetLinestyle(SOLID);
	flag = 0;
    }

}

PS_Update()
{

    fflush(plotfile);

}

