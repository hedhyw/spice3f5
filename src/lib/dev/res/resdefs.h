/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#ifndef RES
#define RES

#include "ifsim.h"
#include "cktdefs.h"
#include "gendefs.h"
#include "complex.h"
#include "noisedef.h"

        /* definitions used to describe resistors */


/* information used to describe a single instance */

typedef struct sRESinstance {
    struct sRESmodel *RESmodPtr;    /* backpointer to model */
    struct sRESinstance *RESnextInstance;   /* pointer to next instance of 
                                             * current model*/

    IFuid RESname;  /* pointer to character string naming this instance */
    int RESstate; /* not used */
    int RESposNode; /* number of positive node of resistor */
    int RESnegNode; /* number of negative node of resistor */

    double REStemp;     /* temperature at which this resistor operates */
    double RESconduct;  /* conductance at current analysis temperature */
    double RESresist;   /* resistance at temperature Tnom */
    double RESwidth;    /* width of the resistor */
    double RESlength;   /* length of the resistor */
    double *RESposPosptr;    /* pointer to sparse matrix diagonal at 
                              * (positive,positive) */
    double *RESnegNegptr;    /* pointer to sparse matrix diagonal at 
                              * (negative,negative) */
    double *RESposNegptr;    /* pointer to sparse matrix offdiagonal at 
                              * (positive,negative) */
    double *RESnegPosptr;    /* pointer to sparse matrix offdiagonal at 
                              * (negative,positive) */
    unsigned RESresGiven : 1;   /* flag to indicate resistance was specified */
    unsigned RESwidthGiven  : 1;    /* flag to indicate width given */
    unsigned RESlengthGiven : 1;    /* flag to indicate length given */
    unsigned REStempGiven   : 1;    /* indicates temperature specified */
    int    RESsenParmNo;         /* parameter # for sensitivity use;
                set equal to  0 if not a design parameter*/
#ifndef NONOISE
    double RESnVar[NSTATVARS];
#else /* NONOISE */
	double *RESnVar;
#endif /* NONOISE */

} RESinstance ;


/* per model data */

typedef struct sRESmodel {       /* model structure for a resistor */
    int RESmodType; /* type index of this device type */
    struct sRESmodel *RESnextModel; /* pointer to next possible model in 
                                     * linked list */
    RESinstance * RESinstances; /* pointer to list of instances that have this
                                 * model */
    IFuid RESmodName;       /* pointer to character string naming this model */

    double REStnom;         /* temperature at which resistance measured */
    double REStempCoeff1;   /* first temperature coefficient of resistors */
    double REStempCoeff2;   /* second temperature coefficient of resistors */
    double RESsheetRes;     /* sheet resistance of devices in ohms/square */
    double RESdefWidth;     /* default width of a resistor */
    double RESnarrow;       /* amount by which device is narrower than drawn */
    unsigned REStnomGiven: 1;   /* flag to indicate nominal temp. was given */
    unsigned REStc1Given : 1;   /* flag to indicate tc1 was specified */
    unsigned REStc2Given : 1;   /* flag to indicate tc2 was specified */
    unsigned RESsheetResGiven   :1; /* flag to indicate sheet resistance given*/
    unsigned RESdefWidthGiven   :1; /* flag to indicate default width given */
    unsigned RESnarrowGiven     :1; /* flag to indicate narrow effect given */
} RESmodel;

/* device parameters */
#define RES_RESIST 1
#define RES_WIDTH 2
#define RES_LENGTH 3
#define RES_CONDUCT 4
#define RES_RESIST_SENS 5
#define RES_CURRENT 6
#define RES_POWER 7
#define RES_TEMP 8

/* model parameters */
#define RES_MOD_TC1 101
#define RES_MOD_TC2 102
#define RES_MOD_RSH 103
#define RES_MOD_DEFWIDTH 104
#define RES_MOD_NARROW 105
#define RES_MOD_R 106
#define RES_MOD_TNOM 107

/* device questions */
#define RES_QUEST_SENS_REAL      201
#define RES_QUEST_SENS_IMAG      202
#define RES_QUEST_SENS_MAG       203
#define RES_QUEST_SENS_PH        204
#define RES_QUEST_SENS_CPLX      205
#define RES_QUEST_SENS_DC        206

/* model questions */

#include "resext.h"

#endif /*RES*/
