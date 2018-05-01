/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#ifndef INP
#define INP

    /* structure declarations used by either/both input package */

#include "ifsim.h"
#include "gendefs.h"
#include "inpptree.h"

struct INPtab {
    char *t_ent;
    struct INPtab *t_next;
};

struct INPnTab {
    char *t_ent;
    GENERIC* t_node;
    struct INPnTab *t_next;
};

typedef struct sINPtables{
    struct INPtab **INPsymtab;
    struct INPnTab **INPtermsymtab;
    int INPsize;
    int INPtermsize;
    GENERIC *defAmod;
    GENERIC *defBmod;
    GENERIC *defCmod;
    GENERIC *defDmod;
    GENERIC *defEmod;
    GENERIC *defFmod;
    GENERIC *defGmod;
    GENERIC *defHmod;
    GENERIC *defImod;
    GENERIC *defJmod;
    GENERIC *defKmod;
    GENERIC *defLmod;
    GENERIC *defMmod;
    GENERIC *defNmod;
    GENERIC *defOmod;
    GENERIC *defPmod;
    GENERIC *defQmod;
    GENERIC *defRmod;
    GENERIC *defSmod;
    GENERIC *defTmod;
    GENERIC *defUmod;
    GENERIC *defVmod;
    GENERIC *defWmod;
    GENERIC *defYmod;
    GENERIC *defZmod;
} INPtables;

typedef struct card{
    int linenum;
    char *line;
    char *error;
    struct card *nextcard;
    struct card *actualLine;
} card;

/* structure used to save models in after they are read during pass 1 */
typedef struct sINPmodel{
    IFuid INPmodName;   /* uid of model */
    int INPmodType;     /* type index of device type */
    struct sINPmodel *INPnextModel;  /* link to next model */
    int INPmodUsed;     /* flag to indicate it has already been used */
    card *INPmodLine;   /* pointer to line describing model */
    GENERIC *INPmodfast;   /* high speed pointer to model for access */
} INPmodel;

/* listing types - used for debug listings */
#define LOGICAL 1
#define PHYSICAL 2

#ifdef __STDC__
int IFnewUid(GENERIC*,IFuid*,IFuid,char*,int,GENERIC**);
int IFdelUid(GENERIC*,IFuid,int);
int INPaName(char*,IFvalue*,GENERIC*,int*,char*,GENERIC**,IFsimulator*,int*,
        IFvalue*);
int INPapName(GENERIC*,int,GENERIC*,char*,IFvalue*);
void INPcaseFix(char*);
char * INPdevParse(char**,GENERIC*,int,GENERIC*,double*,int*,INPtables*);
char *INPdomodel(GENERIC *,card*, INPtables*);
void INPdoOpts(GENERIC*,GENERIC*,card*,INPtables*);
char *INPerrCat(char *, char *);
char *INPerror(int);
double INPevaluate(char**,int*,int);
char * INPfindLev(char*,int*);
char * INPgetMod(GENERIC*,char*,INPmodel**,INPtables*);
int INPgetTok(char**,char**,int);
void INPgetTree(char**,INPparseTree**,GENERIC*,INPtables*);
IFvalue * INPgetValue(GENERIC*,char**,int,INPtables*);
int INPgndInsert(GENERIC*,char**,INPtables*,GENERIC**);
int INPinsert(char**,INPtables*);
int INPremove(char*,INPtables*);
int INPlookMod(char*);
int INPmakeMod(char*,int,card*);
char *INPmkTemp(char*);
void INPpas1(GENERIC*,card*,INPtables*);
void INPpas2(GENERIC*,card*,INPtables*,GENERIC *);
int INPpName(char*,IFvalue*,GENERIC*,int,GENERIC*);
int INPtermInsert(GENERIC*,char**,INPtables*,GENERIC**);
int INPmkTerm(GENERIC*,char**,INPtables*,GENERIC**);
int INPtypelook(char*);
void INP2B(GENERIC*,INPtables*,card*);
void INP2C(GENERIC*,INPtables*,card*);
void INP2D(GENERIC*,INPtables*,card*);
void INP2E(GENERIC*,INPtables*,card*);
void INP2F(GENERIC*,INPtables*,card*);
void INP2G(GENERIC*,INPtables*,card*);
void INP2H(GENERIC*,INPtables*,card*);
void INP2I(GENERIC*,INPtables*,card*);
void INP2J(GENERIC*,INPtables*,card*);
void INP2K(GENERIC*,INPtables*,card*);
void INP2L(GENERIC*,INPtables*,card*);
void INP2M(GENERIC*,INPtables*,card*);
void INP2O(GENERIC*,INPtables*,card*);
void INP2Q(GENERIC*,INPtables*,card*,GENERIC*);
void INP2R(GENERIC*,INPtables*,card*);
void INP2S(GENERIC*,INPtables*,card*);
void INP2T(GENERIC*,INPtables*,card*);
void INP2U(GENERIC*,INPtables*,card*);
void INP2V(GENERIC*,INPtables*,card*);
void INP2W(GENERIC*,INPtables*,card*);
void INP2Z(GENERIC*,INPtables*,card*);
int INP2dot(GENERIC*,INPtables*,card*,GENERIC*,GENERIC*);
INPtables *INPtabInit(int);
void INPkillMods(void);
void INPtabEnd(INPtables *);
#else /* stdc */
int IFnewUid();
int IFdelUid();
int INPaName();
IFvalue * INPgetValue();
INPtables *INPtabInit();
char * INPdevParse();
char * INPdomodel();
char * INPerrCat();
char * INPfindLev();
char * INPgetMod();
char *INPerror();
char *INPmkTemp();
double INPevaluate();
int INPapName();
int INPgetTitle();
int INPgetTok();
int INPgndInsert();
int INPlookMod();
int INPmakeMod();
int INPpName();
int INPreadAll();
int INPtermInsert();
int INPmkTerm();
int INPtypelook();
void INPcaseFix();
void INPdoOpts();
int INPinsert();
int INPremove();
void INPkillMods();
void INPlist();
void INPpas1() ;
void INPpas2() ;
void INPtabEnd();
void INPptPrint();
void INPgetTree();
void INP2B();
void INP2C();
void INP2D();
void INP2E();
void INP2F();
void INP2G();
void INP2H();
void INP2I();
void INP2J();
void INP2K();
void INP2L();
void INP2M();
void INP2O();
void INP2Q();
void INP2R();
void INP2S();
void INP2T();
void INP2U();
void INP2V();
void INP2W();
void INP2Z();
int INP2dot();
#endif /* stdc */

#endif /*INP*/
