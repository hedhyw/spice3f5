/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1988 Jeffrey M. Hsu
**********/

/*
    MFB driver
*/

#include "spice.h"

#ifdef HAS_MFB

/* note:  I would have liked to place this after CPdefs,
    but CPdefs #defines false and true, which are enums in mfb.h. */
#include "mfb.h"

#include "ftedev.h"
#include "cpdefs.h"                     /* for VT_ */
#include "ftegraph.h"           /* for rnd */
#include "suffix.h"

extern char ErrorMessage[];
extern char *MFBHardcopy;	/* output filename for MFB hardcopy */

static int mfbgraphid;
static int mfbscreenflag;
static void mfbsetlinestyles();

static	char old_term[BSIZE_SP] = "";
static	char old_dev[BSIZE_SP] = "";
static  char success = false;

Mfb_Init()
{

    char buf[BSIZE_SP], dev[BSIZE_SP], *term;
    char *devp = dev;
    int err;

    /* Try to identify the terminal type -- check variable,
     * environment, then ask the user.
     */
#ifdef HAS_PCTERM
    term = "pc";
#else
    if (cp_getvar("term", VT_STRING, buf)) {
      term = buf;
    } else {
      term = getenv("TERM");
      if (term == NULL) { 
        /* prompt user w/ input() */
        fprintf(cp_out, "Terminal type = ");
        (void) fflush(cp_out);
        (void) fscanf(cp_in, " %s", buf);
        term = copy(buf);
      }
    }
#endif
    if ((devp = getenv("DEVICE")) == NULL) {
        if (!cp_getvar("device", VT_STRING, dev))
          dev[0] = '\0';
    } else {
	strcpy(dev, devp);
    }
    /* hack for MFB hardcopy */
    if (MFBHardcopy) {
      strcpy(dev, MFBHardcopy);
      if (!strncmp(MFBHardcopy, "/tmp", 4))     /* click to hardcopy */
        mfbscreenflag = 1;
      else
        mfbscreenflag = 0;                      /* hardcopy command */
      MFBHardcopy = (char *) NULL;
    } else {
        mfbscreenflag = 2;                      /* MFB plot */
    }

/* We only re-open if things have changed */
    if( (!success) 
	|| (strcmp(old_term, term) != 0) || (strcmp(old_dev,dev) != 0)) {
    	(void) MFBOpen( term, (dev[0] ? dev : (char *) NULL), &err);
        strcpy( old_term, term); strcpy( old_dev, dev );
        if (err == MFBOK) {
	  success = true;
        } else {
          success = false;
          sprintf(ErrorMessage, "MFB error: %s", MFBError(err));
          externalerror(ErrorMessage);
          return (1);
        }
    }

    dispdev->numlinestyles = MFBInfo(MAXLINESTYLES);
    dispdev->numcolors = MFBInfo(MAXCOLORS);
    dispdev->width = MFBInfo(MAXX) + 1;
    dispdev->height = MFBInfo(MAXY) + 1;

#ifdef DEFCOLORS
    if (dispdev->numcolors > 2 && MFBInfo(DEFCOLORS))
      mfbsetcolors();
#endif
    if (dispdev->numlinestyles > 2 && MFBInfo(DEFLINEPATTERN))
      mfbsetlinestyles();

/*  MFBSetALUMode(MFBALUJAM);   /* graphics overstrikes */
    MFBSetTextMode(1);      /* text overstrikes */

    /* get control back */
    if (MFBHalt() != MFBOK) {
      externalerror("Can't restore control of tty.");
      return(1);
    }

    return (0);

}

Mfb_NewViewport(graph)
GRAPH *graph;
{
    mfbgraphid = graph->graphid;

    if (!MFBInitialize() == MFBOK) {
      externalerror("Can't initialize MFB.");
      return(1);
    }

    if (graph->absolute.width) {
	/* hardcopying from the screen */
	mfbscreenflag = 1;
    }

/* note: for now, do nothing
    later, divide screen into n+1, and redraw existing n plots */

    graph->fontwidth = MFBInfo(FONTWIDTH);
    graph->fontheight = MFBInfo(FONTHEIGHT);

    graph->viewport.height = dispdev->height;
    graph->viewport.width = dispdev->width;

    graph->absolute.xpos = 0;
    graph->absolute.ypos = 0;
    graph->absolute.width = dispdev->width;
    graph->absolute.height = dispdev->height;
    return(0);

}

Mfb_Close()
{

    /* In case of hardcopy command destroy the hardcopy graph
     * and reset currentgraph to graphid 1, if possible
     */
    MFBClose();

    if (!mfbscreenflag) {
	/* DestroyGraph(mfbgraphid); */
	SetGraphContext(1);
    }
    if (mfbscreenflag != 2)
	Mfb_Init();
    mfbscreenflag = 2;

}

Mfb_Clear()
{

    MFBSetColor(0);
    MFBFlood();

}

Mfb_DrawLine(x1, y1, x2, y2)
int x1, y1, x2, y2;
{

    MFBLine(x1, y1, x2, y2);

}

Mfb_Arc(x0, y0, radius, theta1, theta2)
int x0, y0, radius;
double theta1, theta2;
{
	double theta3, theta4;

/* note: figure some better value for nsides as a fn of radius
        and delta theta */
    if( (theta1 != theta2)
		 &&  sin(fabs( theta1 - theta2 )/2.0) < 0.5  ) {
		theta3 = ceil((double) 180.0/M_PI * theta1);
                theta4 = floor((double) 180.0/M_PI * theta2);

        if( theta3 < theta4  && theta3 < 360.0 ) {
    	   MFBArc(x0, y0, radius, (int)theta3, (int)theta4, 50);

	   theta3 *= M_PI/180.0;
	   theta4 *= M_PI/180.0;
	} else {
	   theta3 = theta2;
	   theta4 = theta1;
        }

    	MFBLine((int)(x0+radius*cos(theta1)),(int)(y0+radius*sin(theta1)), 
    	   (int)(x0+radius*cos(theta3)),(int)(y0+radius*sin(theta3)));
    	MFBLine((int)(x0+radius*cos(theta2)),(int)(y0+radius*sin(theta2)), 
    	   (int)(x0+radius*cos(theta4)),(int)(y0+radius*sin(theta4)));
    } else {
    	MFBArc(x0, y0, radius, 
		rnd((double) 180.0/M_PI * theta1),
                rnd((double) 180.0/M_PI * theta2), 50);
    }


}

rrnd( x )
double	x;
{
	if( x - floor( x ) >= 0.5 )
		return( (int)ceil( x ) );
	else
		return( (int)floor( x ) );
}


Mfb_Text(text, x, y)
char *text;
int x, y;
{

    MFBText(text, x, y, 0);

}

Mfb_DefineColor(colorid, red, green, blue)
int colorid;
double red, green, blue;
{

/* note: */
    MFBDefineColor(colorid,
            rnd(red * 1000), rnd(green * 1000), rnd(blue * 1000));

}

Mfb_DefineLinestyle()
{

/* note: */

}

Mfb_SetLinestyle(linestyle)
int linestyle;
{

/* note: do error checking */
    if (MFBSetLineStyle(linestyle) != MFBOK) {
      externalerror("Bad linestyle");
    }

}

Mfb_SetColor(colorid)
int colorid;
{

    if (MFBSetColor(colorid) != MFBOK) {
      externalerror("bad color");
    }

}

Mfb_Update()
{

    MFBUpdate();

}

Mfb_MakeMenu()
{

/* note: */

}

Mfb_MakeDialog()
{

/* note: */

}

/* Set colors properly. Assume that we won't need more than 20 colors. 
 * Also we should try to set the foreground and background colors.
 * These color values are just off of the top of my head...
 * Make the grid color (color 1) blue.
 */

#ifdef DEFCOLORS

static int reds[] = { 999,   0, 999,   0,   0, 999, 999,   0, 999, 500,
              500, 999, 999, 500, 999, 999, 600, 300, 600, 300 } ;
static int grns[] = { 999,   0,   0, 999,   0, 999,   0, 999, 500, 999,
              500, 999, 500, 999, 600, 300, 999, 999, 300, 600 } ;
static int blus[] = { 999,   0,   0,   0, 999,   0, 999, 999, 500, 500,
              999, 500, 999, 999, 300, 600, 300, 600, 999, 999 } ;

static
mfbsetcolors()
{
    int i;
    int red, green, blue;

    MFBDefineColor(1, 0, 0, 999);
    for (i = 2; (i < dispdev->numcolors) && (i < 22); i++) {
        red = reds[i - 2];
        green = grns[i - 2];
        blue = blus[i - 2];
        MFBDefineColor(i, red, green, blue);
    }
    return;
}

#endif

/* Set line styles if necessary. Assume we won't use more than 20 line
 * styles. These line styles also are pretty random. Make sure that
 * line style 0 is solid and line style 1 is dotted.
 */

static int styles[] = { 0333, 0347, 0360, 0071, 0216, 0366, 0155, 0011,
            0123, 0317, 0330, 0145, 0275, 0016, 0315, 0101,
            0222, 0174, 0251, 0376 } ;

static void
mfbsetlinestyles()
{
    int i, j;

    MFBDefineLineStyle(0, 0377);    /* Solid. */
    MFBDefineLineStyle(1, 0125);    /* Dotted. */
    for (i = 2; (i < dispdev->numlinestyles) && (i < 22); i++) {
        j = styles[i - 2];
        MFBDefineLineStyle(i, j);
    }
    return;
}

#endif

