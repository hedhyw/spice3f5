/*
 * Copyright (c) 1985 Thomas L. Quarles
 */
#ifndef CKT
#define CKT "CKTdefs.h $Revision: 1.13 $  on $Date: 89/04/05 13:09:39 $ "

#define MAXNUMDEVS 32
extern int DEVmaxnum;
#define MAXNUMDEVNODES 4

#include "smpdefs.h"
#include "ifsim.h"
#include "acdefs.h"
#include "gendefs.h"
#include "trcvdefs.h"
#include "optdefs.h"
#include "sen2defs.h"
#include "pzdefs.h"



typedef struct sCKTnode {
    IFuid name;
    int type;

#define SP_VOLTAGE 3
#define SP_CURRENT 4
#define NODE_VOLTAGE SP_VOLTAGE
#define NODE_CURRENT SP_CURRENT

    int number;
    double ic;
    double nodeset;
    double *ptr;
    struct sCKTnode *next;
    unsigned int icGiven:1;
    unsigned int nsGiven:1;
} CKTnode;

/* defines for node parameters */
#define PARM_NS 1
#define PARM_IC 2
#define PARM_NODETYPE 3



typedef struct {
    GENmodel *CKThead[MAXNUMDEVS];
    STATistics *CKTstat;
    double *(CKTstates[8]);
#define CKTstate0 CKTstates[0]
#define CKTstate1 CKTstates[1]
#define CKTstate2 CKTstates[2]
#define CKTstate3 CKTstates[3]
#define CKTstate4 CKTstates[4]
#define CKTstate5 CKTstates[5]
#define CKTstate6 CKTstates[6]
#define CKTstate7 CKTstates[7]
    double CKTtime;
    double CKTdelta;
    double CKTdeltaOld[7];
    double CKTtemp;
    double CKTnomTemp;
    double CKTvt;
    double CKTag[7];        /* the gear variable coefficient matrix */
#ifdef PREDICTOR
    double CKTagp[7];       /* the gear predictor variable coefficient matrix */
#endif /*PREDICTOR*/
    int CKTorder;           /* the integration method order */
    int CKTmaxOrder;        /* maximum integration method order */
    int CKTintegrateMethod; /* the integration method to be used */

/* known integration methods */
#define TRAPEZOIDAL 1
#define GEAR 2

    SMPmatrix *CKTmatrix;   /* pointer to sparse matrix */
    int CKTniState;         /* internal state */
    double *CKTrhs;         /* current rhs value - being loaded */
    double *CKTrhsOld;      /* previous rhs value for convergence testing */
    double *CKTrhsSpare;    /* spare rhs value for reordering */
    double *CKTirhs;        /* current rhs value - being loaded (imag) */
    double *CKTirhsOld;     /* previous rhs value (imaginary)*/
    double *CKTirhsSpare;   /* spare rhs value (imaginary)*/
#ifdef PREDICTOR
    double *CKTpred;        /* predicted solution vector */
    double *CKTsols[8];     /* previous 8 solutions */
#endif /* PREDICTOR */

    double *CKTrhsOp;      /* opearating point values */
    double *CKTsenRhs;      /* current sensitivity rhs  values */
    double *CKTseniRhs;      /* current sensitivity rhs  values (imag)*/


/*
 *  symbolic constants for CKTniState
 *      Note that they are bitwise disjoint
 */

#define NISHOULDREORDER 0x1
#define NIREORDERED 0x2
#define NIUNINITIALIZED 0x4
#define NIACSHOULDREORDER 0x10
#define NIACREORDERED 0x20
#define NIACUNINITIALIZED 0x40
#define NIDIDPREORDER 0x100
#define NIPZSHOULDREORDER 0x200

    int CKTmaxEqNum;
    int CKTcurrentAnalysis; /* the analysis in progress (if any) */

/* defines for the value of  CKTcurrentAnalysis */
/* are in TSKdefs.h */

    CKTnode *CKTnodes;
    CKTnode *CKTlastNode;
#define NODENAME(ckt,nodenum) CKTnodName(ckt,nodenum)
    int CKTnumStates;
    long CKTmode;

/* defines for CKTmode */

/* old 'mode' parameters */
#define MODE 0x3
#define MODETRAN 0x1
#define MODEAC 0x2

/* old 'modedc' parameters */
#define MODEDC 0x70
#define MODEDCOP 0x10
#define MODETRANOP 0x20
#define MODEDCTRANCURVE 0x40

/* old 'initf' parameters */
#define INITF 0x3f00
#define MODEINITFLOAT 0x100
#define MODEINITJCT 0x200
#define MODEINITFIX 0x400
#define MODEINITSMSIG 0x800
#define MODEINITTRAN 0x1000
#define MODEINITPRED 0x2000

/* old 'nosolv' paramater */
#define MODEUIC 0x10000l

    int CKTbypass;
    int CKTdcMaxIter;       /* iteration limit for dc op.  (itl1) */
    int CKTdcTrcvMaxIter;   /* iteration limit for dc tran. curv (itl2) */
    int CKTtranMaxIter;     /* iteration limit for each timepoint for tran*/
                            /* (itl4) */
    int CKTbreakSize;
    int CKTbreak;
    double CKTsaveDelta;
    double CKTminBreak;
    double *CKTbreaks;
    double CKTabstol;
    double CKTpivotAbsTol;
    double CKTpivotRelTol;
    double CKTreltol;
    double CKTchgtol;
    double CKTvoltTol;
#ifdef NEWTRUNC
    double CKTlteReltol;
    double CKTlteAbstol;
#endif /* NEWTRUNC */
    double CKTgmin;
    double CKTdelmin;
    double CKTtrtol;
    double CKTfinalTime;
    double CKTstep;
    double CKTmaxStep;
    double CKTinitTime;
    double CKTomega;
    double CKTsrcFact;
    double CKTdiagGmin;
    int CKTnumSrcSteps;
    int CKTnumGminSteps;
    int CKTnoncon;
    double CKTdefaultMosL;
    double CKTdefaultMosW;
    double CKTdefaultMosAD;
    double CKTdefaultMosAS;
    unsigned int CKThadNodeset:1;
    unsigned int CKTfixLimit:1; /* flag to indicate that the limiting of 
                                 * MOSFETs should be done as in SPICE2 */
    unsigned int CKTnoOpIter:1; /* flag to indicate not to try the operating
                                 * point brute force, but to use gmin stepping
                                 * first */
    unsigned int CKTisSetup:1;  /* flag to indicate if CKTsetup done */
    JOB *CKTcurJob;

    SENstruct *CKTsenInfo;	/* the sensitivity information */
    double *CKTtimePoints;	/* list of all accepted timepoints in the
				   current transient simulation */
    double *CKTdeltaList;	/* list of all timesteps in the current
				   transient simulation */
    int CKTtimeListSize;	/* size of above lists */
    int CKTtimeIndex;		/* current position in above lists */
    int CKTsizeIncr;		/* amount to increment size of above arrays
				   when you run out of space */
    unsigned int CKTtryToCompact:1; /* try to compact past history for LTRA
				   lines */
    unsigned int CKTbadMos3:1; /* Use old, unfixed MOS3 equations */
    unsigned int CKTkeepOpInfo:1; /* flag for small signal analyses */
    int CKTtroubleNode;		/* Non-convergent node number */
    GENinstance *CKTtroubleElt;	/* Non-convergent device instance */

} CKTcircuit;

#ifdef __STDC__
int ACan( CKTcircuit *, int );
int ACaskQuest( CKTcircuit *, GENERIC *, int , IFvalue *);
int ACsetParm( CKTcircuit *, GENERIC *, int , IFvalue *);
int CKTacDump( CKTcircuit *, double , GENERIC *);
int CKTacLoad( CKTcircuit *);
int CKTaccept( CKTcircuit *);
int CKTacct( CKTcircuit *, GENERIC *, int , IFvalue *);
int CKTask( GENERIC *, GENERIC *, int , IFvalue *, IFvalue *);
int CKTaskAnalQ( GENERIC *, GENERIC *, int , IFvalue *, IFvalue *);
int CKTaskNodQst( GENERIC *, GENERIC *, int , IFvalue *, IFvalue *);
int CKTbindNode( GENERIC *, GENERIC *, int , GENERIC *);
void CKTbreakDump( CKTcircuit *);
int CKTclrBreak( CKTcircuit *);
int CKTconvTest( CKTcircuit *);
int CKTcrtElt( GENERIC *, GENERIC *, GENERIC **, IFuid );
int CKTdelTask( GENERIC *, GENERIC *);
int CKTdestroy( GENERIC *);
int CKTdltAnal( GENERIC *, GENERIC *, GENERIC *);
int CKTdltInst( GENERIC *, GENERIC *);
int CKTdltMod( GENERIC *, GENERIC *);
int CKTdltNod( GENERIC *, GENERIC *);
int CKTdoJob( GENERIC *, int , GENERIC *);
void CKTdump( CKTcircuit *, double, GENERIC *);
int CKTfndAnal( GENERIC *, int *, GENERIC **, IFuid , GENERIC *, IFuid );
int CKTfndBranch( CKTcircuit *, IFuid);
int CKTfndDev( GENERIC *, int *, GENERIC **, IFuid , GENERIC *, IFuid );
int CKTfndMod( GENERIC *, int *, GENERIC **, IFuid );
int CKTfndNode( GENERIC *, GENERIC **, IFuid );
int CKTfndTask( GENERIC *, GENERIC **, IFuid  );
int CKTground( GENERIC *, GENERIC **, IFuid );
int CKTic( CKTcircuit *);
int CKTinit( GENERIC **);
int CKTinst2Node( GENERIC *, GENERIC *, int , GENERIC **, IFuid *);
int CKTlinkEq(CKTcircuit*,CKTnode*);
int CKTload( CKTcircuit *);
int CKTmapNode( GENERIC *, GENERIC **, IFuid );
int CKTmkCur( CKTcircuit  *, CKTnode **, IFuid , char *);
int CKTmkNode(CKTcircuit*,CKTnode**);
int CKTmkVolt( CKTcircuit  *, CKTnode **, IFuid , char *);
int CKTmodAsk( GENERIC *, GENERIC *, int , IFvalue *, IFvalue *);
int CKTmodCrt( GENERIC *, int , GENERIC **, IFuid );
int CKTmodParam( GENERIC *, GENERIC *, int , IFvalue *, IFvalue *);
int CKTnames(CKTcircuit *, int *, IFuid **);
int CKTnewAnal( GENERIC *, int , IFuid , GENERIC **, GENERIC *);
int CKTnewEq( GENERIC *, GENERIC **, IFuid );
int CKTnewNode( GENERIC *, GENERIC **, IFuid );
int CKTnewTask( GENERIC *, GENERIC **, IFuid );
IFuid CKTnodName( CKTcircuit *, int );
void CKTnodOut( CKTcircuit *);
CKTnode * CKTnum2nod( CKTcircuit *, int );
int CKTop(CKTcircuit *, long, long, int );
int CKTpModName( char *, IFvalue *, CKTcircuit *, int , IFuid , GENmodel **);
int CKTpName( char *, IFvalue *, CKTcircuit *, int , char *, GENinstance **);
int CKTparam( GENERIC *, GENERIC *, int , IFvalue *, IFvalue *);
int CKTpzFindZeros( CKTcircuit *, PZtrial **, int * );
int CKTpzLoad( CKTcircuit *, SPcomplex * );
int CKTpzSetup( CKTcircuit *, int);
int CKTsenAC( CKTcircuit *);
int CKTsenComp( CKTcircuit *);
int CKTsenDCtran( CKTcircuit *);
int CKTsenLoad( CKTcircuit *);
void CKTsenPrint( CKTcircuit *);
int CKTsenSetup( CKTcircuit *);
int CKTsenUpdate( CKTcircuit *);
int CKTsetAnalPm( GENERIC *, GENERIC *, int , IFvalue *, IFvalue *);
int CKTsetBreak( CKTcircuit *, double );
int CKTsetNodPm( GENERIC *, GENERIC *, int , IFvalue *, IFvalue *);
int CKTsetOpt( GENERIC *, GENERIC *, int , IFvalue *);
int CKTsetup( CKTcircuit *);
int CKTtemp( CKTcircuit *);
char *CKTtrouble(GENERIC *, char *);
void CKTterr( int , CKTcircuit *, double *);
int CKTtrunc( CKTcircuit *, double *);
int CKTtypelook( char *);
int DCOaskQuest( CKTcircuit *, GENERIC *, int , IFvalue *);
int DCOsetParm( CKTcircuit  *, GENERIC *, int , IFvalue *);
int DCTaskQuest( CKTcircuit *, GENERIC *, int , IFvalue *);
int DCTsetParm( CKTcircuit  *, GENERIC *, int , IFvalue *);
int DCop( CKTcircuit *);
int DCtrCurv( CKTcircuit *, int );
int DCtran( CKTcircuit *, int );
int DISTOan(CKTcircuit *, int);
int NOISEan(CKTcircuit *, int);
int PZan( CKTcircuit *, int );
int PZinit( CKTcircuit * );
int PZpost( CKTcircuit * );
int PZaskQuest( CKTcircuit *, GENERIC *, int , IFvalue *);
int PZsetParm( CKTcircuit *, GENERIC *, int , IFvalue *);
int SENaskQuest( CKTcircuit *, GENERIC *, int , IFvalue *);
void SENdestroy( SENstruct *);
int SENsetParm( CKTcircuit *, GENERIC *, int , IFvalue *);
int SENstartup( CKTcircuit *);
int SPIinit( IFfrontEnd *, IFsimulator **);
char * SPerror( int );
int TFanal( CKTcircuit *, int );
int TFaskQuest( CKTcircuit *, GENERIC *, int , IFvalue *);
int TFsetParm( CKTcircuit *, GENERIC *, int , IFvalue *);
int TRANaskQuest( CKTcircuit *, GENERIC *, int , IFvalue *);
int TRANsetParm( CKTcircuit *, GENERIC *, int , IFvalue *);
int TRANinit(CKTcircuit *, JOB *);
int NIacIter( CKTcircuit * );
int NIcomCof( CKTcircuit * ); 
int NIconvTest(CKTcircuit * );
void NIdestroy(CKTcircuit * );
int NIinit( CKTcircuit  * );
int NIintegrate( CKTcircuit *, double *, double *, double , int );
int NIiter( CKTcircuit * , int );
int NIpzMuller(PZtrial **, PZtrial *);
int NIpzComplex(PZtrial **, PZtrial *);
int NIpzSym(PZtrial **, PZtrial *);
int NIpzSym2(PZtrial **, PZtrial *);
int NIreinit( CKTcircuit *);
int NIsenReinit( CKTcircuit *);
#else /* stdc */
int ACan();
int ACaskQuest();
int ACsetParm();
int CKTacDump();
int CKTacLoad();
int CKTaccept();
int CKTacct();
int CKTask();
int CKTaskAnalQ();
int CKTaskNodQst();
int CKTbindNode();
void CKTbreakDump();
int CKTclrBreak();
int CKTcrtElt();
int CKTdelTask();
int CKTdestroy();
int CKTdltAnal();
int CKTdltInst();
int CKTdltMod();
int CKTdltNod();
int CKTdoJob();
void CKTdump();
int CKTfndAnal();
int CKTfndBranch();
int CKTfndDev();
int CKTfndMod();
int CKTfndNode();
int CKTfndTask();
int CKTground();
int CKTic();
int CKTinit();
int CKTinst2Node();
int CKTlinkEq();
int CKTload();
int CKTmapNode();
int CKTmkCur();
int CKTmkNode();
int CKTmkVolt();
int CKTmodAsk();
int CKTmodCrt();
int CKTmodParam();
int CKTnames();
int CKTnewAnal();
int CKTnewEq();
int CKTnewNode();
int CKTnewTask();
IFuid CKTnodName();
void CKTnodOut();
CKTnode * CKTnum2nod();
int CKTop();
int CKTpModName();
int CKTpName();
int CKTparam();
int CKTpzLoad();
int CKTpzSetup();
int CKTsenAC();
int CKTsenComp();
int CKTsenDCtran();
int CKTsenLoad();
void CKTsenPrint();
int CKTsenSetup();
int CKTsenUpdate();
int CKTsetAnalPm();
int CKTsetBreak();
int CKTsetNodPm();
int CKTsetOpt();
int CKTsetup();
int CKTpzSetup();
int CKTtemp();
char *CKTtrouble( );
void CKTterr();
int CKTtrunc();
int CKTtypelook();
int DCOaskQuest();
int DCOsetParm();
int DCTaskQuest();
int DCTsetParm();
int DCop();
int DCtrCurv();
int DCtran();
int DISTOan();
int NOISEan();
int PZan();
int PZaskQuest();
int PZsetParm();
int SENaskQuest();
void SENdestroy();
int SENsetParm();
int SENstartup();
int SPIinit();
char * SPerror();
int TFanal();
int TFaskQuest();
int TFsetParm();
int TRANaskQuest();
int TRANsetParm();
int TRANinit( );
int NIacIter();
int NIcomCof(); 
int NIconvTest();
void NIdestroy();
int NIinit();
int NIintegrate();
int NIiter();
int NIpzMuller();
int NIpzSolve();
int NIreinit();
int NIsenReinit();
#endif /* stdc */

extern IFfrontEnd *SPfrontEnd;

#endif /*CKT*/
