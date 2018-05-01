/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: -C- 1982 Giles C. Billingsley
**********/
/*
 * mfbtext.c
 *
 * sccsid "@(#)mfbtext.c    1.9  9/3/83"
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

#define MFBFORMAT   MFBCurrent->strings


/**************************************************************************
 *
 *                             MFBText
 *
 *     MFBText contains MFB routines for the management of graphtext.
 *
 **************************************************************************/

void
MFBMore(Left,Bottom,Right,Top,Textfile)
    int Left;
    int Bottom;
    int Right;
    int Top;
    FILE *Textfile;
    {
    char cbuf[200];     /* 200 chars per line max */
    int c,i;
    int linecount;
    int done = 0;
    int oldfillpattern;
    int oldforeground;
    int nlines;
    int nchars;
    int controlchar;

    /* test to be sure of window area */
    if(Top < Bottom)
        MFBSwapInt(Top,Bottom);
    if(Right < Left)
        MFBSwapInt(Left,Right);

    /* calculate parameters */
    nlines = (Top - Bottom)/MFBCurrent->fontHeight;
    nchars = (Right - Left)/(MFBCurrent->fontWidth + MFBCurrent->fontXOffset);
    nchars = MFBmin(nchars,200);

    /* save old style ID's */
    oldforeground = MFBCurrent->fgColorId;
    oldfillpattern = MFBCurrent->fillPattern;

    if(nlines <= 0) return;
    linecount = 1;
    MFBSetFillPattern(0);
    MFBSetColor(0);
    MFBBox(Left,Bottom,Right,Top);
    MFBSetColor(oldforeground);
    while(! done) {
    i = 0;
    controlchar = 0;
    while(i < nchars && (c = getc(Textfile)) != '\n' && c != EOF){
        if(c == 9){ /* tab */
        cbuf[i++] = ' ';
        while(i < nchars && (i % 8) != 0) cbuf[i++] = ' ';
        }
        else if(c < ' '){
        if(controlchar == 0){
            cbuf[i++] = '^';
            controlchar = 1;
            (void) ungetc(c,Textfile);
            }
        else{
            cbuf[i++] = c + '@';
            controlchar = 0;
            }
        }
        else if(c <= '~')
            cbuf[i++] = c;
        }
    cbuf[i] = 0;
    if(c == EOF) done = 1;
    MFBText(cbuf,Left,Top - (linecount) * MFBCurrent->fontHeight,0);
    MFBUpdate();
    if(done || ++linecount >= nlines){
        linecount = 1;
        if(done)
            MFBText("-DONE-",Left,Bottom,0);
        else
            MFBText("-MORE- (^D to exit)",Left,Bottom,0);
        MFBUpdate();
        c = (*MFBCurrent->kybrd_getchar)();
        if(c == 4) 
        done = 1;
            MFBSetColor(0);
        MFBBox(Left,Bottom,Right,Top);
            MFBSetColor(oldforeground);
        }
    }
    MFBSetFillPattern(oldfillpattern);
    }


void
MFBScroll(Left,Bottom,Right,Top,Textfile)
    int Left;
    int Bottom;
    int Right;
    int Top;
    FILE *Textfile;
    {
    char cbuf[200];     /* 200 chars per line max */
    int c,i,j;
    int curline;        /* current line in the textfile */
    int linecount;
    int done = 0;
    int oldfillpattern;
    int oldforeground;
    int nlines;
    int nchars;
    int controlchar;

    /* test to be sure of window area */
    if(Top < Bottom)
        MFBSwapInt(Top,Bottom);
    if(Right < Left)
        MFBSwapInt(Left,Right);

    /* calculate parameters */
    nlines = (Top - Bottom)/MFBCurrent->fontHeight;
    nchars = (Right - Left)/(MFBCurrent->fontWidth + MFBCurrent->fontXOffset);
    nchars = MFBmin(nchars,200);

    /* save old style ID's */
    oldforeground = MFBCurrent->fgColorId;
    oldfillpattern = MFBCurrent->fillPattern;

    if(nlines <= 0) return;
    curline = 1;
    MFBSetFillPattern(0);
    MFBSetColor(0);
    MFBBox(Left,Bottom,Right,Top);
    MFBSetColor(oldforeground);
    while( !done ){
    rewind(Textfile);
    /* skip first curline's */
    for(i = 1; i < curline; ++i){
        while((c = getc(Textfile)) != '\n' && c != EOF) ;
        }
        /* output nlines of text */
    linecount = 0;
        for(j = 1; j < nlines; ++j){
        i = 0;
        controlchar = 0;
        while(i < nchars && (c = getc(Textfile)) != '\n' && c != EOF){
            if(c == 9){ /* tab */
            cbuf[i++] = ' ';
            while(i < nchars && (i % 8) != 0) cbuf[i++] = ' ';
            }
            else if(c < ' '){
            if(controlchar == 0){
                cbuf[i++] = '^';
                controlchar = 1;
                (void) ungetc(c,Textfile);
                }
            else{
                cbuf[i++] = c + '@';
                controlchar = 0;
                }
            }
            else if(c <= '~')
                cbuf[i++] = c;
            }
        if(c == '\n') ++linecount;
        cbuf[i] = 0;
        MFBText(cbuf,Left,Top - (j) * MFBCurrent->fontHeight,0);
        MFBUpdate();
        if(c == EOF){
        j = nlines;
        done = 1;
        }
        }
    if(done)
        MFBText("-DONE-",Left,Bottom,0);
    else
        MFBText("-MORE- (^D to exit, ^U to scroll up)",Left,Bottom,0);
    MFBUpdate();
    c = (*MFBCurrent->kybrd_getchar)();
    --linecount;
    if(c == 4)
        done = 1;
    else if(c == 21){
        curline -= linecount;
        if(curline < 1)
        curline = 1;
        }
    else
        curline += linecount;
        MFBSetColor(0);
    MFBBox(Left,Bottom,Right,Top);
        MFBSetColor(oldforeground);
    }
    MFBSetFillPattern(oldfillpattern);
    }
