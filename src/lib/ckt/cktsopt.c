/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

    /*
     *  CKTsetOpt(ckt,opt,value)
     *  set the specified 'opt' to have value 'value' in the 
     *  given circuit 'ckt'.
     */

#include "spice.h"
#include <stdio.h>
#include "const.h"
#include "optdefs.h"
#include "tskdefs.h"
#include "ifsim.h"
#include "util.h"
#include "cktdefs.h"
#include "sperror.h"
#include "suffix.h"

/* ARGSUSED */
int
CKTsetOpt(ckt,anal,opt,val)
    GENERIC *ckt;
    GENERIC *anal;
    int opt;
    IFvalue *val;
{
    register TSKtask *task = (TSKtask *)anal;

    switch(opt) {
        
    case OPT_NOOPITER:
        task->TSKnoOpIter = val->iValue;
        break;
    case OPT_GMIN:
        task->TSKgmin = val->rValue;
        break;
    case OPT_RELTOL:
        task->TSKreltol = val->rValue;
        break;
    case OPT_ABSTOL:
        task->TSKabstol = val->rValue;
        break;
    case OPT_VNTOL:
        task->TSKvoltTol = val->rValue;
        break;
    case OPT_TRTOL:
        task->TSKtrtol = val->rValue;
        break;
    case OPT_CHGTOL:
        task->TSKchgtol = val->rValue;
        break;
    case OPT_PIVTOL:
        task->TSKpivotAbsTol = val->rValue;
        break;
    case OPT_PIVREL:
        task->TSKpivotRelTol = val->rValue;
        break;
    case OPT_TNOM:
        task->TSKnomTemp = val->rValue + CONSTCtoK; /* Centegrade to Kelvin */
        break;
    case OPT_TEMP:
        task->TSKtemp = val->rValue + CONSTCtoK; /* Centegrade to Kelvin */
        break;
    case OPT_ITL1:
        task->TSKdcMaxIter = val->iValue;
        break;
    case OPT_ITL2:
        task->TSKdcTrcvMaxIter = val->iValue;
        break;
    case OPT_ITL3:
        break;
    case OPT_ITL4:
        task->TSKtranMaxIter = val->iValue;
        break;
    case OPT_ITL5:
        break;
    case OPT_SRCSTEPS:
        task->TSKnumSrcSteps = val->iValue;
        break;
    case OPT_GMINSTEPS:
        task->TSKnumGminSteps = val->iValue;
        break;
    case OPT_DEFL:
        task->TSKdefaultMosL = val->rValue;
        break;
    case OPT_DEFW:
        task->TSKdefaultMosW = val->rValue;
        break;
    case OPT_DEFAD:
        task->TSKdefaultMosAD = val->rValue;
        break;
    case OPT_DEFAS:
        task->TSKdefaultMosAD = val->rValue;
        break;
    case OPT_BYPASS:
        task->TSKbypass = val->iValue;
        break;
    case OPT_MAXORD:
        task->TSKmaxOrder = val->iValue;
        break;
    case OPT_OLDLIMIT:
        task->TSKfixLimit = val->iValue;
        break;
    case OPT_MINBREAK:
        task->TSKminBreak = val->rValue;
        break;
    case OPT_METHOD:
        if(strncmp(val->sValue,"trap", 4)==0)
		task->TSKintegrateMethod=TRAPEZOIDAL;
        else if (strcmp(val->sValue,"gear")==0)
		task->TSKintegrateMethod=GEAR;
        else return(E_METHOD);
        break;
    case OPT_TRYTOCOMPACT:
	task->TSKtryToCompact = val->iValue;
	break;
    case OPT_BADMOS3:
	task->TSKbadMos3 = val->iValue;
	break;
    case OPT_KEEPOPINFO:
	task->TSKkeepOpInfo = val->iValue;
	break;
    default:
        return(-1);
    }
    return(0);
}
static IFparm OPTtbl[] = {
 { "noopiter", OPT_NOOPITER,IF_SET|IF_FLAG,"Go directly to gmin stepping" },
 { "gmin", OPT_GMIN,IF_SET|IF_REAL,"Minimum conductance" },
 { "reltol", OPT_RELTOL,IF_SET|IF_REAL ,"Relative error tolerence"},
 { "abstol", OPT_ABSTOL,IF_SET|IF_REAL,"Absolute error tolerence" },
 { "vntol", OPT_VNTOL,IF_SET|IF_REAL,"Voltage error tolerence" },
 { "trtol", OPT_TRTOL,IF_SET|IF_REAL,"Truncation error overestimation factor" },
 { "chgtol", OPT_CHGTOL,IF_SET|IF_REAL, "Charge error tolerence" },
 { "pivtol", OPT_PIVTOL,IF_SET|IF_REAL, "Minimum acceptable pivot" },
 { "pivrel", OPT_PIVREL,IF_SET|IF_REAL, "Minimum acceptable ratio of pivot" },
 { "tnom", OPT_TNOM,IF_SET|IF_ASK|IF_REAL, "Nominal temperature" },
 { "temp", OPT_TEMP,IF_SET|IF_ASK|IF_REAL, "Operating temperature" },
 { "itl1", OPT_ITL1,IF_SET|IF_INTEGER,"DC iteration limit" },
 { "itl2", OPT_ITL2,IF_SET|IF_INTEGER,"DC transfer curve iteration limit" },
 { "itl3", OPT_ITL3, IF_INTEGER,"Lower transient iteration limit"},
 { "itl4", OPT_ITL4,IF_SET|IF_INTEGER,"Upper transient iteration limit" },
 { "itl5", OPT_ITL5, IF_INTEGER,"Total transient iteration limit"},
 { "itl6", OPT_SRCSTEPS, IF_SET|IF_INTEGER,"number of source steps"},
 { "srcsteps", OPT_SRCSTEPS, IF_SET|IF_INTEGER,"number of source steps"},
 { "gminsteps", OPT_GMINSTEPS, IF_SET|IF_INTEGER,"number of Gmin steps"},
 { "acct", 0, IF_FLAG ,"Print accounting"},
 { "list", 0, IF_FLAG, "Print a listing" },
 { "nomod", 0, IF_FLAG, "Don't print a model summary" },
 { "nopage", 0, IF_FLAG, "Don't insert page breaks" },
 { "node", 0, IF_FLAG,"Print a node connection summary" },
 { "opts", 0, IF_FLAG, "Print a list of the options" },
 { "oldlimit", OPT_OLDLIMIT, IF_SET|IF_FLAG, "use SPICE2 MOSfet limiting" },
 { "numdgt", 0, IF_INTEGER, "Set number of digits printed"},
 { "cptime", 0, IF_REAL, "Total cpu time in seconds" },
 { "limtim", 0, IF_INTEGER, "Time to reserve for output" },
 { "limpts", 0,IF_INTEGER,"Maximum points per analysis"},
 { "lvlcod", 0, IF_INTEGER,"Generate machine code" },
 { "lvltim", 0, IF_INTEGER,"Type of timestep control" },
 { "method", OPT_METHOD, IF_SET|IF_STRING,"Integration method" },
 { "maxord", OPT_MAXORD, IF_SET|IF_INTEGER,"Maximum integration order" },
 { "defl", OPT_DEFL,IF_SET|IF_REAL,"Default MOSfet length" },
 { "defw", OPT_DEFW,IF_SET|IF_REAL,"Default MOSfet width" },
 { "minbreak", OPT_MINBREAK,IF_SET|IF_REAL,"Minimum time between breakpoints" },
 { "defad", OPT_DEFAD,IF_SET|IF_REAL,"Default MOSfet area of drain" },
 { "defas", OPT_DEFAS,IF_SET|IF_REAL,"Default MOSfet area of source" },
 { "bypass",OPT_BYPASS,IF_SET|IF_INTEGER,"Allow bypass of unchanging elements"},
 { "totiter", OPT_ITERS, IF_ASK|IF_INTEGER,"Total iterations" },
 { "traniter", OPT_TRANIT, IF_ASK|IF_INTEGER ,"Transient iterations"},
 { "equations", OPT_EQNS, IF_ASK|IF_INTEGER,"Circuit Equations" },
 { "tranpoints", OPT_TRANPTS, IF_ASK|IF_INTEGER,"Transient timepoints" },
 { "accept", OPT_TRANACCPT, IF_ASK|IF_INTEGER,"Accepted timepoints" },
 { "rejected", OPT_TRANRJCT, IF_ASK|IF_INTEGER,"Rejected timepoints" },
 { "time", OPT_TOTANALTIME, IF_ASK|IF_REAL,"Total Analysis Time" },
 { "trantime", OPT_TRANTIME, IF_ASK|IF_REAL,"Transient time" },
 { "reordertime", OPT_REORDTIME, IF_ASK|IF_REAL,"matrix reordering time" },
 { "lutime", OPT_DECOMP, IF_ASK|IF_REAL,"L-U decomposition time" },
 { "solvetime", OPT_SOLVE, IF_ASK|IF_REAL,"Matrix solve time" },
 { "tranlutime", OPT_TRANDECOMP,IF_ASK|IF_REAL,"transient L-U decomp time"},
 { "transolvetime", OPT_TRANSOLVE, IF_ASK|IF_REAL,"Transient solve time" },
 { "trancuriters", OPT_TRANCURITER, IF_ASK|IF_INTEGER,
        "Transient iters per point" },
 { "loadtime", OPT_LOADTIME, IF_ASK|IF_REAL,"Load time" },
 { "trytocompact", OPT_TRYTOCOMPACT, IF_SET|IF_FLAG,
	"Try compaction for LTRA lines" },
 { "badmos3", OPT_BADMOS3, IF_SET|IF_FLAG,
	 "use old mos3 model (discontinuous with respect to kappa)" },
 { "keepopinfo", OPT_KEEPOPINFO, IF_SET|IF_FLAG,
	 "Record operating point for each small-signal analysis" }
};

int OPTcount = sizeof(OPTtbl)/sizeof(IFparm);

SPICEanalysis OPTinfo = {
    {
        "options",
        "Task option selection",
        sizeof(OPTtbl)/sizeof(IFparm),
        OPTtbl
    },
    0, /* no size associated with options */
    NODOMAIN,
    0,
    CKTsetOpt,
    CKTacct,
    NULL,
    NULL
};
