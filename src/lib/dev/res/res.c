/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "resdefs.h"
#include "devdefs.h"
#include "ifsim.h"
#include "suffix.h"

IFparm RESpTable[] = { /* parameters */ 
 IOPP( "resistance", RES_RESIST, IF_REAL,"Resistance"),
 IOPZU( "temp",        RES_TEMP,   IF_REAL,"Instance operating temperature"),
 IOPQU( "l",          RES_LENGTH, IF_REAL,"Length"),
 IOPZU( "w",          RES_WIDTH,  IF_REAL,"Width"),
 IP(   "sens_resist", RES_RESIST_SENS, IF_FLAG,   
        "flag to request sensitivity WRT resistance"),
 OP( "i",          RES_CURRENT,IF_REAL,"Current"),
 OP( "p",          RES_POWER,  IF_REAL,"Power"),
 OPU( "sens_dc",  RES_QUEST_SENS_DC,  IF_REAL,    "dc sensitivity "),
 OPU( "sens_real",RES_QUEST_SENS_REAL,IF_REAL,    
        "dc sensitivity and real part of ac sensitivity"),
 OPU( "sens_imag",RES_QUEST_SENS_IMAG,IF_REAL,    
        "dc sensitivity and imag part of ac sensitivity"),
 OPU( "sens_mag", RES_QUEST_SENS_MAG, IF_REAL, "ac sensitivity of magnitude"),
 OPU( "sens_ph",  RES_QUEST_SENS_PH,  IF_REAL,    "ac sensitivity of phase"),
 OPU( "sens_cplx",RES_QUEST_SENS_CPLX,IF_COMPLEX,    "ac sensitivity")
} ;

IFparm RESmPTable[] = { /* model parameters */
 IOPQ( "rsh",    RES_MOD_RSH,      IF_REAL,"Sheet resistance"),
 IOPZ( "narrow", RES_MOD_NARROW,   IF_REAL,"Narrowing of resistor"),
 IOPQ( "tc1",    RES_MOD_TC1,      IF_REAL,"First order temp. coefficient"),
 IOPQO( "tc2",    RES_MOD_TC2,      IF_REAL,"Second order temp. coefficient"),
 IOPX( "defw",   RES_MOD_DEFWIDTH, IF_REAL,"Default device width"),
 IOPXU("tnom",  RES_MOD_TNOM,     IF_REAL,"Parameter measurement temperature"),
 IP( "r",      RES_MOD_R,        IF_FLAG,"Device is a resistor model")
};

char *RESnames[] = {
    "R+",
    "R-"
};

int	RESnSize = NUMELEMS(RESnames);
int	RESpTSize = NUMELEMS(RESpTable);
int	RESmPTSize = NUMELEMS(RESmPTable);
int	RESiSize = sizeof(RESinstance);
int	RESmSize = sizeof(RESmodel);
