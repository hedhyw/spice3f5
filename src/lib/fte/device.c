/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1986 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Routines to query and alter devices.
 */

#include "spice.h"
#include "gendefs.h"
#include "cktdefs.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "dgen.h"

static wordlist *devexpand();

/*
 *	show: list device operating point info
 *		show
 *		show devs : params
 *		show devs : params ; devs : params
 *		show dev dev dev : param param param , dev dev : param param
 *		show t : param param param, t : param param
 *
 */

int	printstr( );
int	listparam( );
int	printvals( );
int	bogus1( );
int	bogus2( );
void	old_show( );
static void all_show( );

static	int	count;

void
com_showmod(wl)
    wordlist *wl;
{
    all_show(wl, 1);
}

void
com_show(wl)
    wordlist *wl;
{
    all_show(wl, 0);
}

static void
all_show(wl, mode)
    wordlist *wl;
    int mode;
{
    wordlist	*params, *nextgroup, *thisgroup;
    wordlist	*prev, *next, *w;
    int		screen_width;
    dgen	*dg, *listdg;
    int		instances;
    int		i, j, n;
    int		param_flag, dev_flag;

    if (!ft_curckt) {
        fprintf(cp_err, "Error: no circuit loaded\n");
        return;
    }

    if (wl && wl->wl_word && eq(wl->wl_word, "-v")) {
	old_show(wl->wl_next);
	return;
    }

    if (!cp_getvar("width", VT_NUM, (char *) &screen_width))
	    screen_width = DEF_WIDTH;
    count = screen_width / 11 - 1;

    n = 0;
    do {
	prev = NULL;
	params = NULL;
	nextgroup = NULL;
	thisgroup = wl;
	param_flag = 0;
	dev_flag = 0;

	/* find the parameter list and the nextgroup */
	for (w = wl; w && !nextgroup; w = next) {
	    next = w->wl_next;
	    if (eq(w->wl_word, "++") || eq(w->wl_word, "all")) {
		if (params) {
			param_flag = DGEN_ALLPARAMS;
			if (prev)
				prev->wl_next = w->wl_next;
			else
				params = next;
		} else {
			dev_flag = DGEN_ALLDEVS;
			if (prev)
				prev->wl_next = w->wl_next;
			else
				thisgroup = next;
		}
		/*
		tfree(w->wl_word);
		tfree(w);
		*/
		w = NULL;
	    } else if (eq(w->wl_word, "+")) {
		if (params) {
			param_flag = DGEN_DEFPARAMS;
			if (prev)
				prev->wl_next = w->wl_next;
			else
				params = next;
		} else {
			dev_flag = DGEN_DEFDEVS;
			if (prev)
				prev->wl_next = w->wl_next;
			else
				thisgroup = next;
		}
		tfree(w->wl_word);
		tfree(w);
		w = NULL;
	    } else if (eq(w->wl_word, ":")) {
		tfree(w->wl_word);
		tfree(w);
		w = NULL;
		if (!params) {
		    params = next;
		    if (prev)
			    prev->wl_next = NULL;
		    else
			    thisgroup = NULL;
		} else {
		    if (prev)
			prev->wl_next = next;
		    else
			params = next;
		}
	    } else if (eq(w->wl_word, ";") || eq(w->wl_word, ",")) {
		    nextgroup = next;
		    tfree(w->wl_word);
		    tfree(w);
		    w = NULL;
		    if (prev)
			prev->wl_next = NULL;
		    break;
	    }
	    prev = w;
	}

	instances = 0;
	for (dg = dgen_init(ft_curckt->ci_ckt, thisgroup, 1, dev_flag, mode);
		dg; dgen_nth_next(&dg, count))
	{
	    instances = 1;
	    if (dg->flags & DGEN_INSTANCE) {
		instances = 2;
		printf(" %s: %s\n",
			ft_sim->devices[dg->dev_type_no]->name,
			ft_sim->devices[dg->dev_type_no]->description);
		n += 1;

		i = 0;
		do {
			printf(" device   ");
			j = dgen_for_n(dg, count, printstr, "n", i);
			i += 1;
			printf("\n");
		} while (j);

		if (ft_sim->devices[dg->dev_type_no]->numModelParms) {
			i = 0;
			do {
				printf(" model    ");
				j = dgen_for_n(dg, count, printstr, "m", i);
				i += 1;
				printf("\n");
			} while (j);
		}
		listdg = dg;

		if (param_flag)
		    param_forall(dg, param_flag);
		else if (!params)
		    param_forall(dg, DGEN_DEFPARAMS);
		if (params)
		    wl_forall(params, listparam, dg);
		printf("\n");

	    } else if (ft_sim->devices[dg->dev_type_no]->numModelParms) {
		printf(" %s models (%s)\n",
			ft_sim->devices[dg->dev_type_no]->name,
			ft_sim->devices[dg->dev_type_no]->description);
		n += 1;
		i = 0;
		do {
			printf(" model    ");
			j = dgen_for_n(dg, count, printstr, "m", i);
			i += 1;
			printf("\n");
		} while (j);
		printf("\n");

		if (param_flag)
		    param_forall(dg, param_flag);
		else if (!params)
		    param_forall(dg, DGEN_DEFPARAMS);
		if (params)
		    wl_forall(params, listparam, dg);
		printf("\n");
	    }
	}

	wl = nextgroup;

    } while (wl);

    if (!n) {
	    if (instances == 0)
		printf("No matching instances or models\n");
	    else if (instances == 1)
		printf("No matching models\n");
	    else
		printf("No matching elements\n");
    }
}

int
printstr(dg, name)
    dgen	*dg;
    char	*name;
{
    if (*name == 'n') {
	if (dg->instance)
	    printf(" % 9.9s", dg->instance->GENname);
	else
	    printf(" <???????>");
    } else if (*name == 'm') {
	if (dg->model)
	    printf(" % 9.9s", dg->model->GENmodName);
	else
	    printf(" <???????>");
    } else
	printf("  <error> ");

    return 0;
}

param_forall(dg, flags)
    dgen *dg;
    int	flags;
{
    int	i, j, k, found;
    int xcount;
    IFparm *plist;

    found = 0;

    if (dg->flags & DGEN_INSTANCE) {
	xcount = *ft_sim->devices[dg->dev_type_no]->numInstanceParms;
	plist = ft_sim->devices[dg->dev_type_no]->instanceParms;
    } else {
	xcount = *ft_sim->devices[dg->dev_type_no]->numModelParms;
	plist = ft_sim->devices[dg->dev_type_no]->modelParms;
    }

    for (i = 0; i < xcount; i++) {
	if (plist[i].dataType & IF_ASK) {
	    if ((((CKTcircuit *) (dg->ckt))->CKTrhsOld
		|| (plist[i].dataType & IF_SET))
		&& (!(plist[i].dataType & (IF_REDUNDANT | IF_UNINTERESTING))
		|| (flags == DGEN_ALLPARAMS
		&& !(plist[i].dataType & IF_REDUNDANT))))
	    {
		j = 0;
		do {
			if (!j)
			    printf("%10.10s", plist[i].keyword);
			else
			    printf("          ");
			k = dgen_for_n(dg, count, printvals,
				(char *) (plist + i), j);
			printf("\n");
			j += 1;
		} while (k);
	    }
	}
    }
}

listparam(p, dg)
	wordlist *p;
	dgen *dg;
{
    int	i, j, k, found;
    int	xcount;
    IFparm *plist;

    found = 0;

    if (dg->flags & DGEN_INSTANCE) {
	xcount = *ft_sim->devices[dg->dev_type_no]->numInstanceParms;
	plist = ft_sim->devices[dg->dev_type_no]->instanceParms;
    } else {
	xcount = *ft_sim->devices[dg->dev_type_no]->numModelParms;
	plist = ft_sim->devices[dg->dev_type_no]->modelParms;
    }

    for (i = 0; i < xcount; i++) {
	if (eqc(p->wl_word, plist[i].keyword) && (plist[i].dataType & IF_ASK))
	{
	    found = 1;
	    break;
	}
    }

    if (found) {
	if ((((CKTcircuit *) (dg->ckt))->CKTrhsOld
	    || (plist[i].dataType & IF_SET)))
	{
	    j = 0;
	    do {
		if (!j)
		    printf("%10.10s", p->wl_word);
		else
		    printf("          ");
		k = dgen_for_n(dg, count, printvals, plist + i, j);
		printf("\n");
		j += 1;
	    } while (k > 0);
	} else {
	    j = 0;
	    do {
		if (!j)
		    printf("%10.10s", p->wl_word);
		else
		    printf("          ");
		k = dgen_for_n(dg, count, bogus1, 0, j);
		printf("\n");
		j += 1;
	    } while (k > 0);
	}
    } else {
	j = 0;
	do {
	    if (!j)
		printf("%10.10s", p->wl_word);
	    else
		printf("          ");
	    k = dgen_for_n(dg, count, bogus2, 0, j);
	    printf("\n");
	    j += 1;
	} while (k > 0);
    }
}

bogus1(dg)
    dgen	*dg;
{
    printf(" ---------");
    return 0;
}

bogus2(dg)
    dgen	*dg;
{
    printf(" ?????????");
    return 0;
}

int
printvals(dg, p, i)
	dgen	*dg;
	IFparm	*p;
	int	i;
{
    IFvalue	val;
    int		n;
    IFvalue	*vp;

    if (dg->flags & DGEN_INSTANCE)
	(*ft_sim->askInstanceQuest)(ft_curckt->ci_ckt, dg->instance,
	    p->id, &val, &val);
    else
	(*ft_sim->askModelQuest)(ft_curckt->ci_ckt, dg->model,
	    p->id, &val, &val);

    if (p->dataType & IF_VECTOR)
	n = val.v.numValue;
    else
	n = 1;

    if (((p->dataType & IF_VARTYPES) & ~IF_VECTOR) == IF_COMPLEX)
	n *= 2;

    if (i >= n) {
	if (i == 0)
	    printf("         -");
	else
	    printf("          ");
	return 0;
    }

    if (p->dataType & IF_VECTOR) {
	switch ((p->dataType & IF_VARTYPES) & ~IF_VECTOR) {
	    case IF_FLAG:
		    printf(" % 9d", val.v.vec.iVec[i]);
		    break;
	    case IF_INTEGER:
		    printf(" % 9d", val.v.vec.iVec[i]);
		    break;
	    case IF_REAL:
		    printf(" % 9.3g", val.v.vec.rVec[i]);
		    break;
	    case IF_COMPLEX:
		    if (!(i % 2))
			    printf(" % 9.3g", val.v.vec.cVec[i / 2].real);
		    else
			    printf(" % 9.3g", val.v.vec.cVec[i / 2].imag);
		    break;
	    case IF_STRING:
		    printf(" % 9.9s", val.v.vec.sVec[i]);
		    break;
	    case IF_INSTANCE:
		    printf(" % 9.9s", val.v.vec.uVec[i]);
		    break;
	    default:
		    printf(" ******** ");
	}
    } else {
	switch ((p->dataType & IF_VARTYPES) & ~IF_VECTOR) {
	    case IF_FLAG:
		    printf(" % 9d", val.iValue);
		    break;
	    case IF_INTEGER:
		    printf(" % 9d", val.iValue);
		    break;
	    case IF_REAL:
		    printf(" % 9.3g", val.rValue);
		    break;
	    case IF_COMPLEX:
		    if (i % 2)
			    printf(" % 9.3g", val.cValue.real);
		    else
			    printf(" % 9.3g", val.cValue.imag);
		    break;
	    case IF_STRING:
		    printf(" % 9.9s", val.sValue);
		    break;
	    case IF_INSTANCE:
		    printf(" % 9.9s ", val.uValue);
		    break;
	    default:
		    printf(" ******** ");
	}
    }

    return n - 1;
}


/* (old "show" command)
 * Display various device parameters.  The syntax of this command is 
 *   show devicelist : parmlist
 * where devicelist can be "all", the name of a device, a string like r*,
 * which means all devices with names that begin with 'r', repeated one
 * or more times.   The parms are names of parameters that are (hopefully)
 * valid for all the named devices, or "all".
 */

void
old_show(wl)
    wordlist *wl;
{
    wordlist *devs, *parms, *tw, *ww;
    struct variable *v;
    char *nn;

    devs = wl;
    while (wl && !eq(wl->wl_word, ":"))
        wl = wl->wl_next;
    if (!wl)
        parms = NULL;
    else {
        if (wl->wl_prev)
            wl->wl_prev->wl_next = NULL;
        parms = wl->wl_next;
        if (parms)
            parms->wl_prev = NULL;
    }

    /* Now expand the devicelist... */
    for (tw = NULL; devs; devs = devs->wl_next) {
        inp_casefix(devs->wl_word);
        tw = wl_append(tw, devexpand(devs->wl_word));
    }
    
    devs = tw;
    for (tw = parms; tw; tw = tw->wl_next)
        if (eq(tw->wl_word, "all"))
            break;
    if (tw)
        parms = NULL;

    /* This is a crock... */
    if (!devs)
        devs = cp_cctowl(ft_curckt->ci_devices);

    out_init();

    while (devs) {
        out_printf("%s:\n", devs->wl_word);
        if (parms) {
            for (tw = parms; tw; tw = tw->wl_next) {
                nn = copy(devs->wl_word);
                v = (*if_getparam)(ft_curckt->ci_ckt,
			&nn, tw->wl_word, 0, 0);
		if (!v)
		    v = (*if_getparam)(ft_curckt->ci_ckt,
			    &nn, tw->wl_word, 0, 1);
                if (v) {
                    out_printf("\t%s =", tw->wl_word);
                    for (ww = cp_varwl(v); ww; ww =
                            ww->wl_next)
                        out_printf(" %s", ww->wl_word);
                    out_send("\n");
                }
            }
        } else {
            nn = copy(devs->wl_word);
            v = (*if_getparam)(ft_curckt->ci_ckt, &nn, "all", 0, 0);
	    if (!v)
		v = (*if_getparam)(ft_curckt->ci_ckt, &nn, "all", 0, 1);
            while (v) {
                out_printf("\t%s =", v->va_name);
                for (ww = cp_varwl(v); ww; ww = ww->wl_next)
                    out_printf(" %s", ww->wl_word);
                out_send("\n");
                v = v->va_next;
            }
        }
        devs = devs->wl_next;
    }
    return;
}

/* Alter a device parameter.  The new syntax here is
 *	alter @device[parameter] = expr
 *	alter device = expr
 *	alter device parameter = expr
 * expr must be real (complex isn't handled right now, integer is fine though,
 * but no strings ... for booleans, use 0/1).
 */

void
com_alter(wl)
    wordlist *wl;
{
    if (!wl) {
	fprintf(cp_err, "usage: alter dev param = expression\n");
	fprintf(cp_err, "  or   alter @dev[param] = expression\n");
	fprintf(cp_err, "  or   alter dev = expression\n");
	return;
    }
    com_alter_common(wl, 0);
}

com_altermod(wl)
    wordlist *wl;
{
    com_alter_common(wl, 1);
}

com_alter_common(wl, do_model)
    wordlist *wl;
    int	do_model;
{
#ifdef notdef
    struct variable var, *nv, *prev;
    double *dd;
#endif
    char *dev, *p;
    char *param;
    struct dvec *dv;
    struct pnode *names, *n2;

    if (!ft_curckt) {
        fprintf(cp_err, "Error: no circuit loaded\n");
        return;
    }

    names = ft_getpnames(wl, false);

    /* device parameter = expr, param = expr
       @dev[param] = expr
     */

    dev = NULL;
    if (names->pn_value) {
	/* first item is device; move forward and procede */
	dev = names->pn_name;
	/* Hack off trailing whitespace... parser problem */
	for (p = dev + strlen(dev) - 1; p > dev && *p <= ' '; p--)
	    *p = 0;
	n2 = names->pn_next;
    } else if (names->pn_func) {
	fprintf(cp_err, "alter: don't understand \"%s\"\n", names->pn_name);
	fprintf(cp_err, "usage: alter dev param = expression\n");
	fprintf(cp_err, "  or   alter @dev[param] = expression\n");
	fprintf(cp_err, "  or   alter dev = expression\n");
	free_pnode(names);
	return;
    } else {
	n2 = names;
    }

    if (!n2 || !eq(n2->pn_op->op_name, "=")) {
	fprintf(cp_err, "alter: don't understand \"%s\"\n", names->pn_name);
	fprintf(cp_err, "usage: alter dev param = expression\n");
	fprintf(cp_err, "  or   alter @dev[param] = expression\n");
	fprintf(cp_err, "  or   alter dev = expression\n");
	free_pnode(names);
	return;
    }

    /* extrace device/param name from left side */
    if (!n2->pn_left->pn_value || n2->pn_next) {
	fprintf(cp_err, "alter: don't understand \"%s\"\n", names->pn_name);
	fprintf(cp_err, "usage: alter dev param = expression\n");
	fprintf(cp_err, "  or   alter @dev[param] = expression\n");
	fprintf(cp_err, "  or   alter dev = expression\n");
	free_pnode(names);
	return;
    }

    param = NULL;

    if (dev)
	param = n2->pn_left->pn_value->v_name;
    else {
	/* extract @dev[param] */
	p = n2->pn_left->pn_value->v_name;
	if (*p == '@') {
	    dev = p + 1;
	    p = index(p, '[');
	    if (p) {
		*p++ = 0;
		param = p;
		p = index(p, '[');
		if (p)
		    *p = 0;
	    }
	} else {
	    dev = p;
	}
    }

    dv = ft_evaluate(n2->pn_right);
    free_pnode(names);
    if (!dv)
	return;
    if (dv->v_length < 1) {
	fprintf(cp_err, "alter: RHS has zero length\n");
	return;
    }

    if_setparam(ft_curckt->ci_ckt, &dev, param, dv, do_model);

    free_pnode(names);

    /* Vector data (dv) should get garbage-collected. */

    return;

#ifdef notdef
    while (wl) {
	param = wl->wl_word;
	wl = wl->wl_next;

	if (!wl) {
	    val = param;
	    param = NULL;
	} else {
	    val = wl->wl_word;
	    wl = wl->wl_next;
	}

	/* Now figure out what the value should be... */
	if (eq(val, "true")) {
	    var.va_type = VT_BOOL;
	    var.va_bool = true;
	} else if (eq(val, "false")) { 
	    var.va_type = VT_BOOL;
	    var.va_bool = false;
	} else if (eq(val, "[")) { 
	    var.va_type = VT_LIST;
	    prev = NULL;
	    while (wl && !eq(wl->wl_word, "]")) {
		val = wl->wl_word;
		nv = alloc(struct variable);
		if (dd = ft_numparse(&val, false)) {
			nv->va_type = VT_REAL;
			nv->va_real = *dd;
		} else {
			fprintf(cp_err, "Error: \"%s\" is not a number\n", val);
			break;
		}
		if (!prev)
		    var.va_vlist = nv;
		else
		    prev->va_next = nv;
		nv->va_next = NULL;
		wl = wl->wl_next;
		prev = nv;
	    }
	    if (wl && eq(wl->wl_word, "]")) {
		wl = wl->wl_next;
	    } else {
		while (nv) {
			prev = nv->va_next;
			tfree(nv);
			nv = prev;
		}
		return;
	    }
	} else if (dd = ft_numparse(&val, false)) {
	    var.va_type = VT_REAL;
	    var.va_real = *dd;
	} else {
	    var.va_type = VT_STRING;
	    var.va_string = val;
	}

        if_setparam(ft_curckt->ci_ckt, &dev, param, &var, do_model);

	if (var.va_type == VT_LIST) {
	    for (nv = var.va_vlist; nv; nv = prev) {
		prev = nv->va_next;
		tfree(nv);
	    }
	}

    }
#endif

}

/* Given a device name, possibly with wildcards, return the matches. */

static wordlist *
devexpand(name)
    char *name;
{
    wordlist *wl, *devices, *tw;

    if (index(name, '*') || index(name, '[') || index(name, '?')) {
        devices = cp_cctowl(ft_curckt->ci_devices);
        for (wl = NULL; devices; devices = devices->wl_next)
            if (cp_globmatch(name, devices->wl_word)) {
                tw = alloc(struct wordlist);
                if (wl) {
                    wl->wl_prev = tw;
                    tw->wl_next = wl;
                    wl = tw;
                } else
                    wl = tw;
                wl->wl_word = devices->wl_word;
            }
    } else if (eq(name, "all")) {
        wl = cp_cctowl(ft_curckt->ci_devices);
    } else {
        wl = alloc(struct wordlist);
        wl->wl_word = name;
    }
    wl_sort(wl);
    return (wl);
}

