/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Interface routines. These are specific to spice. The only changes to FTE
 * that should be needed to make FTE work with a different simulator is
 * to rewrite this file. What each routine is expected to do can be
 * found in the programmer's manual. This file should be the only one
 * that includes spice header files.
 */

#include "spice.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "fteinp.h"
#include "fteconst.h"
#include "inpdefs.h"
#include "iferrmsg.h"
#include "ifsim.h"
#include "util.h"
#include "suffix.h"

static IFvalue *doask();
static int doset( );
static int finddev();
static IFparm * parmlookup();
static struct variable *parmtovar();

/* Input a single deck, and return a pointer to the circuit. */

char *
if_inpdeck(deck, tab)
    struct line *deck;
    INPtables **tab;
{
    GENERIC *ckt;
    int err, i, j;
    struct line *ll;
    IFuid taskUid;
    IFuid optUid;
    int which = -1;

    for (i = 0, ll = deck; ll; ll = ll->li_next)
        i++;
    *tab = INPtabInit(i);
    ft_curckt->ci_symtab = *tab;
    if ((err = (*(ft_sim->newCircuit))(&ckt))
            != OK) {
        ft_sperror(err, "CKTinit");
        return (NULL);
    }
    err = IFnewUid(ckt,&taskUid,(IFuid)NULL,"default",UID_TASK,(GENERIC**)NULL);
    if(err) {
        ft_sperror(err,"newUid");
        return(NULL);
    }
    err = (*(ft_sim->newTask))(ckt,(GENERIC**)&(ft_curckt->ci_defTask),taskUid);
    if(err) {
        ft_sperror(err,"newTask");
        return(NULL);
    }
    for(j=0;j<ft_sim->numAnalyses;j++) {
        if(strcmp(ft_sim->analyses[j]->name,"options")==0) {
            which = j;
            break;
        }
    } 
    if(which != -1) {
        err = IFnewUid(ckt,&optUid,(IFuid)NULL,"options",UID_ANALYSIS,
                (GENERIC**)NULL);
        if(err) {
            ft_sperror(err,"newUid");
            return(NULL);
        }
        err = (*(ft_sim->newAnalysis))(ft_curckt->ci_ckt,which,optUid,
                (GENERIC**)&(ft_curckt->ci_defOpt),
                (GENERIC*)ft_curckt->ci_defTask);
        if(err) {
            ft_sperror(err,"createOptions");
            return(NULL);
        }
        ft_curckt->ci_curOpt  = ft_curckt->ci_defOpt;
    }
    ft_curckt->ci_curTask = ft_curckt->ci_defTask;
    INPpas1((GENERIC *) ckt, (card *) deck->li_next,(INPtables *)*tab);
    INPpas2((GENERIC *) ckt, (card *) deck->li_next,
            (INPtables *) *tab,ft_curckt->ci_defTask);
    INPkillMods();
    return (ckt);
}

/* Do a run of the circuit, of the given type. Type "resume" is special --
 * it means to resume whatever simulation that was in progress. The
 * return value of this routine is 0 if the exit was ok, and 1 if there was
 * a reason to interrupt the circuit (interrupt typed at the keyboard,
 * error in the simulation, etc). args should be the entire command line,
 * e.g. "tran 1 10 20 uic"
 */

int
if_run(t, what, args, tab)
    char *t;
    char *what;
    wordlist *args;
    char *tab;
{
    GENERIC *ckt = (GENERIC *) t;
    int err;
    struct line deck;
    char buf[BSIZE_SP];
    int j;
    int which = -1;
    IFuid specUid,optUid;

    /* First parse the line... */
    if (eq(what, "tran") || eq(what, "ac") || eq(what, "dc")
            || eq(what, "op") || eq(what, "pz") || eq(what,"disto")
            || eq(what, "adjsen") || eq(what, "sens") || eq(what,"tf")
	    || eq(what, "noise")) {
        (void) sprintf(buf, ".%s", wl_flatten(args));
        deck.li_next = deck.li_actual = NULL;
        deck.li_error = NULL;
        deck.li_linenum = 0;
        deck.li_line = buf;
        if(ft_curckt->ci_specTask) {
            err=(*(ft_sim->deleteTask))(ft_curckt->ci_ckt,
                    ft_curckt->ci_specTask);
            if(err) {
                ft_sperror(err,"deleteTask");
                return(2);
            }
        }
        err = IFnewUid(ft_curckt->ci_ckt,&specUid,(IFuid)NULL,"special",
                UID_TASK,(GENERIC**)NULL);
        if(err) {
            ft_sperror(err,"newUid");
            return(2);
        }
        err = (*(ft_sim->newTask))(ft_curckt->ci_ckt,
                (GENERIC**)&(ft_curckt->ci_specTask),specUid);
        if(err) {
            ft_sperror(err,"newTask");
            return(2);
        }
        for(j=0;j<ft_sim->numAnalyses;j++) {
            if(strcmp(ft_sim->analyses[j]->name,"options")==0) {
                which = j;
                break;
            }
        } 
        if(which != -1) {
            err = IFnewUid(ft_curckt->ci_ckt,&optUid,(IFuid)NULL,"options",
                    UID_ANALYSIS,(GENERIC**)NULL);
            if(err) {
                ft_sperror(err,"newUid");
                return(2);
            }
            err = (*(ft_sim->newAnalysis))(ft_curckt->ci_ckt,which,optUid,
                    (GENERIC**)&(ft_curckt->ci_specOpt),
                    (GENERIC*)ft_curckt->ci_specTask);
            if(err) {
                ft_sperror(err,"createOptions");
                return(2);
            }
            ft_curckt->ci_curOpt  = ft_curckt->ci_specOpt;
        }
        ft_curckt->ci_curTask = ft_curckt->ci_specTask;
        INPpas2(ckt, (card *) &deck, (INPtables *)tab, ft_curckt->ci_specTask);
        if (deck.li_error) {
            fprintf(cp_err, "Warning: %s\n", deck.li_error);
	    return 2;
        }
    }
    if( eq(what,"run") ) {
        ft_curckt->ci_curTask = ft_curckt->ci_defTask;
        ft_curckt->ci_curOpt = ft_curckt->ci_defOpt;
    }

    if (  (eq(what, "tran"))  ||
          (eq(what, "ac"))  ||
          (eq(what, "dc")) ||
          (eq(what, "op")) ||
          (eq(what, "pz")) ||
          (eq(what, "disto")) ||
          (eq(what, "noise")) ||
            eq(what, "adjsen") || eq(what, "sens") || eq(what,"tf") ||
          (eq(what, "run"))     )  {
        if ((err = (*(ft_sim->doAnalyses))(ckt, 1, ft_curckt->ci_curTask))!=OK){
            ft_sperror(err, "doAnalyses");
            /* wrd_end(); */
	    if (err == E_PAUSE)
		return (1);
	    else
		return (2);
        }
    } else if (eq(what, "resume")) {
        if ((err = (*(ft_sim->doAnalyses))(ckt, 0, ft_curckt->ci_curTask))!=OK){
            ft_sperror(err, "doAnalyses");
            /* wrd_end(); */
	    if (err == E_PAUSE)
		return (1);
	    else
		return (2);
        }
    } else {
        fprintf(cp_err, "if_run: Internal Error: bad run type %s\n",
                what);
	return (2);
    }
    return (0);
}

/* Set an option in the circuit. Arguments are option name, type, and
 * value (the last a char *), suitable for casting to whatever needed...
 */

static char *unsupported[] = {
    "itl3",
    "itl5",
    "lvltim",
    "maxord",
    "method",
    NULL
} ;

static char *obsolete[] = {
    "limpts",
    "limtim",
    "lvlcod",
    NULL
} ;

int
if_option(ckt, name, type, value)
    char *ckt;
    char *name;
    int type;
    char *value;
{
    IFvalue pval;
    int err, i;
    GENERIC *cc = (GENERIC *) ckt;
    char **vv;
    int which = -1;

    if (eq(name, "acct")) {
        ft_acctprint = true;
	return 0;
    } else if (eq(name, "list")) {
        ft_listprint = true;
	return 0;
    } else if (eq(name, "node")) {
        ft_nodesprint = true;
	return 0;
    } else if (eq(name, "opts")) {
        ft_optsprint = true;
	return 0;
    } else if (eq(name, "nopage")) {
	ft_nopage = true;
	return 0;
    } else if (eq(name, "nomod")) {
	ft_nomod = true;
	return 0;
    }

    for(i=0;i<ft_sim->numAnalyses;i++) {
        if(strcmp(ft_sim->analyses[i]->name,"options")==0) {
            which = i;
            break;
        }
    }
    if(which==-1) {
        fprintf(cp_err,"Warning:  .options line unsupported\n");
        return 0;
    }

    for (i = 0; i < ft_sim->analyses[which]->numParms; i++)
        if (eq(ft_sim->analyses[which]->analysisParms[i].keyword, name) &&
                (ft_sim->analyses[which]->analysisParms[i].dataType & IF_SET))
            break;
    if (i == ft_sim->analyses[which]->numParms) {
        /* See if this is unsupported or obsolete. */
        for (vv = unsupported; *vv; vv++)
            if (eq(name, *vv)) {
                fprintf(cp_err, 
            "Warning: option %s is currently unsupported.\n", name);
                return 1;
            }
        for (vv = obsolete; *vv; vv++)
            if (eq(name, *vv)) {
                fprintf(cp_err, 
                "Warning: option %s is obsolete.\n", name);
                return 1;
            }
        return 0;
    }

    switch (ft_sim->analyses[which]->analysisParms[i].dataType & IF_VARTYPES) {
        case IF_REAL:
            if (type == VT_REAL)
                pval.rValue = *((double *) value);
            else if (type == VT_NUM)
                pval.rValue = *((int *) value);
            else
                goto badtype;
            break;
        case IF_INTEGER:
            if (type == VT_NUM)
                pval.iValue = *((int *) value);
            else if (type == VT_REAL)
                pval.iValue = *((double *) value);
            else
                goto badtype;
            break;
        case IF_STRING:
            if (type == VT_STRING)
                pval.sValue = copy(value);
            else
                goto badtype;
            break;
        case IF_FLAG:
            /* Do nothing. */
            pval.iValue = *((int *) value);
            break;
        default:
            fprintf(cp_err, 
            "if_option: Internal Error: bad option type %d.\n",
                    ft_sim->analyses[which]->analysisParms[i].dataType);
    }

    if (!ckt) {
	/* XXX No circuit loaded */
	fprintf(cp_err, "Simulation parameter \"%s\" can't be set until\n",
		name);
	fprintf(cp_err, "a circuit has been loaded.\n");
	return 1;
    }

    if ((err = (*(ft_sim->setAnalysisParm))(cc, (GENERIC *)ft_curckt->ci_curOpt,
            ft_sim->analyses[which]->analysisParms[i].id, &pval,
            (IFvalue *)NULL)) != OK)
        ft_sperror(err, "setAnalysisParm(options)");
    return 1;
badtype:
    fprintf(cp_err, "Error: bad type given for option %s --\n", name);
    fprintf(cp_err, "\ttype given was ");
    switch (type) {
        case VT_BOOL:   fputs("boolean", cp_err); break;
        case VT_NUM:    fputs("integer", cp_err); break;
        case VT_REAL:   fputs("real", cp_err); break;
        case VT_STRING: fputs("string", cp_err); break;
        case VT_LIST:   fputs("list", cp_err); break;
        default:    fputs("something strange", cp_err); break;
    }
    fprintf(cp_err, ", type expected was ");
    switch(ft_sim->analyses[which]->analysisParms[i].dataType & IF_VARTYPES) {
        case IF_REAL:   fputs("real.\n", cp_err); break;
        case IF_INTEGER:fputs("integer.\n", cp_err); break;
        case IF_STRING: fputs("string.\n", cp_err); break;
        case IF_FLAG:   fputs("flag.\n", cp_err); break;
        default:    fputs("something strange.\n", cp_err); break;
    }
    if (type == VT_BOOL)
fputs("\t(Note that you must use an = to separate option name and value.)\n", 
                    cp_err); 
    return 0;
}

/* ARGSUSED */
void
if_dump(ckt, file)
    char *ckt;
    FILE *file;
{
    /*GENERIC *cc = (GENERIC *) ckt;*/

    fprintf(file,"diagnostic output dump unavailable.");
    return;
}

void
if_cktfree(ckt, tab)
    char *ckt;
    char *tab;
{
    GENERIC *cc = (GENERIC *) ckt;

    (*(ft_sim->deleteCircuit))(cc);
    INPtabEnd((INPtables *) tab);
    return;
}

/* Return a string describing an error code. */


/* BLOW THIS AWAY.... */

char *
if_errstring(code)
    int code;
{
    return (INPerror(code));
}

/* Get a parameter value from the circuit. If name is left unspecified,
 * we want a circuit parameter.
 */

struct variable *
spif_getparam(ckt, name, param, ind, do_model)
    char *ckt;
    char **name;
    char *param;
    int ind;
    int do_model;
{
    struct variable *vv = NULL, *tv;
    IFvalue *pv;
    IFparm *opt;
    int typecode, i;
    GENinstance *dev=(GENinstance *)NULL;
    GENmodel *mod=(GENmodel *)NULL;
    IFdevice *device;

    /* fprintf(cp_err, "Calling if_getparam(%s, %s)\n", *name, param); */

    if (param && eq(param, "all")) {
        INPinsert(name,(INPtables *)ft_curckt->ci_symtab);
        typecode = finddev(ckt, *name,(GENERIC**) &dev,(GENERIC**) &mod);
        if (typecode == -1) {
            fprintf(cp_err,
                "Error: no such device or model name %s\n",
                    *name);
            return (NULL);
        }
        device = ft_sim->devices[typecode];
        for (i = 0; i < *(device->numInstanceParms); i++) {
            opt = &device->instanceParms[i];
            if(opt->dataType & IF_REDUNDANT || !opt->description)
		    continue;
            if(!(opt->dataType & IF_ASK)) continue;
            pv = doask(ckt, typecode, dev, mod, opt, ind);
            if (pv) {
                tv = parmtovar(pv, opt);
                if (vv)
                    tv->va_next = vv;
                vv = tv;
            } else
                fprintf(cp_err,
            "Internal Error: no parameter '%s' on device '%s'\n",
                    device->instanceParms[i].keyword,
                    device->name);
        }
        return (vv);
    } else if (param) {
        INPinsert(name,(INPtables *)ft_curckt->ci_symtab);
        typecode = finddev(ckt, *name, (GENERIC**)&dev, (GENERIC**)&mod);
        if (typecode == -1) {
            fprintf(cp_err,
                "Error: no such device or model name %s\n",
                    *name);
            return (NULL);
        }
        device = ft_sim->devices[typecode];
        opt = parmlookup(device, &dev, param, do_model, 0);
        if (!opt) {
            fprintf(cp_err, "Error: no such parameter %s.\n",
                    param);
            return (NULL);
        }
        pv = doask(ckt, typecode, dev, mod, opt, ind);
        if (pv)
            vv = parmtovar(pv, opt);
        return (vv);
    } else
        return (if_getstat(ckt, *name));
}

void
if_setparam(ckt, name, param, val, do_model)
    char *ckt;
    char **name;
    char *param;
    struct dvec *val;
    int do_model;
{
    IFparm *opt;
    IFdevice *device;
    GENmodel *mod=(GENmodel *)NULL;
    GENinstance *dev=(GENinstance *)NULL;
    int typecode;

    INPinsert(name,(INPtables *)ft_curckt->ci_symtab);
    typecode = finddev(ckt, *name, (GENERIC**)&dev, (GENERIC**)&mod);
    if (typecode == -1) {
	fprintf(cp_err, "Error: no such device or model name %s\n", *name);
	return;
    }
    device = ft_sim->devices[typecode];
    opt = parmlookup(device, &dev, param, do_model, 1);
    if (!opt) {
	if (param)
		fprintf(cp_err, "Error: no such parameter %s.\n", param);
	else
		fprintf(cp_err, "Error: no default parameter.\n");
	return;
    }
    doset(ckt, typecode, dev, mod, opt, val);
}

static struct variable *
parmtovar(pv, opt)
    IFvalue *pv;
    IFparm *opt;
{
    struct variable *vv = alloc(struct variable);
    struct variable *nv;
    int i = 0;

    switch (opt->dataType & IF_VARTYPES) {
        case IF_INTEGER:
            vv->va_type = VT_NUM;
            vv->va_num = pv->iValue;
            break;
        case IF_REAL:
        case IF_COMPLEX:
            vv->va_type = VT_REAL;
            vv->va_real = pv->rValue;
            break;
        case IF_STRING:
            vv->va_type = VT_STRING;
            vv->va_string = pv->sValue;
            break;
        case IF_FLAG:
            vv->va_type = VT_BOOL;
            vv->va_bool = pv->iValue ? true : false;
            break;
        case IF_REALVEC:
            vv->va_type = VT_LIST;
            for (i = 0; i < pv->v.numValue; i++) {
                nv = alloc(struct variable);
                nv->va_next = vv->va_vlist;
                vv->va_vlist = nv;
                nv->va_type = VT_REAL;
                nv->va_real = pv->v.vec.rVec[i];
            }
            break;
        default:
            fprintf(cp_err,  
            "parmtovar: Internal Error: bad PARM type %d.\n",
                    opt->dataType);
            return (NULL);
    }

    /* It's not clear whether we want the keyword or the desc here... */
    vv->va_name = copy(opt->description);
    vv->va_next = NULL;
    return (vv);
}

/* Extract the IFparm structure from the device. If isdev is true, then get
 * the DEVmodQuest, otherwise get the DEVquest.
 */

static IFparm *
parmlookup(dev, inptr, param, do_model, inout)
    IFdevice *dev;
    GENinstance **inptr;
    char *param;
    int do_model;
    int inout;
{
    int i;

    /* fprintf(cp_err, "Called: parmlookup(%x, %c, %s)\n", 
            dev, isdev, param); */

    /* First try the device questions... */
    if (!do_model && dev->numInstanceParms) {
        for (i = 0; i < *(dev->numInstanceParms); i++) {
            if (!param && (dev->instanceParms[i].dataType & IF_PRINCIPAL))
                return (&dev->instanceParms[i]);
            else if ((((dev->instanceParms[i].dataType & IF_SET) && inout == 1)
	    	|| ((dev->instanceParms[i].dataType & IF_ASK) && inout == 0))
	    	&& eq(dev->instanceParms[i].keyword, param))
	    {
		if (dev->instanceParms[i].dataType & IF_REDUNDANT)
		    i -= 1;
                return (&dev->instanceParms[i]);
	    }
        }
	return NULL;
    }

    if (dev->numModelParms) {
	for (i = 0; i < *(dev->numModelParms); i++)
	    if ((((dev->modelParms[i].dataType & IF_SET) && inout == 1)
		    || ((dev->modelParms[i].dataType & IF_ASK) && inout == 0))
		    && eq(dev->modelParms[i].keyword, param))
	    {
		if (dev->instanceParms[i].dataType & IF_REDUNDANT)
		    i -= 1;
		return (&dev->modelParms[i]);
	    }
    }

    return (NULL);
}

/* Perform the CKTask call. We have both 'fast' and 'modfast', so the other
 * parameters aren't necessary.
 */

/* ARGSUSED */
static IFvalue *
doask(ckt, typecode, dev, mod, opt, ind)
    char *ckt;
    GENinstance *dev;
    GENmodel *mod;
    IFparm *opt;
    int ind;
{
    static IFvalue pv;
    int err;

    pv.iValue = ind;    /* Sometimes this will be junk and ignored... */

    /* fprintf(cp_err, "Calling doask(%d, %x, %x, %x)\n", 
            typecode, dev, mod, opt); */
    if (dev)
        err = (*(ft_sim->askInstanceQuest))((GENERIC *)ckt, (GENERIC *)dev, 
                opt->id, &pv, (IFvalue *)NULL);
    else
        err = (*(ft_sim->askModelQuest))((GENERIC*)ckt, (GENERIC*) mod, 
                opt->id, &pv, (IFvalue *)NULL);
    if (err != OK) {
        ft_sperror(err, "if_getparam");
        return (NULL);
    }

    return (&pv);
}

/* Perform the CKTset call. We have both 'fast' and 'modfast', so the other
 * parameters aren't necessary.
 */

/* ARGSUSED */
static int
doset(ckt, typecode, dev, mod, opt, val)
    char *ckt;
    GENinstance *dev;
    GENmodel *mod;
    IFparm *opt;
    struct dvec *val;
{
    IFvalue nval;
    int err;
    int n;
    int *iptr;
    double *dptr;
    int i;

    /* Count items */
    if (opt->dataType & IF_VECTOR) {
	n = nval.v.numValue = val->v_length;

	dptr = val->v_realdata;
	/* XXXX compdata!!! */

	switch (opt->dataType & (IF_VARTYPES & ~IF_VECTOR)) {
	    case IF_FLAG:
	    case IF_INTEGER:
		iptr = nval.v.vec.iVec = NEWN(int, n);

		for (i = 0; i < n; i++)
		    *iptr++ = *dptr++;
		break;

	    case IF_REAL:
		nval.v.vec.rVec = val->v_realdata;
		break;

	    default:
		fprintf(cp_err,
		    "Can't assign value to \"%s\" (unsupported vector type)\n",
		    opt->keyword);
		return E_UNSUPP;
	}
    } else {
	switch (opt->dataType & IF_VARTYPES) {
	    case IF_FLAG:
	    case IF_INTEGER:
		nval.iValue = *val->v_realdata;
		break;

	    case IF_REAL:
		nval.rValue = *val->v_realdata;
		break;

	    default:
		fprintf(cp_err,
		    "Can't assign value to \"%s\" (unsupported type)\n",
		    opt->keyword);
		return E_UNSUPP;
	}
    }

    /* fprintf(cp_err, "Calling doask(%d, %x, %x, %x)\n", 
            typecode, dev, mod, opt); */

    if (dev)
        err = (*(ft_sim->setInstanceParm))((GENERIC *)ckt, (GENERIC *)dev, 
                opt->id, &nval, (IFvalue *)NULL);
    else
        err = (*(ft_sim->setModelParm))((GENERIC*)ckt, (GENERIC*) mod, 
                opt->id, &nval, (IFvalue *)NULL);

    return err;
}



/* Get pointers to a device, its model, and its type number given the name. If
 * there is no such device, try to find a model with that name.
 */

static int
finddev(ck, name, devptr, modptr)
    char *ck;
    char *name;
    GENERIC **devptr;
    GENERIC **modptr;
{
    int err;
    int type = -1;

    err = (*(ft_sim->findInstance))((GENERIC *)ck,&type,devptr,name,NULL,NULL);
    if(err == OK) return(type);
    type = -1;
    *devptr = (GENERIC *)NULL;
    err = (*(ft_sim->findModel))((GENERIC *)ck,&type,modptr,name);
    if(err == OK) return(type);
    *modptr = (GENERIC *)NULL;
    return(-1);

}

#ifdef notdef
/* XXX Not useful */
/* Extract the node and device names from the line and add them to the command
 * completion structure.  This is probably not a good thing to do if it
 * takes too much time.
 */

 /* BLOW THIS AWAY */

void
if_setndnames(line)
    char *line;
{
    char *t;
    int i;

    while (isspace(*line))
        line++;

    if (!*line || (*line == '*') || (*line == '.'))
        return;
    t = gettok(&line);

    if (!(i = inp_numnodes(*t)))
        return;
    if ((*t == 'q') || (*t == 'Q'))
        i = 3;
    
    cp_addkword(CT_DEVNAMES, t);
    while (i-- > 0) {
        t = gettok(&line);
        if (t)
            cp_addkword(CT_NODENAMES, t);
    }
    return;
}
#endif

/* get an analysis parameter by name instead of id */

int if_analQbyName(ckt,which,anal,name,parm)
    GENERIC *ckt;
    int which;
    GENERIC *anal;
    char *name;
    IFvalue *parm;

{
    int i;
    for(i=0;i<ft_sim->analyses[which]->numParms;i++) {
        if(strcmp(ft_sim->analyses[which]->analysisParms[i].keyword,name)==0) {
            return( (*(ft_sim->askAnalysisQuest))(ckt,anal,
                    ft_sim->analyses[which]->analysisParms[i].id,parm,
                    (IFvalue *)NULL) );
        }
    }
    return(E_BADPARM);
}

/* Get the parameters tstart, tstop, and tstep from the CKT struct. */

/* BLOW THIS AWAY TOO */

bool
if_tranparams(ci, start, stop, step)
    struct circ *ci;
    double *start, *stop, *step;
{
    IFvalue tmp;
    int err;
    int which = -1;
    int i;
    GENERIC *anal;
    IFuid tranUid;

    if(!ci->ci_curTask) return(false);
    for(i=0;i<ft_sim->numAnalyses;i++) {
        if(strcmp(ft_sim->analyses[i]->name,"TRAN")==0){
            which = i;
            break;
        }
    }
    if(which == -1) return(false);

    err = IFnewUid(ci->ci_ckt,&tranUid,(IFuid)NULL,"Transient Analysis",
	UID_ANALYSIS, (GENERIC**)NULL);
    if(err != OK) return(false);
    err =(*(ft_sim->findAnalysis))(ci->ci_ckt,&which, &anal,tranUid,
            ci->ci_curTask,(IFuid *)NULL);
    if(err != OK) return(false);
    err = if_analQbyName(ci->ci_ckt,which,anal,"tstart",&tmp);
    if(err != OK) return(false);
    *start = tmp.rValue;
    err = if_analQbyName(ci->ci_ckt,which,anal,"tstop",&tmp);
    if(err != OK) return(false);
    *stop = tmp.rValue;
    err = if_analQbyName(ci->ci_ckt,which,anal,"tstep",&tmp);
    if(err != OK) return(false);
    *step = tmp.rValue;
    return (true);
}

/* Get the statistic called 'name'.  If this is NULL get all statistics
 * available.
 */

struct variable *
if_getstat(ckt, name)
    char *ckt;
    char *name;
{
    int i;
    struct variable *v, *vars;
    IFvalue parm;
    int which = -1;

    for(i=0;i<ft_sim->numAnalyses;i++) {
        if(strcmp(ft_sim->analyses[i]->name,"options")==0) {
            which = i;
            break;
        }
    }
    if(which==-1) {
        fprintf(cp_err,"Warning:  statistics unsupported\n");
        return(NULL);
    }

    if (name) {
        for (i = 0; i < ft_sim->analyses[which]->numParms; i++)
            if (eq(ft_sim->analyses[which]->analysisParms[i].keyword, name))
                break;
        if (i == ft_sim->analyses[which]->numParms)
            return (NULL);
        if ((*(ft_sim->askAnalysisQuest))(ckt, ft_curckt->ci_curTask,
                ft_sim->analyses[which]->analysisParms[i].id, &parm, 
                (IFvalue *)NULL) == -1) {
            fprintf(cp_err, 
                "if_getstat: Internal Error: can't get %s\n",
                name);
            return (NULL);
        }
        return (parmtovar(&parm, &(ft_sim->analyses[which]->analysisParms[i])));
    } else {
        for (i = 0, vars = v = NULL; i<ft_sim->analyses[which]->numParms; i++) {
            if(!(ft_sim->analyses[which]->analysisParms[i].dataType&IF_ASK)) {
                continue;
            }
            if ((*(ft_sim->askAnalysisQuest))(ckt, ft_curckt->ci_curTask, 
                    ft_sim->analyses[which]->analysisParms[i].id, 
                    &parm, (IFvalue *)NULL) == -1) {
                fprintf(cp_err, 
                "if_getstat: Internal Error: can't get %s\n",
                    name);
                return (NULL);
            }
            if (v) {
                v->va_next = parmtovar(&parm, 
                        &(ft_sim->analyses[which]->analysisParms[i]));
                v = v->va_next;
            } else {
                vars = v = parmtovar(&parm, 
                        &(ft_sim->analyses[which]->analysisParms[i])); 
            }
        }
        return (vars);
    }
}

