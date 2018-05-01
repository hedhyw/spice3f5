/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1986 Wayne A. Christopyher, U. C. Berkeley CAD Group
Author: 1982 Giles Billingsley
**********/

/*
 * Some routines to do clipping of polygons, etc to boxes.  Most of this code
 * was rescued from MFB:
 *  sccsid "@(#)mfbclip.c   1.2  12/21/83"
 */

#include "spice.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "suffix.h"

#define POLYGONBUFSIZE 512
/* XXX */

#define CODEMINX 1
#define CODEMINY 2
#define CODEMAXX 4
#define CODEMAXY 8
#define CODE(x,y,c)  c = 0;\
                     if (x < l)\
                         c = CODEMINX;\
                     else if (x > r)\
                         c = CODEMAXX;\
                     if (y < b)\
                         c |= CODEMINY;\
                     else if (y > t)\
                         c |= CODEMAXY;

/* XXX */
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define SWAPINT(a, b) { int xxxx = (a); (a) = (b); (b) = xxxx; }

/* clip_line will clip a line to a rectangular area.  The returned
 * value is 'true' if the line is out of the AOI (therefore does not
 * need to be displayed) and 'false' if the line is in the AOI.
 */

bool
clip_line(pX1,pY1,pX2,pY2,l,b,r,t)
    int *pX1, *pY1, *pX2, *pY2, l, b, r, t;
{
    int x1 = *pX1;
    int y1 = *pY1;
    int x2 = *pX2;
    int y2 = *pY2;
    int x,y,c,c1,c2;

    CODE(x1,y1,c1)
    CODE(x2,y2,c2)
    while (c1 || c2) {
        if (c1 & c2)
            return (true); /* Line is invisible. */
        if (!(c = c1))
            c = c2;
        if (c & CODEMINX) {
            y = y1+(y2-y1)*(l-x1)/(x2-x1);
            x = l;
        } else if (c & CODEMAXX) {
            y = y1+(y2-y1)*(r-x1)/(x2-x1);
            x = r;
        } else if (c & CODEMINY) {
            x = x1+(x2-x1)*(b-y1)/(y2-y1);
            y = b;
        } else if (c & CODEMAXY) {
            x = x1+(x2-x1)*(t-y1)/(y2-y1);
            y = t;
        }
        if (c == c1) {
            x1 = x;
            y1 = y;
            CODE(x,y,c1)
        } else {
            x2 = x;
            y2 = y; 
            CODE(x,y,c2)
        }
    }
    *pX1 = x1;
    *pY1 = y1;
    *pX2 = x2;
    *pY2 = y2;
    return (false); /* Line is at least partially visible.*/
}

/* This routine will clip a line to a circle, returning true if the line
 * is entirely outside the circle.  Note that we have to be careful not
 * to switch the points around, since in grid.c we need to know which is
 * the outer point for putting the label on.
 */

bool
clip_to_circle(x1, y1, x2, y2, cx, cy, rad)
    int *x1, *y1, *x2, *y2,  cx, cy, rad;
{
    double perplen, a, b, c;
    double tx, ty, dt;
    double dtheta;
    double theta1, theta2, tt, alpha, beta, gamma;
    bool flip = false;
    int i;

    /* Get the angles between the origin and the endpoints. */
    if ((*x1-cx) || (*y1-cy))
	theta1 = atan2((double) *y1 - cy, (double) *x1 - cx);
    else
	theta1 = M_PI;
    if ((*x2-cx) || (*y2-cy))
	theta2 = atan2((double) *y2 - cy, (double) *x2 - cx);
    else
	theta2 = M_PI;

    if (theta1 < 0.0)
        theta1 = 2 * M_PI + theta1;
    if (theta2 < 0.0)
        theta2 = 2 * M_PI + theta2;

    dtheta = theta2 - theta1;
    if (dtheta > M_PI)
        dtheta = dtheta - 2 * M_PI;
    else if (dtheta < - M_PI)
        dtheta = 2 * M_PI - dtheta;

    /* Make sure that p1 is the first point */
    if (dtheta < 0) {
        tt = theta1;
        theta1 = theta2;
        theta2 = tt;
        i = *x1;
        *x1 = *x2;
        *x2 = i;
        i = *y1;
        *y1 = *y2;
        *y2 = i;
        flip = true;
        dtheta = -dtheta;
    }

    /* Figure out the distances between the points */
    a = sqrt((double) ((*x1 - cx) * (*x1 - cx) + (*y1 - cy) * (*y1 - cy)));
    b = sqrt((double) ((*x2 - cx) * (*x2 - cx) + (*y2 - cy) * (*y2 - cy)));
    c = sqrt((double) ((*x1 - *x2) * (*x1 - *x2) +
            (*y1 - *y2) * (*y1 - *y2)));

    /* We have three cases now -- either the midpoint of the line is
     * closest to the origon, or point 1 or point 2 is.  Actually the
     * midpoint won't in general be the closest, but if a point besides
     * one of the endpoints is closest, the midpoint will be closer than
     * both endpoints.
     */
    tx = (*x1 + *x2) / 2;
    ty = (*y1 + *y2) / 2;
    dt = sqrt((double) ((tx - cx) * (tx - cx) + (ty - cy) * (ty - cy)));
    if ((dt < a) && (dt < b)) {
        /* This is wierd -- round-off errors I guess. */
        tt = (a * a + c * c - b * b) / (2 * a * c);
        if (tt > 1.0)
            tt = 1.0;
        else if (tt < -1.0)
            tt = -1.0;
        alpha = acos(tt);
        perplen = a * sin(alpha);
    } else if (a < b) {
        perplen = a;
    } else {
        perplen = b;
    }

    /* Now we should see if the line is outside of the circle */
    if (perplen >= rad)
        return (true);

    /* It's at least partially inside */
    if (a > rad) {
        tt = (a * a + c * c - b * b) / (2 * a * c);
        if (tt > 1.0)
            tt = 1.0;
        else if (tt < -1.0)
            tt = -1.0;
        alpha = acos(tt);
        gamma = asin(sin(alpha) * a / rad);
        if (gamma < M_PI / 2)
            gamma = M_PI - gamma;
        beta = M_PI - alpha - gamma;
        *x1 = cx + rad * cos(theta1 + beta);
        *y1 = cy + rad * sin(theta1 + beta);
    }
    if (b > rad) {
        tt = (c * c + b * b - a * a) / (2 * b * c);
        if (tt > 1.0)
            tt = 1.0;
        else if (tt < -1.0)
            tt = -1.0;
        alpha = acos(tt);
        gamma = asin(sin(alpha) * b / rad);
        if (gamma < M_PI / 2)
            gamma = M_PI - gamma;
        beta = M_PI - alpha - gamma;
        *x2 = cx + rad * cos(theta2 - beta);
        *y2 = cy + rad * sin(theta2 - beta);
    }
    if (flip) {
        i = *x1;
        *x1 = *x2;
        *x2 = i;
        i = *y1;
        *y1 = *y2;
        *y2 = i;
    }
    return (false);
}
