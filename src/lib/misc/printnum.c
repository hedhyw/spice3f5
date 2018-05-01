/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/* Print a number in a reasonable form.  This is the sort of thing that
 * %G does, but more appropriate for spice.  Returns static data.
 */

#include "spice.h"
#include "suffix.h"

int cp_numdgt = -1;

char *
printnum(num)
    double num;
{
    static char buf[128];
    int n;

    if (cp_numdgt > 1)
        n = cp_numdgt;
    else
        n = 6;
    if (num < 0.0)
        n--;

    (void) sprintf(buf, "%.*le", n, num);

    return (buf);
}
