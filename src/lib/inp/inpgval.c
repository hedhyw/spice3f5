/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "ifsim.h"
#include "util.h"
#include "inpdefs.h"
#include "inpptree.h"
#include "suffix.h"

IFvalue *
INPgetValue(ckt,line,type,tab)
    GENERIC *ckt;
    char ** line;
    int type;
    INPtables *tab;
{
    double *list;
    int *ilist;
    double tmp;
    char *word;
    int error;
    static IFvalue temp;
    INPparseTree *pt;

    /* make sure we get rid of extra bits in type */
    type &= IF_VARTYPES;
    if(type == IF_INTEGER) {
        temp.iValue = INPevaluate(line,&error,1);
        /*printf(" returning integer value %d\n",temp.iValue);*/
    } else if (type == IF_REAL) {
        temp.rValue = INPevaluate(line,&error,1);
        /*printf(" returning real value %e\n",temp.rValue);*/
    } else if (type == IF_REALVEC) {
        temp.v.numValue = 0;
        list = (double *)MALLOC(sizeof(double));
        tmp = INPevaluate(line,&error,1);
        while (error == 0) {
            /*printf(" returning vector value %g\n",tmp); */
            temp.v.numValue++;
            list=(double *)REALLOC((char *)list,temp.v.numValue*sizeof(double));
            *(list+temp.v.numValue-1) = tmp;
            tmp = INPevaluate(line,&error,1);
        }
        temp.v.vec.rVec=list;
    } else if (type == IF_INTVEC) {
        temp.v.numValue = 0;
        ilist = (int *)MALLOC(sizeof(int));
        tmp = INPevaluate(line,&error,1);
        while (error == 0) {
            /*printf(" returning vector value %g\n",tmp); */
            temp.v.numValue++;
            ilist=(int *)REALLOC((char *)ilist,temp.v.numValue*sizeof(int));
            *(ilist+temp.v.numValue-1) = tmp;
            tmp = INPevaluate(line,&error,1);
        }
        temp.v.vec.iVec=ilist;
    } else if (type == IF_FLAG) {
        temp.iValue = 1;
    } else if (type == IF_NODE) {
        INPgetTok(line,&word,1);
        INPtermInsert(ckt,&word,tab,&(temp.nValue));
    } else if (type == IF_INSTANCE) {
        INPgetTok(line,&word,1);
        INPinsert(&word,tab);
        temp.nValue = word;
    } else if (type == IF_STRING) {
        INPgetStr(line,&word,1);
        temp.sValue = word;
    } else if (type == IF_PARSETREE) {
        INPgetTree(line, &pt, ckt, tab);
        if (!pt)
            return((IFvalue *)NULL);
        temp.tValue = (IFparseTree *) pt;
        /*INPptPrint("Parse tree is: ", temp.tValue);*/
    } else { /* don't know what type of parameter caller is talking about! */
        return((IFvalue *)NULL);
    }
    return(&temp);
}
