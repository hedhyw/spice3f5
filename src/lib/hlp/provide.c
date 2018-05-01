/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1986 Wayne A. Christopher, U. C. Berkeley CAD Group 
**********/

/*
 *   faustus@cad.berkeley.edu, ucbvax!faustus
 * Permission is granted to modify and re-distribute this code in any manner
 * as long as this notice is preserved.  All standard disclaimers apply.
 *
 * Toss the help window up on the screen, and deal with the graph...
 */

#include "spice.h"
#include "cpstd.h"
#include "hlpdefs.h"
#include "suffix.h"

bool hlp_usex = false;

void
hlp_provide(top)
    topic *top;
{
    toplink *res;
    topic *parent, *newtop;

    if (!top)
	return;

#ifdef HAS_X_
    if (getenv("DISPLAY") || hlp_displayname)
        hlp_usex = true;
#endif

    top->xposition = top->yposition = 0;
    if (hlp_usex) {
        if (!hlp_xdisplay(top)) {
            fprintf(stderr, "Couldn't open X display.\n");
            return;
        }
    } else {
        if (!hlp_tdisplay(top)) {
            fprintf(stderr, "Couldn't display text\n");
            return;
        }
    }

#ifdef HAS_X11       /* X11 does this asynchronously */
    if (hlp_usex) return;
#endif
    
    for (;;) {
        if (hlp_usex)
            res = hlp_xhandle(&parent);
        else
            res = hlp_thandle(&parent);
        if (!res && !parent) {
            /* No more windows. */
            hlp_killfamily(top);
            if (hlp_usex) hlp_xclosedisplay(); /* need to change 
                    display pointer back J.H. */
            return;
        }
        if (res) {
            /* Create a new window... */
            if (hlp_usex)
                hlp_xwait(parent, true);
            if (!(newtop = hlp_read(res->place))) {
                fprintf(stderr, "Internal error: bad link\n");
                hlp_xwait(parent, false);
                continue;
            }
            if (hlp_usex)
                hlp_xwait(parent, false);
            newtop->next = parent->children;
            parent->children = newtop;
            newtop->parent = parent;
            newtop->xposition = parent->xposition + 50;
            newtop->yposition = parent->yposition + 50;
            if (hlp_usex) {
                if (!hlp_xdisplay(newtop)) {
                    fprintf(stderr, "Couldn't open win\n"); 
                    return; 
                }
            } else {
                if (!hlp_tdisplay(newtop)) {
                    fprintf(stderr, "Couldn't display\n"); 
                    return; 
                }
            }
        } else {
            /* Blow this one and its descendants away. */
            hlp_killfamily(parent);
        hlp_fixchildren(parent);
            if (parent == top)
                return;
        }
    }

}

void
hlp_fixchildren(parent)
topic *parent;
{

    topic *pa;

    if (parent->parent) {
        if (parent->parent->children == parent)
            parent->parent->children =
                    parent->next;
        else {
            for (pa = parent->parent->children;
                    pa->next; pa = pa->next)
                if (pa->next == parent)
                    break;
            if (!pa->next) {
                fprintf(stderr, "bah...\n");
            }
            pa->next = pa->next->next;
        }
    }
}

/* Note that this doesn't actually free the data structures, just gets
 * rid of the window.
 */

void
hlp_killfamily(top)
    topic *top;
{
    topic *ch;

    for (ch = top->children; ch; ch = ch->next)
        hlp_killfamily(ch);
    if (hlp_usex)
        hlp_xkillwin(top);
    else
        hlp_tkillwin(top);
    top->children = NULL;
    return;
}

