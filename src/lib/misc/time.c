/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
**********/

/*
 * Date and time utility functions
 */

#include "spice.h"
#include "stdio.h"
#include "misc.h"

#ifdef HAS_LOCALTIME
#include <time.h>
#endif

#ifdef HAS_BSDRUSAGE
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#else
#ifdef HAS_SYSVRUSAGE
#include <sys/types.h>
#include <sys/times.h>
#include <sys/param.h>
#else
#ifdef HAS_FTIME
/* default to ftime if we can't get real CPU times */
#include <sys/types.h>
#include <sys/timeb.h>
#endif
#endif
#endif

#include "suffix.h"

/* Return the date. Return value is static data. */

char *
datestring()
{

#ifdef HAS_LOCALTIME
    static char tbuf[45];
    struct tm *tp;
    char *ap;
    int i;

    time_t tloc;
    time(&tloc);
    tp = localtime(&tloc);
    ap = asctime(tp);
    (void) sprintf(tbuf, "%.20s", ap);
    (void) strcat(tbuf, ap + 19);
    i = strlen(tbuf);
    tbuf[i - 1] = '\0';
    return (tbuf);

#else

    return ("today");

#endif
}

/* return time interval in seconds and milliseconds */

#ifndef HAS_BSDRUSAGE
#ifndef HAS_SYSVRUSAGE
#ifdef HAS_FTIME

struct timeb timebegin;

timediff(now, begin, sec, msec)
struct timeb *now, *begin;
int *sec, *msec;
{

    *msec = now->millitm - begin->millitm;
    *sec = now->time - begin->time;
    if (*msec < 0) {
      *msec += 1000;
      (*sec)--;
    }
    return;

}

#endif
#endif
#endif

/* How many seconds have elapsed in running time. */

double
seconds()
{
#ifdef HAS_BSDRUSAGE
    struct rusage ruse;

    (void) getrusage(RUSAGE_SELF, &ruse);
    return (ruse.ru_utime.tv_sec + (double) ruse.ru_utime.tv_usec / 1000000.0);
#else
#ifdef HAS_SYSVRUSAGE
#ifdef ipsc
/* times() doesn't work on hypercube nodes, use the double-prec. clock */
   double dclock();

   return( dclock() );
#else
    struct tms tmsbuf;

    times(&tmsbuf);
    return((double) tmsbuf.tms_utime / HZ);
#endif
#else
#ifdef HAS_FTIME
    struct timeb timenow;
    int sec, msec;

    ftime(&timenow);
    timediff(&timenow, &timebegin, &sec, &msec);
    return(sec + (double) msec / 1000.0);
#else
#ifdef HAS_VMSHACK
    return((double)clock()/(double)CLK_TCK);
#else
#ifdef HAS_MACCLOCK
    return((double)clock()/(double)CLOCKS_PER_SEC);
#else /* unknown */
    /* don't know how to do this in general. */
    return(-1.0);	/* Obvious error condition */
#endif /* !CLOCKS_PER_SEC */
#endif /* !VMS */
#endif /* !FTIME */
#endif /* !SYSV */
#endif /* !BSD */
}
