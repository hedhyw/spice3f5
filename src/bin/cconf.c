/*
 * Analyses
 */
#ifndef TABLES_ONLY
#define AN_op
#define AN_dc
#define AN_tf
#define AN_ac
#define AN_tran
#define AN_pz
#endif

/*
 * Devices
 */
#define DEV_asrc
#define DEV_bjt
#define DEV_cap
#define DEV_cccs
#define DEV_ccvs
#define DEV_csw
#define DEV_dio
#define DEV_ind
#define DEV_isrc
#define DEV_mos1
#define DEV_res
#define DEV_vccs
#define DEV_vcvs
#define DEV_vsrc

/* DO NOT MODIFY ANYTHING BELOW THIS LINE */

char	Spice_Version[ ] = "3f5";
char	Spice_Notice[ ] = "";
char	Spice_Build_Date[ ] = "";

/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
**********/

#include "spice.h"

#ifdef DEVICES_USED
static char *devs = DEVICES_USED;
#endif
#ifdef ANALYSES_USED
static char *ans = ANALYSES_USED;
#endif

#define CONFIG

#ifndef AN_dc

#ifdef AN_ac
#define AN_dc
#endif
#ifdef AN_pz
#define AN_dc
#endif
#ifdef AN_tran
#define AN_dc
#endif
#ifdef AN_noise
#define AN_dc
#endif
#ifdef AN_disto
#define AN_dc
#endif
#ifdef AN_sense
#define AN_dc
#endif
#ifdef AN_sense2
#define AN_dc
#endif

#endif

#include <stdio.h>
#include "noisedef.h"
#include "devdefs.h"
#include "suffix.h"

#ifdef HAS_FLAT_INCLUDES

#include "asrcitf.h"
#include "bjtitf.h"
#include "capitf.h"
#include "cccsitf.h"
#include "ccvsitf.h"
#include "cswitf.h"
#include "dioitf.h"
#include "inditf.h"
#include "isrcitf.h"
#include "mos1itf.h"
#include "mos6itf.h"
#include "resitf.h"
#include "switf.h"
#include "vccsitf.h"
#include "vcvsitf.h"
#include "vsrcitf.h"

#include "bsim1itf.h"
#include "bsim2itf.h"
#ifdef DEV_bsim3
#include "bsim3itf.h"
#endif
#include "mos2itf.h"
#include "mos3itf.h"
#include "jfetitf.h"
#include "mesitf.h"
#include "ltraitf.h"
#include "traitf.h"
#include "urcitf.h"

#else

#include "asrc/asrcitf.h"
#include "bjt/bjtitf.h"
#include "cap/capitf.h"
#include "cccs/cccsitf.h"
#include "ccvs/ccvsitf.h"
#include "csw/cswitf.h"
#include "dio/dioitf.h"
#include "ind/inditf.h"
#include "isrc/isrcitf.h"
#include "mos1/mos1itf.h"
#include "mos6/mos6itf.h"
#include "res/resitf.h"
#include "sw/switf.h"
#include "vccs/vccsitf.h"
#include "vcvs/vcvsitf.h"
#include "vsrc/vsrcitf.h"

#include "bsim1/bsim1itf.h"
#include "bsim2/bsim2itf.h"
#ifdef DEV_bsim3
#include "bsim3/bsim3itf.h"
#endif
#include "mos2/mos2itf.h"
#include "mos3/mos3itf.h"
#include "jfet/jfetitf.h"
#include "mes/mesitf.h"
#include "ltra/ltraitf.h"
#include "tra/traitf.h"
#include "urc/urcitf.h"
#endif

#ifndef TABLES_ONLY
extern SPICEanalysis OPTinfo;
extern SPICEanalysis ACinfo;
extern SPICEanalysis DCTinfo;
extern SPICEanalysis DCOinfo;
extern SPICEanalysis TRANinfo;
extern SPICEanalysis PZinfo;
extern SPICEanalysis SEN2info;
extern SPICEanalysis TFinfo;
extern SPICEanalysis DISTOinfo;
extern SPICEanalysis NOISEinfo;
extern SPICEanalysis SENSinfo;
#endif

SPICEanalysis *analInfo[] = {
#ifndef TABLES_ONLY
    &OPTinfo,
#ifdef AN_ac
    &ACinfo,
#endif
#ifdef AN_dc
    &DCTinfo,
#endif
#ifdef AN_op
    &DCOinfo,
#endif
#ifdef AN_tran
    &TRANinfo,
#endif
#ifdef AN_pz
    &PZinfo,
#endif
#ifdef AN_sense2
    &SEN2info,
#endif
#ifdef AN_tf
    &TFinfo,
#endif
#ifdef AN_disto
    &DISTOinfo,
#endif
#ifdef AN_noise
    &NOISEinfo,
#endif
#ifdef AN_sense
    &SENSinfo,
#endif
#else
    NULL
#endif
};

int ANALmaxnum = sizeof(analInfo)/sizeof(SPICEanalysis*);
SPICEdev *DEVices[] = {

#ifdef DEV_urc
	/* URC must appear before the resistor, capacitor, and diode */
        &URCinfo,
#endif
#ifdef DEV_asrc
        &ASRCinfo,
#endif
#ifdef DEV_bjt
        &BJTinfo,
#endif
#ifdef DEV_bsim1
        &B1info,
#endif
#ifdef DEV_bsim2
        &B2info,
#endif
#ifdef DEV_bsim3
        &B3info,
#endif
#ifdef DEV_cap
        &CAPinfo,
#endif
#ifdef DEV_cccs
        &CCCSinfo,
#endif
#ifdef DEV_ccvs
        &CCVSinfo,
#endif
#ifdef DEV_csw
        &CSWinfo,
#endif
#ifdef DEV_dio
        &DIOinfo,
#endif
#ifdef DEV_ind
        &INDinfo,
        &MUTinfo,
#endif
#ifdef DEV_isrc
        &ISRCinfo,
#endif
#ifdef DEV_jfet
        &JFETinfo,
#endif
#ifdef DEV_ltra
        &LTRAinfo,
#endif
#ifdef DEV_mes
        &MESinfo,
#endif
#ifdef DEV_mos1
        &MOS1info,
#endif
#ifdef DEV_mos2
        &MOS2info,
#endif
#ifdef DEV_mos3
        &MOS3info,
#endif
#ifdef DEV_mos6
        &MOS6info,
#endif
#ifdef DEV_res
        &RESinfo,
#endif
#ifdef DEV_sw
        &SWinfo,
#endif
#ifdef DEV_tra
        &TRAinfo,
#endif
#ifdef DEV_vccs
        &VCCSinfo,
#endif
#ifdef DEV_vcvs
        &VCVSinfo,
#endif
#ifdef DEV_vsrc
        &VSRCinfo,
#endif
};

/* my internal global constant for number of device types */
int DEVmaxnum = sizeof(DEVices)/sizeof(SPICEdev *);
/* XXX Should be -1 ? There is always an extra null element at the end ? */
static char * specSigList[] = {
    "time"
};

static IFparm nodeParms[] = {
    IP( "nodeset",PARM_NS ,IF_REAL,"suggested initial voltage"),
    IP( "ic",PARM_IC ,IF_REAL,"initial voltage"),
    IP( "type",PARM_NODETYPE ,IF_INTEGER,"output type of equation")
};

IFsimulator SIMinfo = {
    "Spice",        /* name */
    "Spice3 circuit level simulation program",  /* more about me */
#ifndef TABLES_ONLY
    Spice_Version,          /* version */

    CKTinit,        /* newCircuit function */
    CKTdestroy,     /* deleteCircuit function */

    CKTnewNode,     /* newNode function */
    CKTground,      /* groundNode function */
    CKTbindNode,    /* bindNode function */
    CKTfndNode,     /* findNode function */
    CKTinst2Node,   /* instToNode function */
    CKTsetNodPm,    /* setNodeParm function */
    CKTaskNodQst,   /* askNodeQuest function */
    CKTdltNod,      /* deleteNode function */

    CKTcrtElt,      /* newInstance function */
    CKTparam,       /* setInstanceParm function */
    CKTask,         /* askInstanceQuest function */
    CKTfndDev,      /* findInstance funciton */
    CKTdltInst,     /* deleteInstance function */

    CKTmodCrt,      /* newModel function */
    CKTmodParam,    /* setModelParm function */
    CKTmodAsk,      /* askModelQuest function */
    CKTfndMod,      /* findModel function */
    CKTdltMod,      /* deleteModel function */

    CKTnewTask,     /* newTask function */
    CKTnewAnal,     /* newAnalysis function */
    CKTsetAnalPm,   /* setAnalysisParm function */
    CKTaskAnalQ,    /* askAnalysisQuest function */
    CKTfndAnal,     /* findAnalysis function */
    CKTfndTask,     /* findTask function */
    CKTdelTask,     /* deleteTask function */

    CKTdoJob,       /* doAnalyses function */
    CKTtrouble,	    /* non-convergence message function */

    sizeof(DEVices)/sizeof(SPICEdev *),
    (IFdevice**)DEVices,

    sizeof(analInfo)/sizeof(SPICEanalysis *),
    (IFanalysis **)analInfo,

    sizeof(nodeParms)/sizeof(IFparm),
    nodeParms,

    sizeof(specSigList)/sizeof(char *),
    specSigList,
#endif
};
