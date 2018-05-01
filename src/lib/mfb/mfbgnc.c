/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: -C- 1982 Giles C. Billingsley
**********/
/*
 * mfbgncode.c
 *
 * sccsid "@(#)mfbgncode.c  1.9  9/3/83"
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

/* Library routines */
#ifdef HAS_FTIME
extern void ftime();
#endif

/*
 * This routine perform graphics conversion of data as specified by
 * a mfbcap entry pointed to by 'PM'.
 * 'PM' is a string containing printf type escapes to allow
 * graphics coordinate conversion.  The converted data is stored in a
 * character buffer called 'result', and a pointer to 'result' is returned.
 *
 * The following escapes are defined for substituting x/y coordinates:
 *
 *    %X    the X coordinate
 *    %Y    the Y coordinate
 *    %Z    the Z coordinate
 *    %T    the T coordinate
 *    %C    the C Parameter
 *    %F    the F Parameter
 *    %L    the L Parameter
 *
 *    The codes below affect the state but don't use up a value.
 *
 *    %c    gives %c hacking special case characters
 *    %d    as in printf
 *    %2    like %2d
 *    %3    like %3d
 *    %h1   gives one   ASCII hex of low order four    bits
 *    %h2   gives two   ASCII hex of low order eight   bits
 *    %h3   gives three ASCII hex of low order twelve  bits
 *    %h4   gives four  ASCII hex of low order sixteen bits
 *    %o1   gives one   ASCII octal of low order three   bits
 *    %o2   gives two   ASCII octal of low order six     bits
 *    %o3   gives three ASCII octal of low order nine    bits
 *    %o4   gives four  ASCII octal of low order twelve  bits
 *    %o5   gives five  ASCII octal of low order fifteen bits
 *    %o6   gives six   ASCII octal of low order sixteen bits
 *    %t1   gives X and Y in Tektronix format with coordinate compression
 *    %t2   gives Z and T in Tektronix format with coordinate compression
 *    %t3   gives X and R in Tektronix format with coordinate compression
 *    %t4   gives R and Y in Tektronix format with coordinate compression
 *    %t5   gives R and r in Tektronix format with coordinate compression
 *    %ti   gives value in Tektronix integer format
 *    %tr   gives value in Tektronix real format
 *    %@    gives a NULL character
 *    %R    store value in temporary buffer 1
 *    %r    store value in temporary buffer 2
 *    %+x   add x to the value
 *    %-x   subtract x from the value
 *    %*x   multiply value by x
 *    %/x   divide value by x
 *    %>>x  shift value right by x bits
 *    %<<x  shift value left by x bits
 *    %|x   OR value with x
 *    %&x   AND value with x
 *    %^x   XOR value with x
 *    %=x   set value equal to x
 *    %ax   set value equal to absolute value of x
 *    %~    complement value (1's complement)
 *    %%    gives %
 *    %B    BCD (2 decimal digits encoded in one byte)
 *    %D    Delta Data (backwards bcd)
 *
 *
 * where x can be:
 *
 * (1)   one byte: the numberic value of this byte is used as x.
 *
 * (2)   '#' followed by a decimal number: the decimal value is used.
 *
 * (3)   '%' followed by C, F, L, X, Y, Z, T, r, or R: the C, F, L, X, Y, Z,
 *       T, r or R value is used.
 *
 * all other characters are ``self-inserting''.
 */

MFBGenCode(PM)
    char *PM;
    {
#ifdef HAS_FTIME
    struct timeb time1, time2;
    float reftime, elapsedtime;
#endif
    static char result[512];    /* 512 chars maximum per transfer */
    char hexadec();         /* return a hexadecimal character */
    char octal();           /* return an octal character */
    char *strcpy();
    char *cp;
    register char *dp;
    register int c;
    register int which = 0;
    register int reg=0, Reg=0;
    int i,x1,y1;
    int tmp,lo1,hi1,hi2;
    int exponent;
    int hiy,hix,loy,lox,extra,ohiy,ohix,oloy,oextra;
    int mfbnumarg();

    cp = PM;
    dp = result;
    if (cp == NULL) {
    return;
    }
    while (c = *cp++) {
    if(c == '$') {
        if(*cp != '<'){
        *dp++ = c;
        }
        else{
        ++cp;
        *dp='\0';
        i = dp - result;
        /* Output the guts of the string. */
        if (i != 0) {
            (*MFBCurrent->outstr)(result,i);
            }
        /* Flush the buffer */
        MFBUpdate();
        /* generate the delay */
        mfbarg(cp, Reg, reg, tmp);
        if(*cp++ != '>')
            continue;
#ifdef HAS_FTIME
        ftime(&time1);
        reftime = (float)(time1.time * 1000 + time1.millitm);
        while(tmp > 0){
            ftime(&time2);
            elapsedtime = (float)(time2.time * 1000 + time2.millitm) - reftime;
            if(elapsedtime > (float)tmp)
            break;
            }
#else
	sleep((tmp + 999) / 1000);	/* Yuk; have to round up to seconds */
#endif
        dp = result;
        }
        continue;
        }
    else if (c != '%') {
        *dp++ = c;
        continue;
        }
    switch (*cp++) {

    case 'X':
        which = MFBCurrent->X;
        continue;

    case 'Y':
        which = MFBCurrent->Y;
        continue;

    case 'Z':
        which = MFBCurrent->Z;
        continue;

    case 'T':
        which = MFBCurrent->T;
        continue;

    case 'R': 
        Reg = which;
        continue;

    case 'r': 
        reg = which;
        continue;

    case 'C':
        which = MFBCurrent->fgColorId;
        continue;

    case 'F':
        which = MFBCurrent->fillPattern;
        continue;

    case 'L':
        which = MFBCurrent->lineStyle;
        continue;

    case 'd':
        (void) sprintf(dp,"%d",which);
        while(*dp != 0) ++dp;
        which = 0;
        continue;

    case '3':
        *dp++ = (which / 100) + '0';
        which %= 100;
        /* fall into... */

    case '2':
        *dp++ = (which / 10) + '0';
        *dp++ = (which % 10) + '0';
        which=0;
        continue;

    case 'h':
        if  (*cp >= '1'  &&  *cp <= '4') {
        for  (i = *cp++ - '1'; i >= 0; i--){
               *dp++ = hexadec(which>>(i*4));
            }
        which=0;
        }
        continue;

    case 'o':
        if  (*cp >= '1'  &&  *cp <= '6')  {
           for  (i = *cp++ - '1'; i >= 0; i--){
            *dp++ = octal(which>>(i*3));
            }
           which=0;
        }
        continue;

    case 't':             /* TEKTRONIX TERMINALS */
        if(*cp == 'i' || *cp == 'r'){
        tmp = which;
        if (tmp < 0) tmp = -tmp;
        if(*cp == 'r'){
            exponent = 0;
            while(tmp > 32767){
                tmp /= 2;
                ++exponent;
                }
            }
        hi1 = tmp/16;
        if(which < 0)
            lo1 = tmp - hi1 * 16 + 32;
        else
            lo1 = tmp - hi1 * 16 + 48;
        tmp = hi1;
        if (tmp > 64) {
            hi1 = tmp/64;
            hi2 = tmp - hi1 * 64 + 64;
            *dp++ = hi1 + 64;
            *dp++ = hi2;
            }
        else if (tmp)
            *dp++ = tmp + 64;
        *dp++ = lo1;
        if(*cp == 'r'){
            /* exponent is always non-negative */
            hi1 = exponent/16;
            lo1 = exponent - hi1 * 16 + 48;
            exponent = hi1;
            if (exponent > 64) {
                hi1 = exponent/64;
                hi2 = exponent - hi1 * 64 + 64;
                *dp++ = hi1 + 64;
                *dp++ = hi2;
                }
            else if (exponent)
                *dp++ = exponent + 64;
            *dp++ = lo1;
            }
        }
        else{
        if(*cp == '1'){
            x1 = MFBCurrent->X;
            y1 = MFBCurrent->Y;
            }
        else if(*cp == '2'){
            x1 = MFBCurrent->Z;
            y1 = MFBCurrent->T;
            }
        else if(*cp == '3'){
            x1 = MFBCurrent->X;
            y1 = Reg;
            }
        else if(*cp == '4'){
            x1 = Reg;
            y1 = MFBCurrent->Y;
            }
        else if(*cp == '5'){
            x1 = Reg;
            y1 = reg;
            }
        else
            return;
        hiy   = (y1 >> 7) & 037;
        extra = x1 & 03 | ((y1 & 03) << 2);
        loy   = (y1 >> 2) & 037;
        hix   = (x1 >> 7) & 037;
        lox   = (x1 >> 2) & 037;
        ohiy   = (MFBCurrent->lastY >> 7) & 037;
        oextra = MFBCurrent->lastX & 03 | ((MFBCurrent->lastY & 3)<<2);
        oloy   = (MFBCurrent->lastY >> 2) & 037;
        ohix   = (MFBCurrent->lastX >> 7) & 037;
        MFBCurrent->lastX = x1;
        MFBCurrent->lastY = y1;
        if(hiy != ohiy)
            *dp++ = hiy | 040;
        if(hix != ohix){
            if(extra != oextra){
            *dp++ = extra | 0140;
            }
            *dp++ = loy | 0140;
            *dp++ = hix | 040;
            }
        else{
            if(extra != oextra){
            *dp++ = extra | 0140;
            *dp++ = loy | 0140;
            }
            else if(loy != oloy){
            *dp++ = loy | 0140;
            }
            }
        *dp++ = lox | 0100;
        }
        cp++;
        continue;

    case '>':
        if (*cp++ == '>'){
        mfbarg(cp, Reg, reg, c);
        which >>= c;
        }
        else {
        cp--;
        *dp++ = '>';
        }
        continue;

    case '<':
        if (*cp++ == '<'){
        mfbarg(cp, Reg, reg, c);
        which <<= c;
        }
        else {
        cp--;
        *dp++ = '<';
        }
        continue;

    case '|':
        mfbarg(cp, Reg, reg, c);
        which |= c;
        continue;

    case '&':
        mfbarg(cp, Reg, reg, c);
        which &= c;
        continue;

    case '^':
        mfbarg(cp, Reg, reg, c);
        which ^= c;
        continue;

    case '~':
        which = ~which;
        continue;

    case '-':
        mfbarg(cp, Reg, reg, c);
        which -= c;
        continue;

    case '+':
        mfbarg(cp, Reg, reg, c);
        which += c;
        continue;

    case '*':
        mfbarg(cp, Reg, reg, c);
        which *= c;
        continue;

    case '/':
        mfbarg(cp, Reg, reg, c);
        which /= c;
        continue;

    case 'c':
        *dp++ = (char)which;
        which = 0;
        continue;

    case '@':
        *dp++ = 0;
        continue;

    case 'a':
        which = abs(which);
        continue;

    case '=':
        mfbarg(cp, Reg, reg, c);
        which = c;
        continue;

    case '%':
        *dp++ = c;
        continue;

    case 'B':
        which = (which/10 << 4) + which%10;
        continue;

    case 'D':
        which = which - 2 * (which%16);
        continue;

    default:
        return;
        }
    }
    *dp='\0';
    i = dp - result;

    /*
     * Output the guts of the string.
     */
    if (i != 0) {
    (*MFBCurrent->outstr)(result,i);
    }
    }



char hexadec(num)
     /* return a hexadecimal character corresponding to the
     low order 4 bits of the argument    */
     int num;
     {
     num &= 0xf;
     if(num <= 9) 
     return('0'+num);
     else
     return('A'+num-10);
     }




char octal(num)
    /* return an octal character corresponding to the low
       order 3 bits of the argument */
    int num;
    {
    num &= 07;
    return('0' + num);
    }
