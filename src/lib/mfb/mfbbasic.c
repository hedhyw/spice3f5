/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: -C- 1982 Giles C. Billingsley
**********/
/*
 * mfbbasic.c
 *
 * sccsid "@(#)mfbbasic.c   1.9  9/3/83"
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
#include <stdio.h>
#include "suffix.h"

#define RadToDeg    57.29577951
#define MFBFORMAT   MFBCurrent->strings

/* Here we take care of externs */
MFB *MFBCurrent;


/*****************************************************************************
 *
 *          BASIC MACROS AND SERVICE ROUTINES
 *
 *****************************************************************************/

int
MFBInfo(Info)
    int Info;
    {
    /*
     * Notes:
     *    MFBInfo is the only routine for acquiring device
     *    specific information.  The valid arguments are to
     *    MFBInfo are defined in mfb.h.  If an invalid argument
     *    is used, MFBInfo returns -1.
     */
    switch(Info){
    case  MAXX:
        return(MFBCurrent->maxX);
    case  MAXY:
        return(MFBCurrent->maxY);
    case  MAXCOLORS:
        return(MFBCurrent->maxColors);
    case  MAXINTENSITY:
        return(MFBCurrent->maxIntensity);
    case  MAXFILLPATTERNS:
        return(MFBCurrent->maxFillPatterns);
    case  MAXLINESTYLES:
        return(MFBCurrent->maxLineStyles);
    case  MAXBLINKERS:
        return(MFBCurrent->maxBlinkers);
    case  POINTINGDEVICE:
        return((int)MFBCurrent->PointingDeviceBool);
    case  POINTINGBUTTONS:
        return((int)MFBCurrent->buttonsBool);
    case  NUMBUTTONS:
        return(MFBCurrent->numberOfButtons);
    case  BUTTON1:
        return(MFBCurrent->buttonMask[0]);
    case  BUTTON2:
        return(MFBCurrent->buttonMask[1]);
    case  BUTTON3:
        return(MFBCurrent->buttonMask[2]);
    case  BUTTON4:
        return(MFBCurrent->buttonMask[3]);
    case  BUTTON5:
        return(MFBCurrent->buttonMask[4]);
    case  BUTTON6:
        return(MFBCurrent->buttonMask[5]);
    case  BUTTON7:
        return(MFBCurrent->buttonMask[6]);
    case  BUTTON8:
        return(MFBCurrent->buttonMask[7]);
    case  BUTTON9:
        return(MFBCurrent->buttonMask[8]);
    case  BUTTON10:
        return(MFBCurrent->buttonMask[9]);
    case  BUTTON11:
        return(MFBCurrent->buttonMask[10]);
    case  BUTTON12:
        return(MFBCurrent->buttonMask[11]);
    case  TEXTPOSITIONALBE:
        return((int)MFBCurrent->textPositionableBool);
    case  TEXTROTATABLE:
        return((int)MFBCurrent->textRotateBool);
    case  FONTHEIGHT:
        return(MFBCurrent->fontHeight);
    case  FONTWIDTH:
        return(MFBCurrent->fontWidth);
    case  FONTXOFFSET:
        return(MFBCurrent->fontXOffset);
    case  FONTYOFFSET:
        return(MFBCurrent->fontYOffset);
    case  DESTRUCTIVETEXT:
        return((int)MFBCurrent->replaceTextBool);
    case  OVERSTRIKETEXT:
        return((int)MFBCurrent->overstrikeTextBool);
    case  VLT:
        return((int)MFBCurrent->vltBool);
    case  BLINKERS:
        return((int)MFBCurrent->blinkersBool);
    case  FILLEDPOLYGONS:
        return((int)MFBCurrent->filledPlygnBool);
    case  DEFFILLPATTERNS:
        return((int)MFBCurrent->fillPtrnDefineBool);
    case  DEFREADMASK:
        return((int)MFBCurrent->readMaskBool);
    case  DEFCHANNELMASK:
        return((int)MFBCurrent->channelMaskBool);
    case  DEFLINEPATTERN:
        return((int)MFBCurrent->linePatternDefineBool);
    case  CURFGCOLOR:
        return(MFBCurrent->fgColorId);
    case  CURFILLPATTERN:
        return(MFBCurrent->fillPattern);
    case  CURLINESTYLE:
        return(MFBCurrent->lineStyle);
    case  CURCHANNELMASK:
        return(MFBCurrent->channelMask);
    case  CURREADMASK:
        return(MFBCurrent->readMask);
    case  NUMBITPLANES:
        return(MFBCurrent->lengthOfVLT);
    case  RASTERCOPY:
        return((int)MFBCurrent->rastCopyBool);
    case  OFFSCREENX:
        return(MFBCurrent->minOffScreenX);
    case  OFFSCREENY:
        return(MFBCurrent->minOffScreenY);
    case  OFFSCREENDX:
        return(MFBCurrent->offScreenDX);
    case  OFFSCREENDY:
        return(MFBCurrent->offScreenDY);
    default:
        return(-1);
    }
    }


void
MFBConvertToHLS(r,g,b)
    int r,g,b;
    /*
     * Notes:
     *    The HLS values are returned in MFBCurrent->Y,
     *    MFBCurrent->Z, and MFBCurrent->T.
     */
    {
    double rrr,ggg,bbb,rr,gg,bb,sum,dif,mx,mn,lit,intensity;
    intensity = (double)MFBCurrent->maxIntensity;
    /* RGB is normalized to 1000 */
    rr = ((double) r)/1000.0;
    gg = ((double) g)/1000.0;
    bb = ((double) b)/1000.0;
    mx = rr;
    if(mx < gg) mx = gg;
    if(mx < bb) mx = bb;
    mn = rr;
    if(mn > gg) mn = gg;
    if(mn > bb) mn = bb;
    sum = mx + mn; dif = mx - mn;
    if(dif != 0.0){
        rrr = (mx - rr)/dif; ggg = (mx - gg)/dif; bbb = (mx - bb)/dif;
        }
    /* lightness calculation */
    lit = sum/2.0;
    /* saturation calculation */
    if(mx == mn) MFBCurrent->T = 0;
    else if(lit <= .5) MFBCurrent->T = (int)(intensity * dif/sum);
    else MFBCurrent->T = (int)((intensity * dif)/(2.0 - sum));
    /* hue calculation */
    if(MFBCurrent->T == 0) MFBCurrent->Y = 0;
    else if(rr == mx) MFBCurrent->Y = (int)(60.0 * (2.0 + bbb - ggg));
    else if(gg == mx) MFBCurrent->Y = (int)(60.0 * (4.0 + rrr - bbb));
    else MFBCurrent->Y = (int)(60.0 * (6.0 + ggg - rrr));
    MFBCurrent->Z = (int)(intensity * lit);
    }


void
SetCurrentMFB(mfb)
    MFB *mfb;
    /*
     * Notes:
     *    Set the current frame buffer.  Since each MFB struct contains
     *    all necessary I/O information, it is possible for a host
     *    program to manipulate several different graphics displays by
     *    simply maintaining a MFB struct for each device.
     */
    {
    MFBCurrent = mfb;
    }


void
MFBZeroCounters() {
#ifdef DEBUG
    if(MFBCurrent == NULL) return;
    MFBCurrent->nChars = MFBCurrent->nBoxes
    = MFBCurrent->nLines = MFBCurrent->nChars
    = MFBCurrent->sumBoxArea = MFBCurrent->sumLineLength = 0;
#endif
    }


void
MFBCounters(nChars,nBoxes,meanBoxArea,nLines,meanLineLength,nPixels)
    int *nChars,*nBoxes,*meanBoxArea,*nLines,*meanLineLength,*nPixels;
    {
#ifdef DEBUG
    /*
     * routine for measuring approximate pixel bandwidth
     */
    *nChars = MFBCurrent->nChars;
    *nBoxes = MFBCurrent->nBoxes;
    *nLines = MFBCurrent->nLines;
    *meanLineLength = 0;
    *meanBoxArea = 0;
    if(MFBCurrent->nBoxes != 0)
    *meanBoxArea = MFBCurrent->sumBoxArea/MFBCurrent->nBoxes;
    if(MFBCurrent->nLines != 0)
    *meanLineLength = MFBCurrent->sumLineLength/MFBCurrent->nLines;
    *nPixels = 
    MFBCurrent->nChars*MFBCurrent->fontHeight*MFBCurrent->fontWidth+
    MFBCurrent->sumBoxArea+
    MFBCurrent->sumLineLength;
#else
    *nChars = *nBoxes = *nLines = *meanLineLength = *meanBoxArea = 0;
#endif
    }





/*****************************************************************************
 *
 *      SET VIEWPORT ATTRIBUTES
 *
 *****************************************************************************/

MFBSetLineStyle(styleId)
    int styleId;
    /*
     * Notes:
     *    Sets current line style.
     *    Return immediately if line style is already current.
     *    Return MFBBADLST if styleId is out of range.
     */
    {
    /*
     * Check if styleId is already the current line style
     */
    if(styleId == MFBCurrent->lineStyle && !MFBCurrent->reissueLineStyleBool)
    return(MFBOK);
    if(styleId < 0 || styleId >= MFBCurrent->maxLineStyles)
    /* bad styleId */
    return(MFBBADLST);
    else {
    MFBCurrent->X = MFBCurrent->lineStyle = styleId;
    if(styleId == 0 && MFBFORMAT.setSolidLineStyle != NULL
        && *MFBFORMAT.setSolidLineStyle != 0)
        MFBGenCode(MFBFORMAT.setSolidLineStyle);
    else
        MFBGenCode(MFBFORMAT.setLineStyle);
    }
    return(MFBOK);
    }


MFBSetFillPattern(styleId)
    int styleId;
    /*
     * Notes:
     *    Sets current fill pattern.
     *
     *    CURRENT COLOR MUST BE SET PRIOR TO SETTING FILL STYLE!!!!!!!!
     *    Some frame buffers allow multi-colored stipple patterns.
     *    MFB sets all pixels in the bit array to either the
     *    background color (ID = 0) or the current forground color.
     */
    {

    /*
     * Check if styleId is already the current fill style
     */
    if(styleId == MFBCurrent->fillPattern)
    return(MFBOK);
    if(styleId < 0 || styleId >= MFBCurrent->maxFillPatterns)
    /* bad styleId */
    return(MFBBADFST);
    else {
    MFBCurrent->X = MFBCurrent->fillPattern = styleId;
    if(styleId == 0 && MFBFORMAT.setSolidFillPattern != NULL
        && *MFBFORMAT.setSolidFillPattern != 0)
        MFBGenCode(MFBFORMAT.setSolidFillPattern);
    else
        MFBGenCode(MFBFORMAT.setFillPattern);
    }
    return(MFBOK);
    }


int
MFBSetChannelMask(channelMask)
    int channelMask;
    /*
     * Notes:
     *    Sets write enable mask.  This is relevant only to terminals
     *    which have write enable masks.
     */
    {

    /* Is channel mask already set? */
    if(channelMask == MFBCurrent->channelMask)
    return(MFBOK);
    /* Are there channel or write-enable masks? */
    if(!MFBCurrent->channelMaskBool)
    return(MFBNOMASK);
    else{
    MFBCurrent->X = MFBCurrent->channelMask = channelMask;
    MFBGenCode(MFBFORMAT.channelMaskSet);
    }
    return(MFBOK);
    }


int
MFBSetReadMask(readMask)
    int readMask;
    /*
     * Notes:
     *    Sets write enable mask.  This is relevant only to terminals
     *    which have write enable masks.
     */
    {

    /* Is channel mask already set? */
    if(readMask == MFBCurrent->readMask)
    return(MFBOK);
    /* Are there channel or write-enable masks? */
    if(!MFBCurrent->readMaskBool)
    return(MFBNOMASK);
    else{
    MFBCurrent->X = MFBCurrent->readMask = readMask;
    MFBGenCode(MFBFORMAT.readMaskSet);
    }
    return(MFBOK);
    }


int
MFBSetColor(colorId)
    int colorId;
    /*
     * Notes:
     *    Sets the current (foreground) color to `colorId'.
     */
    {

    /*
     * Check if styleId is already the foreground color
     */
    if(colorId == MFBCurrent->fgColorId) return(MFBOK);

    if(colorId < 0 || colorId >= MFBCurrent->maxColors)
    /* bad colorId */
/*    return(MFBBADCST); commented out for test, p.w.h. */
    return(MFBOK); /* quickhack, p.w.h. */
    else {
    MFBCurrent->X = MFBCurrent->fgColorId = colorId;
    MFBGenCode(MFBFORMAT.setForegroundColor);
    }
    return(MFBOK);
    }


int
MFBSetTextMode(destructiveBool)
    Bool destructiveBool;
    /*
     * Notes:
     *    Changes mode of displaying text if possible.
     *    If destructiveBool is true,the the destructive
     *    text mode is set.  Also,the text index is changed
     *    to colorId.
     */
    {
    if(MFBCurrent->textMode != (int)destructiveBool) {
    if(destructiveBool){
        if(!MFBCurrent->replaceTextBool)
        return(MFBBADTM1);
        MFBGenCode(MFBFORMAT.replaceON);
        }
    else{
        if(!MFBCurrent->overstrikeTextBool)
        return(MFBBADTM2);
        MFBGenCode(MFBFORMAT.overstrikeON);
        }
    MFBCurrent->textMode = (int)destructiveBool;
    }
    return(MFBOK);
    }


int
MFBSetALUMode(mode)
    int mode;
    /*
     * Notes:
     *    This routine changes the mode by which pixels on the screen
     *    are changed when written over.  The four possible operations
     *    are:
     *      mode = MFBALUJAM    JAM (replace mode)
     *      mode = MFBALUOR     OR
     *      mode = MFBALUNOR    NOR
     *      mode = MFBALUEOR    EOR
     *    If MFB cannot set the ALU mode, MFBBADALU is returned.  Otherwise,
     *    MFBOK is returned.
     */
    {
    char *cp;
    switch(mode){
        case MFBALUJAM:
        cp = MFBFORMAT.setALUJAM;
        break;
        case MFBALUOR:
        cp = MFBFORMAT.setALUOR;
        break;
        case MFBALUNOR:
        cp = MFBFORMAT.setALUNOR;
        break;
        case MFBALUEOR:
        cp = MFBFORMAT.setALUEOR;
        break;
        default:
        cp = NULL;
    }
    if(cp == NULL || *cp == 0)
        return(MFBBADALU);
    MFBGenCode(cp);
    return(MFBOK);
    }


int
MFBSetCursorColor(colorId1,colorId2)
    int colorId1,colorId2;
    /*
     * Notes:
     *    The cursor color is set when the pointing device is initialized
     */
    {
    MFBCurrent->cursorColor1Id = colorId1;
    MFBCurrent->cursorColor2Id = colorId2;
    return(MFBOK);
    }


int
MFBSetBlinker(colorId,r,g,b,onFlag)
    int colorId,r,g,b,onFlag;
    /*
     * Notes:
     *    Turn ON or OF a blinking color.
     */
    {
    if(!MFBCurrent->blinkersBool)
    return(MFBNOBLNK);

    MFBCurrent->X = colorId;
    if(MFBCurrent->vltUseHLSBool){
    MFBConvertToHLS(r,g,b);
    }
    else{
        /* Normalize to 1000 */
        MFBCurrent->Y = r * MFBCurrent->maxIntensity / 1000;
        MFBCurrent->Z = g * MFBCurrent->maxIntensity / 1000;
        MFBCurrent->T = b * MFBCurrent->maxIntensity / 1000;
    }

    if(onFlag == ON) {
    MFBGenCode(MFBFORMAT.blinkerON);
    ++MFBCurrent->numBlinkers;
    }
    else {
    MFBGenCode(MFBFORMAT.blinkerOFF);
    --MFBCurrent->numBlinkers;
    }
    if(MFBCurrent->numBlinkers >= MFBCurrent->maxBlinkers)
    return(MFBTMBLNK);
    return(MFBOK);
    }


int
MFBSetRubberBanding(RubberBandingBool,X,Y)
    Bool RubberBandingBool;
    int X,Y;
    /*
     * Notes:
     *    Turn ON or OFF rubberbanding of pointing device.
     *    The center of the rubberbanding is assumed to be the current
     *    graphics position,but may be kludged in the enable sequence.
     */
    {
    if(MFBFORMAT.enableRubberBanding == NULL 
	    || *MFBFORMAT.enableRubberBanding == 0)
    return(MFBNORBND);
    if(RubberBandingBool){
    MFBCurrent->X = X;
    MFBCurrent->Y = Y;
    MFBGenCode(MFBFORMAT.movePenSequence);
    MFBGenCode(MFBFORMAT.enableRubberBanding);
    }
    else
    MFBGenCode(MFBFORMAT.disableRubberBanding);
    return(MFBOK);
    }






/*****************************************************************************
 *
 *      DEFINE VIEWPORT ATTRIBUTES
 *
 *****************************************************************************/

int
MFBDefineColor(colorId,r,g,b)
    int colorId,r,g,b;
    /*
     * Notes:
     *    Defines an entry in the VLT contained in the terminal.
     *    r,g, and b are the red, green, blue intensities normalized
     *    to 1000.
     */
    {

    /* Are there user defineable colors? */
    if(!MFBCurrent->vltBool)
    return(MFBNODFCO);
    if(MFBCurrent->vltUseHLSBool){
    /* HLS return in MFBCurrent->Y, MFBCurrent->Z, MFBCurrent->T */
    MFBConvertToHLS(r,g,b);
    }
    else{
        /* Normalize to 1000 */
        MFBCurrent->Y = r * MFBCurrent->maxIntensity / 1000;
        MFBCurrent->Z = g * MFBCurrent->maxIntensity / 1000;
        MFBCurrent->T = b * MFBCurrent->maxIntensity / 1000;
    }
    MFBCurrent->X = MFBmax(0,MFBmin(MFBCurrent->maxColors,colorId));
    MFBGenCode(MFBFORMAT.vltEntry);
    return(MFBOK);
    }


int
MFBDefineFillPattern(styleId,BitArray)
    int styleId;
    int *BitArray;
    /*
     *  Notes:
     *    A very difficult procedure to generalize.
     *    BitArray is a pointer to 8 integers for which the 8 least
     *    significant bits represent one row in the fill pattern.
     */
    {
    int i,*j,Count;

    /* save current stipple pattern */
    for(i=0; i<8; ++i)
    MFBCurrent->stipplePattern[i] = BitArray[i];

    /*
     * Are there defineable fill patterns for this frame buffer?
     */
    if(!MFBCurrent->fillPtrnDefineBool) 
    return(MFBNODFFP);

    MFBCurrent->X = styleId;
    if(MFBCurrent->fillDefineRowMajorBool)
    Count = MFBCurrent->fillDefineHeight;
    else
    Count = MFBCurrent->fillDefineWidth;
    MFBGenCode(MFBFORMAT.fillDefineStart);
    for(i=0; i<Count; ++i){
    if( (int)((i/8) * 8) == i) j = BitArray;
    MFBCurrent->Y = *j++;
    MFBGenCode(MFBFORMAT.fillDefineFormat);
    }
    MFBGenCode(MFBFORMAT.fillDefineEnd);
    /* return with the new fill pattern as the current one */
    return( MFBSetFillPattern(styleId) );
    }


int
MFBDefineLineStyle(styleId,BitArray)
    int styleId;
    int BitArray;
    {
    int l;

    /*
     * Are there defineable line styles for this frame buffer?
     */
    if(!MFBCurrent->linePatternDefineBool)
    return(MFBNODFLP);

    MFBCurrent->X = styleId;
    MFBGenCode(MFBFORMAT.lineDefineStart);
    MFBCurrent->Y = BitArray;
    for(l = 0; l < MFBCurrent->lineDefineLength; ++l)
    MFBGenCode(MFBFORMAT.lineDefineFormat);
    MFBGenCode(MFBFORMAT.lineDefineEnd);
    /* return with the new line style as the current one */
    return( MFBSetLineStyle(styleId) );
    }






/*****************************************************************************
 *
 *      BASIC VIEWPORT GEOMETRIES
 *
 *****************************************************************************/

int LastX, LastY;   /* used for terminals which have no drawLineToSeq. */

void
MFBMoveTo(X1,Y1)
    int X1,Y1;
    /*
     * Notes:
     *    Move current graphics position to X1,Y1.
     */
    {
    /* the assignment of lastX/Y is redundant for a Tek terminal */
    LastX = MFBCurrent->X = X1; LastY = MFBCurrent->Y = Y1;
    MFBGenCode(MFBFORMAT.movePenSequence);
    }


void
MFBDrawPath(path)
    MFBPATH *path;
    /*
     * Notes:
     *    Draw the Path defined in "path" in the current linestyle.
     *    Without a 'DrawSolidLineToSequence',we must call MFBLine
     *    for the first segment to be certain that the linestyle is
     *    properly set.
     */
    {
    int n,x,y;
    if(MFBCurrent->reissueLineStyleBool)
    MFBSetLineStyle(MFBCurrent->lineStyle);
    if(path->nvertices < 2)
    return;
    n = 2;
    MFBLine(path->xy[0],path->xy[1],path->xy[2],path->xy[3]);
    while(n < path->nvertices){
    x = path->xy[(n << 1)];
    y = path->xy[(n << 1) + 1];
    MFBDrawLineTo(x,y);
    n++;
    }
    }


void
MFBDrawLineTo(X1,Y1)
    int X1,Y1;
    /*
     * Notes:
     *    Draws a line in the current color and line style from
     *    the last graphics position to X1,Y1.
     */
    {
    if(MFBCurrent->reissueLineStyleBool)
    MFBSetLineStyle(MFBCurrent->lineStyle);
    if(MFBFORMAT.drawLineToSequence == NULL
	    || *MFBFORMAT.drawLineToSequence == 0) {
    MFBLine(LastX,LastY,X1,Y1);
    /* the assignment of lastX/Y is redundant for a Tek terminal */
    LastX = X1; LastY = Y1;
    }
    else{
    MFBCurrent->X = X1; MFBCurrent->Y = Y1;
    if(MFBCurrent->lineStyle == 0
        && MFBFORMAT.drawSolidLineToSequence != NULL
        && *MFBFORMAT.drawSolidLineToSequence != 0)
        MFBGenCode(MFBFORMAT.drawSolidLineToSequence);
    else
        MFBGenCode(MFBFORMAT.drawLineToSequence);
    }
    }


void
MFBLine(X1,Y1,X2,Y2)
    int X1,Y1,X2,Y2;
    /*
     * Notes:
     *    Draws a line in the current color and line style from X1,Y1
     *    to X2,Y2.
     */
    {
#ifdef DEBUG
    MFBCurrent->nLines += 1;
    MFBCurrent->sumLineLength += MFBmax(abs(X1-X2),abs(Y1-Y2));
#endif
    if(MFBCurrent->reissueLineStyleBool)
    MFBSetLineStyle(MFBCurrent->lineStyle);
    MFBCurrent->X = X1; MFBCurrent->Y = Y1;
    MFBCurrent->Z = X2; MFBCurrent->T = Y2;
    if(MFBCurrent->lineStyle == 0 && MFBFORMAT.drawSolidLineSequence != NULL
	    && *MFBFORMAT.drawSolidLineSequence != 0)
    MFBGenCode(MFBFORMAT.drawSolidLineSequence);
    else
    MFBGenCode(MFBFORMAT.drawLineSequence);
    LastX = X2; LastY = Y2;
    }


void
MFBBox(l,b,r,t)
    int l,b,r,t;
    /*
     * Notes:
     *    Draws a box in the current fill pattern from lower left
     *    corner (l,b) to upper right corner (r,t).  After
     *    this sequence,pen must be left at (r,t).
     */
    {
#ifdef DEBUG
    MFBCurrent->nBoxes += 1;
    MFBCurrent->sumBoxArea += (r-l)*(t-b);
#endif
    MFBCurrent->X = l; MFBCurrent->Y = b; MFBCurrent->Z = r; MFBCurrent->T = t;
    if(MFBCurrent->fillPattern == 0
	    && MFBFORMAT.drawSolidBoxSequence != NULL
	    && *MFBFORMAT.drawSolidBoxSequence != 0) {
	MFBGenCode(MFBFORMAT.drawSolidBoxSequence);
    }
    else if (MFBFORMAT.drawBoxSequence != NULL
	    && *MFBFORMAT.drawBoxSequence != 0) {
	MFBGenCode(MFBFORMAT.drawBoxSequence);
    }
    else
    MFBNaiveBoxFill(l,b,r,t);
    }


void
MFBArc(x,y,r,astart,astop,s)
    int x;      /* x coordinate of center */
    int y;      /* y coordinate of center */
    int r;      /* radius of arc */
    int astart;       /* initial angle ( +x axis = 0 degrees ) */
    int astop;  /* final angle ( +x axis = 0 degrees ) */
    int s;      /* number of segments in a 360 degree arc */
    /*
     * Notes:
     *    Draws an arc of radius r and center at (x,y) beginning at
     *    angle astart (in degrees) and ending at astop
     */
    {
    int i,j,xx,yy;
    double d;
    d = astart / RadToDeg;
    xx = x + (int)(r * cos(d));
    yy = y + (int)(r * sin(d));
    /* Must use MFBLine to force a DSL sequence if style Id is solid */
    MFBLine(xx,yy,xx,yy);
    while(astart >= astop)
    astop += 360;
    if(s>2 && s<181)
    j = MFBmax(1,(astop - astart)/s);
    else
    j = MFBmax(1,(astop - astart)/18);
    for(i = astart + j; i <= astop; i += j){
    d = i / RadToDeg;
    MFBCurrent->X = x + (int)(r * cos(d));
    MFBCurrent->Y = y + (int)(r * sin(d));
    MFBGenCode(MFBFORMAT.drawLineToSequence);
    }
    d = astop / RadToDeg;
    MFBCurrent->X = x + (int)(r * cos(d));
    MFBCurrent->Y = y + (int)(r * sin(d));
    MFBGenCode(MFBFORMAT.drawLineToSequence);
    }


void
MFBCircle(x,y,r,s)
    int x;  /* x coordinate of center */
    int y;  /* y coordinate of center */
    int r;  /* radius of circle */
    int s;  /* number of line segments in circle (default = 72) */
    /*
     * Notes:
     *    Draws a circle of radius r and center at (x,y) in the
     *    current line style.
     */
{
    int i,j=5;
    double d=0;
    if(MFBFORMAT.drawCircleSequence == NULL 
	    || *MFBFORMAT.drawCircleSequence == 0) {
	MFBCurrent->X = x + r;
	MFBCurrent->Y = y;
	if(s>2 && s<181)
	    j = MFBmax(1,360/s);
	for(i = j; i <= 360; i += j){
	    d = i / RadToDeg;
	    MFBCurrent->Z = x + (int)(r * cos(d));
	    MFBCurrent->T = y + (int)(r * sin(d));
	    MFBGenCode(MFBFORMAT.drawLineSequence);
	    MFBCurrent->X = MFBCurrent->Z;
	    MFBCurrent->Y = MFBCurrent->T;
	    }
    } else {
	MFBCurrent->X = x;
	MFBCurrent->Y = y;
	MFBCurrent->Z = r;
	MFBGenCode(MFBFORMAT.drawCircleSequence);
	}
}


void
MFBFlash(x,y,r,s)
    int x,y,r,s;
    /*
     * Notes:
     *    Draws a roundflash of radius r and center at (x,y) in the
     *    current fill style.
     */
    {
    int i=0,j;
    double d=0;
    MFBCurrent->X = x + r;
    MFBCurrent->Y = y;
    if(s>2 && s<181)
    j = 360/s;
    else {
    j = 5;
    s = 72;
    }
    MFBCurrent->Z = s;
    MFBGenCode(MFBFORMAT.beginPlygnSequence);
    for(i = j; i < 360; i += j){
    d = i / RadToDeg;
    MFBCurrent->X = x + (int)(r * cos(d));
    MFBCurrent->Y = y + (int)(r * sin(d));
    MFBGenCode(MFBFORMAT.sendPlygnVertex);
    }
    MFBCurrent->X = x + r;
    MFBCurrent->Y = y;
    MFBGenCode(MFBFORMAT.endPlygnSequence);
    }


void
MFBPolygon(p)
    MFBPOLYGON *p;
    /*
     * Notes:
     *    Draws a polygon with n vertices in the current fill pattern.
     */
{
    int *xy0,*xy1,n;
    xy0 = xy1 = p->xy;
    MFBCurrent->Z = n = p->nvertices;
    if(n <= 0) return;
    MFBCurrent->X = *xy1++;
    MFBCurrent->Y = *xy1++;
    if(MFBCurrent->fillPattern == 0 &&
	    MFBFORMAT.beginSolidPlygnSequence != NULL &&
	    *MFBFORMAT.beginSolidPlygnSequence != 0)
	MFBGenCode(MFBFORMAT.beginSolidPlygnSequence);
    else
	MFBGenCode(MFBFORMAT.beginPlygnSequence);
    n--;
    while(n--) {
	MFBCurrent->X = *xy1++;
	MFBCurrent->Y = *xy1++;
	MFBGenCode(MFBFORMAT.sendPlygnVertex);
    }
    MFBCurrent->X = *xy0;
    MFBCurrent->Y = *(xy0+1);
    MFBGenCode(MFBFORMAT.endPlygnSequence);
}


void
MFBText(text,x,y,phi)
    char *text;
    int x,y;
    int phi;    /* angle of rotation in degrees */
    /*
     * Notes:
     *    Draws the string `text' in the current color with the lower
     *    left corner at (x,y).
     */
    {
    char *c;
    
#ifdef DEBUG
    MFBCurrent->nChars += strlen(text);
#endif
    if((phi != 0) && (MFBCurrent->textRotateBool == true)){
    MFBCurrent->X = phi;
    MFBGenCode(MFBFORMAT.rotateTextSequence);
    }

    /* Move to (x + GTW),(Y + GTH). */
    MFBCurrent->X = x + MFBCurrent->fontXOffset;
    MFBCurrent->Y = y + MFBCurrent->fontYOffset;

    /* Get ready for graphics text. */
    c = text;
    MFBCurrent->Z = 0;
    while(*c++ != 0) (MFBCurrent->Z)++;

    MFBGenCode(MFBFORMAT.graphicsTextStart);

    /* Output the text. */
    while(*text) MFBPutchar(*text++);

    /* Finish the graphics text */
    MFBGenCode(MFBFORMAT.graphicsTextEnd);

    /* rotate text back to zero */
    if((phi != 0) && (MFBCurrent->textRotateBool == true)){
    MFBCurrent->X = 0;
    MFBGenCode(MFBFORMAT.rotateTextSequence);
    }
    }


void
MFBPixel(x,y)
    int x,y;
    /*
     * Notes:
     *    Sets the indicated pixel to the current foreground color.
     */
    {
    MFBCurrent->X = x; MFBCurrent->Y = y;
    MFBGenCode(MFBFORMAT.writePixel);
    }


void
MFBFlood()
    /*
     * Notes:
     *    Floods the display with the current (foreground) color.
     */
    {
    MFBCurrent->X = MFBCurrent->fgColorId;
    MFBGenCode(MFBFORMAT.screenFlood);

    /*
     * MFBFlood may destroy last x/y,so be safe about it.
     */
    MFBCurrent->lastX  = MFBCurrent->lastY  = -1;
    }


MFBPATH
*MFBArcPath(x,y,r,astart,astop,s)
    int x;      /* x coordinate of center */
    int y;      /* y coordinate of center */
    int r;      /* radius of arc */
    int astart;       /* initial angle ( +x axis = 0 degrees ) */
    int astop;  /* final angle ( +x axis = 0 degrees ) */
    int s;      /* number of segments in a 360 degree arc */
    /*
     * Notes:
     *    Returns MFBPATH containing an arc.
     */
    {
    static MFBPATH pth;
    static int xy[400];
    int *ip,i,j;
    double d;
    pth.xy = ip = xy;
    while(astart >= astop)
    astop += 360;
    if(s<2 || s>180)
    s = 20;
    pth.nvertices = 2;
    j = MFBmax(1,(astop - astart)/s);
    d = astart / RadToDeg;
    *ip++ = x + (int)(r * cos(d));
    *ip++ = y + (int)(r * sin(d));
    for(i = astart + j; i <= astop; i += j){
    d = i / RadToDeg;
    *ip++ = x + (int)(r * cos(d));
    *ip++ = y + (int)(r * sin(d));
    ++pth.nvertices;
    }
    d = astop / RadToDeg;
    *ip++ = x + (int)(r * cos(d));
    *ip   = y + (int)(r * sin(d));
    return(&pth);
    }


MFBPOLYGON
*MFBEllipse(x,y,rx,ry,s)
    int x;  /* x coordinate of center */
    int y;  /* y coordinate of center */
    int rx; /* radius of ellipse in x direction */
    int ry; /* radius of ellipse in y direction */
    int s;  /* number of line segments in circle (default = 20) */
    /*
     * Notes:
     *    Returns MFBPOLYGON containing an ellipse with no more than 200 coords.
     */
    {
    static MFBPOLYGON poly;
    static int xy[400];
    int i=0;
    int j;
    int *ip;
    double d=0;
    poly.xy = ip = xy;
    *ip++ = x + rx;
    *ip++ = y;
    if(s>2 && s<181)
    j = 360/s;
    else {
    j = 18;
    s = 20;
    }
    poly.nvertices = s + 1;
    while(--s) {
    i += j;
    d = i / RadToDeg;
    *ip++ = x + (int)(rx * cos(d));
    *ip++ = y + (int)(ry * sin(d));
    }
    *ip++ = x + rx;
    *ip++ = y;
    return(&poly);
    }






/***********************************************************************/
/*
/*           WINDOW CONTROL
/*
/***********************************************************************/

#define WNDW    MFBCurrent->currentWindow
#define VWPRT   MFBCurrent->currentViewport

int
MFBScaleX(X)
    int X;
    {
    int XX;
    XX = (int)(((double)(X-WNDW.left)*VWPRT.length)/WNDW.length) + VWPRT.left;
    if(XX < VWPRT.left) XX = VWPRT.left;
    else if(XX > VWPRT.right) XX = VWPRT.right; 
    return(XX);
    }

int
MFBScaleY(Y)
    int Y;
    {
    int YY;
    YY = (int)(((double)(Y-WNDW.bottom)*VWPRT.width)/WNDW.width) + VWPRT.bottom;
    if(YY < VWPRT.bottom) YY = VWPRT.bottom;
    else if(YY > VWPRT.top) YY = VWPRT.top;
    return(YY);
    }

int
MFBDescaleX(X)
    int X;
    {
    int XX;
    XX = ((X-VWPRT.left)*WNDW.length)/VWPRT.length+WNDW.left;
    return(XX);
    }

int
MFBDescaleY(Y)
    int Y;
    {
    int YY;

    YY = ((Y-VWPRT.bottom)*WNDW.width)/VWPRT.width+WNDW.bottom;
    return(YY);
    }

void
MFBSetViewport(left,bottom,right,top)
    int left,bottom,right,top;
    {
    VWPRT.bottom = bottom;
    VWPRT.top = top;
    VWPRT.left = left;
    VWPRT.right = right;
    VWPRT.length = VWPRT.right - VWPRT.left;
    VWPRT.width = VWPRT.top - VWPRT.bottom;
    }

void
MFBSetWindow(left,bottom,right,top)
    int left,bottom,right,top;
    {
    WNDW.bottom = bottom;
    WNDW.top = top;
    WNDW.left = left;
    WNDW.right = right;
    WNDW.length = WNDW.right - WNDW.left;
    WNDW.width = WNDW.top - WNDW.bottom;
    }

/* This wasn't in here, for some reason. */

/* MFBNaiveBoxFill simulates stipple patterns */

void
MFBNaiveBoxFill(left, bottom, right, top)
{
    int     step;
    int     incr;
    int     incr2;
    int     ll, bb, rr, tt;

    if (top < bottom)
        MFBSwapInt (top, bottom);
    if (right < left)
        MFBSwapInt (left, right);
    incr = MFBmax (MFBmax (MFBCurrent->maxX, MFBCurrent->maxY) / 400, 1);
    step = incr * 5;
    incr2 = incr + (incr >> 1);
    ll = left;
    bb = bottom;
    rr = right;
    tt = top;
    switch (MFBCurrent->fillPattern) {

        case 0: 
            /* 
             * solid
             */
            if ((right - left) > (top - bottom)) {
                for (bb = bottom; bb <= top; bb += incr)
                    MFBLine (ll, bb, rr, bb);
            }
            else {
                for (ll = left; ll <= right; ll += incr)
                    MFBLine (ll, bb, ll, tt);
            }
            break;

        case 1: 
            /* 
             * horizontal striped
             */
            for (bb = bottom; bb < top; bb += step)
                MFBLine (ll, bb, rr, bb);
            break;

        case 2: 
            /* 
             * vertical striped
             */
            for (ll = left; ll < right; ll += step)
                MFBLine (ll, bb, ll, tt);
            break;

        case 3: 
            /* 
             * diagonal lines
             */
            ll += step;
            bb += step;
            while (ll < right && bb < top) {
                MFBLine (left, bb, ll, bottom);
                ll += step;
                bb += step;
            }
            if (ll >= right) {
                while (bb < top) {
                    MFB_Y_Intercept (left, bb, ll, bottom, right, &tt);
                    MFBLine (left, bb, right, tt);
                    bb += step;
                    ll += step;
                }
            }
            else {
                while (ll < right) {
                    MFB_X_Intercept (left, bb, ll, bottom, top, &rr);
                    MFBLine (rr, top, ll, bottom);
                    bb += step;
                    ll += step;
                }
            }
            while (rr <= right && tt <= top) {
                MFB_X_Intercept (left, bb, ll, bottom, top, &rr);
                MFB_Y_Intercept (left, bb, ll, bottom, right, &tt);
                if (rr > right || tt > top)
                    break;
                MFBLine (rr, top, right, tt);
                bb += step;
                ll += step;
            }
            break;

        case 4: 
            /* 
             * diagonal lines
             */
            rr -= step;
            bb += step;
            while (rr > left && bb < top) {
                MFBLine (rr, bottom, right, bb);
                rr -= step;
                bb += step;
            }
            if (rr <= left) {
                while (bb < top) {
                    MFB_Y_Intercept (rr, bottom, right, bb, left, &tt);
                    MFBLine (left, tt, right, bb);
                    bb += step;
                    rr -= step;
                }
            }
            else {
                while (rr > left) {
                    MFB_X_Intercept (rr, bottom, right, bb, top, &ll);
                    MFBLine (rr, bottom, ll, top);
                    bb += step;
                    rr -= step;
                }
            }
            while (ll >= left && tt <= top) {
                MFB_X_Intercept (rr, bottom, right, bb, top, &ll);
                MFB_Y_Intercept (rr, bottom, right, bb, left, &tt);
                if (ll < left || tt > top)
                    break;
                MFBLine (left, tt, ll, top);
                bb += step;
                rr -= step;
            }
            break;

        case 5: 
            /* 
             * plus signs
             */
            for (bb = bottom; bb < top; bb += (step + step)) {
                for (ll = left; ll < right; ll += step) {
                    MFBLine (ll - incr2, bb, ll + incr2, bb);
                    MFBLine (ll, bb - incr2, ll, bb + incr2);
                }
            }
            break;

        case 6: 
            /* 
             * x's
             */
            for (bb = bottom; bb < top; bb += (step + step)) {
                for (ll = left; ll < right; ll += step) {
                    MFBLine (ll - incr2, bb - incr2, ll + incr2, bb + incr2);
                    MFBLine (ll - incr2, bb + incr2, ll + incr2, bb - incr2);
                }
            }
            break;

        default: 
        case 7: 
            /* 
             * outline
             */
            MFBLine (left, bottom, left, top);
            MFBLine (left, top, right, top);
            MFBLine (right, top, right, bottom);
            MFBLine (right, bottom, left, bottom);
            break;

    }
}

