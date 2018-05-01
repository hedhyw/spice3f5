/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "ifsim.h"
#include "iferrmsg.h"
#include "inpptree.h"
#include "suffix.h"

#ifdef __STDC__
static int PTeval(INPparseNode*, double, double*, double*);
#else /* stdc */
static int PTeval();
#endif /* stdc */

extern double PTfudge_factor;

int
IFeval(tree, gmin, result, vals, derivs)
    IFparseTree *tree;
    double gmin;
    double *result;
    double *vals;
    double *derivs;
{
    int i, err;
    INPparseTree *myTree = (INPparseTree*) tree;;

/*
INPptPrint("calling PTeval, tree = ", myTree);
printf("values:");
for (i = 0; i < myTree->p.numVars; i++)
printf("\tvar%d = %lg\n", i, vals[i]);
*/

    if ((err = PTeval(myTree->tree, gmin, result, vals)) != OK)
        return (err);

    for (i = 0; i < myTree->p.numVars; i++)
        if ((err = PTeval(myTree->derivs[i], gmin, &derivs[i], vals)) != OK)
            return (err);

/*
printf("results: function = %lg\n", *result);
for (i = 0; i < myTree->p.numVars; i++)
printf("\td / d var%d = %lg\n", i, derivs[i]);
*/

    return (OK);
}

static int
PTeval(tree, gmin, res, vals)
    INPparseNode *tree;
    double gmin;
    double *res;
    double *vals;
{
    double r1, r2;
    int err;

    PTfudge_factor = gmin;
    switch (tree->type) {
    case PT_CONSTANT:
        *res = tree->constant;
        break;

    case PT_VAR:
        *res = vals[tree->valueIndex];
        break;
    
    case PT_FUNCTION:
        err = PTeval(tree->left, gmin, &r1, vals);
        if (err != OK)
            return (err);
        *res = (*tree->function)(r1);
        if (*res == HUGE) {
            fprintf(stderr, "Error: %lg out of range for %s\n",
                    r1, tree->funcname);
            return (E_PARMVAL);
        }
        break;

    case PT_PLUS:
    case PT_MINUS:
    case PT_TIMES:
    case PT_DIVIDE:
    case PT_POWER:
        err = PTeval(tree->left, gmin, &r1, vals);
        if (err != OK)
            return (err);
        err = PTeval(tree->right, gmin, &r2, vals);
        if (err != OK)
            return (err);
        *res = (*tree->function)(r1, r2);
        if (*res == HUGE) {
            fprintf(stderr, "Error: %lg, %lg out of range for %s\n",
                    r1, r2, tree->funcname);
            return (E_PARMVAL);
        }
        break;
    
    default:
        fprintf(stderr, "Internal Error: bad node type %d\n",
                tree->type);
        return (E_PANIC);
    }

    return (OK);
}

