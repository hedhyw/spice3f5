/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1987 Thomas L. Quarles
**********/
/*
 */

/*
 * This routine gives access to the internal model parameters
 * of Current controlled SWitch
 */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "devdefs.h"
#include "ifsim.h"
#include "cswdefs.h"
#include "sperror.h"
#include "suffix.h"


/* ARGSUSED */
int
CSWmAsk(ckt,inst,which,value)
    CKTcircuit *ckt;
    GENmodel *inst;
    int which;
    IFvalue *value;
{
    CSWmodel *here = (CSWmodel*)inst;
    switch(which) {
        case CSW_RON:
            value->rValue = here->CSWonResistance;
            return (OK);
        case CSW_ROFF:
            value->rValue = here->CSWoffResistance;
            return (OK);
        case CSW_ITH:
            value->rValue = here->CSWiThreshold;
            return (OK);
        case CSW_IHYS:
            value->rValue = here->CSWiHysteresis;
            return (OK);
        case CSW_GON:
            value->rValue = here->CSWonConduct;
            return (OK);
        case CSW_GOFF:
            value->rValue = here->CSWoffConduct;
            return (OK);
        default:
            return (E_BADPARM);
    }
    /* NOTREACHED */
}
