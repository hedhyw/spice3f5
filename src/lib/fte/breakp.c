/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Code to deal with breakpoints and tracing.
 */

#include "spice.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "ftedata.h"
#include "ftedebug.h"
#include "suffix.h"

static bool satisfied();
static void printcond();

void dbfree();
extern struct dbcomm *dbs;      /* export for iplot */
extern int debugnumber;
static int howmanysteps = 0;
static int steps = 0;

/* Set a breakpoint. Possible commands are:
 *  stop after n
 *  stop when var cond val
 *
 * If more than one is given on a command line, then this is a conjunction.
 */

void
com_stop(wl)
    wordlist *wl;
{
    struct dbcomm *d, *thisone = NULL;
    char *s, buf[64];
    int i;
    double *val;

    while (wl) {
        if (thisone == NULL)
            thisone = d = alloc(struct dbcomm);
        else {
            d->db_also = alloc(struct dbcomm);
            d = d->db_also;
        }

        /* Figure out what the first condition is. */
	d->db_analysis = NULL;
        if (eq(wl->wl_word, "after") && wl->wl_next) {
            d->db_type = DB_STOPAFTER;
            d->db_number = debugnumber;
	    if (!wl->wl_next->wl_word)
		i = 0;
	    else {
#ifdef HAS_CTYPE
		for (s = wl->wl_next->wl_word, i = 0; *s; s++)
		    if (!isdigit(*s))
			goto bad;
		    else
			i = i * 10 + (*s - '0');
#else
		i = atoi(wl->wl_next->wl_word); /* etoi ??? */
#endif
	    }
            d->db_iteration = i;
            wl = wl->wl_next->wl_next;
        } else if (eq(wl->wl_word, "when") && wl->wl_next &&
                    wl->wl_next->wl_next && /* ick */
                    wl->wl_next->wl_next->wl_next) {
            wl = wl->wl_next;
            d->db_number = debugnumber;
            d->db_type = DB_STOPWHEN;
            s = wl->wl_word;
            val = ft_numparse(&s, false);
            if (val)
                d->db_value1 = *val;
            else
                d->db_nodename1 = copy(wl->wl_word);
            wl = wl->wl_next;

            /* Now get the condition */
            if (eq(wl->wl_word, "eq") || eq(wl->wl_word, "="))
                d->db_op = DBC_EQU;
            else if (eq(wl->wl_word, "ne") || eq(wl->wl_word, "<>"))
                d->db_op = DBC_NEQ;
            else if (eq(wl->wl_word, "gt") || eq(wl->wl_word, ">"))
                d->db_op = DBC_GT;
            else if (eq(wl->wl_word, "lt") || eq(wl->wl_word, "<"))
                d->db_op = DBC_LT;
            else if (eq(wl->wl_word, "ge") || eq(wl->wl_word, ">="))
                d->db_op = DBC_GTE;
            else if (eq(wl->wl_word, "le") || eq(wl->wl_word, "<="))
                d->db_op = DBC_LTE;
            else
                goto bad;
            wl = wl->wl_next;

            /* Now see about the second one. */
            s = wl->wl_word;
            val = ft_numparse(&s, false);
            if (val)
                d->db_value2 = *val;
            else
                d->db_nodename2 = copy(wl->wl_word);
            wl = wl->wl_next;
        } else
            goto bad;
    }
    if (thisone) {
        if (dbs) {
            for (d = dbs; d->db_next; d = d->db_next);
            d->db_next = thisone;
        } else
            dbs = thisone;
        (void) sprintf(buf, "%d", debugnumber);
        cp_addkword(CT_DBNUMS, buf);
        debugnumber++;
    }

    return;

bad:    fprintf(cp_err, "Syntax error parsing breakpoint specification.\n");
    return;
}

/* Trace a node (have wrd_point print it). Usage is "trace node ..."*/

void
com_trce(wl)
    wordlist *wl;
{
    settrace(wl, VF_PRINT, 0);
    return;
}

/* Incrementally plot a value. This is just like trace. */

void
com_iplot(wl)
    wordlist *wl;
{

    /* settrace(wl, VF_PLOT); */

    struct dbcomm *d, *td, *currentdb = NULL;
    char *s;

    /* We use a modified ad-hoc algorithm here where db_also denotes
        vectors on the same command line and db_next denotes
        separate iplot commands. */
    while (wl) {
        s = cp_unquote(wl->wl_word);
        d = alloc(struct dbcomm);
	d->db_analysis = NULL;
        d->db_number = debugnumber++;
        if (eq(s, "all")) {
            d->db_type = DB_IPLOTALL;
        } else {
            d->db_type = DB_IPLOT;
            d->db_nodename1 = copy(s);
        }
        d->db_also = currentdb;
        currentdb = d;
        wl = wl->wl_next;
    }
    if (dbs) {
        for (td = dbs; td->db_next; td = td->db_next)
            ;
        td->db_next = currentdb;
    } else
        dbs = currentdb;

    return;

}

/* Step a number of iterations. */

void
com_step(wl)
    wordlist *wl;
{
    if (wl)
        steps = howmanysteps = atoi(wl->wl_word);
    else
        steps = howmanysteps = 1;
    com_resume((wordlist *) NULL);
    return;
}

/* Print out the currently active breakpoints and traces. If we are printing
 * to a file, assume that the file will be used for a later source and leave
 * off the event numbers (with UNIX, that is).  -- I don't like this.
 */

#define isatty(xxxx) 1

/* ARGSUSED */
void
com_sttus(wl)
    wordlist *wl;
{
    struct dbcomm *d, *dc;

    for (d = dbs; d; d = d->db_next) {
        if (d->db_type == DB_TRACENODE) {
            if (isatty(fileno(cp_out)))
                fprintf(cp_out, "%-4d trace %s", d->db_number,
                    d->db_nodename1);
            else
                fprintf(cp_out, "trace %s", d->db_nodename1);
        } else if (d->db_type == DB_IPLOT) {
            if (isatty(fileno(cp_out))) {
                fprintf(cp_out, "%-4d iplot %s", d->db_number,
                    d->db_nodename1);
            } else {
                fprintf(cp_out, "iplot %s", d->db_nodename1);
            }
            for (dc = d->db_also; dc; dc = dc->db_also) {
              fprintf(cp_out, " %s", dc->db_nodename1);
            }
        } else if (d->db_type == DB_SAVE) {
            if (isatty(fileno(cp_out)))
                fprintf(cp_out, "%-4d save %s", d->db_number,
                    d->db_nodename1);
            else
                fprintf(cp_out, "save %s", d->db_nodename1);
        } else if (d->db_type == DB_TRACEALL) {
            if (isatty(fileno(cp_out)))
                fprintf(cp_out, "%-4d trace all", d->db_number);
            else
                fprintf(cp_out, "trace all");
        } else if (d->db_type == DB_IPLOTALL) {
            if (isatty(fileno(cp_out)))
                fprintf(cp_out, "%-4d iplot all", d->db_number);
            else
                fprintf(cp_out, "iplot all");
        } else if (d->db_type == DB_SAVEALL) {
            if (isatty(fileno(cp_out)))
                fprintf(cp_out, "%-4d save all", d->db_number);
            else
                fprintf(cp_out, "save all");
        } else if ((d->db_type == DB_STOPAFTER) ||
                (d->db_type == DB_STOPWHEN)) {
            if (isatty(fileno(cp_out)))
                fprintf(cp_out, "%-4d stop", d->db_number);
            else
                fprintf(cp_out, "stop");
            printcond(d, cp_out);
        } else if ((d->db_type == DB_DEADIPLOT)) {
            if (isatty(fileno(cp_out))) {
                fprintf(cp_out, "%-4d exiting iplot %s", d->db_number,
                    d->db_nodename1);
            } else {
                fprintf(cp_out, "exiting iplot %s", d->db_nodename1);
            }
            for (dc = d->db_also; dc; dc = dc->db_also) {
              fprintf(cp_out, " %s", dc->db_nodename1);
            }
        } else
            fprintf(cp_err, 
                "com_sttus: Internal Error: bad db %d\n", d->db_type);
        (void) putc('\n', cp_out);
    }
    return;
}

void
dbfree(db)
    struct dbcomm *db;
{
    struct dbcomm *dd, *dn;

    for (dd = db; dd; dd = dn) {
        dn = dd->db_also;
        tfree(dd->db_nodename1);
        tfree(dd->db_nodename2);
        tfree(dd);
    }
    return;
}

/* Delete breakpoints and traces. Usage is delete [number ...] */

void
com_delete(wl)
    wordlist *wl;
{
    int i;
    char *s, buf[64];
    struct dbcomm *d, *dt;

    if (wl && eq(wl->wl_word, "all")) {
        for (dt = dbs; dt; dt = d) {
            d = dt->db_next;
            dbfree(dt);
        }
        dbs = NULL;
        return;
    } else if (!wl) {
        if (!dbs) {
            fprintf(cp_err, "Error: no debugs in effect\n");
            return;
        }
    }
    while (wl) {
	if (wl->wl_word) {
#ifdef HAS_CTYPE
	    for (s = wl->wl_word, i = 0; *s; s++)
		if (!isdigit(*s)) {
		    fprintf(cp_err, "Error: %s isn't a number.\n",
			    wl->wl_word);
		    goto bad;
		} else
		    i = i * 10 + (*s - '0');
#else
	    i = atoi(wl->wl_next->wl_word); /* etoi ??? */
#endif
	} else
	    i = 0;
        for (d = dbs, dt = NULL; d; d = d->db_next) {
            if (d->db_number == i) {
                if (dt)
                    dt->db_next = d->db_next;
                else
                    dbs = d->db_next;
                dbfree(d);
                (void) sprintf(buf, "%d", i);
                cp_remkword(CT_DBNUMS, buf);
                break;
            }
            dt = d;
        }
bad:        wl = wl->wl_next;
    }
    return;
}

/* Writedata calls this routine to see if it should keep going. If it
 * returns true, then the run should resume.
 */

bool
ft_bpcheck(runplot, iteration)
    struct plot *runplot;
    int iteration;
{
    struct dbcomm *d, *dt;

    if ((howmanysteps > 0) && (--howmanysteps == 0)) {
        if (steps > 1)
            fprintf(cp_err, "Stopped after %d steps.\n", steps);
        return (false);
    }

    /* Check the debugs set. */
    for (d = dbs; d; d = d->db_next) {
        for (dt = d; dt; dt = dt->db_also) {
            switch (dt->db_type) {
                case DB_TRACENODE:
                case DB_TRACEALL:
                case DB_IPLOT:
                case DB_DEADIPLOT:
                case DB_IPLOTALL:
                case DB_SAVE:
                case DB_SAVEALL:
                    goto more;
                case DB_STOPAFTER:
                    if (iteration == dt->db_iteration)
                        break;
                    else
                        goto more;
                case DB_STOPWHEN:
                    /* See if the condition is true. */
                    if (satisfied(dt, runplot))
                        break;
                    else
                        goto more;
                default:
                    fprintf(cp_err, "ft_bpcheck: Internal Error: bad db %d\n", 
			    dt->db_type);
            }
        }
        if (dt == NULL) {
            /* It made it */
            fprintf(cp_err, "%-2d: condition met: stop ",
                d->db_number);
            printcond(d, cp_err);
            (void) putc('\n', cp_err);
            return (false);
        }
more:   /* Just continue */ ;
    }
    return (true);
}

/* This is called to determine whether a STOPWHEN is true. */

static bool
satisfied(d, plot)
    struct dbcomm *d;
    struct plot *plot;
{
    struct dvec *v1 = NULL, *v2 = NULL;
    double d1, d2;

    if (d->db_nodename1) {
        if ((v1 = vec_fromplot(d->db_nodename1, plot)) == NULL) {
            fprintf(cp_err, "Error: %s: no such node\n", 
                    d->db_nodename1);
            return (false);
        }
        if (isreal(v1))
            d1 = v1->v_realdata[v1->v_length - 1];
        else
            d1 = realpart(&(v1->v_compdata[v1->v_length - 1]));
    } else
        d1 = d->db_value1;

    if (d->db_nodename2) {
        if ((v2 = vec_fromplot(d->db_nodename2, plot)) == NULL) {
            fprintf(cp_err, "Error: %s: no such node\n", 
                    d->db_nodename2);
            return (false);
        }
        if (isreal(v2))
            d2 = v2->v_realdata[v2->v_length - 1];
        else
            d2 = realpart(&(v2->v_compdata[v2->v_length - 1]));
    } else
        d2 = d->db_value2;

    switch (d->db_op) {
        case DBC_EQU:
            return ((d1 == d2) ? true : false);
        case DBC_NEQ:
            return ((d1 != d2) ? true : false);
        case DBC_GTE:
            return ((d1 >= d2) ? true : false);
        case DBC_LTE:
            return ((d1 <= d2) ? true : false);
        case DBC_GT:
            return ((d1 > d2) ? true : false);
        case DBC_LT:
            return ((d1 < d2) ? true : false);
        default:
            fprintf(cp_err, 
                "satisfied: Internal Error: bad cond %d\n", 
                    d->db_op);
            return (false);
    }
}

/* Writedata calls this before it starts a run, to set the proper flags
 * on the dvecs. If a change is made during a break, then the routine
 * wrd_chtrace is used from these routines. We have to be clever with save:
 * if there was no save given, then save everything. Unfortunately you
 * can't stop in the middle, do a save, and have the rest then discarded.
 */

void
ft_trquery()
{
    return;
}

static void
printcond(d, fp)
    struct dbcomm *d;
    FILE *fp;
{
    struct dbcomm *dt;

    for (dt = d; dt; dt = dt->db_also) {
        if (dt->db_type == DB_STOPAFTER)
            fprintf(fp, " after %ld",
                    dt->db_iteration);
        else {
            if (dt->db_nodename1)
                fprintf(fp, " when %s",
                    dt->db_nodename1);
            else
                fprintf(fp, " when %lg",
                    dt->db_value1);
            switch (dt->db_op) {
                case DBC_EQU:
                    fputs(" =", fp);
                    break;
                case DBC_NEQ:
                    fputs(" <>", fp);
                    break;
                case DBC_GT:
                    fputs(" >", fp);
                    break;
                case DBC_LT:
                    fputs(" <", fp);
                    break;
                case DBC_GTE:
                    fputs(" >=", fp);
                    break;
                case DBC_LTE:
                    fputs(" <=", fp);
                    break;
                default:
                    fprintf(cp_err,
                "printcond: Internal Error: bad cond %d", 
                        dt->db_op);
            }
            if (dt->db_nodename2)
                fprintf(fp, " %s",
                    dt->db_nodename2);
            else
                fprintf(fp, " %lg",
                    dt->db_value2);
        }
    }
    return;
}

