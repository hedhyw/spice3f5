/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Convert a parse tree to a list of data vectors.
 */

#include "spice.h"
#include "util.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "ftedata.h"
#include "fteparse.h"
#include "ftecmath.h"
#ifdef HAS_UNIX_SIGS
#ifdef HAS_LONGJUMP
#define HAS_SIGS_AND_LJMP
#include <setjmp.h>
#include <signal.h>
#endif
#endif
#include "suffix.h"

static struct dvec *apply_func();
static struct dvec *doop();
static char *mkcname();
static SIGNAL_TYPE sig_matherr();

#ifdef HAS_SIGS_AND_LJMP

/* We are careful here to catch SIGILL and recognise them as math errors.
 * The only trouble is that the (void) signal handler we installed before will
 * be lost, but that's no great loss.
 */

static jmp_buf matherrbuf;

static SIGNAL_TYPE
sig_matherr()
{
    fprintf(cp_err, "Error: argument out of range for math function\n");
    longjmp(matherrbuf, 1);
    /* NOTREACHED */
}

#endif

/* Note that ft_evaluate will return NULL on invalid expressions. */

struct dvec *
ft_evaluate(node)
    struct pnode *node;
{
    struct dvec *d;

    if (!node)
        return (NULL);
    else if (node->pn_value)
        d = node->pn_value;
    else if (node->pn_func)
        d = apply_func(node->pn_func, node->pn_left);
    else if (node->pn_op) {
        if (node->pn_op->op_arity == 1)
            d = (struct dvec *)
                ((*node->pn_op->op_func) (node->pn_left));
        else if (node->pn_op->op_arity == 2)
            d = (struct dvec *) ((*node->pn_op->op_func)
                (node->pn_left, node->pn_right));
    } else {
        fprintf(cp_err, "ft_evaluate: Internal Error: bad node\n");
        return (NULL);
    }

    if (!d) {
	return NULL;
    }

    if (node->pn_name && !ft_evdb && d && !d->v_link2)
        d->v_name = copy(node->pn_name);

    if (!d->v_length) {
        fprintf(cp_err, "Error: no such vector %s\n", d->v_name);
        return (NULL);
    } else
        return (d);
}

/* The binary operations. */

struct dvec *
op_plus(arg1, arg2)
    struct pnode *arg1, *arg2;
{
    return (doop('+', cx_plus, arg1, arg2));
}

struct dvec *
op_minus(arg1, arg2)
    struct pnode *arg1, *arg2;
{
    return (doop('-', cx_minus, arg1, arg2));
}

struct dvec *
op_comma(arg1, arg2)
    struct pnode *arg1, *arg2;
{
    return (doop(',', cx_comma, arg1, arg2));
}

struct dvec *
op_times(arg1, arg2)
    struct pnode *arg1, *arg2;
{
    return (doop('*', cx_times, arg1, arg2));
}

struct dvec *
op_mod(arg1, arg2)
    struct pnode *arg1, *arg2;
{
    return (doop('%', cx_mod, arg1, arg2));
}

struct dvec *
op_divide(arg1, arg2)
    struct pnode *arg1, *arg2;
{
    return (doop('/', cx_divide, arg1, arg2));
}

struct dvec *
op_power(arg1, arg2)
    struct pnode *arg1, *arg2;
{
    return (doop('^', cx_power, arg1, arg2));
}

struct dvec *
op_eq(arg1, arg2)
    struct pnode *arg1, *arg2;
{
    return (doop('=', cx_eq, arg1, arg2));
}

struct dvec *
op_gt(arg1, arg2)
    struct pnode *arg1, *arg2;
{
    return (doop('>', cx_gt, arg1, arg2));
}

struct dvec *
op_lt(arg1, arg2)
    struct pnode *arg1, *arg2;
{
    return (doop('<', cx_lt, arg1, arg2));
}

struct dvec *
op_ge(arg1, arg2)
    struct pnode *arg1, *arg2;
{
    return (doop('G', cx_ge, arg1, arg2));
}

struct dvec *
op_le(arg1, arg2)
    struct pnode *arg1, *arg2;
{
    return (doop('L', cx_le, arg1, arg2));
}

struct dvec *
op_ne(arg1, arg2)
    struct pnode *arg1, *arg2;
{
    return (doop('N', cx_ne, arg1, arg2));
}

struct dvec *
op_and(arg1, arg2)
    struct pnode *arg1, *arg2;
{
    return (doop('&', cx_and, arg1, arg2));
}

struct dvec *
op_or(arg1, arg2)
    struct pnode *arg1, *arg2;
{
    return (doop('|', cx_or, arg1, arg2));
}

/* This is an odd operation.  The first argument is the name of a vector, and
 * the second is a range in the scale, so that v(1)[[10, 20]] gives all the
 * values of v(1) for which the TIME value is between 10 and 20.  If there is
 * one argument it picks out the values which have that scale value.
 * NOTE that we totally ignore multi-dimensionality here -- the result is
 * a 1-dim vector.
 */

struct dvec *
op_range(arg1, arg2)
    struct pnode *arg1, *arg2;
{
    struct dvec *v, *ind, *res, *scale; /* , *nscale; */
    double up, low, td;
    int len, i, j;
    bool rev = false;

    v = ft_evaluate(arg1);
    ind = ft_evaluate(arg2);
    if (!v || !ind)
        return (NULL);
    scale = v->v_scale;
    if (!scale)
        scale = v->v_plot->pl_scale;
    if (!scale) {
        fprintf(cp_err, "Error: no scale for vector %s\n", v->v_name);
        return (NULL);
    }

    if (ind->v_length != 1) {
        fprintf(cp_err, "Error: strange range specification\n");
        return (NULL);
    }
    if (isreal(ind)) {
        up = low = *ind->v_realdata;
    } else {
        up = imagpart(ind->v_compdata);
        low = realpart(ind->v_compdata);
    }
    if (up < low) {
        td = up;
        up = low;
        low = td;
        rev = true;
    }
    for (i = len = 0; i < scale->v_length; i++) {
        td = isreal(scale) ? scale->v_realdata[i] :
                realpart(&scale->v_compdata[i]);
        if ((td <= up) && (td >= low))
            len++;
    }

    res = alloc(struct dvec);
    ZERO(res,struct dvec);
    res->v_name = mkcname('R', v->v_name, ind->v_name);
    res->v_type = v->v_type;
    res->v_flags = v->v_flags;

    res->v_gridtype = v->v_gridtype;
    res->v_plottype = v->v_plottype;
    res->v_defcolor = v->v_defcolor;
    res->v_length = len;
    res->v_scale = /* nscale; */ scale;
    /* Dave says get rid of this
    res->v_numdims = v->v_numdims;
    for (i = 0; i < v->v_numdims; i++)
        res->v_dims[i] = v->v_dims[i];
    */
    res->v_numdims = 1;
    res->v_dims[0] = len;

    if (isreal(res))
        res->v_realdata = (double *) tmalloc(sizeof (double) * len);
    else
        res->v_compdata = (complex *) tmalloc(sizeof (complex) * len);

    /* Toss in the data */

    j = 0;
    for (i = (rev ? v->v_length - 1 : 0); i != (rev ? -1 : v->v_length);
            rev ? i-- : i++) {
        td = isreal(scale) ? scale->v_realdata[i] :
                realpart(&scale->v_compdata[i]);
        if ((td <= up) && (td >= low)) {
            if (isreal(res)) {
                res->v_realdata[j] = v->v_realdata[i];
            } else {
                realpart(&res->v_compdata[j]) =
                        realpart(&v->v_compdata[i]);
                imagpart(&res->v_compdata[j]) =
                        imagpart(&v->v_compdata[i]);
            }
            j++;
        }
    }
    if (j != len)
        fprintf(cp_err, "Error: something funny..\n");

    /* Note that we DON'T do a vec_new, since we want this vector to be
     * invisible to everybody except the result of this operation.
     * Doing this will cause a lot of core leaks, though. XXX
     */

    vec_new(res);
    return (res);
}

/* This is another operation we do specially -- if the argument is a vector of
 * dimension n, n > 0, the result will be either a vector of dimension n - 1,
 * or a vector of dimension n with only a certain range of vectors present.
 */

struct dvec *
op_ind(arg1, arg2)
    struct pnode *arg1, *arg2;
{
    struct dvec *v, *ind, *res;
    int length, newdim, i, j, k, up, down;
    int majsize, blocksize;
    bool rev = false;

    v = ft_evaluate(arg1);
    ind = ft_evaluate(arg2);
    if (!v || !ind)
        return (NULL);

    /* First let's check to make sure that the vector is consistent */
    if (v->v_numdims > 1) {
        for (i = 0, j = 1; i < v->v_numdims; i++)
            j *= v->v_dims[i];
        if (v->v_length != j) {
            fprintf(cp_err,
                "op_ind: Internal Error: len %d should be %d\n",
                v->v_length, j);
            return (NULL);
        }
    } else {
        /* Just in case we were sloppy */
        v->v_numdims = 1;
        v->v_dims[0] = v->v_length;
        if (v->v_length <= 1) {
            fprintf(cp_err, "Error: no indexing on a scalar (%s)\n",
                    v->v_name);
            return (NULL);
        }
    }

    if (ind->v_length != 1) {
        fprintf(cp_err, "Error: index %s is not of length 1\n",
                ind->v_name);
        return (NULL);
    }

    majsize = v->v_dims[0];
    blocksize = v->v_length / majsize;

    /* Now figure out if we should put the dim down by one.  Because of the
     * way we parse the index, we figure that if the value is complex
     * (e.g, "[1,2]"), the guy meant a range.  This is sort of bad though.
     */
    if (isreal(ind)) {
        newdim = v->v_numdims - 1;
        down = up = ind->v_realdata[0];
    } else {
        newdim = v->v_numdims;
        down = realpart(&ind->v_compdata[0]);
        up = imagpart(&ind->v_compdata[0]);
    }
    if (up < down) {
        i = up;
        up = down;
        down = i;
        rev = true;
    }
    if (up < 0) {
        fprintf(cp_err, "Warning: upper limit %d should be 0\n", up);
        up = 0;
    }
    if (up >= majsize) {
        fprintf(cp_err, "Warning: upper limit %d should be %d\n", up,
                majsize - 1);
        up = majsize - 1;
    }
    if (down < 0) {
        fprintf(cp_err, "Warning: lower limit %d should be 0\n", down);
        down = 0;
    }
    if (down >= majsize) {
        fprintf(cp_err, "Warning: lower limit %d should be %d\n", down,
                majsize - 1);
        down = majsize - 1;
    }

    if (up == down)
        length = blocksize;
    else
        length = blocksize * (up - down + 1);

    /* Make up the new vector. */
    res = alloc(struct dvec);
    ZERO(res,struct dvec);
    res->v_name = mkcname('[', v->v_name, ind->v_name);
    res->v_type = v->v_type;
    res->v_flags = v->v_flags;

    res->v_defcolor = v->v_defcolor;
    res->v_gridtype = v->v_gridtype;
    res->v_plottype = v->v_plottype;
    res->v_length = length;
    res->v_numdims = newdim;
    if (up != down) {
	for (i = 0; i < newdim; i++)
	    res->v_dims[i] = v->v_dims[i];
        res->v_dims[0] = up - down + 1;
    } else {
	for (i = 0; i < newdim; i++)
	    res->v_dims[i] = v->v_dims[i + 1];
    }

    if (isreal(res))
        res->v_realdata = (double *) tmalloc(sizeof (double) * length);
    else
        res->v_compdata = (complex *) tmalloc(sizeof (complex) * length);

    /* And toss in the new data */
    for (j = 0; j < up - down + 1; j++) {
        if (rev)
            k = (up - down) - j;
        else
            k = j;
        for (i = 0; i < blocksize; i++)
            if (isreal(res))
                res->v_realdata[k * blocksize + i] =
                    v->v_realdata[(down + j) * blocksize + i];
            else {
                realpart(&res->v_compdata[k * blocksize + i]) =
                    realpart(&v->v_compdata[(down + j) * blocksize + i]);
                imagpart(&res->v_compdata[k * blocksize + i]) =
                    imagpart(&v->v_compdata[(down + j) * blocksize + i]);
            }
    }

    /* This is a problem -- the old scale will be no good.  I guess we
     * should make an altered copy of the old scale also.
     */
    /* Even though the old scale is no good and we should somehow decide
     * on a new scale, using the vector as its own scale is not the
     * solution.
     */
    /*
     * res->v_scale = res;
     */

    vec_new(res);
    return (res);
}

/* Apply a function to an argument. Complex functions are called as follows:
 *  cx_something(data, type, length, &newlength, &newtype),
 *  and returns a char * that is cast to complex or double.
 */

static struct dvec *
apply_func(func, arg)
    struct func *func;
    struct pnode *arg;
{
    struct dvec *v, *t, *newv = NULL, *end = NULL;
    int len, i;
    short type;
    char *data, buf[BSIZE_SP];

    /* Special case. This is not good -- happens when vm(), etc are used
     * and it gets caught as a user-definable function.  Usually v()
     * is caught in the parser.
     */
    if (!func->fu_func) {
        if (!arg->pn_value /* || (arg->pn_value->v_length != 1) XXX */) {
            fprintf(cp_err, "Error: bad v() syntax\n");
            return (NULL);
        }
        (void) sprintf(buf, "v(%s)", arg->pn_value->v_name);
        t = vec_fromplot(buf, plot_cur);
        if (!t) {
            fprintf(cp_err, "Error: no such vector %s\n", buf);
            return (NULL);
        }
        t = vec_copy(t);
        vec_new(t);
        return (t);
    }
    v = ft_evaluate(arg);
    if (v == NULL)
        return (NULL);


    for (; v; v = v->v_link2) {
#ifdef HAS_SIGS_AND_LJMP
        /* Some of the math routines generate SIGILL if the argument is
         * out of range.  Catch this here.
         */
        if (setjmp(matherrbuf)) {
            (void) signal(SIGILL, SIG_DFL);
            return (NULL);
        }
        (void) signal(SIGILL, (SIGNAL_FUNCTION) sig_matherr);
#endif

        if (eq(func->fu_name, "interpolate")
            || eq(func->fu_name, "deriv"))       /* Ack */
	{
            data = (char *) ((*func->fu_func) ((isreal(v) ? (char *)
                v->v_realdata : (char *) v->v_compdata),
                (short) (isreal(v) ? VF_REAL : VF_COMPLEX),
                v->v_length, &len, &type, v->v_plot,
                plot_cur, v->v_dims[0]));
        } else {
            data = (char *) ((*func->fu_func) ((isreal(v) ? (char *)
                v->v_realdata : (char *) v->v_compdata),
                (short) (isreal(v) ? VF_REAL : VF_COMPLEX),
                v->v_length, &len, &type));
	}
#ifdef HAS_SIGS_AND_LJMP
        /* Back to normal */
        (void) signal(SIGILL, SIG_DFL);
#endif

        if (!data)
            return (NULL);

        t = alloc(struct dvec);
	ZERO(t,struct dvec);

        t->v_flags = (v->v_flags & ~VF_COMPLEX & ~VF_REAL &
                ~VF_PERMANENT & ~VF_MINGIVEN & ~VF_MAXGIVEN);
        t->v_flags |= type;
#ifdef FTEDEBUG
        if (ft_evdb)
            fprintf(cp_err,
                "apply_func: func %s to %s len %d, type %d\n",
                    func->fu_name, v->v_name, len, type);
#endif
	if (isreal(t))
	    t->v_realdata = (double *) data;
	else
	    t->v_compdata = (complex *) data;
	if (eq(func->fu_name, "minus"))
	    t->v_name = mkcname('a', func->fu_name, v->v_name);
	else if (eq(func->fu_name, "not"))
	    t->v_name = mkcname('c', func->fu_name, v->v_name);
	else
	    t->v_name = mkcname('b', v->v_name, (char *) NULL);
	t->v_type = v->v_type; /* This is strange too. */
	t->v_length = len;
	t->v_scale = v->v_scale;

	/* Copy a few useful things */
	t->v_defcolor = v->v_defcolor;
	t->v_gridtype = v->v_gridtype;
	t->v_plottype = v->v_plottype;
	t->v_numdims = v->v_numdims;
	for (i = 0; i < t->v_numdims; i++)
	    t->v_dims[i] = v->v_dims[i];

	vec_new(t);

	if (end)
	    end->v_link2 = t;
	else
	    newv = t;
	end = t;
    }

    return (newv);
}

/* The unary minus operation. */

struct dvec *
op_uminus(arg)
    struct pnode *arg;
{
    return (apply_func(&func_uminus, arg));
}

struct dvec *
op_not(arg)
    struct pnode *arg;
{
    return (apply_func(&func_not, arg));
}

/* Create a reasonable name for the result of a function application, etc.
 * The what values 'a' and 'b' mean "make a function name" and "make a
 * unary minus", respectively.
 */

static char *
mkcname(what, v1, v2)
    char what;
    char *v1, *v2;
{
    char buf[BSIZE_SP], *s;

    if (what == 'a')
	(void) sprintf(buf, "%s(%s)", v1, v2);
    else if (what == 'b')
	(void) sprintf(buf, "-(%s)", v1);
    else if (what == 'c')
	(void) sprintf(buf, "~(%s)", v1);
    else if (what == '[')
	(void) sprintf(buf, "%s[%s]", v1, v2);
    else if (what == 'R')
	(void) sprintf(buf, "%s[[%s]]", v1, v2);
    else
	(void) sprintf(buf, "(%s)%c(%s)", v1, what, v2);
    s = copy(buf);
    return (s);
}

/* Operate on two vectors, and return a third with the data, length, and flags
 * fields filled in. Add it to the current plot and get rid of the two args.
 */

static struct dvec *
doop(what, func, arg1, arg2)
    char what;
    char *(*func)();
    struct pnode *arg1, *arg2;
{
    struct dvec *v1, *v2, *res;
    complex *c1, *c2, lc;
    double *d1, *d2, ld;
    int length, i;
    char *data;
    bool free1 = false, free2 = false, relflag = false;

    v1 = ft_evaluate(arg1);
    v2 = ft_evaluate(arg2);
    if (!v1 || !v2)
	return (NULL);

    /* Now the question is, what do we do when one or both of these
     * has more than one vector?  This is definitely not a good
     * thing.  For the time being don't do anything.
     */
    if (v1->v_link2 || v2->v_link2) {
	fprintf(cp_err, "Warning: no operations on wildcards yet.\n");
	if (v1->v_link2 && v2->v_link2)
	    fprintf(cp_err, "\t(You couldn't do that one anyway)\n");
	return (NULL);
    }

    /* How do we handle operations on multi-dimensional vectors?
     * For now, we only allow operations between one-D vectors,
     * equivalently shaped multi-D vectors, or a multi-D vector and
     * a one-D vector.  It's not at all clear what to do in the other cases.
     * So only check shape requirement if its an operation between two multi-D
     * arrays.
     */
    if ((v1->v_numdims > 1) && (v2->v_numdims > 1)) {
	if (v1->v_numdims != v2->v_numdims) {
	    fprintf(cp_err,
		"Warning: operands %s and %s have incompatible shapes.\n",
		v1->v_name, v2->v_name);
	    return (NULL);
	}
	for (i = 1; i < v1->v_numdims; i++) {
	    if ((v1->v_dims[i] != v2->v_dims[i])) {
		fprintf(cp_err,
		    "Warning: operands %s and %s have incompatible shapes.\n",
		    v1->v_name, v2->v_name);
		return (NULL);
	    }
	}
    }

    /* This is a bad way to do this. */
    switch (what) {
	case '=':
	case '>':
	case '<':
	case 'G':
	case 'L':
	case 'N':
	case '&':
	case '|':
	case '~':
	    relflag = true;
    }

    /* Don't bother to do type checking.  Maybe this should go in at
     * some point.
     */

    /* Make sure we have data of the same length. */
    length = ((v1->v_length > v2->v_length) ? v1->v_length : v2->v_length);
    if (v1->v_length < length) {
	free1 = true;
	if (isreal(v1)) {
	    ld = 0.0;
	    d1 = (double *) tmalloc(length * sizeof (double));
	    for (i = 0; i < v1->v_length; i++)
		d1[i] = v1->v_realdata[i];
	    if (length > 0)
		ld = v1->v_realdata[v1->v_length - 1];
	    for ( ; i < length; i++)
		d1[i] = ld;
	} else {
	    realpart(&lc) = 0.0;
	    imagpart(&lc) = 0.0;
	    c1 = (complex *) tmalloc(length * sizeof (complex));
	    for (i = 0; i < v1->v_length; i++)
		c1[i] = v1->v_compdata[i];
	    if (length > 0)
		lc = v1->v_compdata[v1->v_length - 1];
	    for ( ; i < length; i++)
		c1[i] = lc;
	}
    } else
	if (isreal(v1))
	    d1 = v1->v_realdata;
	else
	    c1 = v1->v_compdata;
    if (v2->v_length < length) {
	free2 = true;
	if (isreal(v2)) {
	    ld = 0.0;
	    d2 = (double *) tmalloc(length * sizeof (double));
	    for (i = 0; i < v2->v_length; i++)
		d2[i] = v2->v_realdata[i];
	    if (length > 0)
		ld = v2->v_realdata[v2->v_length - 1];
	    for ( ; i < length; i++)
		d2[i] = ld;
	} else {
	    realpart(&lc) = 0.0;
	    imagpart(&lc) = 0.0;
	    c2 = (complex *) tmalloc(length * sizeof (complex));
	    for (i = 0; i < v2->v_length; i++)
		c2[i] = v2->v_compdata[i];
	    if (length > 0)
		lc = v2->v_compdata[v1->v_length - 1];
	    for ( ; i < length; i++)
		c2[i] = lc;
	}
    } else
	if (isreal(v2))
	    d2 = v2->v_realdata;
	else
	    c2 = v2->v_compdata;

#ifdef HAS_SIGS_AND_LJMP
    /* Some of the math routines generate SIGILL if the argument is
     * out of range.  Catch this here.
     */
    if (setjmp(matherrbuf)) {
        return (NULL);
    }
    (void) signal(SIGILL, (SIGNAL_FUNCTION) sig_matherr);
#endif

    /* Now pass the vectors to the appropriate function. */
    data = (char *) ((*func) ((isreal(v1) ? (char *) d1 : (char *) c1),
                  (isreal(v2) ? (char *) d2 : (char *) c2),
                  (isreal(v1) ? VF_REAL : VF_COMPLEX),
                  (isreal(v2) ? VF_REAL : VF_COMPLEX),
                  length));
#ifdef HAS_SIGS_AND_LJMP
    /* Back to normal */
    (void) signal(SIGILL, SIG_DFL);
#endif

    if (!data)
	return (NULL);
    /* Make up the new vector. */
    res = alloc(struct dvec);
    ZERO(res,struct dvec);
    if (relflag || (isreal(v1) && isreal(v2) && (func != cx_comma))) {
        res->v_flags = (v1->v_flags | v2->v_flags |
                        VF_REAL) & ~ VF_COMPLEX;
        res->v_realdata = (double *) data;
    } else {
        res->v_flags = (v1->v_flags | v2->v_flags |
                        VF_COMPLEX) & ~ VF_REAL;
        res->v_compdata = (complex *) data;
    }

    res->v_name = mkcname(what, v1->v_name, v2->v_name);
    res->v_length = length;

    /* This is a non-obvious thing */
    if (v1->v_scale != v2->v_scale) {
        fprintf(cp_err, "Warning: scales of %s and %s are different.\n",
                v1->v_name, v2->v_name);
        res->v_scale = NULL;
    } else
        res->v_scale = v1->v_scale;

    /* Copy a few useful things */
    res->v_defcolor = v1->v_defcolor;
    res->v_gridtype = v1->v_gridtype;
    res->v_plottype = v1->v_plottype;

    /* Copy dimensions. */
    if (v1->v_numdims > v2->v_numdims) {
	res->v_numdims = v1->v_numdims;
	for (i = 0; i < v1->v_numdims; i++)
	    res->v_dims[i] = v1->v_dims[i];
    } else {
	res->v_numdims = v2->v_numdims;
	for (i = 0; i < v2->v_numdims; i++)
	    res->v_dims[i] = v2->v_dims[i];
    }

    /* This depends somewhat on what the operation is.  XXX Should fix */
    res->v_type = v1->v_type;
    vec_new(res);

    /* Free the temporary data areas we used, if we allocated any. */
    if (free1) {
        if (isreal(v1)) {
            tfree(d1);
        } else {
            tfree(c1);
        }
    }
    if (free2) {
        if (isreal(v2)) {
            tfree(d2);
        } else {
            tfree(c2);
        }
    }

    return (res);
}

