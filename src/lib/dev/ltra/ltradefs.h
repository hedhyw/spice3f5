/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1990 Jaijeet S. Roychowdhury
**********/

#ifndef LTRA
#define LTRA 
#undef LTRALTEINFO
#undef LTRADEBUG

#include "ifsim.h"
#include "cktdefs.h"
#include "gendefs.h"
#include "complex.h"

/* structures used to describe lossy transmission liness */

/* information used to describe a single instance */

typedef struct sLTRAinstance {
    struct sLTRAmodel *LTRAmodPtr;    /* backpointer to model */
    struct sLTRAinstance *LTRAnextInstance;   /* pointer to next instance of 
                                             * current model*/
    IFuid LTRAname;      /* pointer to character string naming this instance */
    int LTRAstate;	 /* not used */
    int LTRAposNode1;    /* number of positive node of end 1 of t. line */
    int LTRAnegNode1;    /* number of negative node of end 1 of t. line */
    int LTRAposNode2;    /* number of positive node of end 2 of t. line */
    int LTRAnegNode2;    /* number of negative node of end 2 of t. line */

    int LTRAbrEq1;       /* number of branch equation for end 1 of t. line */
    int LTRAbrEq2;       /* number of branch equation for end 2 of t. line */
    double LTRAinput1;   /* accumulated excitation for port 1 */
    double LTRAinput2;   /* accumulated excitation for port 2 */
    double LTRAinitVolt1;    /* initial condition:  voltage on port 1 */
    double LTRAinitCur1;     /* initial condition:  current at port 1 */
    double LTRAinitVolt2;    /* initial condition:  voltage on port 2 */
    double LTRAinitCur2;     /* initial condition:  current at port 2 */
    double *LTRAv1;     /* past values of v1 */
    double *LTRAi1;     /* past values of i1 */
    double *LTRAv2;     /* past values of v2 */
    double *LTRAi2;     /* past values of i2 */
    int LTRAinstListSize; /* size of above lists */

    double *LTRAibr1Ibr1Ptr;     /* pointer to sparse matrix */
    double *LTRAibr1Ibr2Ptr;     /* pointer to sparse matrix */
    double *LTRAibr1Pos1Ptr;     /* pointer to sparse matrix */
    double *LTRAibr1Neg1Ptr;     /* pointer to sparse matrix */
    double *LTRAibr1Pos2Ptr;     /* pointer to sparse matrix */
    double *LTRAibr1Neg2Ptr;     /* pointer to sparse matrix */
    double *LTRAibr2Ibr1Ptr;     /* pointer to sparse matrix */
    double *LTRAibr2Ibr2Ptr;     /* pointer to sparse matrix */
    double *LTRAibr2Pos1Ptr;     /* pointer to sparse matrix */
    double *LTRAibr2Neg1Ptr;     /* pointer to sparse matrix */
    double *LTRAibr2Pos2Ptr;     /* pointer to sparse matrix */
    double *LTRAibr2Neg2Ptr;     /* pointer to sparse matrix */
    double *LTRAneg1Ibr1Ptr;     /* pointer to sparse matrix */
    double *LTRAneg2Ibr2Ptr;     /* pointer to sparse matrix */
    double *LTRApos1Ibr1Ptr;     /* pointer to sparse matrix */
    double *LTRApos2Ibr2Ptr;     /* pointer to sparse matrix */
    double *LTRApos1Pos1Ptr;     /* pointer to sparse matrix */
    double *LTRAneg1Neg1Ptr;     /* pointer to sparse matrix */
    double *LTRApos2Pos2Ptr;     /* pointer to sparse matrix */
    double *LTRAneg2Neg2Ptr;     /* pointer to sparse matrix */

    unsigned LTRAicV1Given : 1;  /* flag to ind. init. voltage at port 1 given */
    unsigned LTRAicC1Given : 1;  /* flag to ind. init. current at port 1 given */
    unsigned LTRAicV2Given : 1;  /* flag to ind. init. voltage at port 2 given */
    unsigned LTRAicC2Given : 1;  /* flag to ind. init. current at port 2 given */
} LTRAinstance ;


/* per model data */

typedef struct sLTRAmodel {       /* model structure for a transmission lines */
    int LTRAmodType; /* type index of this device type */
    struct sLTRAmodel *LTRAnextModel; /* pointer to next possible model in 
                                     * linked list */
    LTRAinstance * LTRAinstances; /* pointer to list of instances that have this
                                 * model */
    IFuid LTRAmodName;       /* pointer to character string naming this model */

	double LTRAh1dashFirstVal; /* first needed value of h1dasg at 
									current timepoint */
	double LTRAh2FirstVal;	 /* first needed value of h2 at current 
									timepoint */
	double LTRAh3dashFirstVal; /* first needed value of h3dash at 
									current timepoint */

	/*double *LTRAh1dashValues;*/ /* values of h1dash for all previous 
									times */
	/*double *LTRAh2Values;*/	/* values of h2 for all previous times */
	/*double *LTRAh3dashValues;*/ /* values of h3dash for all previous
								 times */

	/*double LTRAh2FirstOthVal;/* needed for LTE calc; but their values */
	/*double LTRAh3dashFirstOthVal;*//*may depend on the current timepoint*/

/*	double *LTRAh1dashOthVals;*/ /* these lists of other values are */
/*	double *LTRAh2OthVals;*/	   /* needed for truncation error  */
/*	double *LTRAh3dashOthVals;*/ /* calculation */

				/* the OthVals do not depend on the current
				 * timepoint; hence they are set up in LTRAaccept.c.
				 * They are used in LTRAtrunc.c
				*/

	double LTRAh1dashFirstCoeff; /* first needed coeff of h1dash for 
									the current timepoint */
	double LTRAh2FirstCoeff; /* first needed coeff of h2 for the 
									current timepoint */
	double LTRAh3dashFirstCoeff; /* first needed coeff of h3dash for 
									the current timepoint */

    double *LTRAh1dashCoeffs; /* list of other coefficients for h1dash */
    double *LTRAh2Coeffs;     /* list of other coefficients for h2 */
    double *LTRAh3dashCoeffs; /* list of other coefficients for h3dash */
    int LTRAmodelListSize; /* size of above lists */

    double LTRAconduct;  /* conductance G  - input */
    double LTRAresist;   /* resistance R  - input */
    double LTRAinduct;   /* inductance L - input */
    double LTRAcapac;    /* capacitance C - input */
    double LTRAlength;	 /* length l - input */
    double LTRAtd;       /* propagation delay T - calculated*/
    double LTRAimped;    /* impedance Z - calculated*/
    double LTRAadmit;    /* admittance Y - calculated*/
    double LTRAalpha; 	 /* alpha - calculated */
    double LTRAbeta; 	 /* beta - calculated */
    double LTRAattenuation; /* e^(-beta T) - calculated */
	double LTRAcByR;     /* C/R - for the RC line - calculated */
	double LTRArclsqr;   /* RCl^2 - for the RC line - calculated */
    double LTRAintH1dash;/* \int_0^\inf h'_1(\tau) d \tau - calculated*/
    double LTRAintH2;/* \int_0^\inf h_2(\tau) d \tau - calculated*/
    double LTRAintH3dash;/* \int_0^\inf h'_3(\tau) d \tau - calculated*/

    double LTRAnl;       /* normalized length - historical significance only*/
    double LTRAf;        /* frequency at which nl is measured - historical significance only*/
	double LTRAcoshlrootGR; /* cosh(l*sqrt(G*R)), used for DC anal */
	double LTRArRsLrGRorG; /* sqrt(R)*sinh(l*sqrt(G*R))/sqrt(G) */
	double LTRArGsLrGRorR; /* sqrt(G)*sinh(l*sqrt(G*R))/sqrt(R) */

	/*int LTRAh1dashIndex;*/ /* index for h1dash that points to the
							latest nonzero coefficient in the list */
	/*int LTRAh2Index;*/	 /* ditto for h2 */
	/*int LTRAh3dashIndex;*/ /* ditto for h3dash */
	int LTRAauxIndex;    /* auxiliary index for h2 and h3dash */
	double LTRAstLineReltol;  /* separate reltol for checking st. lines */
	double LTRAchopReltol;  /* separate reltol for truncation of impulse responses*/
	double LTRAstLineAbstol;  /* separate abstol for checking st.  lines */
	double LTRAchopAbstol;  /* separate abstol for truncation of impulse responses */

    unsigned LTRAreltolGiven:1;  /* flag to ind. relative deriv. tol. given */
    unsigned LTRAabstolGiven:1;  /* flag to ind. absolute deriv. tol. given */
	unsigned LTRAtruncNR:1; /* flag to ind. use N-R iterations for calculating step in LTRAtrunc */
	unsigned LTRAtruncDontCut:1; /* flag to ind. don't bother about errors in impulse response calculations due to large steps*/
	double LTRAmaxSafeStep; /* maximum safe step for impulse response calculations */
    unsigned LTRAresistGiven : 1; /* flag to indicate R was specified */
    unsigned LTRAconductGiven : 1; /* flag to indicate G was specified */
    unsigned LTRAinductGiven : 1; /* flag to indicate L was specified */
    unsigned LTRAcapacGiven : 1; /* flag to indicate C was specified */
    unsigned LTRAlengthGiven : 1; /* flag to indicate length was specified */
    unsigned LTRAnlGiven : 1;    /* flag to indicate norm length was specified */
	int LTRAlteConType; 	/* indicates whether full control, half control or no control */
	int LTRAhowToInterp;  /* indicates how to interpolate for delayed timepoint */
	unsigned LTRAprintFlag: 1; /* flag to indicate whether debugging output should be printed */
	/*unsigned LTRArOnly: 1;*/ /* flag to indicate G=0, use known Bessel
								integrals for accuracy and speed */
	int LTRAstepLimit; 	/* flag to indicate that the timestep
									should always be limited to
									0.8*LTRAtd */
    unsigned LTRAfGiven : 1;     /* flag to indicate freq was specified */
    double LTRAabstol;       /* absolute deriv. tol. for breakpoint setting */
    double LTRAreltol;       /* relative deriv. tol. for breakpoint setting */
	int LTRAspecialCase; /* what kind of model (RC, RLC, RL, ...) */
} LTRAmodel;

/* device parameters */
#define LTRA_MOD_LTRA	0
#define LTRA_MOD_R 1
#define LTRA_MOD_L 2
#define LTRA_MOD_G 3
#define LTRA_MOD_C 4
#define LTRA_MOD_LEN 5
#define LTRA_V1 6
#define LTRA_I1 7
#define LTRA_V2 8
#define LTRA_I2 9
#define LTRA_IC 10
#define LTRA_MOD_RELTOL 11
#define LTRA_MOD_ABSTOL 12
#define LTRA_POS_NODE1 13
#define LTRA_NEG_NODE1 14
#define LTRA_POS_NODE2 15
#define LTRA_NEG_NODE2 16
#define LTRA_INPUT1 17
#define LTRA_INPUT2 18
#define LTRA_DELAY 19
#define LTRA_BR_EQ1 20
#define LTRA_BR_EQ2 21
#define LTRA_MOD_NL 22
#define LTRA_MOD_FREQ 23
#define LTRA_MOD_Z0 24
#define LTRA_MOD_TD 25

#define LTRA_MOD_FULLCONTROL 26
#define LTRA_MOD_HALFCONTROL 27
#define LTRA_MOD_NOCONTROL 28
#define LTRA_MOD_PRINT 29
#define LTRA_MOD_NOPRINT 30
/*
#define LTRA_MOD_RONLY 31
*/
#define LTRA_MOD_STEPLIMIT	32
#define LTRA_MOD_NOSTEPLIMIT 33
#define LTRA_MOD_LININTERP 34
#define LTRA_MOD_QUADINTERP 35
#define LTRA_MOD_MIXEDINTERP 36
#define LTRA_MOD_RLC	37
#define LTRA_MOD_RC	38
#define LTRA_MOD_RG 39
#define LTRA_MOD_LC 40
#define LTRA_MOD_RL 41
#define LTRA_MOD_STLINEREL 42
#define LTRA_MOD_STLINEABS 43
#define LTRA_MOD_CHOPREL 44
#define LTRA_MOD_CHOPABS 45
#define LTRA_MOD_TRUNCNR 46
#define LTRA_MOD_TRUNCDONTCUT	47



/* model parameters */

/* device questions */

/* model questions */

#include "ltraext.h"

#endif /*LTRA*/

