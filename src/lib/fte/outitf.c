/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1988 Wayne A. Christopher, U. C. Berkeley CAD Group 
**********/

/*
 * This module replaces the old "writedata" routines in nutmeg.
 * Unlike the writedata routines, the OUT routines are only called by
 * the simulator routines, and only call routines in nutmeg.  The rest
 * of nutmeg doesn't deal with OUT at all.
 */

#include "spice.h"
#include "misc.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "ftedata.h"
#include "fteconst.h"
#include "inpdefs.h"        /* for INPtables */
#include "ifsim.h"
#include "jobdefs.h"
#include "iferrmsg.h"
#include "suffix.h"

extern void gr_end_iplot();
extern SPICEanalysis *analInfo[];

#define DOUBLE_PRECISION    15

typedef struct dataDesc {
    char *name;     /* The name of the vector. */
    int type;       /* The type. */
    int	gtype;		/* default plot scale */
    bool regular;       /* Is this given to us? */
    int outIndex;       /* If regular then the index. */
    char *specName;     /* The device name if special. */
    char *specParamName;    /* The parameter name if special. */
    int specIndex;      /* For sensitivity, if special. */
    int specType;
    GENERIC *specFast;
    int refIndex;       /* The index of our ref vector. */
    struct dvec *vec;
} dataDesc;

typedef struct runDesc {
    GENERIC *analysis;
    GENERIC *circuit;
    char *name;
    char *type;
    int numData;
    int refIndex;
    dataDesc *data;
    bool writeOut;
    bool windowed;
    bool binary;
    struct plot *runPlot;
    FILE *fp;
    long pointPos;          /* where to write pointCount */
    int pointCount;
    int isComplex;
    int windowCount;
} runDesc;

static int beginPlot();
static int addDataDesc();
static int addSpecialDesc();
static void fileInit();
static void fileInit_pass2( );
static void fileStartPoint();
static void fileAddRealValue();
static void fileAddComplexValue();
static void fileEndPoint();
static void fileEnd();
static void plotInit();
static void plotAddRealValue();
static void plotAddComplexValue();
static void plotEnd();
static bool parseSpecial();
static bool name_eq();
static bool getSpecial();
static void freeRun();

static bool shouldstop = false; /* Tell simulator to stop next time it asks. */
static bool printinfo = false;	/* Print informational "error messages". */

/* The two "begin plot" routines share all their internals... */

int
OUTpBeginPlot(circuitPtr, analysisPtr, analName, refName, refType,
        numNames, dataNames, dataType, plotPtr)
    GENERIC *circuitPtr;
    GENERIC *analysisPtr;
    IFuid analName;
    IFuid refName;
    int refType;
    int numNames;
    IFuid *dataNames;
    int dataType;
    GENERIC **plotPtr;
{
    char *name;

    if (ft_curckt->ci_ckt == circuitPtr)
      name = ft_curckt->ci_name;
    else
      name = "circuit name";

    return (beginPlot(analysisPtr, circuitPtr, name,
            (char *) analName, (char *) refName, refType, numNames,
            (char **) dataNames, dataType, false,
            (runDesc **) plotPtr));
}

int
OUTwBeginPlot(circuitPtr, analysisPtr, analName, refName, refType,
        numNames, dataNames, dataType, plotPtr)
    GENERIC *circuitPtr;
    GENERIC *analysisPtr;
    IFuid analName;
    IFuid refName;
    int refType;
    int numNames;
    IFuid *dataNames;
    int dataType;
    GENERIC **plotPtr;
{
    return (beginPlot(analysisPtr, circuitPtr, "circuit name",
            (char *) analName, (char *) refName, refType, numNames,
            (char **) dataNames, dataType, true,
            (runDesc **) plotPtr));
}

static int
beginPlot(analysisPtr, circuitPtr, cktName, analName, refName, refType,
        numNames, dataNames, dataType, windowed, runp)
    GENERIC *analysisPtr;
    GENERIC *circuitPtr;
    char *cktName;
    char *analName;
    char *refName;
    int refType;
    int numNames;
    char **dataNames;
    int dataType;
    bool windowed;
    runDesc **runp;
{
    runDesc *run;
    struct save_info *saves;
    bool *savesused;
    int numsaves;
    int i, j, depind;
    char namebuf[BSIZE_SP], parambuf[BSIZE_SP], depbuf[BSIZE_SP];
    bool saveall = true;
    char *an_name;

    /* Check to see if we want to print informational data. */
    if (cp_getvar("printinfo", VT_BOOL, (char *) &printinfo))
	fprintf(cp_err, "(debug printing enabled)\n");

    *runp = run = alloc(struct runDesc);

    /* First fill in some general information. */
    run->analysis = analysisPtr;
    run->circuit = circuitPtr;
    run->name = copy(cktName);
    run->type = copy(analName);
    run->windowed = windowed;
    run->numData = 0;

    an_name = analInfo[((JOB *) analysisPtr)->JOBtype]->public.name;

    /* Now let's see which of these things we need.  First toss in the
     * reference vector.  Then toss in anything that getSaves() tells
     * us to save that we can find in the name list.  Finally unpack
     * the remaining saves into parameters.
     */
    numsaves = ft_getSaves(&saves);
    if (numsaves) {
        savesused = (bool *) tmalloc(sizeof (bool) * numsaves);
        saveall = false;
        for (i = 0; i < numsaves; i++) {
            if (saves[i].analysis && !cieq(saves[i].analysis, an_name)) {
		/* ignore this one this time around */
		savesused[i] = true;
		continue;
	    }
	    if (cieq(saves[i].name, "all")) {
                saveall = true;
                savesused[i] = true;
		saves[i].used = 1;
                continue;
            }
	}
    }

    /* Pass 0. */
    if (refName) {
        addDataDesc(run, refName, refType, -1);
        for (i = 0; i < numsaves; i++)
            if (!savesused[i] && name_eq(saves[i].name, refName)) {
                savesused[i] = true;
		saves[i].used = 1;
            }
    } else {
        run->refIndex = -1;
    }


    /* Pass 1. */
    if (numsaves && !saveall) {
        for (i = 0; i < numsaves; i++) {
	    if (!savesused[i]) {
		for (j = 0; j < numNames; j++) {
		    if (name_eq(saves[i].name, dataNames[j])) {
			addDataDesc(run, dataNames[j], dataType, j);
			savesused[i] = true;
			saves[i].used = 1;
			break;
		    }
		}
	    }
        }
    } else {
        for (i = 0; i < numNames; i++)
            if (!refName || !name_eq(dataNames[i], refName)) {
                addDataDesc(run, dataNames[i], dataType, i);
            }
    }

    /* Pass 2. */
    for (i = 0; i < numsaves; i++) {
        if (savesused[i])
            continue;
        if (!parseSpecial(saves[i].name, namebuf, parambuf, depbuf)) {
	    if (saves[i].analysis)
		fprintf(cp_err, "Warning: can't parse '%s': ignored\n",
			saves[i].name);
            continue;
        }
        /* Now, if there's a dep variable, do we already have it? */
        if (*depbuf) {
            for (j = 0; j < run->numData; j++)
                if (name_eq(depbuf, run->data[j].name))
                    break;
            if (j == run->numData) {
                /* Better add it. */
                for (j = 0; j < numNames; j++)
                    if (name_eq(depbuf, dataNames[j]))
                        break;
                if (j == numNames) {
                    fprintf(cp_err,
            "Warning: can't find '%s': value '%s' ignored\n",
                        depbuf, saves[i].name);
                    continue;
                }
                addDataDesc(run, dataNames[j], dataType, j);
                savesused[i] = true;
		saves[i].used = 1;
                depind = j;
            } else
                depind = run->data[j].outIndex;
        }
        addSpecialDesc(run, saves[i].name, namebuf, parambuf, depind);
    }

    if (numsaves) {
	for (i = 0; i < numsaves; i++) {
	    tfree(saves[i].analysis);
	    tfree(saves[i].name);
	}
        tfree(savesused);
    }

    if (numNames && (run->numData == 1 && run->refIndex != -1
	    || run->numData == 0 && run->refIndex == -1))
    {
	fprintf(cp_err, "Error: no data saved for %s; analysis not run\n",
	    analInfo[((JOB *) analysisPtr)->JOBtype]->public.description);
	return E_NOTFOUND;
    }
    
    /* Now that we have our own data structures built up, let's see what
     * nutmeg wants us to do.
     */
    run->writeOut = ft_getOutReq(&run->fp, &run->runPlot, &run->binary,
            run->type, run->name);

    if (run->writeOut)
        fileInit(run);
    else {
        plotInit(run);
	if (refName)
	    run->runPlot->pl_ndims = 1;
    }

    return (OK);
}

static int
addDataDesc(run, name, type, ind)
    runDesc *run;
    char *name;
    int type;
    int ind;
{
    dataDesc *data;

    if (!run->numData)
        run->data = (dataDesc *) tmalloc(sizeof (dataDesc));
    else
        run->data = (dataDesc *) trealloc((char *) run->data,
                sizeof (dataDesc) * (run->numData + 1));
    data = &run->data[run->numData];
    /* so freeRun will get nice NULL pointers for the fields we don't set */
    bzero(data, sizeof(dataDesc));

    data->name = copy(name);
    data->type = type;
    data->gtype = GRID_LIN;
    data->regular = true;
    data->outIndex = ind;

    if (ind == -1) {
        /* It's the reference vector. */
        run->refIndex = run->numData;
    }

    run->numData++;

    return (OK);
}

static int
addSpecialDesc(run, name, devname, param, depind)
    runDesc *run;
    char *name;
    char *devname;
    char *param;
    int depind;
{
    dataDesc *data;
    char *unique;       /* unique char * from back-end */

    if (!run->numData)
        run->data = (dataDesc *) tmalloc(sizeof (dataDesc));
    else
        run->data = (dataDesc *) trealloc((char *) run->data,
                sizeof (dataDesc) * (run->numData + 1));
    data = &run->data[run->numData];
    /* so freeRun will get nice NULL pointers for the fields we don't set */
    bzero(data, sizeof(dataDesc));

    data->name = copy(name);

    unique = devname;
    INPinsert(&unique, (INPtables *) ft_curckt->ci_symtab);
    data->specName = unique;

    data->specParamName = copy(param);

    data->specIndex = depind;
    data->specType = -1;
    data->specFast = NULL;
    data->regular = false;

    run->numData++;

    return (OK);
}



int
OUTpData(plotPtr, refValue, valuePtr)
    GENERIC *plotPtr;
    IFvalue *refValue;
    IFvalue *valuePtr;
{
    runDesc *run = (runDesc *) plotPtr;
    IFvalue val;
    int i;

    run->pointCount++;

    if (run->writeOut) {
	if (run->pointCount == 1)
	    fileInit_pass2(plotPtr);
        fileStartPoint(run->fp, run->binary, run->pointCount);

        if (run->refIndex != -1) {
          if (run->isComplex)
            fileAddComplexValue(run->fp, run->binary, refValue->cValue);
          else
            fileAddRealValue(run->fp, run->binary, refValue->rValue);
        }

        for (i = 0; i < run->numData; i++) {
            /* we've already printed reference vec first */
            if (run->data[i].outIndex == -1) continue;

            if (run->data[i].regular) {
                if(run->data[i].type == IF_REAL)
                  fileAddRealValue(run->fp, run->binary,
                    valuePtr->v.vec.rVec
                    [run->data[i].outIndex]);
                else if (run->data[i].type == IF_COMPLEX)
                  fileAddComplexValue(run->fp, run->binary,
                    valuePtr->v.vec.cVec
                    [run->data[i].outIndex]);
                else
                  fprintf(stderr, "OUTpData: unsupported data type\n");
            } else {
                /* should pre-check instance */
                if (!getSpecial(&run->data[i], run, &val))
                    continue;
                if (run->data[i].type == IF_REAL)
                  fileAddRealValue(run->fp, run->binary,
                     val.rValue);
                else if (run->data[i].type == IF_COMPLEX)
                  fileAddComplexValue(run->fp, run->binary,
                     val.cValue);
                else
                  fprintf(stderr, "OUTpData: unsupported data type\n");
            }
        }
        fileEndPoint(run->fp, run->binary);
    } else {
        for (i = 0; i < run->numData; i++) {
            if (run->data[i].outIndex == -1) {
                if (run->data[i].type == IF_REAL)
                    plotAddRealValue(&run->data[i],
                            refValue->rValue);
                else if (run->data[i].type == IF_COMPLEX)
                    plotAddComplexValue(&run->data[i],
                            refValue->cValue);
            } else if (run->data[i].regular) {
                if (run->data[i].type == IF_REAL)
                    plotAddRealValue(&run->data[i],
                        valuePtr->v.vec.rVec
                        [run->data[i].outIndex]);
                else if (run->data[i].type == IF_COMPLEX)
                    plotAddComplexValue(&run->data[i],
                        valuePtr->v.vec.cVec
                        [run->data[i].outIndex]);
            } else {
                /* should pre-check instance */
                if (!getSpecial(&run->data[i], run, &val))
                    continue;
                if (run->data[i].type == IF_REAL)
                    plotAddRealValue(&run->data[i],
                            val.rValue);
                else if (run->data[i].type == IF_COMPLEX)
                    plotAddComplexValue(&run->data[i],
                            val.cValue);
                else 
                    fprintf(stderr, "OUTpData: unsupported data type\n");
            }
        }
        gr_iplot(run->runPlot);
    }

    if (ft_bpcheck(run->runPlot, run->pointCount) == false)
        shouldstop = true;

    return (OK);
}



/* ARGSUSED */ /* until some code gets written */
int
OUTwReference(plotPtr, valuePtr, refPtr)
    GENERIC *plotPtr;
    IFvalue *valuePtr;
    GENERIC **refPtr;
{
    return (OK);
}

/* ARGSUSED */ /* until some code gets written */
int
OUTwData(plotPtr, dataIndex, valuePtr, refPtr)
    GENERIC *plotPtr;
    int dataIndex;
    IFvalue *valuePtr;
    GENERIC *refPtr;
{
    return (OK);
}

/* ARGSUSED */ /* until some code gets written */
int
OUTwEnd(plotPtr)
    GENERIC *plotPtr;
{
    return (OK);
}



int
OUTendPlot(plotPtr)
    GENERIC *plotPtr;
{
    runDesc *run = (runDesc *) plotPtr;
    
    if (run->writeOut)
        fileEnd(run);
    else {
        gr_end_iplot();
        plotEnd(run);
    }

    freeRun(run);

    return (OK);
}



/* ARGSUSED */ /* until some code gets written */
int
OUTbeginDomain(plotPtr, refName, refType, outerRefValue)
    GENERIC *plotPtr;
    char *refName;
    int refType;
    IFvalue *outerRefValue;
{
    return (OK);
}

/* ARGSUSED */ /* until some code gets written */
int
OUTendDomain(plotPtr)
    GENERIC *plotPtr;
{
    return (OK);
}



/* ARGSUSED */ /* until some code gets written */
int
OUTattributes(plotPtr, varName, param, value)
    GENERIC *plotPtr;
    char *varName;
    int param;
    IFvalue *value;
{
    runDesc *run = (runDesc *) plotPtr;
    struct dvec *d;
    struct dataDesc *dd;
    int type;
    int i;

    if (param == OUT_SCALE_LIN)
	type = GRID_LIN;
    else if (param == OUT_SCALE_LOG)
	type = GRID_XLOG;
    else
	return E_UNSUPP;

    if (run->writeOut) {
	if (varName) {
	    for (i = 0; i < run->numData; i++)
		if (!strcmp(varName, run->data[i].name))
		    run->data[i].gtype = type;
	} else {
	    run->data[run->refIndex].gtype = type;
	}
    } else {
	if (varName) {
	    for (d = run->runPlot->pl_dvecs; d; d = d->v_next)
		if (!strcmp(varName, d->v_name))
		    d->v_gridtype = type;
	} else {
	    run->runPlot->pl_scale->v_gridtype = type;
	}
    }

    return (OK);
}



/* The file writing routines. */

static void
fileInit(run)
    runDesc *run;
{
    char buf[513];
    int i;

    /* This is a hack. */
    run->isComplex = false;
    for (i = 0; i < run->numData; i++)
        if (run->data[i].type == IF_COMPLEX)
            run->isComplex = true;

    i = 0;
    sprintf(buf, "Title: %s\n", run->name);
    i += strlen(buf);
    fputs(buf, run->fp);
    sprintf(buf, "Date: %s\n", datestring());
    i += strlen(buf);
    fputs(buf, run->fp);
    sprintf(buf, "Plotname: %s\n", run->type);
    i += strlen(buf);
    fputs(buf, run->fp);
    sprintf(buf, "Flags: %s\n", run->isComplex ? "complex" : "real");
    i += strlen(buf);
    fputs(buf, run->fp);
    sprintf(buf, "No. Variables: %d\n", run->numData);
    i += strlen(buf);
    fputs(buf, run->fp);
    sprintf(buf, "No. Points: ");
    i += strlen(buf);
    fputs(buf, run->fp);

    fflush(run->fp);        /* Gotta do this for LATTICE. */
    if (run->fp == stdout || (run->pointPos = ftell(run->fp)) <= 0)
	run->pointPos = i;
    fprintf(run->fp, "0       \n"); /* Save 8 spaces here. */

    fprintf(run->fp, "Command: version %s\n", ft_sim->version);
    fprintf(run->fp, "Variables:\n");

    return;
}

static void
fileInit_pass2(run)
    runDesc *run;
{
    int i, type;
    char *name, buf[BSIZE_SP];

    for (i = 0; i < run->numData; i++) {
        if (isdigit(*run->data[i].name)) {
        (void) sprintf(buf, "V(%s)", run->data[i].name);
          name = buf;
        } else {
          name = run->data[i].name;
        }
        if (substring("#branch", name))
            type = SV_CURRENT;
        else if (cieq(name, "time"))
            type = SV_TIME;
        else if (cieq(name, "frequency"))
            type = SV_FREQUENCY;
        else 
            type = SV_VOLTAGE;

        fprintf(run->fp, "\t%d\t%s\t%s", i, name,
                ft_typenames(type));
	if (run->data[i].gtype == GRID_XLOG)
	    fprintf(run->fp, "\tgrid=3");
	fprintf(run->fp, "\n");

    }

    fprintf(run->fp, "%s:\n", run->binary ? "Binary" : "Values");

    return;
}

static void
fileStartPoint(fp, bin, num)
    FILE *fp;
    bool bin;
    int num;
{
    if (!bin)
        fprintf(fp, "%d\t", num - 1);

    return;
}

static void
fileAddRealValue(fp, bin, value)
    FILE *fp;
    bool bin;
    double value;
{
    if (bin)
        fwrite((char *) &value, sizeof (double), 1, fp);
    else
        fprintf(fp, "\t%.*e\n", DOUBLE_PRECISION, value);
    
    return;
}

static void
fileAddComplexValue(fp, bin, value)
    FILE *fp;
    bool bin;
    IFcomplex value;
{

    if (bin) {
        fwrite((char *) &value.real, sizeof (double), 1, fp);
        fwrite((char *) &value.imag, sizeof (double), 1, fp);
    } else {
        fprintf(fp, "\t%.*e,%.*e\n", DOUBLE_PRECISION, value.real,
                DOUBLE_PRECISION, value.imag);
    }

}

/* ARGSUSED */ /* until some code gets written */
static void
fileEndPoint(fp, bin)
    FILE *fp;
    bool bin;
{
    return;
}

/* Here's the hack...  Run back and fill in the number of points. */

static void
fileEnd(run)
    runDesc *run;
{
    long place;

    if (run->fp != stdout) {
	place = ftell(run->fp);
	fseek(run->fp, run->pointPos, 0);
	fprintf(run->fp, "%d", run->pointCount);
	fseek(run->fp, place, 0);
    } else {
	/* Yet another hack-around */
	fprintf(stderr, "@@@ %ld %d\n", run->pointPos, run->pointCount);
    }
    fflush(run->fp);

    return;
}



/* The plot maintenance routines. */

static void
plotInit(run)
    runDesc *run;
{
    struct plot *pl = plot_alloc(run->type);
    char buf[100];
    struct dvec *v;
    dataDesc *dd;
    int i;

    pl->pl_title = copy(run->name);
    pl->pl_name = copy(run->type);
    pl->pl_date = copy(datestring( ));
    pl->pl_ndims = 0;
    plot_new(pl);
    plot_setcur(pl->pl_typename);
    run->runPlot = pl;

    /* This is a hack. */
    /* if any of them complex, make them all complex */
    run->isComplex = false;
    for (i = 0; i < run->numData; i++) {
      if (run->data[i].type == IF_COMPLEX) run->isComplex = true;
    }

    for (i = 0; i < run->numData; i++) {
        dd = &run->data[i];
        v = alloc(struct dvec);
        if (isdigit(*dd->name)) {
            (void) sprintf(buf, "V(%s)", dd->name);
            v->v_name = copy(buf);
        } else
            v->v_name = copy(dd->name);
        if (substring("#branch", v->v_name))
            v->v_type = SV_CURRENT;
        else if (cieq(v->v_name, "time"))
            v->v_type = SV_TIME;
        else if (cieq(v->v_name, "frequency"))
            v->v_type = SV_FREQUENCY;
        else 
            v->v_type = SV_VOLTAGE;
        v->v_length = 0;
        v->v_scale = NULL;
        if (!run->isComplex) {
            v->v_flags = VF_REAL;
            v->v_realdata = NULL;
        } else {
            v->v_flags = VF_COMPLEX;
            v->v_compdata = NULL;
        }

        v->v_flags |= VF_PERMANENT;

        vec_new(v);
        dd->vec = v;
    }
}

static void
plotAddRealValue(desc, value)
    dataDesc *desc;
    double value;
{
    struct dvec *v = desc->vec;

    if (isreal(v)) {
      v->v_realdata = (double *) trealloc((char *) v->v_realdata,
            sizeof (double) * (v->v_length + 1));
      v->v_realdata[v->v_length] = value;
    } else {
      /* a real parading as a VF_COMPLEX */
      v->v_compdata = (complex *) trealloc((char *) v->v_compdata,
            sizeof (complex) * (v->v_length + 1));
      v->v_compdata[v->v_length].cx_real = value;
      v->v_compdata[v->v_length].cx_imag = (double) 0;
    }
    v->v_length++;

    return;
}

static void
plotAddComplexValue(desc, value)
    dataDesc *desc;
    IFcomplex value;
{
    struct dvec *v = desc->vec;

    v->v_compdata = (complex *) trealloc((char *) v->v_compdata,
            sizeof (complex) * (v->v_length + 1));
    v->v_compdata[v->v_length].cx_real = value.real;
    v->v_compdata[v->v_length].cx_imag = value.imag;
    v->v_length++;

    return;
}

/* ARGSUSED */ /* until some code gets written */
static void
plotEnd(run)
    runDesc *run;
{

    return;
}



/* ParseSpecial takes something of the form "@name[param,index]" and rips
 * out name, param, and index.
 */

static bool
parseSpecial(name, dev, param, ind)
    char *name;
    char *dev;
    char *param;
    char *ind;
{
    char *s;

    *dev = *param = *ind = '\0';

    if (*name != '@')
        return (false);
    name++;
    
    s = dev;
    while (*name && (*name != '['))
        *s++ = *name++;
    *s = '\0';
    if (!*name)
        return (true);
    name++;

    s = param;
    while (*name && (*name != ',') && (*name != ']'))
        *s++ = *name++;
    *s = '\0';
    if (*name == ']')
        return (!name[1] ? true : false);
    else if (!*name)
        return (false);
    name++;

    s = ind;
    while (*name && (*name != ']'))
        *s++ = *name++;
    *s = '\0';
    if (*name && !name[1])
        return (true);
    else
        return (false);
}

/* This routine must match two names with or without a V() around them. */

static bool
name_eq(n1, n2)
    char *n1, *n2;
{
    char buf1[BSIZE_SP], buf2[BSIZE_SP], *s;

    if (s = index(n1, '(')) {
        strcpy(buf1, s);
        if (!(s = index(buf1, ')')))
            return (false);
        *s = '\0';
        n1 = buf1;
    }
    if (s = index(n2, '(')) {
        strcpy(buf2, s);
        if (!(s = index(buf2, ')')))
            return (false);
        *s = '\0';
        n2 = buf2;
    }

    return (strcmp(n1, n2) ? false : true);
}

static bool
getSpecial(desc, run, val)
    dataDesc *desc;
    runDesc *run;
    IFvalue *val;
{
    IFvalue selector;
    struct variable *vv;

    selector.iValue = desc->specIndex;
    if (INPaName(desc->specParamName, val, run->circuit, &desc->specType,
            desc->specName, &desc->specFast, ft_sim, &desc->type,
            &selector) == OK) {
      desc->type &= (IF_REAL | IF_COMPLEX);   /* mask out other bits */
      return(true);
    } else if (vv = if_getstat(run->circuit, &desc->name[1])) {
						/* skip @ sign */
      desc->type = IF_REAL;
      if (vv->va_type == VT_REAL)
	val->rValue = vv->va_real;
      else if (vv->va_type == VT_NUM)
	val->rValue = vv->va_num;
      else if (vv->va_type == VT_BOOL)
	val->rValue = (vv->va_bool ? 1.0 : 0.0);
      else {
	return (false); /* not a real */
      }
      tfree(vv);
      return(true);
    }

    return (false);
}

static void
freeRun(run)
    runDesc *run;
{

    int i;

    for (i=0; i < run->numData; i++) {
/*    vec_free(run->data[i].vec); */ /* kill run, leave plot */
      tfree(run->data[i].name);
      tfree(run->data[i].specParamName);
    }
    tfree(run->data);

/*  killplot(run->runPlot); */ /* kill run, leave plot */

    free(run->type);
    free(run->name);
    free(run);

}

int
OUTstopnow()
{

    if (ft_intrpt || shouldstop) {
        ft_intrpt = shouldstop = false;
        return (1);
    } else
        return (0);

}

/* Print out error messages. */

static struct mesg {
        char *string;
        long flag;
} msgs[] = {
        { "Warning", ERR_WARNING } ,
        { "Fatal error", ERR_FATAL } ,
        { "Panic", ERR_PANIC } ,
        { "Note", ERR_INFO } ,
        { NULL, 0 }
} ;

void
OUTerror(flags,format,names) 
    int flags;
    char *format;
    IFuid *names;
{

    struct mesg *m;
    char buf[BSIZE_SP], *s, *bptr;
    int nindex = 0;

    if ((flags == ERR_INFO) && cp_getvar("printinfo", VT_BOOL,
	    (char *) &printinfo))
	return;

    for (m = msgs; m->flag; m++)
                if (flags & m->flag)
                        fprintf(cp_err, "%s: ", m->string);

    for (s = format, bptr = buf; *s; s++) {
      if (*s == '%' && (s == format || *(s-1) != '%') && *(s+1) == 's') {
	if (names[nindex])
	  strcpy(bptr, names[nindex]);
	else
	  strcpy(bptr, "(null)");
        bptr += strlen(bptr);
        s++;
        nindex++;
      } else {
        *bptr++ = *s;
      }
    }
    *bptr = '\0';
        fprintf(cp_err, "%s\n", buf);
        fflush(cp_err);

}
