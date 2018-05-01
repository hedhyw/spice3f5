/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#ifndef UTIL
#define UTIL

/* #define MALLOC(x) calloc(1,(unsigned)(x)) */
#define MALLOC(x) tmalloc((unsigned)(x))
#define FREE(x) {if (x) {free((char *)(x));(x) = 0;}}
#define REALLOC(x,y) trealloc((char *)(x),(unsigned)(y))
#define ZERO(PTR,TYPE)	(bzero((PTR),sizeof(TYPE)))

#ifdef HAS_STDLIB
#ifndef _STDLIB_INCLUDED
#define _STDLIB_INCLUDED
#include <stdlib.h>
#endif
#else
extern char *malloc();
extern char *calloc();
extern char *realloc();
extern void free();
#endif

extern char *trealloc();
extern char *tmalloc();

#define TRUE 1
#define FALSE 0

#ifdef DEBUG
#define DEBUGMSG(textargs) printf(textargs)
#else
#define DEBUGMSG(testargs) 
#endif

#ifdef HAS_NOINLINE
#define FABS(a) fabs(a)
double fabs();
#else
#define FABS(a) ( ((a)<0) ? -(a) : (a) )
#endif

/* XXX Move these into the above ifdef someday */
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define SIGN(a,b) ( b >= 0 ? (a >= 0 ? a : - a) : (a >= 0 ? - a : a))

#define ABORT() fflush(stderr);fflush(stdout);abort();

#define ERROR(CODE,MESSAGE)	{					      \
	errMsg = MALLOC(strlen(MESSAGE) + 1);				      \
	strcpy(errMsg, (MESSAGE));					      \
	return (CODE);							      \
	}

#define	NEW(TYPE)	((TYPE *) MALLOC(sizeof(TYPE)))
#define	NEWN(TYPE,COUNT) ((TYPE *) MALLOC(sizeof(TYPE) * (COUNT)))

#endif /*UTIL*/

#define	R_NORM(A,B) {							      \
	if ((A) == 0.0) {						      \
	    (B) = 0;							      \
	} else {							      \
	    while (FABS(A) > 1.0) {					      \
		(B) += 1;						      \
		(A) /= 2.0;						      \
	    }								      \
	    while (FABS(A) < 0.5) {					      \
		(B) -= 1;						      \
		(A) *= 2.0;						      \
	    }								      \
	}								      \
    }
