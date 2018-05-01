/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: -C- 1982 Giles C. Billingsley
**********/
/*
 * mfbdecode.c
 *
 * sccsid "@(#)mfbdecode.c  1.9  9/3/83"
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
#include <stdio.h>
#include "mfb.h"
#include "suffix.h"

char *digits = "0123456789abcdefABCDEF";
int numbers[22] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    10, 11, 12, 13, 14, 15};


/*
 * This routine performs graphics conversion of data received from the
 * graphics terminal through stdio, as specified by a mfbcap entry
 * pointed to by 'PM'.  'PM' contains printf type escapes to allow
 * graphics coordinate conversion.
 *
 * The following escapes are defined for substituting x/y coordinates:
 *
 *    %X    the X coordinate
 *    %Y    the Y coordinate
 *    %Z    the Z coordinate
 *    %T    the T coordinate
 *
 *    The codes below affect the state but don't use up a value.
 *
 *    %d    as in printf
 *    %2    like %2d
 *    %3    like %3d
 *    %c    gives %c hacking special case characters
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
 *    %t1   gives X and Y in Tektronix format
 *    %t2   gives Z and T in Tektronix format
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



MFBDecode(PM)
    char *PM;
    {
    char *cp = PM;
    register int c, d;
    register int which = 0;
    register int Reg = 0, reg = 0;
    int extra,hix,hiy,lox,loy;
    int mfbnumarg(), mfbdecnum();

    if (cp == NULL) {
        MFBCurrent->X = MFBCurrent->Y = MFBCurrent->Z = MFBCurrent->T = -1;
        return(MFBCAPNOFORMAT);
        }
    while ((c = *cp++)) {
        if (c != '%') {
            if(c != (d = (*MFBCurrent->dsply_getchar)())) return(d);
            continue;
            }
        switch (*cp++) {

        case 'X':
            MFBCurrent->X = which;
            continue;

        case 'Y':
            MFBCurrent->Y = which;
            continue;

        case 'Z':
            MFBCurrent->Z = which;
            continue;

        case 'T':
            MFBCurrent->T = which;
            continue;

        case 'R': 
            Reg = which;
            continue;

        case 'r': 
            reg = which;
            continue;

        case 'd':
            which = mfbdecnum( 10, 100 );
            continue;

        case '3':
            which = mfbdecnum( 10, 3 );
            continue;

        case '2':
            which = mfbdecnum( 10, 2 );
            continue;

        case 'h':
            if  (*cp >= '1'  &&  *cp <= '4')
                which = mfbdecnum( 16, *cp++ - '0' );
            else
                which = mfbdecnum( 16, 100 );
            continue ;

        case 'o':
            if  (*cp >= '1'  &&  *cp <= '6')
                which = mfbdecnum( 8, *cp++ - '0' );
            else
                which = mfbdecnum( 8, 100 );
            continue ;

        case 't':               /* Tektronix terminals */
        c = *cp++;
        if(c == 'i'){
        /* skip control chars */
                while((hix = (*MFBCurrent->dsply_getchar)()) < 33);
        hix -= 32;
        hiy = (*MFBCurrent->dsply_getchar)() - 32;
        lox = (*MFBCurrent->dsply_getchar)() - 32;
        extra = 0; /* use extra as negative flag */
        if(lox < 16)
            extra = 1;
        else
            lox -= 16;
        which = lox + 16*hiy + 1024*hix;
        if(extra)
            which = -which;
        }
        else if(c == '1' || c == '2'){
                hiy = (*MFBCurrent->dsply_getchar)();
                extra = (*MFBCurrent->dsply_getchar)();
                loy = (*MFBCurrent->dsply_getchar)();
                hix = (*MFBCurrent->dsply_getchar)();
                lox = (*MFBCurrent->dsply_getchar)();
                if(c == '1') {
                    MFBCurrent->X=((hix&037)<<7)+((lox&037)<<2)+(extra&3);
                    MFBCurrent->Y=((hiy&037)<<7)+((loy&037)<<2)+((extra>>2)&3);
                    }
                else if(c == '2') {
                    MFBCurrent->Z=((hix&037)<<7)+((lox&037)<<2)+(extra&3);
                    MFBCurrent->T=((hiy&037)<<7)+((loy&037)<<2)+((extra>>2)&3);
                    }
            }
        else{
        --cp;
        }
            continue;

        case '>':
              if (*cp++ == '>'){
          mfbarg(cp, Reg, reg, c);
                  which = which >> c;
          }
              else {
                  cp--;
                  if((d = (*MFBCurrent->dsply_getchar)()) != '>')
              return(MFBCAPSYNTAXERR);
                  }
              continue;

        case '<':
              if (*cp++ == '<'){
          mfbarg(cp, Reg, reg, c);
                  which = which << c;
          }
              else {
                  cp--;
                  if((d = (*MFBCurrent->dsply_getchar)()) != '<')
              return(MFBCAPSYNTAXERR);
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
              continue ;

        case '+':
          mfbarg(cp, Reg, reg, c);
              which += c;
              continue ;

        case '*':
          mfbarg(cp, Reg, reg, c);
              which *= c;
              continue ;

        case '/':
          mfbarg(cp, Reg, reg, c);
              which /= c;
              continue ;

        case '=':
          mfbarg(cp, Reg, reg, c);
              which = c;
              continue ;

        case 'c':
              which = (*MFBCurrent->dsply_getchar)();
              continue;

        case 'a':
          mfbarg(cp, Reg, reg, c);
              which = abs(c);
              continue;

        case '%':
              if((d = (*MFBCurrent->dsply_getchar)()) != '%')
          return(MFBCAPSYNTAXERR);
              continue;

        default:
              MFBCurrent->X = MFBCurrent->Y 
          = MFBCurrent->Z = MFBCurrent->T = -1;
              return(MFBCAPNOFORMAT);
              }
        }
    return(MFBCAPOK);
    }




mfbdecnum( base, maxdigits )
    int base, maxdigits;
    {
    register int i, j, k, result = 0;
    register char c;

    for( i = 0; i < maxdigits; i++ ) {
        c = (*MFBCurrent->dsply_getchar)();
        for( j = 0; j < 22; j++ ) {
            if( digits[j] == c ) {
                k = numbers[j];
                break;
                }
            }
        if( j >= 22 || k >= base ) {
            (*(MFBCurrent->dsply_ungetchar))();
            return(result);
            }
        result *= base;
        result += k;
        }
    return(result);
    }
