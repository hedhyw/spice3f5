/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: -C- 1982 Giles Billingsley
**********/
/*
 * mfbclip.c
 *
 * sccsid "@(#)mfbclip.c    1.9  9/3/83"
 *
 *     MFB is a graphics package that was developed by the integrated
 * circuits group of the Electronics Research Laboratory and the
 * Department of Electrical Engineering and Computer Sciences at
 * the University of California, Berkeley, California.  The programs
 * in MFB are available free of charge to any interested party.
 * The sale, resale, or use of these program for profit without the
 * express written consent of the Department of Electrical Engineering
 * and Computer Sciences, University of California, Berkeley, California,
 * is forbidden.
 */


#include "spice.h"
#include "mfb.h"
#include "suffix.h"


/************************************************************************
 *
 *                              MFBClip
 *
 *     MFBClip contains routines for special viewport management
 *     and geometry clipping.
 *
 *     MFBPolygonClip
 *     MFBLineClip
 *     MFBClipArc
 *     MFB_X_Intercept
 *     MFB_Y_Intercept
 *
 ************************************************************************/

#define CODELEFT 1
#define CODEBOTTOM 2
#define CODERIGHT 4
#define CODETOP 8
#define CODE(x,y,c)  c = 0;\
                     if(x < l)\
                         c = CODELEFT;\
                     else if(x > r)\
                         c = CODERIGHT;\
                     if(y < b)\
                         c |= CODEBOTTOM;\
                     else if(y > t)\
                         c |= CODETOP;

void
MFB_Y_Intercept(x1,y1,x2,y2,e,yi)
    int x1,y1,x2,y2;                /* two points on line */
    int e;                          /* vertical line of intercept */
    int *yi;                        /* y coordinate of intercept */
    {
    /*
     * MFB_Y_Intercept will return the value 'yi' where the the coordinate
     * (e,yi) is the intersection of the vertical line x = e and the line
     * determined by the coordinates (x1,y1) and (x2,y2).
     */
    *yi = y1;
    if(x1 == x2) return;            /* vertical line */
    *yi = y1 + ((e - x1) * (y2 - y1))/(x2 - x1);
    }


void
MFB_X_Intercept(x1,y1,x2,y2,e,xi)
    int x1,y1,x2,y2;                /* two points on line */
    int e;                          /* horizontal line of intercept */
    int *xi;                        /* x coordinate of intercept */
    {
    /*
     * MFB_X_Intercept will return the value 'xi' where the the coordinate
     * (xi,e) is the intersection of the horizontal line y = e and the line
     * determined by the coordinates (x1,y1) and (x2,y2).
     */
    *xi = x1;
    if(y1 == y2) return;            /* horizontal line */
    *xi = x1 + ((e - y1) * (x2 - x1))/(y2 - y1);
    }


Bool
MFBLineClip(pX1,pY1,pX2,pY2,l,b,r,t)
    int *pX1,*pY1,*pX2,*pY2,l,b,r,t;
    {
    /*
     * MFBLineClip will clip a line to a rectangular area.  The returned
     * value is 'true' if the line is out of the AOI (therefore does not
     * need to be displayed) and 'false' if the line is in the AOI.
     */
    int x1 = *pX1;
    int y1 = *pY1;
    int x2 = *pX2;
    int y2 = *pY2;
    int x,y,c,c1,c2;

    CODE(x1,y1,c1)
    CODE(x2,y2,c2)
    while(c1 != 0 || c2 != 0) {
        if((c1 & c2) != 0) return(true); /*Assert:  Line is invisible.*/
        if((c = c1) == 0) c = c2;
        if(c & CODELEFT) {
            y = y1+(y2-y1)*(l-x1)/(x2-x1);
            x = l;
        }
        else if(c & CODERIGHT) {
            y = y1+(y2-y1)*(r-x1)/(x2-x1);
            x = r;
        }
        else if(c & CODEBOTTOM) {
            x = x1+(x2-x1)*(b-y1)/(y2-y1);
            y = b;
        }
        else if(c & CODETOP) {
            x = x1+(x2-x1)*(t-y1)/(y2-y1);
            y = t;
        }
        if(c == c1) {
            x1 = x; y1 = y; CODE(x,y,c1)
        }
        else {
            x2 = x; y2 = y; CODE(x,y,c2)
        }
    }
    *pX1 = x1; *pY1 = y1;
    *pX2 = x2; *pY2 = y2;
    return(false); /*Assert:  Line is at least partially visible.*/
    }


void
MFBPolygonClip(poly,left,bottom,right,top)
    MFBPOLYGON *poly;
    int top,bottom,left,right;      /* bounding box */
    {
    /*
     * MFBPolygonClip will take the polygon 'poly' and clip it to the
     * box described by 'top','bottom','left', and 'right'.
     */
    MFBPOLYGON p1,p2;
    static int polybuffer1[MFBPOLYGONBUFSIZE];
    static int polybuffer2[MFBPOLYGONBUFSIZE];
    int i,k,n,*swap;
    int minx, miny, maxx, maxy;

    if(top < bottom)
        MFBSwapInt(top,bottom);
    if(right < left)
        MFBSwapInt(left,right);

    n = poly->nvertices << 1;     /* fast multiplication by 2 */
    if(n < 4) return;
    n = MFBmin(MFBPOLYGONBUFSIZE,n);
    p1.xy = polybuffer1;
    p2.xy = polybuffer2;
    minx = maxx = poly->xy[0];
    miny = maxy = poly->xy[1];
    for(i=0; i<n; i++){
        p2.xy[i] = poly->xy[i];
    maxx = MFBmax(p2.xy[i],maxx);
    minx = MFBmin(p2.xy[i],minx);
    ++i;
        p2.xy[i] = poly->xy[i];
    maxy = MFBmax(p2.xy[i],maxy);
    miny = MFBmin(p2.xy[i],miny);
    }

    /* test if clipping is necessary */
    poly->nvertices = 0;
    if(maxx < left || minx > right || maxy < bottom || miny > top)
    return;

    /*
     * start with the right side first
     */
    k = i = 0;
    while(p2.xy[i] > right && i < n) i += 2;
    if(i == 0){
        p1.xy[k++] = p2.xy[i++];
        p1.xy[k++] = p2.xy[i++];
        }
    else if(i == n)
        return;
    else {
        p1.xy[k++] = right;
        MFB_Y_Intercept(p2.xy[i-2],p2.xy[i-1],p2.xy[i],p2.xy[i+1],
        right,&p1.xy[k++]);
        }
    while(i < n){
        if(p2.xy[i] <= right){
            p1.xy[k++] = p2.xy[i++];
            p1.xy[k++] = p2.xy[i++];
            }
        else {
            if(p2.xy[i-2] != right){
                p1.xy[k++] = right;
                MFB_Y_Intercept(p2.xy[i-2],p2.xy[i-1],p2.xy[i],p2.xy[i+1],
            right,&p1.xy[k++]);
                }
            while(i < n && p2.xy[i] > right) i += 2;
            if(i < n && p2.xy[i] <= right){
                p1.xy[k++] = right;
                MFB_Y_Intercept(p2.xy[i-2],p2.xy[i-1],p2.xy[i],p2.xy[i+1],
            right,&p1.xy[k++]);
                }
            }
        }
    if((p2.xy[0] > right && p2.xy[n-2] <= right) ||
        (p2.xy[0] <= right && p2.xy[n-2] > right)){
        p1.xy[k++] = right;
        MFB_Y_Intercept(p2.xy[0],p2.xy[1],p2.xy[n-2],p2.xy[n-1],right,
        &p1.xy[k++]);
        }
    if(k < 4) return;
    n = MFBmin(MFBPOLYGONBUFSIZE,k);
    swap = p1.xy;
    p1.xy = p2.xy;
    p2.xy = swap;

    /*
     * next the left side
     */
    k = i = 0;
    while(p2.xy[i] < left && i < n) i += 2;
    if(i == 0){
        p1.xy[k++] = p2.xy[i++];
        p1.xy[k++] = p2.xy[i++];
        }
    else if(i == n)
        return;
    else {
        p1.xy[k++] = left;
        MFB_Y_Intercept(p2.xy[i-2],p2.xy[i-1],p2.xy[i],p2.xy[i+1],left,
        &p1.xy[k++]);
        }
    while(i < n){
        if(p2.xy[i] >= left){
            p1.xy[k++] = p2.xy[i++];
            p1.xy[k++] = p2.xy[i++];
            }
        else {
            if(p2.xy[i-2] != left){
                p1.xy[k++] = left;
                MFB_Y_Intercept(p2.xy[i-2],p2.xy[i-1],p2.xy[i],p2.xy[i+1],
            left,&p1.xy[k++]);
                }
            while(i < n && p2.xy[i] < left) i += 2;
            if(i < n && p2.xy[i] >= left){
                p1.xy[k++] = left;
                MFB_Y_Intercept(p2.xy[i-2],p2.xy[i-1],p2.xy[i],p2.xy[i+1],
            left,&p1.xy[k++]);
                }
            }
        }
    if((p2.xy[0] < left && p2.xy[n-2] >= left) ||
        (p2.xy[0] >= left && p2.xy[n-2] < left)){
        p1.xy[k++] = left;
        MFB_Y_Intercept(p2.xy[0],p2.xy[1],p2.xy[n-2],p2.xy[n-1],left,
        &p1.xy[k++]);
        }
    if(k < 4) return;
    n = MFBmin(MFBPOLYGONBUFSIZE,k);
    swap = p1.xy;
    p1.xy = p2.xy;
    p2.xy = swap;

    /*
     * next the top side
     */
    k = i = 0;
    while(p2.xy[i+1] > top && i < n) i += 2;
    if(i == 0){
        p1.xy[k++] = p2.xy[i++];
        p1.xy[k++] = p2.xy[i++];
        }
    else if(i == n)
        return;
    else {
        MFB_X_Intercept(p2.xy[i-2],p2.xy[i-1],p2.xy[i],p2.xy[i+1],top,
        &p1.xy[k++]);
        p1.xy[k++] = top;
        }
    while(i < n){
        if(p2.xy[i+1] <= top){
            p1.xy[k++] = p2.xy[i++];
            p1.xy[k++] = p2.xy[i++];
            }
        else {
            if(p2.xy[i-1] != top){
                MFB_X_Intercept(p2.xy[i-2],p2.xy[i-1],p2.xy[i],p2.xy[i+1],
            top,&p1.xy[k++]);
                p1.xy[k++] = top;
                }
            while(i < n && p2.xy[i+1] > top) i += 2;
            if(i < n && p2.xy[i+1] <= top){
                MFB_X_Intercept(p2.xy[i-2],p2.xy[i-1],p2.xy[i],p2.xy[i+1],
            top,&p1.xy[k++]);
                p1.xy[k++] = top;
                }
            }
        }
    if((p2.xy[1] > top && p2.xy[n-1] <= top) ||
        (p2.xy[1] <= top && p2.xy[n-1] > top)){
        MFB_X_Intercept(p2.xy[0],p2.xy[1],p2.xy[n-2],p2.xy[n-1],top,
        &p1.xy[k++]);
        p1.xy[k++] = top;
        }
    if(k < 4) return;
    n = MFBmin(MFBPOLYGONBUFSIZE,k);
    swap = p1.xy;
    p1.xy = p2.xy;
    p2.xy = swap;

    /*
     * finally the bottom side
     */
    i = k = 0;
    while(p2.xy[i+1] < bottom && i < n) i += 2;
    if(i == 0){
        p1.xy[k++] = p2.xy[i++];
        p1.xy[k++] = p2.xy[i++];
        }
    else if(i == n)
        return;
    else {
        MFB_X_Intercept(p2.xy[i-2],p2.xy[i-1],p2.xy[i],p2.xy[i+1],bottom,
        &p1.xy[k++]);
        p1.xy[k++] = bottom;
        }
    while(i < n){
        if(p2.xy[i+1] >= bottom){
            p1.xy[k++] = p2.xy[i++];
            p1.xy[k++] = p2.xy[i++];
            }
        else {
            if(p2.xy[i-1] != bottom){
                MFB_X_Intercept(p2.xy[i-2],p2.xy[i-1],p2.xy[i],p2.xy[i+1],
            bottom,&p1.xy[k++]);
                p1.xy[k++] = bottom;
                }
            while(i < n && p2.xy[i+1] < bottom) i += 2;
            if(i < n && p2.xy[i+1] >= bottom){
                MFB_X_Intercept(p2.xy[i-2],p2.xy[i-1],p2.xy[i],p2.xy[i+1],
            bottom,&p1.xy[k++]);
                p1.xy[k++] = bottom;
                }
            }
        }
    if((p2.xy[1] < bottom && p2.xy[n-1] >= bottom) ||
        (p2.xy[1] >= bottom && p2.xy[n-1] < bottom)){
        MFB_X_Intercept(p2.xy[0],p2.xy[1],p2.xy[n-2],p2.xy[n-1],bottom,
        &p1.xy[k++]);
        p1.xy[k++] = bottom;
        }
    if(k < 4) return;
    k = MFBmin(MFBPOLYGONBUFSIZE,k);
    poly->nvertices = k >> 1;

    /*
     * Transfer coordinates to poly buffer and one last clipping
     */
    for(i=0; i<k; i += 2){
        poly->xy[i] = MFBmax(MFBmin(p1.xy[i],right),left);
        poly->xy[i+1] = MFBmax(MFBmin(p1.xy[i+1],top),bottom);
        }
    }



MFBPATH
*MFBClipArc(path,left,bottom,right,top)
    MFBPATH *path;
    int left,bottom,right,top;
    /*
     *  Notes:
     */
    {
    int n,x1,y1,x2,y2,lastX,lastY,newX,newY,pathnum=0;
    static MFBPATH pbuf[5];
    static int xy[5 * MFBPOLYGONBUFSIZE];

    /* allocate buffer */
    for(n=0; n<5; ++n){
    pbuf[n].xy = &xy[n * MFBPOLYGONBUFSIZE];
    pbuf[n].nvertices = 0;
    }

    if(right < left)
    MFBSwapInt(right,left);
    if(top < bottom)
    MFBSwapInt(top,bottom);

    n = 0;
    while(n < path->nvertices){
        newX = path->xy[n + n];
        newY = path->xy[n + n + 1];
        n++;
    /* find first point (newX,newY) which is in AOI */
    if(left <= newX && right >= newX && bottom <= newY && top >= newY)
        break;
    lastX = newX;
    lastY = newY;
    }

    if(n > 1){
    x1 = lastX;
    y1 = lastY;
    x2 = newX;
    y2 = newY;
    if(MFBLineClip(&x1,&y1,&x2,&y2,left,bottom,right,top))
        /* not even close to being in AOI */
        return(pbuf);
    pbuf[pathnum].xy[0] = x1;
    pbuf[pathnum].xy[1] = y1;
    pbuf[pathnum].xy[2] = x2;
    pbuf[pathnum].xy[3] = y2;
    pbuf[pathnum].nvertices = 4;
    }
    else{
    pbuf[pathnum].xy[0] = newX;
    pbuf[pathnum].xy[1] = newY;
    pbuf[pathnum].nvertices = 2;
    }
    while(n < path->nvertices && pathnum < 5){
    x1 = lastX = newX; y1 = lastY = newY;
    x2 = newX = path->xy[n + n]; y2 = newY = path->xy[n + n + 1];
    n++;
    MFBLineClip(&x1,&y1,&x2,&y2,left,bottom,right,top);
    /* are we in the AOI? */
    if(left < lastX && right > lastX && bottom < lastY && top > lastY){
        pbuf[pathnum].xy[pbuf[pathnum].nvertices++] = x2;
        pbuf[pathnum].xy[pbuf[pathnum].nvertices++] = y2;
        }
    /* or are we entering the AOI? */
    else if(left < newX && right > newX && bottom < newY && top > newY){
        if(++pathnum < 5){
            pbuf[pathnum].xy[0] = x1; pbuf[pathnum].xy[1] = y1;
            pbuf[pathnum].xy[2] = x2; pbuf[pathnum].xy[3] = y2;
            pbuf[pathnum].nvertices=4;
        }
        }
    }
    for(n=0; n<5; ++n)
    pbuf[n].nvertices /= 2;
    return(pbuf);
    }

