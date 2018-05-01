/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1988 Jeffrey M. Hsu
**********/

/*
    X10 drivers.
*/

#include "spice.h"
#ifdef HAS_X10

#include "ftegraph.h"
#include "ftedbgra.h"
#include "ftedev.h"
#include "fteinput.h"
#include "cpdefs.h"
#include "ftedefs.h"

#include <X/Xlib.h>
#include <X/cursors/left_ptr.cursor>
#include <X/cursors/left_ptr_mask.cursor>

/* forward declarations */
static void eventhandler(), instantcolors(), initcolors();
int errorhandler();

/* X dependent default parameters */
#define DEF_FONT "6x10"
#define NUMLINESTYLES 8
#define NXPLANES 5      /* note: What is this used for? */
#define BOXSIZE 30      /* initial size of bounding box for zoomin */

typedef struct {
    Window window;
    FontInfo *fontinfo;
    int lastx, lasty;   /* used in X_DrawLine */
    int lastlinestyle;  /* used in X_DrawLine */
} X10devdep;

#define DEVDEP(g) (*((X10devdep *) (g)->devdep))

static Display *display;
/* ick, have to have this global version of display for HLP, change HLP  XXX */
Display *X_display;
static XAssocTable *xtable; /* for getting from window id to graphid */
static Pattern xlinestyles[NUMLINESTYLES];
static int colors[NUMCOLORS];

X_Init()
{

    char buf[512];
    char *displayname;

    if (cp_getvar("display", VT_STRING, buf)) {
      displayname = buf;
    } else if (!(displayname = getenv("DISPLAY"))) {
      internalerror("Can't open X display.");
      return (1);
    }

    if (!(display = XOpenDisplay(displayname))) {
      sprintf(ErrorMessage, "Can't open %s.", displayname);
      internalerror(ErrorMessage);
      return (1);
    }

    X_display = display;        /* for HLP */

#ifdef HAS_XT
    /* initialize X toolkit */
    XtInitialize();
#endif

    /* assume maximum around 100 windows */
    xtable = XCreateAssocTable(32);

    /* note: set up input sources and handlers for Xt */

    /* set correct information */
    dispdev->numlinestyles = NUMLINESTYLES;

    initcolors();           /* sets dispdev->numcolors */

#define DisplayHeight() (_XlibCurrentDisplay->height)
#define DisplayWidth()  (_XlibCurrentDisplay->width)

    dispdev->width = DisplayWidth();
    dispdev->height = DisplayHeight();

    /* we don't want non-fatal X errors to call exit */
    XErrorHandler(errorhandler);

    return (0);

}

int
errorhandler(display, errorev)
Display *display;
XErrorEvent *errorev;
{
    externalerror(XErrDescrip(errorev->error_code));
    return;
}

/* Recover from bad NewViewPort call. */
#define RECOVERNEWVIEWPORT()    free((char *) graph);\
                graph = (GRAPH *) NULL; 
        /* need to do this or else DestroyGraph will free it again */

/* NewViewport is responsible for filling in graph->viewport */
X_NewViewport(graph)
GRAPH *graph;
{

    OpaqueFrame frame;
    char geometry[32], defaultgeometry[32];
    WindowInfo wininfo;
    char fontname[64];
    Cursor cursor;

    graph->devdep = tmalloc(sizeof(X10devdep));

    if (!cp_getvar("geometry", VT_STRING, geometry)) {
      *geometry = '\0';
    }

/* note: tweak this */
    (void) sprintf(defaultgeometry, "=%dx%d+%d+%d", 300, 300, 100, 100);

    frame.bdrwidth = 2;
    frame.background = WhitePixmap;
    frame.border = BlackPixmap;
    DEVDEP(graph).window = XCreate("nutmeg graph", "nutmeg",
            geometry, defaultgeometry, &frame, 300, 300);
    if (!DEVDEP(graph).window) {
      internalerror("can't open graph window");
      RECOVERNEWVIEWPORT();
      return(1);
    }
    XQueryWindow(DEVDEP(graph).window, &wininfo);
    graph->absolute.xpos = wininfo.x;
    graph->absolute.ypos = wininfo.y;
    graph->absolute.width = wininfo.width;
    graph->absolute.height = wininfo.height;

/* note: set up subwindow and event dispatching */

    /* set up fonts */
    if (!cp_getvar("font", VT_STRING, fontname)) {
      (void) strcpy(fontname, DEF_FONT);
    }
    if (!(DEVDEP(graph).fontinfo = XOpenFont(fontname))) {
      sprintf(ErrorMessage, "can't open font %s", fontname);
      internalerror(ErrorMessage);
      RECOVERNEWVIEWPORT();
      return(1);
    }
    graph->fontwidth = DEVDEP(graph).fontinfo->width;
    graph->fontheight = DEVDEP(graph).fontinfo->height;

    initlinestyles();
    instantcolors(graph);

    /* set up cursor */
    cursor = XCreateCursor(left_ptr_width,
        left_ptr_height, left_ptr_bits, left_ptr_mask_bits,
        left_ptr_x_hot, left_ptr_y_hot, BlackPixel, WhitePixel,
        GXcopy);
    XDefineCursor(DEVDEP(graph).window, cursor);

    XMapWindow(DEVDEP(graph).window);

#define INPUTMASK (ButtonPressed | ExposeWindow | ExposeRegion | KeyPressed)
    XSelectInput(DEVDEP(graph).window, INPUTMASK);

#ifdef HAS_XT
    XtSetEventHandler(DEVDEP(currentgraph).window, eventhandler,
        ButtonPressed | ExposeWindow | ExposeRegion | KeyPressed,
        (char *) DEVDEP(currentgraph).window);
#endif

    /* have to be able to get from window id to graphid
        if your windowing system doesn't allow for this facility,
        you might want to try the following hack
            graph->graphid = DEVDEP(graph).window;
    */
    XMakeAssoc(xtable, DEVDEP(graph).window, (char *) graph);

    return (0);
}

static
initlinestyles()
{

    xlinestyles[0] = XMakePattern(1, 1, 2);     /* Solid. */
    xlinestyles[1] = XMakePattern(1, 4, 2);     /* Spaced dots. */
    if (DisplayPlanes() == 1) {
        xlinestyles[2] = XMakePattern(1, 2, 2); /* Dots. */
        xlinestyles[3] = XMakePattern(13, 5, 2);/* Dot shortdash. */
        xlinestyles[4] = XMakePattern(29, 6, 2);/* Dot longdash. */
        xlinestyles[5] = XMakePattern(3, 3, 2); /* Shortdash. */
        xlinestyles[6] = XMakePattern(7, 5, 2); /* Longdash. */
        xlinestyles[7] = XMakePattern(115, 9, 2);/* Short/long dashed.*/
    } else {
        /* Dotted lines are a distraction when we have colors. */
        xlinestyles[2] = xlinestyles[3] = xlinestyles[4] =
            xlinestyles[5] = xlinestyles[6] = xlinestyles[7] =
            xlinestyles[0];
    }

    return;
}

static void
initcolors()
{
    int numdispplanes = DisplayPlanes(), i;
    static char *colornames[] = {   "#ffffff",  /* white */
                    "#000000",
                    "#ff0000",
                    "#00ff00",
                    "#0000ff",
                    "#ffff00",
                    "#ff00ff",
                    "#00ffff",
                    "#ff7777",
                    "#77ff77",
                    "#7777ff",
                    "#ffff77",
                    "#ff77ff",
                    "#77ffff",
                    "#ffaa33",
                    "#ff33aa",
                    "#aaff33",
                    "#33ffaa",
                    "#aa33ff",
                    "#33aaff",
                    ""
                } ;
    Color col;
    char buf[BSIZE_SP], colorstring[BSIZE_SP];
    int xmaxcolors = NUMCOLORS; /* note: can we get rid of this? */

    if (numdispplanes == 1) {
      /* black and white */
      colors[0] = WhitePixel;
      colors[1] = BlackPixel;
      dispdev->numcolors = 2;
      return;
    } else if (numdispplanes < NXPLANES) {
      xmaxcolors = 1;
      while (numdispplanes-- > 1)
        xmaxcolors *= 2;
    }
    for (i = 0; i < xmaxcolors; i++) {
      (void) sprintf(buf, "color%d", i);
      if (!cp_getvar(buf, VT_STRING, colorstring))
        (void) strcpy(colorstring, colornames[i]);
      if (!XParseColor(colorstring, &col)) {
        (void) sprintf(ErrorMessage,
          "can't parse color %s\n", colorstring);
        internalerror(ErrorMessage);
        colors[i] = i ? BlackPixel : WhitePixel;
        continue;
      }
      if (!XGetHardwareColor(&col)) {
        externalerror("can't get hardware color");
        dispdev->numcolors = i;
        return;
      }
      colors[i] = col.pixel;
    }
    dispdev->numcolors = xmaxcolors;
    return;

}

/* instantiate colors, here, just copy global pixel array */
static void instantcolors(graph)
GRAPH *graph;
{

    int i;

    for (i=0; i < dispdev->numcolors; i++) {
      graph->colors[i] = colors[i];
    }

}

/* This routine closes the X connection.
    It is not to be called for finishing a graph. */
X_Close()
{

/* note: This is too high level to place here.  Find better place for it. */
/*      maybe create a gr_close */
/*  FreeGraphs(); */

    XDestroyAssocTable(xtable);
    XCloseDisplay(display);

}

X_DrawLine(x1, y1, x2, y2)
int x1, y1, x2, y2;
{

    Vertex vertex[2];

    /* dumb, huh? */
    vertex[0].flags = VertexDrawLastPoint;
    vertex[1].flags = VertexDrawLastPoint;

    /* If the last draw line operation on this graph was of the same
        linestyle as the current operation and left off at the
        same point as this one starts, just continue the last
        draw line, else start a new one. */

    if ((DEVDEP(currentgraph).lastlinestyle != currentgraph->linestyle) ||
            ((x1 != DEVDEP(currentgraph).lastx) ||
             (y1 != DEVDEP(currentgraph).lasty))) {
      vertex[0].x = x1;
      vertex[0].y = currentgraph->absolute.height - y1;
      vertex[1].x = x2;
      vertex[1].y = currentgraph->absolute.height - y2;
      XDrawDashed(DEVDEP(currentgraph).window, vertex, 2, 1, 1,
            currentgraph->colors[currentgraph->currentcolor],
            xlinestyles[currentgraph->linestyle],
            GXcopy, AllPlanes);
    } else {

/* I know, we're not supposed to do this.  But it works and it beats buffering
    points just because XDrawDashed doesn't work like it's supposed to. */

      vertex[0].x = x2;
      vertex[0].y = currentgraph->absolute.height - y2;
      switch (XAppendVertex(vertex, 1)) {
        case -1:
        /* will not fit in buffer */
        case 0:
        /* no Draw in progress */
        vertex[0].x = x1;
        vertex[0].y = currentgraph->absolute.height - y1;
        vertex[1].x = x2;
        vertex[1].y = currentgraph->absolute.height - y2;
        XDrawDashed(DEVDEP(currentgraph).window, vertex, 2, 1, 1,
                  currentgraph->colors[currentgraph->currentcolor],
                  xlinestyles[currentgraph->linestyle],
                  GXcopy, AllPlanes);
        break;
        case 1:
        /* O.K. */
        break;
        default:
        /* This will never happen. */
        externalerror("unrecognized return value form XAppendVertex");
        break;
      }
    }
    /* validate cache */
    DEVDEP(currentgraph).lastx = x2;
    DEVDEP(currentgraph).lasty = y2;
    DEVDEP(currentgraph).lastlinestyle = currentgraph->linestyle;

}

/*ARGSUSED*/
X_Arc(x0, y0, radius, theta1, theta2)
int x0, y0, radius;
double theta1, theta2;
{

    Vertex vertices[2];

    vertices[0].x = x0 + radius * cos((double) theta1);
    vertices[0].y = y0 + radius * sin((double) theta1);
    vertices[1].x = x0 + radius * cos((double) theta2);
    vertices[1].y = y0 + radius * sin((double) theta2);
    vertices[0].flags = VertexCurved;
    vertices[1].flags = VertexCurved | VertexDrawLastPoint;

    XDrawDashed(DEVDEP(currentgraph).window, vertices, 2, 1, 1,
        currentgraph->colors[currentgraph->currentcolor],
        xlinestyles[currentgraph->linestyle],
        GXcopy, AllPlanes);

}

/* note: x and y are the LOWER left corner of text */
X_Text(text, x, y)
char *text;
int x, y;
{

/* X has x and y set to UPPER left corner, so have to add fontheight */

    XText(DEVDEP(currentgraph).window, x,
        currentgraph->absolute.height
                - (y + currentgraph->fontheight),
        text, strlen(text), DEVDEP(currentgraph).fontinfo->id,
        currentgraph->colors[currentgraph->currentcolor],
        currentgraph->colors[0]);

    /* note: unlike before, we do not save any text here */

}

/*ARGSUSED*/
X_DefineColor(colorid, red, green, blue)
int colorid;
double red, green, blue;
{

    internalerror("X_DefineColor not implemented.");

    return(0);

}


/*ARGSUSED*/
X_DefineLinestyle(linestyleid, mask)
int linestyleid;
int mask;
{

    internalerror("X_DefineLinestyle not implemented.");

}

X_SetLinestyle(linestyleid)
int linestyleid;
{

    currentgraph->linestyle = linestyleid;

}

X_SetColor(colorid)
int colorid;
{

    currentgraph->currentcolor = colorid;

}

X_Update()
{

    XSync(0);

}

X_Clear()
{

    XClear(DEVDEP(currentgraph).window);

}

X_MakeMenu()
{

/* note: */

}

X_MakeDialog()
{

/* note: */

}

X_Track()
{

/* note: */

}

static handleXev()
{

    XEvent ev;

    XNextEvent(&ev);

#ifdef HAS_XT
    XtDispatchEvent(&ev);
#else
    eventhandler(&ev, (char *) NULL);
#endif

}

/*ARGSUSED*/
static void eventhandler(ev, data)
XEvent *ev;
char *data;
{

    GRAPH *graph;
    XKeyOrButtonEvent *buttonev;
    XKeyPressedEvent *keyev;
    XExposeEvent *exposeev;
    char *text;
    int nbytes;
    char buf[128];          /* for KeyPressed text */

#ifdef HAS_XT
    graph = (GRAPH *) data;
#else
    graph = (GRAPH *) XLookUpAssoc(xtable, (int) ev->window);
#endif

    if (!graph) {
      externalerror("Can't find graph.");
      return;
    }

    switch (ev->type) {
      case ButtonPressed:
        buttonev = (XKeyOrButtonEvent *) ev;
        switch (buttonev->detail) {
          case LeftButton:
        slopelocation(graph, buttonev->x, buttonev->y);
        break;
          case MiddleButton:
        killwin(graph);
        break;
          case RightButton:
        zoomin(graph);
        break;
          default:
        break;
        }
        break;

      case ExposeWindow:
      case ExposeRegion:
        exposeev = (XExposeEvent *) ev;

        if ((ev->type == ExposeWindow) &&
            ((exposeev->width !=
              graph->absolute.height) ||
            (exposeev->height) !=
              graph->absolute.width)) {
          /* a resize event */
          resize(graph, exposeev->width, exposeev->height);
        }
        redraw(graph);
        break;

      case KeyPressed:
        keyev = (XKeyPressedEvent *) ev;
        text = XLookupMapping(keyev, &nbytes);
        /* write it */
        PushGraphContext(graph);
        strncpy(buf, text, nbytes);
        buf[nbytes] = '\0';
        SetColor(1);
        Text(buf, keyev->x, graph->absolute.height - keyev->y);
        /* save it */
        SaveText(graph, buf, keyev->x, graph->absolute.height - keyev->y);
        /* warp mouse so user can type in sequence */
        XWarpMouse(DEVDEP(graph).window,
            keyev->x + XStringWidth(buf, DEVDEP(graph).fontinfo, 0, 0),
            keyev->y);
        PopGraphContext();
        break;

      default:
        break;
    }

}

slopelocation(graph, x0, y0)
GRAPH *graph;
int x0, y0;         /* initial position of mouse */
{

    int x1, y1;
    int x, y;
    Window subw;
    short state;
    double fx0, fx1, fy0, fy1;

    x1 = x0;
    y1 = y0;
    XQueryMouseButtons(DEVDEP(graph).window, &x, &y, &subw, &state);
    XLine(DEVDEP(graph).window, x0, y0, x0, y1-1,
            1, 1, 1, GXxor, AllPlanes);
    XLine(DEVDEP(graph).window, x0, y1, x1, y1,
            1, 1, 1, GXxor, AllPlanes);
    while (state & LeftMask) {
      if (x != x1 || y != y1) {
        XLine(DEVDEP(graph).window, x0, y0, x0, y1-1,
                1, 1, 1, GXxor, AllPlanes);
        XLine(DEVDEP(graph).window, x0, y1, x1, y1,
                1, 1, 1, GXxor, AllPlanes);
        x1 = x;
        y1 = y;
        XLine(DEVDEP(graph).window, x0, y0, x0, y1-1,
                1, 1, 1, GXxor, AllPlanes);
        XLine(DEVDEP(graph).window, x0, y1, x1, y1,
                1, 1, 1, GXxor, AllPlanes);
      }
      XQueryMouseButtons(DEVDEP(graph).window, &x, &y, &subw, &state);
    }
    XLine(DEVDEP(graph).window, x0, y0, x0, y1-1,
            1, 1, 1, GXxor, AllPlanes);
    XLine(DEVDEP(graph).window, x0, y1, x1, y1,
            1, 1, 1, GXxor, AllPlanes);

    X_ScreentoData(graph, x0, y0, &fx0, &fy0);
    X_ScreentoData(graph, x1, y1, &fx1, &fy1);

    /* print it out */
    if (x1 == x0 && y1 == y0) {		/* only one location */
      fprintf(stdout, "\nx0 = %g, y0 = %g\n", fx0, fy0);
    } else {	/* need to print info about two points */
      fprintf(stdout, "\nx0 = %g, y0 = %g    x1 = %g, y1 = %g\n",
          fx0, fy0, fx1, fy1);
      fprintf(stdout, "dx = %g, dy = %g\n", fx1-fx0, fy1 - fy0);
      if (x1 != x0 && y1 != y0) {
	/* add slope info if both dx and dy are zero,
	     because otherwise either dy/dx or dx/dy is zero,
	     which is uninteresting
	*/
        fprintf(stdout, "dy/dx = %g    dx/dy = %g\n",
	    (fy1-fy0)/(fx1-fx0), (fx1-fx0)/(fy1-fy0));
      }
    }

    return;

}

/* should be able to do this by sleight of hand on graph parameters */
zoomin(graph)
GRAPH *graph;
{
/* note: need to add circular boxes XXX */

    int x0, y0, x1, y1;
    Window subw;
    Vertex vlist[5];
    double fx0, fx1, fy0, fy1, ftemp;
    XEvent ev;
    char buf[BSIZE_SP];
    wordlist *wl;
    int dummy;

    /* open box and get area to zoom in on */

    XQueryMouse(DEVDEP(graph).window, &x0, &y0, &subw);

    x1 = x0 + BOXSIZE;
    y1 = y0 + BOXSIZE;
    XWarpMouse(DEVDEP(graph).window, x1, y1);

    vlist[0].x = x0;
    vlist[0].y = y0;
    vlist[0].flags = VertexStartClosed;
    vlist[1].x = x1;
    vlist[1].y = y0;
    vlist[1].flags = 0;
    vlist[2].x = x1;
    vlist[2].y = y1;
    vlist[2].flags = 0;
    vlist[3].x = x0;
    vlist[3].y = y1;
    vlist[3].flags = 0;
    vlist[4].x = x0;
    vlist[4].y = y0;
    vlist[4].flags = VertexEndClosed | VertexDrawLastPoint;

    XDraw(DEVDEP(graph).window, vlist, 5, 1, 1, 1, GXxor, AllPlanes);
    XSelectInput(DEVDEP(graph).window, MouseMoved | ButtonReleased);

    for (;;) {
      XWindowEvent(DEVDEP(graph).window,
            MouseMoved | ButtonReleased, &ev);
      x1 = ((XKeyOrButtonEvent *) &ev)->x;
      y1 = ((XKeyOrButtonEvent *) &ev)->y;
      if (ev.type == MouseMoved) {
        XDraw(DEVDEP(graph).window, vlist, 5, 1, 1, 1, GXxor, AllPlanes);
        vlist[1].x = vlist[2].x = x1;
        vlist[2].y = vlist[3].y = y1;
        XDraw(DEVDEP(graph).window, vlist, 5, 1, 1, 1, GXxor, AllPlanes);
      } else {
        XDraw(DEVDEP(graph).window, vlist, 5, 1, 1, 1, GXxor, AllPlanes);
        XSelectInput(DEVDEP(graph).window, INPUTMASK);
        break;
      }
    }

    X_ScreentoData(graph, x0, y0, &fx0, &fy0);
    X_ScreentoData(graph, x1, y1, &fx1, &fy1);

    if (fx0 > fx1) {
      ftemp = fx0;
      fx0 = fx1;
      fx1 = ftemp;
    }
    if (fy0 > fy1) {
      ftemp = fy0;
      fy0 = fy1;
      fy1 = ftemp;
    }

    if (!eq(plot_cur->pl_typename, graph->plotname)) {
      (void) sprintf(buf,
       "setplot %s ; %s xlimit %lg %lg ylimit %lg %lg ; setplot $curplot\n",
       graph->plotname, graph->commandline,
       fx0, fx1, fy0, fy1);
    } else {
      (void) sprintf(buf, "%s xlimit %lg %lg ylimit %lg %lg\n",
            graph->commandline, fx0, fx1, fy0, fy1);
    }

    /* hack for Gordon Jacobs */
    /* add to history list if plothistory is set */
    if (cp_getvar("plothistory", VT_BOOL, (char *) &dummy)) {
      wl = cp_parse(buf);
      (void) cp_addhistent(cp_event++, wl);
    }

    (void) cp_evloop(buf);

}

killwin(graph)
GRAPH *graph;
{

    XDestroyWindow(DEVDEP(graph).window);
    XDeleteAssoc(xtable, DEVDEP(graph).window);
    DestroyGraph(graph->graphid);

}

/* call higher gr_redraw routine */
redraw(graph)
GRAPH *graph;
{

    gr_redraw(graph);

}

resize(graph, width, height)
GRAPH *graph;
int width, height;
{

    graph->absolute.width = width;
    graph->absolute.height = height;
    gr_resize(graph);

}

X_Input(request, response)
    REQUEST *request;
    RESPONSE *response;
{

    XEvent ev;

    int nfds, readfds;

    switch (request->option) {
      case char_option:
#ifdef HAS_XT
        do {
          XtNextEvent(&ev);
          XtDispatchEvent(&ev);
        } while (!EXTERNkey);
        break;
#else

/* note: XXX move this loop up out of char_option */
        nfds = dpyno() > fileno(request->fp) ?
                    dpyno() :
                    fileno(request->fp);

        while (1) {

          /* first read off the queue before doing the select */
          while (XPending()) {
		handleXev();
          }

          readfds = 1 << dpyno() | 1 << fileno(request->fp);
          /* block on dpyno and request->fp */
          select(nfds+1, &readfds, (int *) NULL, (int *) NULL, NULL);

          /* handle X events first */
          if (readfds & (1 << dpyno())) {
            /* handle ALL X events */
        do {
          handleXev();
        } while (XPending());
          }

          if (readfds & (1 << fileno(request->fp))) {
            response->reply.ch = inchar(request->fp);
        goto out;
          }
        }
#endif
      case click_option:
        /* inform user to hit a key */
        fprintf(cp_out, "click on the desired graph\n");
        /* note: do we want to do a grab here just to change the cursor? */
        XMaskEvent(ButtonPressed, &ev);
        response->reply.graph = (GRAPH *) XLookUpAssoc(xtable,
                (int) ev.window);
        if (!response->reply.graph) {
          externalerror("Can't find graph.");
          response->option = error_option;
          return;
        }
        break;

      case button_option:
        /* sit and handle events until get a button selection */
        internalerror("button_option not implemented");
        response->option = error_option;
        return;

    case checkup_option:
      while (XPending()) {
#ifdef HAS_XT
          XtNextEvent(&ev);
          XtDispatchEvent(&ev);
#else
	handleXev();
#endif
      }
      break;

      default:
        internalerror("unrecognized input type");
        response->option = error_option;
        return;
    }

out:
    if (response)
	response->option = request->option;
    return;

}

static X_ScreentoData(graph, x, y, fx, fy)
GRAPH *graph;
int x,y;
double *fx, *fy;
{

    double low, high;

    /* note: figure something out for polar grids, etc XXX */

    if ((graph->grid.gridtype == GRID_LOGLOG) ||
            (graph->grid.gridtype == GRID_XLOG)) {
      low  = mylog10(graph->datawindow.xmin);
      high = mylog10(graph->datawindow.xmax);
      *fx = pow( (double) 10,
             (((double) x - graph->viewportxoff)
            / graph->viewport.width * (high - low) + low) );
    } else {
      *fx = (x - graph->viewportxoff) * graph->aspectratiox
            + graph->datawindow.xmin;
    }

    if ((graph->grid.gridtype == GRID_LOGLOG) ||
            (graph->grid.gridtype == GRID_YLOG)) {
      low  = mylog10(graph->datawindow.ymin);
      high = mylog10(graph->datawindow.ymax);
      *fy = pow( (double) 10,
             (((double) (graph->absolute.height - y) -
            graph->viewportyoff)
            / graph->viewport.height * (high - low) + low) );
    } else {
      *fy = ((graph->absolute.height - y) - graph->viewportyoff)
            * graph->aspectratioy
            + graph->datawindow.ymin;
    }

}

#endif /* HAS_X10 */
