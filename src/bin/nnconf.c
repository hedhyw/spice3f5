/*
 * Analyses
 */
#ifndef TABLES_ONLY
#endif

/*
 * Devices
 */

/* DO NOT MODIFY ANYTHING BELOW THIS LINE */

char	Spice_Version[ ] = "3f5";
char	Spice_Notice[ ] = "";
char	Spice_Build_Date[ ] = "";

/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
**********/

#include "spice.h"

#define CONFIG
#include <stdio.h>
#include "ifsim.h"
#include "suffix.h"

IFsimulator SIMinfo = {
    "nutmeg",       /* my name */
    "data analysis and manipulation program",  /* more about me */
    Spice_Version,         /* my version */
    NULL,       /* newCircuit function */
    NULL,       /* deleteCircuit function */
    NULL,           /* newNode function */ /* NEEDED */
    NULL,           /* groundNode function */
    NULL,           /* bindNode function */
    NULL,           /* findNode function */ /* NEEDED */
    NULL,           /* instToNode function */ /* NEEDED */
    NULL,           /* setNodeParm function */ /* NEEDED */
    NULL,           /* askNodeQuest function */ /* NEEDED */
    NULL,           /* deleteNode function */ /* NEEDED */
    NULL,           /* newInstance function */
    NULL,           /* setInstanceParm function */
    NULL,           /* askInstanceQuest function */
    NULL,           /* findInstance funciton */
    NULL,           /* deleteInstance function */ /* to be added later */
    NULL,           /* newModel function */
    NULL,           /* setModelParm function */
    NULL,           /* askModelQuest function */
    NULL,           /* findModel function */
    NULL,           /* deleteModel function */ /* to be added later */
    NULL,           /* newTask function */
    NULL,           /* newAnalysis function */
    NULL,           /* setAnalysisParm function */
    NULL,           /* askAnalysisQeust function */
    NULL,           /* findAnalysis function */
    NULL,           /* findTask function */
    NULL,           /* deleteTask function */
    NULL,           /* doAnalyses function */
    NULL,	    /* non-convergence message function */
    0,
    NULL,
    0,
    NULL,
    0,
    NULL,
};
