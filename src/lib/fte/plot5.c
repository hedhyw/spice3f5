/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
**********/

#include "spice.h"
#include "ftegraph.h"
#include "ftedev.h"
#include "fteinput.h"

static FILE *plotfile;

#define putsi(a)    putc((char) (a), plotfile); \
            putc((char) ((a) >> 8), plotfile)

#define SOLID 0
static char *linestyle[] = { "solid", "dotted", "longdashed", "shortdashed",
        "dotdashed" };
static int currentlinestyle = SOLID;

Plt5_Init()
{

    dispdev->numlinestyles = 4;
    dispdev->numcolors = 2;

    /* arbitrary */
    dispdev->width = 1000;
    dispdev->height = 1000;

    return(0);

}

Plt5_NewViewport(graph)
GRAPH *graph;
{

    if (!(plotfile = fopen(graph->devdep, "w"))) {
      graph->devdep = (char *) NULL;
      perror(graph->devdep);
      return(1);
    }

    if (graph->absolute.width) {

      /* hardcopying from the scree,
        ie, we are passed a copy of an existing graph */
      putc('s', plotfile);
      putsi(0);
      putsi(0);
      putsi(graph->absolute.width);
      putsi(graph->absolute.height);

      /* re-scale linestyles */
      gr_relinestyle(graph);

    } else {
      /* scale space */
      putc('s', plotfile);
      putsi(0);
      putsi(0);
      putsi(dispdev->width);
      putsi(dispdev->height);

      /* reasonable values, used in gr_ for placement */
      graph->fontwidth = 12;
      graph->fontheight = 24;

      graph->absolute.width = dispdev->width;
      graph->absolute.height = dispdev->height;

    }

    /* set to NULL so graphdb doesn't incorrectly de-allocate it */
    graph->devdep = (char *) NULL;

    return(0);

}

Plt5_Close()
{

    /* in case Plt5_Close is called as part of an abort,
            w/o having reached Plt5_NewViewport */
    if (plotfile)
        fclose(plotfile);

}

Plt5_Clear()
{

    /* do nothing */

}

Plt5_DrawLine(x1, y1, x2, y2)
int x1, y1, x2, y2;
{

    putc('l', plotfile);
    putsi(x1);
    putsi(y1);
    putsi(x2);
    putsi(y2);

}

/* ARGSUSED */ /* until some code gets written */
Plt5_Arc(x0, y0, radius, theta1, theta2)
int x0, y0, radius;
double theta1, theta2;
{


}

Plt5_Text(text, x, y)
char *text;
int x, y;
{

    int savedlstyle;

    /* set linestyle to solid
        or may get funny color text on some plotters */
    savedlstyle = currentlinestyle;
    Plt5_SetLinestyle(SOLID);

    /* move to (x, y) */
    putc('m', plotfile);
    putsi(x);
    putsi(y);

    /* use the label option */
    fprintf(plotfile, "t%s\n", text);

    /* restore old linestyle */
    Plt5_SetLinestyle(savedlstyle);

}

int
Plt5_SetLinestyle(linestyleid)
int linestyleid;
{

    if (linestyleid < 0 || linestyleid > dispdev->numlinestyles) {
      internalerror("bad linestyleid");
      return 0;
    }
    putc('f', plotfile);
    fprintf(plotfile, "%s\n", linestyle[linestyleid]);
    currentlinestyle = linestyleid;
    return 0;
}

/* ARGSUSED */
Plt5_SetColor(colorid)
{

    /* do nothing */

}

Plt5_Update()
{

    fflush(plotfile);

}

