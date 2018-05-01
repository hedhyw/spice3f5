#ifndef PZDEFS
#define PZDEFS "PZdefs.h $Revision: 1.3 $  on $Date: 88/11/16 21:55:29 $ "

    /* structure used to describe an PZ analysis to be performed */

#include "jobdefs.h"
#include "complex.h"

typedef struct strial {
    SPcomplex	s, f_raw, f_def;
    struct strial *next, *prev;
    int		mag_raw, mag_def;
    int		multiplicity;
    int		flags;
    int		seq_num;
    int		count;
} PZtrial;

typedef struct {
    int JOBtype;
    JOB *JOBnextJob;
    IFuid JOBname;
    int PZin_pos;
    int PZin_neg;
    int PZout_pos;
    int PZout_neg;
    int PZinput_type;
    int PZwhich;
    int PZnumswaps;
    int PZbalance_col;
    int PZsolution_col;
    PZtrial *PZpoleList;
    PZtrial *PZzeroList;
    int PZnPoles;
    int PZnZeros;
    double *PZdrive_pptr;
    double *PZdrive_nptr;
} PZAN;

#define PZ_DO_POLES	0x1
#define PZ_DO_ZEROS	0x2
#define PZ_IN_VOL	1
#define PZ_IN_CUR	2

#define PZ_NODEI 1
#define PZ_NODEG 2
#define PZ_NODEJ 3
#define PZ_NODEK 4
#define PZ_V 5
#define PZ_I 6
#define PZ_POL 7
#define PZ_ZER 8
#define PZ_PZ 9

#endif /*PZDEFS*/
