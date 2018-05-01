/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: -C- 1982 Giles Billingsley
**********/
/*
 * mfbcaps.c
 *
 * sccsid "@(#)mfbcaps.c    1.9  9/3/83"
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



#define MAXHOP      5    /* max number of MCE=indirections */
#define BADNUM      0    /* illegal number for numeric capability */

#include "spice.h"
#include "mfb.h"
#include <ctype.h>
#include "suffix.h"


/*
 * mfbcap - routines for dealing with the terminal capability data base
 *        mfbcap routines must be loaded
 *
 * Essentially all the work here is scanning and decoding escapes
 * in string capabilities.  We don't use stdio because the editor
 * doesn't, and because living w/o it is not hard.
 */

static char *mfb_buf;
static int hopcount;       /* detect infinite loops in mfbcap, init 0 */

FILE   *POpen();
char   *strcpy();
char   *MFBSkip();
char   *MFBGetStr();
char   *MFBCapDecod();
int    MFBGetNum();
int    MFBGetFlag();
int    MFBGetEnt();
int    MFBCheckForMCE();


/*
 * Get an entry for terminal name in buffer bp,
 * from the mfbcap file.  Parse is very rudimentary;
 * we just notice escaped newlines.
 */
int
MFBGetEnt(bp, name, mfbcapFile)
    char *bp;
    char *name;
    char *mfbcapFile;
    {
    register char *cp;
    register int c;
    register int i = 0, cnt = 0;
    register int NewLine;
    char inputBuffer[BUFSIZE];
    FILE *CapFile;
    int mfbcapDesc;

    hopcount = 0;   /* Fixes bug in in spice3a* -- could only plot 5 times. */
    mfb_buf = bp;
    if((CapFile = POpen(mfbcapFile, "r", (char *)NULL, (char **)NULL)) == NULL)
        return(MFBBADMCF);
    mfbcapDesc = fileno(CapFile);

    /*
     * It should be faster to read mfbcap by 4Kbytes at a time
     */
    for(;;){
    NewLine = 0;
        cp = bp;
        for(;;){
            if(i >= cnt){
                cnt = read(mfbcapDesc, inputBuffer, BUFSIZE);
                if(cnt <= 0 && !NewLine){
                    if(close(mfbcapDesc) < 0)
            return(MFBBADMCF);
                    return(MFBBADENT);
                    }
                i = 0;
                }
            c = inputBuffer[i++];

        /*
         * Check for continuation from previous line
         */
        if(NewLine && c != ' ' && c != '\t'){
        --i;
        break;
        }
        /*
         * Check for new line
         */
        NewLine = 0;
        if(c == '\n'){
        NewLine = 1;
        continue;
        }
        /*
         * Check for line too long
         */
            else if(cp >= bp + BUFSIZE){
        return(MFBMCELNG);
                }
            else
                *cp++ = c;
            }
        *cp = 0;

        /*
         * The real work for the match.
         */
        if(mfbnamatch(name)){
            if(close(mfbcapDesc) < 0)
        return(MFBBADMCF);
            return( MFBCheckForMCE(mfbcapFile) );
            }
        }
    }


/*
 * check the last entry, see if it's MCE=xxxxx. If so,
 * recursively find xxxxx and append that entry (minus the names)
 * to take the place of the MCE=xxxxx entry. This allows mfbcap
 * entries to say "like an AED but with a graphics tablet".
 * Note that this works because of the left to right scan.
 */
int
MFBCheckForMCE(mfbcapFile)
    char *mfbcapFile;
    {
    register char *p, *q;
    char mfbname[16];                      /* name of similar terminal */
    char mfbbuf[BUFSIZE];
    char *holdgbuf = mfb_buf;
    int l;

    p = mfb_buf + strlen(mfb_buf);
    for(l=0; l<2; ++l){
        while(*--p != ','){
            if(p < mfb_buf){
                return(MFBBADMCE);
                }
        }
    }
    while(*p == ',' || *p == ' ' || *p == '\t')
        p++;

    /* p now points to beginning of last field */
    if(p[0] != 'M' || p[1] != 'C' || p[2] != 'E') return(MFBOK);
    (void) strcpy(mfbname,p+4);
    q = mfbname;
    while(q && *q != ',')
        q++;
    *q = 0;
    if(++hopcount > MAXHOP){
        return(MFBINFMCE);
        }
    if((l = MFBGetEnt(mfbbuf,mfbname,mfbcapFile)) != 1){
        return(l);
        }
    for(q=mfbbuf; *q != ','; q++) ;
    l = p - holdgbuf + strlen(q);
    if(l > BUFSIZE){
        q[BUFSIZE - (p-mfb_buf)] = 0;
        return(MFBMCELNG);
        }
    (void) strcpy(p, q+1);
    mfb_buf = holdgbuf;
    return(MFBOK);
    }


/*
 * mfbnamatch deals with name matching.  The first field of the mfbcap
 * entry is a sequence of names separated by |'s, so we compare
 * against each such name.  The normal , terminator after the last
 * name (before the first field) stops us.
 */
mfbnamatch(np)
    char *np;
    {
    register char *Np, *Bp;

    Bp = mfb_buf;
    if(*Bp == '#') return(FALSE);
    for(;;){
        for(Np = np; *Np && *Bp == *Np; Bp++, Np++) continue;
        if(*Np == 0 && (*Bp == '|' || *Bp == ',' || *Bp == 0))
            return(TRUE);
        while(*Bp && *Bp != ',' && *Bp != '|') Bp++;
        if(*Bp == 0 || *Bp == ',') return(FALSE);
        Bp++;
        }
    }



/*
 * Skip to the next field.
 */
static char *
MFBSkip(bp)
    char *bp;
    {
    while(*bp && !(*bp == ',' && *(bp - 1) != '\\'))
    bp++;

    /* Now clear the white space */
    while(*bp == ',' || *bp == ' ' || *bp == '\t')
    bp++;
    return(bp);
    }



/*
 * Handle a flag option.
 * Flag options are given "naked", i.e. followed by a , or the end
 * of the buffer.  Return 1 if we find the option, or 0 if it is
 * not given.
 */
MFBGetFlag(id)
    char *id;
    {
    register char *bp = mfb_buf;
    register char *cp;
    register int i;

    for(;;){
        bp = MFBSkip(bp);
        if(!*bp) return(0);
    i = 0;
    cp = id;
    while(*cp != 0 && *bp != 0){
        if(*cp++ != *bp++){
        i = 1;
        break;
        }
        }
    if(!i){
            if(!*bp || *bp == ',')
                return(1);
            else if(*bp == '@')
                return(0);
            }
        }
    }



/*
 * Get a string valued option.
 * These are given as
 *    GIS=\E\E\027
 * Much decoding is done on the strings, and the strings are
 * placed in area, which is a ref parameter which is updated.
 * For speed there is no checking on area overflow.
 */
char *
MFBGetStr(id, area)
    char *id, **area;
    {
    register char *bp = mfb_buf;
    register char *cp;
    register int i;

    for(;;){
        bp = MFBSkip(bp);
        if(!*bp) return(NULL);
    i = 0;
    cp = id;
    while(*cp != 0 && *bp != 0){
        if(*cp++ != *bp++){
        i = 1;
        break;
        }
        }
        if(i || *bp != '=') continue;
        bp++;
        return(MFBCapDecod(bp, area));
        }
    }



/*
 * MFBCapDecod decodes the string capability escapes.
 */
static char *
MFBCapDecod(str, area)
    char *str;
    char **area;
    {
    char *cp;
    int c;
    char *dp;
    int i;

    cp = *area;
    while((c = *str++) && c != ','){
        switch (c){
        case '^':
            c = *str++ & 037;
            break;

        case '\\':
            dp = "E\033^^\\\\,,n\nr\rt\tb\bf\f";
            c = *str++;
    nextc:
                if(*dp++ == c){
                c = *dp++;
                break;
                }
            dp++;
            if(*dp)
                goto nextc;
            if(isdigit(c)){
                c -= '0', i = 2;
                do
                    c <<= 3, c |= *str++ - '0';
                while(--i && isdigit(*str));
                }
            break;
            }
        *cp++ = c;
        }
    *cp++ = 0;
    str = *area;
    *area = cp;
    return(str);
    }



/*
 * Return the (numeric) option id.
 * Numeric options look like
 *    MCL#255
 * i.e. the option string is separated from the numeric value by
 * a # character.  If the option is not found we return -1.
 * Note that we handle hex numbers beginning with "0x" or "0X",
 * and octal numbers beginning with 0.
 *
 */
MFBGetNum(id)
    char *id;
    {
    register int i, j;
    register char *bp = mfb_buf;
    register char negative = 0 ;
    register char *cp;

    for(;;){
        bp = MFBSkip(bp);
        if(*bp == 0) return(BADNUM);
    i = 0;
    cp = id;
    while(*cp != 0 && *bp != 0){
        if(*cp++ != *bp++){
        i = 1;
        break;
        }
        }
        if(*bp == 0) return(BADNUM);
        if(i || *bp != '#') continue;
        bp++;
        if((negative=(*bp == '-'))) bp++ ;
        i = 0;
        while( (j = ((int)(*bp) - 060)) >= 0 && (j < 10) ) {
            bp++; i *= 10; i += j;
        }
        if(negative)
               return(-i) ;
        else
               return(i);
        }
    }

