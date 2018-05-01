/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1986 Thomas L. Quarles
**********/

#ifndef IFSIMULATOR
#define IFSIMULATOR

/*
 * We don't always have access to an ANSI C compiler yet, so we
 * make the following convenient definition
 */

#ifdef __STDC__

    /* using an ansi C compiler, so we have the void* construct */

typedef void GENERIC;

#else

    /* not using an ansi C compiler, so we have to use char* as the */
    /* most generic pointer type */

typedef char GENERIC;

#endif
/*
 * structure:   IFparm
 *
 *
 * The structure used to describe all values passed
 * between the front end and the simulator when there is any
 * possibility one argument of the function could have more
 * than one type.
 *
 * keyword is provided for the front end and is the token
 *    the user is expected to label the data with.
 *
 * id is an integer intended to uniquely identify the parameter
 *    to the simulator
 *
 * dataType is an integer which indicates the type of argument
 *    that must be passed for this parameter
 *
 * description is a longer description intended for help menus
 *    the description should all fit on one line, but should
 *    give a knowledgable user a good idea what the parameter is
 *    used for.
 */

typedef struct sIFparm {
    char *keyword;
    int id;
    int dataType;
    char *description;
} IFparm;

/*
 *
 * datatype: IFuid
 *
 * unique identifier for all name-type data in the simulator.
 * this permits the front end to use something other than
 * a unique, fully qualified character string to identify
 * an object.
 *
 */

typedef GENERIC *IFuid;

/*
 *
 * types for IFnewUid
 *
 */

#define UID_ANALYSIS 0x1
#define UID_TASK 0x2
#define UID_INSTANCE 0x4
#define UID_MODEL 0x8
#define UID_SIGNAL 0x10
#define UID_OTHER 0x20


/*
 * dataType values:
 *
 * Note:  These structures are put together by ORing together the
 *    appropriate bits from the fields below as is shown for the vector
 *    types.
 * IF_REQUIRED indicates that the parameter must be specified.
 *    The front end does not NEED to check for this, but can to save time,
 *    since failure to do so will cause the simulator to fail.
 * IF_SET indicates that the specified item is an input parameter.
 * IF_ASK indicates that the specified item is something the simulator
 *    can provide information about.
 * IF_SET and IF_ASK are NOT mutually exclusive.
 * if IF_SET and IF_ASK are both zero, it indicates a parameter that
 *    the simulator recoginizes are being a reasonable paremeter, but
 *    which this simulator does not implement.
 */

#define IF_FLAG 0x1
#define IF_INTEGER 0x2
#define IF_REAL 0x4
#define IF_COMPLEX 0x8
#define IF_NODE 0x10
#define IF_STRING 0x20
#define IF_INSTANCE 0x40
#define IF_PARSETREE 0x80

/* indicates that for a query the integer field will have a selector
 * in it to pick a sub-field */
#define IF_SELECT 0x800
#define IF_VSELECT 0x400

/* indicates a vector of the specified type */
#define IF_VECTOR 0x8000

#define IF_FLAGVEC     (IF_FLAG|IF_VECTOR)
#define IF_INTVEC      (IF_INTEGER|IF_VECTOR)
#define IF_REALVEC     (IF_REAL|IF_VECTOR)
#define IF_CPLXVEC     (IF_COMPLEX|IF_VECTOR)
#define IF_NODEVEC     (IF_NODE|IF_VECTOR)
#define IF_STRINGVEC   (IF_STRING|IF_VECTOR)
#define IF_INSTVEC     (IF_INSTANCE|IF_VECTOR)

#define IF_REQUIRED 0x4000

#define IF_VARTYPES 0x80ff

#define IF_SET 0x2000
#define IF_ASK 0x1000

/* If you AND with IF_UNIMP_MASK and get 0, it is recognized, but not
 * implemented
 */
#define IF_UNIMP_MASK (~0xfff)

/* Used by sensetivity to check if a parameter is or is not useful */
#define IF_REDUNDANT	0x0010000
#define IF_PRINCIPAL	0x0020000
#define IF_AC		0x0040000
#define IF_AC_ONLY	0x0080000
#define IF_NOISE	0x0100000
#define IF_NONSENSE	0x0200000

#define IF_SETQUERY	0x0400000
#define IF_ORQUERY	0x0800000
#define IF_CHKQUERY	0x1000000

/* For "show" command: do not print value in a table by default */
#define IF_UNINTERESTING 0x2000000

/* Structure:   IFparseTree
 *
 * This structure is returned by the parser for a IF_PARSETREE valued
 * parameter and describes the information that the simulator needs
 * to know about the parse tree in order to use it.
 * It is expected that the front end will have a more extensive
 * structure which this structure will be a prefix of.
 *
 * Note that the function pointer is provided as a hook for
 * versions which may want to compile code for the parse trees
 * if they are used heavily.
 *
 */

typedef struct sIFparseTree {
    int numVars;            /* number of variables used */
    int *varTypes;          /* array of types of variables */
    union uIFvalue * vars;  /* array of structures describing values */
#ifdef __STDC__
    int ((*IFeval)(struct sIFparseTree*,double,double*,double*,double*));
#else
    int ((*IFeval)());      /* function to call to get evaluated */
#endif /* STDC */
} IFparseTree;


/*
 * Structure:    IFvalue
 *
 * structure used to pass the values corresponding to the above
 * dataType.  All types are passed in one of these structures, with
 * relatively simple rules concerning the handling of the structure.
 *
 * whoever makes the subroutine call allocates a single instance of the
 * structure.  The basic data structure belongs to you, and you
 * should arrange to free it when appropriate.
 *
 * The responsibilities of the data supplier are:
 * Any vectors referenced by the structure are to be malloc()'d
 * and are assumed to have been turned over to the recipient and
 * thus should not be re-used or free()'d.
 *
 * The responsibilities of the data recipient are:
 * scalar valued data is to be copied by the recipient
 * vector valued data is now the property of the recipient,
 * and must be free()'d when no longer needed.
 *
 * Character strings are a special case:  Since it is assumed
 * that all character strings are directly descended from input
 * tokens, it is assumed that they are static, thus nobody
 * frees them until the circuit is deleted, when the front end
 * may do so.
 *
 * EVERYBODY's responsibility is to be SURE that the right data
 * is filled in and read out of the structure as per the IFparm
 * structure describing the parameter being passed.  Programs
 * neglecting this rule are fated to die of data corruption
 *
 */

/*
 * Some preliminary definitions:
 *
 * IFnode's are returned by the simulator, thus we don't really
 * know what they look like, just that we get to carry pointers
 * to them around all the time, and will need to save them occasionally
 *
 */


typedef void * IFnode;

/*
 * and of course, the standard complex data type
 */
typedef struct sIFcomplex {
    double real;
    double imag;
} IFcomplex;


typedef union uIFvalue {
    int iValue;             /* integer or flag valued data */
    double rValue;          /* real valued data */
    IFcomplex cValue;       /* complex valued data */
    char *sValue;           /* string valued data */
    IFuid uValue;           /* UID valued data */
    IFnode nValue;          /* node valued data */
    IFparseTree *tValue;    /* parse tree */
    struct {
        int numValue;       /* length of vector */
        union {
            int *iVec;      /* pointer to integer vector */
            double *rVec;   /* pointer to real vector */
            IFcomplex *cVec;/* pointer to complex vector */
            char **sVec;    /* pointer to string vector */
            IFuid *uVec;    /* pointer to UID vector */
            IFnode *nVec;   /* pointer to node vector */
        }vec;
    }v;
} IFvalue;


/*
 * structure:  IFdevice
 *
 * This structure contains all the information available to the
 * front end about a particular device.  The simulator will
 * present the front end with an array of pointers to these structures
 * which it will use to determine legal device types and parameters.
 *
 * Note to simulators:  you are passing an array of pointers to
 * these structures, so you may in fact make this the first component
 * in a larger, more complex structure which includes other data
 * which you need, but which is not needed in the common
 * front end interface.
 *
 */

typedef struct sIFdevice {
    char *name;                 /* name of this type of device */
    char *description;          /* description of this type of device */

    int *terms;                 /* number of terminals on this device */
    int *numNames;              /* number of names in termNames */
    char **termNames;           /* pointer to array of pointers to names */
                                /* array contains 'terms' pointers */

    int *numInstanceParms;      /* number of instance parameter descriptors */
    IFparm *instanceParms;      /* array  of instance parameter descriptors */

    int *numModelParms;         /* number of model parameter descriptors */
    IFparm *modelParms;         /* array  of model parameter descriptors */

    int	flags;			/* DEV_ */

} IFdevice;


/*
 * Structure: IFanalysis
 *
 * This structure contains all the information available to the
 * front end about a particular analysis type.  The simulator will
 * present the front end with an array of pointers to these structures
 * which it will use to determine legal analysis types and parameters.
 *
 * Note to simulators:  As for IFdevice above, you pass an array of pointers
 * to these, so you can make this structure a prefix to a larger structure
 * which you use internally.
 *
 */

typedef struct sIFanalysis {
    char *name;                 /* name of this analysis type */
    char *description;          /* description of this type of analysis */

    int numParms;               /* number of analysis parameter descriptors */
    IFparm *analysisParms;      /* array  of analysis parameter descriptors */

} IFanalysis;


/*
 * Structure: IFsimulator
 *
 * This is what we have been leading up to all along.
 * This structure describes a simulator to the front end, and is
 * returned from the SIMinit command to the front end.
 * This is where all those neat structures we described in the first
 * few hundred lines of this file come from.
 *
 */

typedef struct sIFsimulator {
    char *simulator;                /* the simulator's name */
    char *description;              /* description of this simulator */
    char *version;                  /* version or revision level of simulator*/

#ifdef __STDC__
    int ((*newCircuit)(GENERIC **));
                                    /* create new circuit */
    int ((*deleteCircuit)(GENERIC *));
                                    /* destroy old circuit's data structures*/

    int ((*newNode)(GENERIC *,GENERIC**,IFuid));
                                    /* create new node */
    int ((*groundNode)(GENERIC*,GENERIC**,IFuid));
                                    /* create ground node */
    int ((*bindNode)(GENERIC *,GENERIC*,int,GENERIC*));
                                    /* bind a node to a terminal */
    int ((*findNode)(GENERIC *,GENERIC**,IFuid));
                                    /* find a node by name */
    int ((*instToNode)(GENERIC *,GENERIC *,int,GENERIC **,IFuid *));
                                    /* find the node attached to a terminal */
    int ((*setNodeParm)(GENERIC*,GENERIC*,int,IFvalue*,IFvalue*));
                                    /* set a parameter on a node */
    int ((*askNodeQuest)(GENERIC*,GENERIC*,int,IFvalue*,IFvalue*));
                                    /* ask a question about a node */
    int ((*deleteNode)(GENERIC*,GENERIC*));
                                    /* delete a node from the circuit */

    int ((*newInstance)(GENERIC*,GENERIC*,GENERIC**,IFuid));
                                    /* create new instance */
    int ((*setInstanceParm)(GENERIC*,GENERIC*,int,IFvalue*,IFvalue*));
                                    /* set a parameter on an instance */
    int ((*askInstanceQuest)(GENERIC*,GENERIC*,int,IFvalue*,IFvalue*));
                                    /* ask a question about an instance */
    int ((*findInstance)(GENERIC*,int*,GENERIC**,IFuid,GENERIC*,IFuid));
                                    /* find a specific instance */
    int ((*deleteInstance)(GENERIC*,GENERIC*));
                                    /* delete an instance from the circuit */

    int ((*newModel)(GENERIC*,int,GENERIC**,IFuid));
                                    /* create new model */
    int ((*setModelParm)(GENERIC*,GENERIC*,int,IFvalue*,IFvalue*));
                                    /* set a parameter on a model */
    int ((*askModelQuest)(GENERIC*,GENERIC*,int,IFvalue*,IFvalue*));
                                    /* ask a questions about a model */
    int ((*findModel)(GENERIC*,int*,GENERIC**,IFuid));
                                    /* find a specific model */
    int ((*deleteModel)(GENERIC*,GENERIC*));
                                    /* delete a model from the circuit*/

    int ((*newTask)(GENERIC*,GENERIC**,IFuid));
                                    /* create a new task */
    int ((*newAnalysis)(GENERIC*,int,IFuid,GENERIC**,GENERIC*));
                                    /* create new analysis within a task */
    int ((*setAnalysisParm)(GENERIC*,GENERIC*,int,IFvalue*,IFvalue*));
                                    /* set a parameter on an analysis  */
    int ((*askAnalysisQuest)(GENERIC*,GENERIC*,int,IFvalue*,IFvalue*));
                                    /* ask a question about an analysis */
    int ((*findAnalysis)(GENERIC*,int*,GENERIC**,IFuid,GENERIC*,IFuid));
                                    /* find a specific analysis */
    int ((*findTask)(GENERIC*,GENERIC**,IFuid));
                                    /* find a specific task */
    int ((*deleteTask)(GENERIC*,GENERIC*));
                                    /* delete a task */

    int ((*doAnalyses)(GENERIC*,int,GENERIC*));
    char *((*nonconvErr)(GENERIC*,char *)); /* return nonconvergence error */
#else
    int ((*newCircuit)());          /* create new circuit */
    int ((*deleteCircuit)());       /* destroy old circuit's data structures */

    int ((*newNode)());             /* create new node */
    int ((*groundNode)());          /* create ground node */
    int ((*bindNode)());            /* bind a node to a terminal */
    int ((*findNode)());            /* find a node by name */
    int ((*instToNode)());          /* find the node attached to a terminal */
    int ((*setNodeParm)());         /* set a parameter on a node */
    int ((*askNodeQuest)());        /* ask a question about a node */
    int ((*deleteNode)());          /* delete a node from the circuit */

    int ((*newInstance)());         /* create new instance */
    int ((*setInstanceParm)());     /* set a parameter on an instance */
    int ((*askInstanceQuest)());    /* ask a question about an instance */
    int ((*findInstance)());        /* find a specific instance */
    int ((*deleteInstance)());      /* delete an instance from the circuit */

    int ((*newModel)());            /* create new model */
    int ((*setModelParm)());        /* set a parameter on a model */
    int ((*askModelQuest)());       /* ask a questions about a model */
    int ((*findModel)());           /* find a specific model */
    int ((*deleteModel)());         /* delete a model from the circuit*/

    int ((*newTask)());             /* create a new task */
    int ((*newAnalysis)());         /* create new analysis within a task */
    int ((*setAnalysisParm)());     /* set a parameter on an analysis  */
    int ((*askAnalysisQuest)());    /* ask a question about an analysis */
    int ((*findAnalysis)());        /* find a specific analysis */
    int ((*findTask)());            /* find a specific task */
    int ((*deleteTask)());          /* delete a task */

    int ((*doAnalyses)());          /* run a specified task */
    char *((*nonconvErr)());	    /* return nonconvergence error */
#endif /* STDC */

    int numDevices;                 /* number of device types supported */
    IFdevice **devices;             /* array of device type descriptors */

    int numAnalyses;                /* number of analysis types supported */
    IFanalysis **analyses;          /* array of analysis type descriptors */

    int numNodeParms;               /* number of node parameters supported */
    IFparm *nodeParms;              /* array of node parameter descriptors */

    int numSpecSigs;        /* number of special signals legal in parse trees */
    char **specSigs;        /* names of special signals legal in parse trees */

} IFsimulator;

/*
 * Structure: IFfrontEnd
 *
 * This structure provides the simulator with all the information
 * it needs about the front end.  This is the entire set of
 * front end and back end related routines the simulator
 * should know about.
 *
 */

typedef struct sIFfrontEnd {
#ifdef __STDC__
    int ((*IFnewUid)(GENERIC*,IFuid*,IFuid,char*,int,GENERIC**));
                            /* create a new UID in the circuit */
    int ((*IFdelUid)(GENERIC*,IFuid,int));
			    /* create a new UID in the circuit */
    int ((*IFpauseTest)(void));
                            /* should we stop now? */
    double ((*IFseconds)(void));
                            /* what time is it? */
    int ((*IFerror)(int,char*,IFuid*));
                            /* output an error or warning message */
    int ((*OUTpBeginPlot)(GENERIC*,GENERIC*,IFuid,IFuid,int,
            int,IFuid*,int,GENERIC**));
                            /* start pointwise output plot */
    int ((*OUTpData)(GENERIC*,IFvalue*,IFvalue*));
                            /* data for pointwise plot */
    int ((*OUTwBeginPlot)(GENERIC*,GENERIC*,IFuid,IFuid,int,
            int,IFuid*,int,GENERIC**));
                            /* start windowed output plot */
    int ((*OUTwReference)(GENERIC*,IFvalue*,GENERIC**));
                            /* independent vector for windowed plot */
    int ((*OUTwData)(GENERIC*,int,IFvalue*,GENERIC*));
                            /* data for windowed plot */
    int ((*OUTwEnd)(GENERIC*));
                            /* signal end of windows */
    int ((*OUTendPlot)(GENERIC*));
                            /* end of plot */
    int ((*OUTbeginDomain)(GENERIC*,IFuid,int,IFvalue*));
                            /* start nested domain */
    int ((*OUTendDomain)(GENERIC*));
                            /* end nested domain */
    int ((*OUTattributes)(GENERIC *,IFuid*,int,IFvalue*));
                            /* specify output attributes of node */
#else /* not STDC */
    int ((*IFnewUid)());    /* create a new UID in the circuit */
    int ((*IFdelUid)());    /* create a new UID in the circuit */
    int ((*IFpauseTest)()); /* should we stop now? */
    double ((*IFseconds)());   /* what time is it? */
    int ((*IFerror)());     /* output an error or warning message */
    int ((*OUTpBeginPlot)());   /* start pointwise output plot */
    int ((*OUTpData)());        /* data for pointwise plot */
    int ((*OUTwBeginPlot)());   /* start windowed output plot */
    int ((*OUTwReference)());   /* independent vector for windowed plot */
    int ((*OUTwData)());        /* data for windowed plot */
    int ((*OUTwEnd)());         /* signal end of windows */
    int ((*OUTendPlot)());      /* end of plot */
    int ((*OUTbeginDomain)());  /* start nested domain */
    int ((*OUTendDomain)());    /* end nested domain */
    int ((*OUTattributes)());      /* specify output attributes of node */
#endif /* STDC */
} IFfrontEnd;

/* flags for the first argument to IFerror */
#define ERR_WARNING 0x1
#define ERR_FATAL 0x2
#define ERR_PANIC 0x4
#define ERR_INFO 0x8

    /* valid values for the second argument to doAnalyses */

    /* continue the analysis from where we left off */
#define RESUME 0
    /* start everything over from the beginning of this task*/
#define RESTART 1
    /* abandon the current analysis and go on the the next in the task*/
#define SKIPTONEXT 2

#define OUT_SCALE_LIN   1
#define OUT_SCALE_LOG   2

#endif /*IFSIMULATOR*/
