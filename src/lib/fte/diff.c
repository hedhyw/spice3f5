/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Do a 'diff' of two plots.
 */

#include "spice.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "ftedata.h"
#include "ftecmath.h"
#include "util.h"
#include "suffix.h"

/* Determine if two vectors have the 'same' name. */

static bool
nameeq(n1, n2)
    char *n1, *n2;
{
    char buf1[BSIZE_SP], buf2[BSIZE_SP];
    int i;

    if (eq(n1, n2))
        return (true);
    if (isdigit(*n1))
        (void) sprintf(buf1, "v(%s)", n1);
    else
        (void) strcpy(buf1, n1);
    if (isdigit(*n2))
        (void) sprintf(buf2, "v(%s)", n2);
    else
        (void) strcpy(buf2, n2);
    for (i = 0; buf1[i]; i++)
        if (isupper(buf1[i]))
            buf1[i] = tolower(buf1[i]);
    for (i = 0; buf2[i]; i++)
        if (isupper(buf2[i]))
            buf2[i] = tolower(buf2[i]);
    return (eq(buf1, buf2) ? true : false);
}

void
com_diff(wl)
    wordlist *wl;
{
    double vntol, abstol, reltol, tol, cmax, cm1, cm2;
    struct plot *p1, *p2 = NULL;
    struct dvec *v1, *v2;
    double d1, d2;
    complex c1, c2, c3;
    register int i, j;
    wordlist *tw;

    if (!cp_getvar("diff_vntol", VT_REAL, (char *) &vntol))
        vntol = 1.0e-6;
    if (!cp_getvar("diff_abstol", VT_REAL, (char *) &abstol))
        abstol = 1.0e-12;
    if (!cp_getvar("diff_reltol", VT_REAL, (char *) &reltol))
        reltol = 0.001;

    /* Let's try to be clever about defaults. This code is ugly. */
    if (!wl || !wl->wl_next) {
        if (plot_list && plot_list->pl_next &&
                !plot_list->pl_next->pl_next) {
            p1 = plot_list;
            p2 = plot_list->pl_next;
            if (wl && !eq(wl->wl_word, p1->pl_typename) &&
                    !eq(wl->wl_word, p2->pl_typename)) {
                fprintf(cp_err, "Error: no such plot \"%s\"\n",
                        wl->wl_word);
                return;
            }
            fprintf(cp_err, "Plots are \"%s\" and \"%s\"\n",
                    plot_list->pl_typename,
                    plot_list->pl_next->pl_typename);
            if (wl)
                wl = NULL;
        } else {
            fprintf(cp_err, "Error: plot names not given.\n");
            return;
        }
    } else {
        for (p1 = plot_list; p1; p1 = p1->pl_next)
            if (eq(wl->wl_word, p1->pl_typename))
                break;
        if (!p1) {
            fprintf(cp_err, "Error: no such plot %s\n", 
                    wl->wl_word);
            return;
        }
        wl = wl->wl_next;
    }

    if (!p2) {
        for (p2 = plot_list; p2; p2 = p2->pl_next)
            if (eq(wl->wl_word, p2->pl_typename))
                break;
        if (!p2) {
            fprintf(cp_err, "Error: no such plot %s\n",
                    wl->wl_word);
            return;
        }
        wl = wl->wl_next;
    }

    /* Now do some tests to make sure these plots are really the
     * same type, etc.
     */
    if (!eq(p1->pl_name, p2->pl_name))
        fprintf(cp_err, 
        "Warning: plots %s and %s seem to be of different types\n",
            p1->pl_typename, p2->pl_typename);
    if (!eq(p1->pl_title, p2->pl_title))
        fprintf(cp_err, 
        "Warning: plots %s and %s seem to be from different circuits\n",
            p1->pl_typename, p2->pl_typename);

    /* This may not be the best way to do this */
    for (v1 = p1->pl_dvecs; v1; v1 = v1->v_next)
        v1->v_link2 = NULL;
    for (v2 = p2->pl_dvecs; v2; v2 = v2->v_next)
        v2->v_link2 = NULL;

    for (v1 = p1->pl_dvecs; v1; v1 = v1->v_next) {
        for (v2 = p2->pl_dvecs; v2; v2 = v2->v_next)
            if (!v2->v_link2 && nameeq(v1->v_name, v2->v_name) &&
                ((v1->v_flags & (VF_REAL | VF_COMPLEX)) ==
                (v2->v_flags & (VF_REAL | VF_COMPLEX))) &&
                (v1->v_type == v2->v_type)) {
                v1->v_link2 = v2;
                v2->v_link2 = v1;
                break;
            }
    }

    for (v1 = p1->pl_dvecs; v1; v1 = v1->v_next)
        if (!v1->v_link2)
            fprintf(cp_err, 
        ">>> %s vector %s in %s not in %s, or of wrong type\n",
                    isreal(v1) ? "real" : "complex",
                    v1->v_name, p1->pl_typename,
                    p2->pl_typename);
    for (v2 = p2->pl_dvecs; v2; v2 = v2->v_next)
        if (!v2->v_link2)
            fprintf(cp_err, 
        ">>> %s vector %s in %s not in %s, or of wrong type\n",
                    isreal(v2) ? "real" : "complex",
                    v2->v_name, p2->pl_typename,
                    p1->pl_typename);

    /* Throw out the ones that aren't in the arg list */
    if (wl && !eq(wl->wl_word, "all")) {    /* Just in case */
        for (v1 = p1->pl_dvecs; v1; v1 = v1->v_next)
            if (v1->v_link2) {
                for (tw = wl; tw; tw = tw->wl_next)
                    if (nameeq(v1->v_name, tw->wl_word))
                        break;
                if (!tw)
                    v1->v_link2 = NULL;
            }
        for (v2 = p2->pl_dvecs; v2; v2 = v2->v_next)
            if (v2->v_link2) {
                for (tw = wl; tw; tw = tw->wl_next)
                    if (nameeq(v2->v_name, tw->wl_word))
                        break;
                if (!tw)
                    v2->v_link2 = NULL;
            }
    }
    
    /* Now we have all the vectors linked to their twins.  Travel
     * down each one and print values that differ enough.
     */
    for (v1 = p1->pl_dvecs; v1; v1 = v1->v_next) {
        if (!v1->v_link2)
            continue;
        v2 = v1->v_link2;
        if (v1->v_type == SV_VOLTAGE)
            tol = vntol;
        else
            tol = abstol;
        j = MAX(v1->v_length, v2->v_length);
        for (i = 0; i < j; i++) {
            if (v1->v_length <= i) {
                fprintf(cp_out, 
                ">>> %s is %d long in %s and %d long in %s\n",
                        v1->v_name, v1->v_length,
                        p1->pl_typename, v2->v_length,
                        p2->pl_typename);
                break;
            } else if (v2->v_length <= i) {
                fprintf(cp_out, 
                ">>> %s is %d long in %s and %d long in %s\n",
                        v2->v_name, v2->v_length,
                        p2->pl_typename, v1->v_length,
                        p1->pl_typename);
                break;
            } else {
                if (isreal(v1)) {
                    d1 = v1->v_realdata[i];
                    d2 = v2->v_realdata[i];
                    if (MAX(FABS(d1), FABS(d2)) * reltol +
                            tol < FABS(d1 - d2)) {
                        fprintf(cp_out,
                            "%s.%s[%d] = %-15s ",
                        p1->pl_typename, v1->v_name, i,
                        printnum(d1));
                        fprintf(cp_out,
                            "%s.%s[%d] = %s\n",
                        p2->pl_typename, v2->v_name, i,
                        printnum(d2));
                    }
                } else {
                    c1 = v1->v_compdata[i];
                    c2 = v2->v_compdata[i];
                    realpart(&c3) = realpart(&c1) - realpart(&c2);
                    imagpart(&c3) = imagpart(&c1) - imagpart(&c2);
                    /* Stupid evil PC compilers */
                    cm1 = cmag(&c1);
                    cm2 = cmag(&c2);
                    cmax = MAX(cm1, cm2);
                    if (cmax * reltol +
                            tol < cmag(&c3)) {
                        fprintf(cp_out,
            "%s.%s[%d] = %-10s, %-10s %s.%s[%d] = %-10s, %s\n",
                        p1->pl_typename, v1->v_name, i,
                        copy(printnum(realpart(&c1))),
                        copy(printnum(imagpart(&c1))),
                        p2->pl_typename, v2->v_name, i,
                        copy(printnum(realpart(&c2))),
                        copy(printnum(imagpart(&c2))));
                    }
                }
            }
        }
    }
    return;
}

