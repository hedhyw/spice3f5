/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1988 Jeffrey M. Hsu
**********/

/*
	X11 drivers.
*/

#include "spice.h"

#ifdef HAS_X11

#  include <sys/time.h>

#  include "util.h"

#  include "ftegraph.h"
#  include "ftedbgra.h"
#  include "ftedev.h"
#  include "fteinput.h"
#  include "cpdefs.h"
#  include "ftedefs.h"

#  include <X11/IntrinsicP.h>
#  include <X11/Xatom.h>
#  include <X11/StringDefs.h>
#  include <X11/Xutil.h>
#  include <X11/cursorfont.h>
#  include <X11/Xaw/Box.h>
#  include <X11/Xaw/Command.h>
#  include <X11/Xaw/Form.h>
#  include <X11/Shell.h>

#  ifdef DEBUG
extern int _Xdebug;
#  endif

static void X_ScreentoData();
static void initcolors();
static void initlinestyles();

/* forward declarations */
extern void handlebuttonev(), handlekeypressed(), killwin(), hardcopy(),
	    redraw(), resize();
int errorhandler();

#define RAD_TO_DEG	(180.0 / M_PI)

/* X dependent default parameters */
#define DEF_FONT "6x10"
#define NUMLINESTYLES 8
#define NXPLANES 5      /* note: What is this used for? */
#define BOXSIZE 30      /* initial size of bounding box for zoomin */

typedef struct x11info {
	Window window;
	int	isopen;
	Widget shell, form, view, buttonbox, buttons[2];
	XFontStruct *font;
	GC gc;
	int lastx, lasty;   /* used in X_DrawLine */
	int lastlinestyle;  /* used in X_DrawLine */
} X11devdep;

#define DEVDEP(g) (*((X11devdep *) (g)->devdep))

static void linear_arc( );
static Display *display;
static GC xorgc;
static char *xlinestyles[NUMLINESTYLES] = {	/* test patterns XXX */
	"\001\001\001\001",	/* solid */
	"\001\002\001\002",	/* dots */
	"\007\007\007\007",	/* longdash */
	"\003\003\003\003",	/* shortdash */
	"\007\002\002\002",	/* dots longdash */
	"\003\002\001\002",	/* dots shortdash */
	"\003\003\007\003",	/* short/longdash */
};

static Widget toplevel;
static Bool noclear = False;
static GRAPH *lasthardcopy; /* graph user selected */
static int X11_Open = 0;
static int numdispplanes;

X11_Init()
{

	char buf[512];
	char *displayname;

	XGCValues gcvalues;

	/* grrr, Xtk forced contortions */
	char *argv[2];
	int argc = 2;

	if (cp_getvar("display", VT_STRING, buf)) {
	  displayname = buf;
	} else if (!(displayname = getenv("DISPLAY"))) {
	  internalerror("Can't open X display.");
	  return (1);
	}

#  ifdef DEBUG
	_Xdebug = 1;
#  endif

	argv[0] = "spice3";
	argv[1] = displayname;
/*
	argv[2] = "-geometry";
	argv[3] = "=1x1+2+2";
*/

	/* initialize X toolkit */
	toplevel = XtInitialize("spice3", "Nutmeg", NULL, 0, &argc, argv);

	display = XtDisplay(toplevel);

	X11_Open = 1;

	/* "invert" works better than "xor" for B&W */

	/* xor gc should be a function of the pixels that are written on */
	gcvalues.function = GXxor;
	gcvalues.line_width = 1;
	gcvalues.foreground = 1;
	gcvalues.background = 0;

	xorgc = XCreateGC(display, DefaultRootWindow(display),
	        GCLineWidth | GCFunction | GCForeground | GCBackground,
	        &gcvalues);

	/* set correct information */
	dispdev->numlinestyles = NUMLINESTYLES;
	dispdev->numcolors = NUMCOLORS;

	dispdev->width = DisplayWidth(display, DefaultScreen(display));
	dispdev->height = DisplayHeight(display, DefaultScreen(display));

	/* we don't want non-fatal X errors to call exit */
	XSetErrorHandler(errorhandler);

	numdispplanes = DisplayPlanes(display, DefaultScreen(display));

	return (0);

}

int
errorhandler(display, errorev)
Display *display;
XErrorEvent *errorev;
{
	XGetErrorText(display, errorev->error_code, ErrorMessage, 1024);
	externalerror(ErrorMessage);
	return;
}

/* Recover from bad NewViewPort call. */
#define RECOVERNEWVIEWPORT()    free((char *) graph);\
	            graph = (GRAPH *) NULL;
	    /* need to do this or else DestroyGraph will free it again */

/* NewViewport is responsible for filling in graph->viewport */
X11_NewViewport(graph)
GRAPH *graph;
{

	char geometry[32], defaultgeometry[32];
	char fontname[513]; /* who knows . . . */
	char *p, *q;
	Cursor cursor;
	XSetWindowAttributes	w_attrs;
	XGCValues gcvalues;
	static Arg formargs[ ] = {
	    { XtNleft, (XtArgVal) XtChainLeft },
	    { XtNresizable, (XtArgVal) TRUE }
	};
	static Arg bboxargs[ ] = {
	    { XtNfromHoriz, (XtArgVal) NULL },
	    { XtNbottom, (XtArgVal) XtChainTop },
	    { XtNtop, (XtArgVal) XtChainTop },
	    { XtNleft, (XtArgVal) XtChainRight },
	    { XtNright, (XtArgVal) XtChainRight }
	};
	static Arg buttonargs[ ] = {
	    { XtNlabel, (XtArgVal) NULL },
	    { XtNfromVert, (XtArgVal) NULL },
	    { XtNbottom, (XtArgVal) XtChainTop },
	    { XtNtop, (XtArgVal) XtChainTop },
	    { XtNleft, (XtArgVal) XtRubber },
	    { XtNright, (XtArgVal) XtRubber },
	    { XtNresizable, (XtArgVal) TRUE }
	};
	static Arg viewargs[] = {
	    { XtNresizable, (XtArgVal) TRUE },
	    { XtNwidth, (XtArgVal) 300 },
	    { XtNheight, (XtArgVal) 300 },
	    { XtNright, (XtArgVal) XtChainRight }
	};
	int	trys;

	graph->devdep = calloc(1, sizeof(X11devdep));

	/* set up new shell */
	DEVDEP(graph).shell = XtCreateApplicationShell("shell",
	        topLevelShellWidgetClass, NULL, 0);

	/* set up form widget */
	DEVDEP(graph).form = XtCreateManagedWidget("form",
	    formWidgetClass, DEVDEP(graph).shell, formargs, XtNumber(formargs));

	/* set up viewport */
	DEVDEP(graph).view = XtCreateManagedWidget("viewport", widgetClass,
	    DEVDEP(graph).form, viewargs, XtNumber(viewargs));
	XtAddEventHandler(DEVDEP(graph).view, ButtonPressMask, FALSE,
	        handlebuttonev, graph);
	XtAddEventHandler(DEVDEP(graph).view, KeyPressMask, FALSE,
	        handlekeypressed, graph);
	XtAddEventHandler(DEVDEP(graph).view, StructureNotifyMask, FALSE,
	        resize, graph);
	XtAddEventHandler(DEVDEP(graph).view, ExposureMask, FALSE,
	        redraw, graph);

	/* set up button box */
	XtSetArg(bboxargs[1], XtNfromHoriz, DEVDEP(graph).view);
	DEVDEP(graph).buttonbox = XtCreateManagedWidget("buttonbox",
	    boxWidgetClass, DEVDEP(graph).form, bboxargs, XtNumber(bboxargs));

	/* set up buttons */
	XtSetArg(buttonargs[0], XtNlabel, "quit");
	XtSetArg(bboxargs[1], XtNfromVert, NULL);
	DEVDEP(graph).buttons[0] = XtCreateManagedWidget("quit",
	    commandWidgetClass, DEVDEP(graph).buttonbox,
	    buttonargs, 1);
	XtAddCallback(DEVDEP(graph).buttons[0], XtNcallback, killwin, graph);

	XtSetArg(buttonargs[0], XtNlabel, "hardcopy");
	XtSetArg(bboxargs[1], XtNfromVert, DEVDEP(graph).buttons[0]);
	DEVDEP(graph).buttons[1] = XtCreateManagedWidget("hardcopy",
	    commandWidgetClass, DEVDEP(graph).buttonbox,
	    buttonargs, 1);
	XtAddCallback(DEVDEP(graph).buttons[1], XtNcallback, hardcopy, graph);

	/* set up fonts */
	if (!cp_getvar("font", VT_STRING, fontname)) {
	  (void) strcpy(fontname, DEF_FONT);
	}

	for (p = fontname; *p && *p <= ' '; p++)
		;
	if (p != fontname) {
		for (q = fontname; *p; *q++ = *p++)
			;
		*q = 0;
	}

	trys = 1;
	while (!(DEVDEP(graph).font = XLoadQueryFont(display, fontname))) {
	  sprintf(ErrorMessage, "can't open font %s", fontname);
	  strcpy(fontname, "fixed");
	  if (trys > 1) {
	      internalerror(ErrorMessage);
	      RECOVERNEWVIEWPORT();
	      return(1);
	  }
	  trys += 1;
	}

	graph->fontwidth = DEVDEP(graph).font->max_bounds.rbearing -
	        DEVDEP(graph).font->min_bounds.lbearing + 1;
	graph->fontheight = DEVDEP(graph).font->max_bounds.ascent +
	        DEVDEP(graph).font->max_bounds.descent + 1;

	XtRealizeWidget(DEVDEP(graph).shell);

	DEVDEP(graph).window = XtWindow(DEVDEP(graph).view);
	DEVDEP(graph).isopen = 0;
	w_attrs.bit_gravity = ForgetGravity;
	XChangeWindowAttributes(display, DEVDEP(graph).window, CWBitGravity,
		&w_attrs);
	/* have to note font and set mask GCFont in XCreateGC, p.w.h. */
	gcvalues.font = DEVDEP(graph).font->fid;
	gcvalues.line_width = 1;
	gcvalues.cap_style = CapNotLast;
	gcvalues.function = GXcopy;
	DEVDEP(graph).gc = XCreateGC(display, DEVDEP(graph).window,
	        GCFont | GCLineWidth | GCCapStyle | GCFunction, &gcvalues);

	/* should absolute.positions really be shell.pos? */
	graph->absolute.xpos = DEVDEP(graph).view->core.x;
	graph->absolute.ypos = DEVDEP(graph).view->core.y;
	graph->absolute.width = DEVDEP(graph).view->core.width;
	graph->absolute.height = DEVDEP(graph).view->core.height;

	initlinestyles();
	initcolors(graph);

	/* set up cursor */
	cursor = XCreateFontCursor(display, XC_left_ptr);
	XDefineCursor(display, DEVDEP(graph).window, cursor);

	return (0);
}

static void
initlinestyles()
{

	int i;

	if (numdispplanes > 1) {
	  /* Dotted lines are a distraction when we have colors. */
	  for (i = 2; i < NUMLINESTYLES; i++) {
	    xlinestyles[i] = xlinestyles[0];
	  }
	}

	return;
}

static void
initcolors(graph)
    GRAPH *graph;
{
    int i;
    static char *colornames[] = {   "white",    /* white */
	"black", "red", "blue",
	"orange", "green", "pink",
	"brown", "khaki", "plum",
	"orchid", "violet", "maroon",
	"turquoise", "sienna", "coral",
	"cyan", "magenta", "gold",
	"yellow", ""
	};

    XColor visualcolor, exactcolor;
    char buf[BSIZE_SP], colorstring[BSIZE_SP];
    int xmaxcolors = NUMCOLORS; /* note: can we get rid of this? */

    if (numdispplanes == 1) {
	/* black and white */
	xmaxcolors = 2;
	graph->colors[0] = DEVDEP(graph).view->core.background_pixel;
	if (graph->colors[0] == WhitePixel(display, DefaultScreen(display)))
	    graph->colors[1] = BlackPixel(display, DefaultScreen(display));
	else
	    graph->colors[1] = WhitePixel(display, DefaultScreen(display));

    } else {
	if (numdispplanes < NXPLANES)
	    xmaxcolors = 1 << numdispplanes;

	for (i = 0; i < xmaxcolors; i++) {
	    (void) sprintf(buf, "color%d", i);
	    if (!cp_getvar(buf, VT_STRING, colorstring))
	    (void) strcpy(colorstring, colornames[i]);
	    if (!XAllocNamedColor(display,
		    DefaultColormap(display, DefaultScreen(display)),
		    colorstring, &visualcolor, &exactcolor)) {
		(void) sprintf(ErrorMessage,
		    "can't get color %s\n", colorstring);
		externalerror(ErrorMessage);
		graph->colors[i] = i ? BlackPixel(display,
		    DefaultScreen(display))
		    : WhitePixel(display, DefaultScreen(display));
		continue;
	    }
	    graph->colors[i] = visualcolor.pixel;
	    if (i > 0 &&
		graph->colors[i] == DEVDEP(graph).view->core.background_pixel) {
		graph->colors[i] = graph->colors[0];
	    }
	}

	if (graph->colors[0] != DEVDEP(graph).view->core.background_pixel) {
	    graph->colors[0] = DEVDEP(graph).view->core.background_pixel;
	}
    }

    for (i = xmaxcolors; i < NUMCOLORS; i++) {
	graph->colors[i] = graph->colors[i + 1 - xmaxcolors];
    }

    return;
}

/* This routine closes the X connection.
	It is not to be called for finishing a graph. */
X11_Close()
{
	XCloseDisplay(display);
}

X11_DrawLine(x1, y1, x2, y2)
int x1, y1, x2, y2;
{

	if (DEVDEP(currentgraph).isopen)
		XDrawLine(display, DEVDEP(currentgraph).window,
			DEVDEP(currentgraph).gc,
			x1, currentgraph->absolute.height - y1,
			x2, currentgraph->absolute.height - y2);


}

/*ARGSUSED*/
X11_Arc(x0, y0, radius, theta1, theta2)
int x0, y0, radius;
double theta1, theta2;
{

    int	t1, t2;

    if (!cp_getvar("x11lineararcs", VT_BOOL, (char *) &t1)) {
	linear_arc(x0, y0, radius, theta1, theta2);
    }

    if (DEVDEP(currentgraph).isopen) {
	if (theta1 >= theta2)
	    theta2 = 2 * M_PI + theta2;
	t1 = 64 * (180.0 / M_PI) * theta1;
	t2 = 64 * (180.0 / M_PI) * theta2 - t1;
	if (t2 == 0)
		return;
	XDrawArc(display, DEVDEP(currentgraph).window, DEVDEP(currentgraph).gc,
		x0 - radius,
		currentgraph->absolute.height - radius - y0,
		2 * radius, 2 * radius, t1, t2);
#  ifdef notdef
	printf("at %d, %d, %g %g x %d :: (%d, %d)\n",
		x0, y0, theta1, theta2, radius, t1, t2);
		printf("skip\n");
	XSync(display, 0);
	printf("XDrawArc(%d, %d, %d, %d, %d, %d)\n", x0 - radius,
		currentgraph->absolute.height - radius - y0,
		2 * radius, 2 * radius, t1, t2);
#  endif
    }
}

/* note: x and y are the LOWER left corner of text */
X11_Text(text, x, y)
char *text;
int x, y;
{

/* We specify text position by lower left corner, so have to adjust for
	X11's font nonsense. */

	if (DEVDEP(currentgraph).isopen)
		XDrawString(display, DEVDEP(currentgraph).window,
		    DEVDEP(currentgraph).gc, x,
		    currentgraph->absolute.height
			- (y + DEVDEP(currentgraph).font->max_bounds.descent),
		    text, strlen(text));

	/* note: unlike before, we do not save any text here */

}

/*ARGSUSED*/
X11_DefineColor(colorid, red, green, blue)
int colorid;
double red, green, blue;
{
	internalerror("X11_DefineColor not implemented.");
	return(0);
}


/*ARGSUSED*/
X11_DefineLinestyle(linestyleid, mask)
int linestyleid;
int mask;
{
	internalerror("X11_DefineLinestyle not implemented.");
}

X11_SetLinestyle(linestyleid)
int linestyleid;
{
	XGCValues values;

	if (currentgraph->linestyle != linestyleid) {

#  ifdef notdef
	  switch (linestyleid %3) {
	  case 0:
	    values.line_style = LineSolid;
		break;
	  case 1:
	    values.line_style = LineOnOffDash;
		break;
	  case 2:
	    values.line_style = LineDoubleDash;
		break;
	  }
#  endif
	  if (linestyleid == 0 || numdispplanes > 1 && linestyleid != 1) {
	    /* solid if linestyle 0 or if has color, allow only one
	     * dashed linestyle */
	    values.line_style = LineSolid;
	  } else {
	    values.line_style = LineOnOffDash;
	  }
	  XChangeGC(display, DEVDEP(currentgraph).gc, GCLineStyle, &values);

	  currentgraph->linestyle = linestyleid;
	  XSetDashes(display, DEVDEP(currentgraph).gc, 0,
		xlinestyles[linestyleid], 4);
	}
}

X11_SetColor(colorid)
int colorid;
{

	currentgraph->currentcolor = colorid;
	XSetForeground(display, DEVDEP(currentgraph).gc,
	        currentgraph->colors[colorid]);

}

X11_Update()
{

	if (X11_Open)
		XSync(display, 0);

}

X11_Clear()
{

	if (!noclear) /* hack so exposures look like they're handled nicely */
	  XClearWindow(display, DEVDEP(currentgraph).window);

}

void
handlekeypressed(w, clientdata, calldata)
Widget w;
caddr_t clientdata, calldata;
{

	XKeyEvent *keyev = (XKeyPressedEvent *) calldata;
	GRAPH *graph = (GRAPH *) clientdata;
	char text[4];
	int nbytes;

	nbytes = XLookupString(keyev, text, 4, NULL, NULL);
	if (!nbytes) return;
	/* write it */
	PushGraphContext(graph);
	text[nbytes] = '\0';
	SetColor(1);
	Text(text, keyev->x, graph->absolute.height - keyev->y);
	/* save it */
	SaveText(graph, text, keyev->x, graph->absolute.height - keyev->y);
	/* warp mouse so user can type in sequence */
	XWarpPointer(display, None, DEVDEP(graph).window, 0, 0, 0, 0,
	    keyev->x + XTextWidth(DEVDEP(graph).font, text, nbytes),
	    keyev->y);
	PopGraphContext();

}

#  ifdef notdef
void
keyhandler(clientdata, source, id)
caddr_t clientdata;
int *source;
XtInputId id;
{

#    ifdef notdef
	KEYwaiting = TRUE;
#  endif

}
#  endif

void
handlebuttonev(w, clientdata, calldata)
Widget w;
caddr_t clientdata, calldata;
{

	XButtonEvent *buttonev = (XButtonEvent *) calldata;

	switch (buttonev->button) {
	  case Button1:
	    slopelocation((GRAPH *) clientdata, buttonev->x, buttonev->y);
	    break;
	  case Button3:
	    zoomin((GRAPH *) clientdata);
	    break;
	}

}

#  ifdef notdef
handlemotionev(w, clientdata, calldata)
Widget w;
caddr_t clientdata, calldata;
{

	XMotionEvent *motionev = (XMotionEvent *) calldata;

	switch
}
#  endif

slopelocation(graph, x0, y0)
GRAPH *graph;
int x0, y0;         /* initial position of mouse */
{

	int x1, y1;
	int x, y;
	Window rootwindow, childwindow;
	int rootx, rooty;
	unsigned int state;
	double fx0, fx1, fy0, fy1;
	double angle;

	x1 = x0;
	y1 = y0;
	XQueryPointer(display, DEVDEP(graph).window, &rootwindow, &childwindow,
	        &rootx, &rooty, &x, &y, &state);
	XDrawLine(display, DEVDEP(graph).window, xorgc, x0, y0, x0, y1-1);
	XDrawLine(display, DEVDEP(graph).window, xorgc, x0, y1, x1, y1);
	while (state & Button1Mask) {
	  if (x != x1 || y != y1) {
	    XDrawLine(display, DEVDEP(graph).window, xorgc,
	        x0, y0, x0, y1-1);
	    XDrawLine(display, DEVDEP(graph).window, xorgc,
	        x0, y1, x1, y1);
	    x1 = x;
	    y1 = y;
	    XDrawLine(display, DEVDEP(graph).window, xorgc, x0, y0, x0, y1-1);
	    XDrawLine(display, DEVDEP(graph).window, xorgc, x0, y1, x1, y1);
	  }
	  XQueryPointer(display, DEVDEP(graph).window, &rootwindow,
	        &childwindow, &rootx, &rooty, &x, &y, &state);
	}
	XDrawLine(display, DEVDEP(graph).window, xorgc, x0, y0, x0, y1-1);
	XDrawLine(display, DEVDEP(graph).window, xorgc, x0, y1, x1, y1);

	X_ScreentoData(graph, x0, y0, &fx0, &fy0);
	X_ScreentoData(graph, x1, y1, &fx1, &fy1);

	/* print it out */
	if (x1 == x0 && y1 == y0) {     /* only one location */
	    fprintf(stdout, "\nx0 = %g, y0 = %g\n", fx0, fy0);
	    if (graph->grid.gridtype == GRID_POLAR
	        || graph->grid.gridtype == GRID_SMITH
		|| graph->grid.gridtype == GRID_SMITHGRID)
	    {
		angle = RAD_TO_DEG * atan2( fy0, fx0 );
		fprintf(stdout, "r0 = %g, a0 = %g\n",
		    sqrt( fx0*fx0 + fy0*fy0 ),
		    (angle>0)?angle:(double) 360+angle);
	    }


	} else {    /* need to print info about two points */
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
	double fx0, fx1, fy0, fy1, ftemp;
	char buf[BSIZE_SP];
	char buf2[128];
	char *t;
	wordlist *wl;
	int dummy;

	Window rootwindow, childwindow;
	int rootx, rooty;
	unsigned int state;
	int x, y, upperx, uppery, lowerx, lowery;

	/* open box and get area to zoom in on */

	XQueryPointer(display, DEVDEP(graph).window, &rootwindow,
	        &childwindow, &rootx, &rooty, &x0, &y0, &state);

	x = lowerx = x1 = x0 + BOXSIZE;
	y = lowery = y1 = y0 + BOXSIZE;
	upperx = x0;
	uppery = y0;

	XDrawRectangle(display, DEVDEP(graph).window, xorgc,
	        upperx, uppery, lowerx - upperx, lowery - uppery);

/* note: what are src_x, src_y, src_width, and src_height for? XXX */
	XWarpPointer(display, None, DEVDEP(graph).window, 0, 0, 0, 0, x1, y1);

	while (state & Button3Mask) {
	  if (x != x1 || y != y1) {
	    XDrawRectangle(display, DEVDEP(graph).window, xorgc,
	        upperx, uppery, lowerx - upperx, lowery - uppery);
	    x1 = x;
	    y1 = y;
	    /* figure out upper left corner */
	    /* remember X11's (and X10's) demented coordinate system */
	    if (y0 < y1) {
	      uppery = y0;
	      upperx = x0;
	      lowery = y1;
	      lowerx = x1;
	    } else {
	      uppery = y1;
	      upperx = x1;
	      lowery = y0;
	      lowerx = x0;
	    }
	    XDrawRectangle(display, DEVDEP(graph).window, xorgc,
	        upperx, uppery, lowerx - upperx, lowery - uppery);
	  }
	  XQueryPointer(display, DEVDEP(graph).window, &rootwindow,
	           &childwindow, &rootx, &rooty, &x, &y, &state);
	}
	XDrawRectangle(display, DEVDEP(graph).window, xorgc,
	        upperx, uppery, lowerx - upperx, lowery - uppery);

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

	strncpy(buf2, graph->plotname, sizeof(buf2));
	if (t = index(buf2, ':'))
		*t = 0;

	if (!eq(plot_cur->pl_typename, buf2)) {
	  (void) sprintf(buf,
"setplot %s; %s xlimit %l.20e %l.20e ylimit %l.20e %l.20e; setplot $curplot\n",
	   buf2, graph->commandline, fx0, fx1, fy0, fy1);
	} else {
	  (void) sprintf(buf, "%s xlimit %le %le ylimit %le %le\n",
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

void
hardcopy(w, client_data, call_data)
    Widget w;
    caddr_t client_data, call_data;
{

	lasthardcopy = (GRAPH *) client_data;
	com_hardcopy(NULL);

}

void
killwin(w, client_data, call_data)
    Widget w;
    caddr_t client_data, call_data;
{

	GRAPH *graph = (GRAPH *) client_data;

	/* Iplots are done asynchronously */
	DEVDEP(graph).isopen = 0;
	DestroyGraph(graph->graphid);
	XtDestroyWidget(DEVDEP(graph).shell);

}

/* call higher gr_redraw routine */
void
redraw(w, client_data, call_data)
    Widget w;
    caddr_t client_data, call_data;
{

	GRAPH *graph = (GRAPH *) client_data;
	XExposeEvent *pev = (XExposeEvent *) call_data;
	XEvent ev;
	XRectangle rects[30];
	int n = 1;

	DEVDEP(graph).isopen = 1;
#  ifdef notdef
	/* if there is a resize, let the resize routine handle the exposures */
	if (XCheckWindowEvent(display, DEVDEP(graph).window,
	 (long) StructureNotifyMask, &ev)) {
	  resize(w, client_data, &ev);
	  return;
	}
#  endif

	rects[0].x = pev->x;
	rects[0].y = pev->y;
	rects[0].width = pev->width;
	rects[0].height = pev->height;

	/* XXX */
	/* pull out all other expose regions that need to be redrawn */
	while (n < 30 && XCheckWindowEvent(display, DEVDEP(graph).window,
	        (long) ExposureMask, &ev)) {
	  pev = (XExposeEvent *) &ev;
	  rects[n].x = pev->x;
	  rects[n].y = pev->y;
	  rects[n].width = pev->width;
	  rects[n].height = pev->height;
	  n++;
	}
	XSetClipRectangles(display, DEVDEP(graph).gc, 0, 0,
	        rects, n, Unsorted);

	noclear = True;
	gr_redraw(graph);
	noclear = False;

	XSetClipMask(display, DEVDEP(graph).gc, None);

}

void
resize(w, client_data, call_data)
Widget w;
caddr_t client_data, call_data;
{

	GRAPH *graph = (GRAPH *) client_data;
	XEvent ev;

	/* pull out all other exposure events
	   Also, get rid of other StructureNotify events on this window. */

	while (XCheckWindowEvent(display, DEVDEP(graph).window,
		(long) /* ExposureMask | */ StructureNotifyMask, &ev))
		;

	XClearWindow(display, DEVDEP(graph).window);
	graph->absolute.width = w->core.width;
	graph->absolute.height = w->core.height;
	gr_resize(graph);

}

#  ifdef notdef
/* stolen from CP/lexical.c */

/* A special 'getc' so that we can deal with ^D properly. There is no way for
 * stdio to know if we have typed a ^D after some other characters, so
 * don't use buffering at all...
 */
static int inchar(fp)
    FILE *fp;
{

	char c;
	int i;
	extern int errno;

#    ifdef HAS_TERMREAD
	if (cp_interactive && !cp_nocc) {
	  i = read((int) fileno(fp), &c, 1);
	  if (i == 0)
	    return (EOF);
	  else if (i == -1) {
	    perror("read");
	    return (EOF);
	  } else
	    return ((int) c);
	}
#    endif
	c = getc(fp);
	return ((int) c);
}
#  endif

X11_Input(request, response)
    REQUEST *request;
    RESPONSE *response;
{

	XEvent ev;
	int nfds, readfds;

	switch (request->option) {
	  case char_option:

	    nfds = ConnectionNumber(display) > fileno(request->fp) ?
	        ConnectionNumber(display) :
	        fileno(request->fp);

	    while (1) {

	      /* first read off the queue before doing the select */
	      while (XtPending()) {
	        XtNextEvent(&ev);
	        XtDispatchEvent(&ev);
	      }

	      readfds = 1 << fileno(request->fp) |
	            1 << ConnectionNumber(display);

	      /* block on ConnectionNumber and request->fp */
	      select(nfds + 1, &readfds, (int *) NULL, (int *) NULL, NULL);

	      /* handle X events first */
	      if (readfds & (1 << ConnectionNumber(display))) {
		    /* handle ALL X events */
		    while (XtPending()) {
			  XtNextEvent(&ev);
			  XtDispatchEvent(&ev);
		    }
	      }

	      if (readfds & (1 << fileno(request->fp))) {
		    response->reply.ch = inchar(request->fp);
		    goto out;
	      }

	    }
	    break;

	  case click_option:
	    /* let's fake this */
	    response->reply.graph = lasthardcopy;
	    break;

	  case button_option:
	    /* sit and handle events until get a button selection */
	    internalerror("button_option not implemented");
	    response->option = error_option;
	    return;
	    break;

	  case checkup_option:
	    /* first read off the queue before doing the select */
	    while (XtPending()) {
	        XtNextEvent(&ev);
	        XtDispatchEvent(&ev);
	    }
	    break;

	  default:
	    internalerror("unrecognized input type");
	    response->option = error_option;
	    return;
	    break;
	}

out:
	if (response)
	    response->option = request->option;
	return;

}

static void X_ScreentoData(graph, x, y, fx, fy)
GRAPH *graph;
int x,y;
double *fx, *fy;
{
	double	lmin, lmax;

	if (graph->grid.gridtype == GRID_XLOG
		|| graph->grid.gridtype == GRID_LOGLOG)
	{
		lmin = log10(graph->datawindow.xmin);
		lmax = log10(graph->datawindow.xmax);
		*fx = exp(((x - graph->viewportxoff)
			* (lmax - lmin) / graph->viewport.width + lmin)
			* M_LN10);
	} else {
		*fx = (x - graph->viewportxoff) * graph->aspectratiox +
			graph->datawindow.xmin;
	}

	if (graph->grid.gridtype == GRID_YLOG
		|| graph->grid.gridtype == GRID_LOGLOG)
	{
		lmin = log10(graph->datawindow.ymin);
		lmax = log10(graph->datawindow.ymax);
		*fy = exp(((graph->absolute.height - y - graph->viewportxoff)
			* (lmax - lmin) / graph->viewport.height + lmin)
			* M_LN10);
	} else {
		*fy = ((graph->absolute.height - y) - graph->viewportyoff)
			* graph->aspectratioy + graph->datawindow.ymin;
	}

}

/*ARGSUSED*/
static void
linear_arc(x0,y0,radius,theta1,theta2)
    int x0;   /* x coordinate of center */
    int y0;   /* y coordinate of center */
    int radius;       /* radius of arc */
    double theta1;    /* initial angle ( +x axis = 0 rad ) */
    double theta2;    /* final angle ( +x axis = 0 rad ) */
    /*
     * Notes:
     *    Draws an arc of radius and center at (x0,y0) beginning at
     *    angle theta1 (in rad) and ending at theta2
     */
{
    int x1, y1, x2, y2;
    int s = 60;
    double dphi, phi;

    x2 = x0 + (int) (radius * cos(theta1));
    y2 = y0 + (int) (radius * sin(theta1));

    while(theta1 >= theta2)
	    theta2 += 2 * M_PI;
    dphi = (theta2 - theta1) / s;

    if ((theta1 + dphi) == theta1) {
	    theta2 += 2 * M_PI;
	    dphi = (theta2 - theta1) / s;
    }


    for(phi = theta1 + dphi; phi < theta2; phi += dphi) {
	    x1 = x2;
	    y1 = y2;
	    x2 = x0 + (int)(radius * cos(phi));
	    y2 = y0 + (int)(radius * sin(phi));
	    X11_DrawLine(x1,y1,x2,y2);
    }

    x1 = x2;
    y1 = y2;
    x2 = x0 + (int)(radius * cos(theta2));
    y2 = y0 + (int)(radius * sin(theta2));
    X11_DrawLine(x1,y1,x2,y2);
}

#else /* not HAS_X11 */
int x11_dummy_symbol;
/* otherwise, some linkers get upset */
#endif /* HAS_X11 */
