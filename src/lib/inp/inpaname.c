/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

    /*
     * INPaName()
     *
     *  Take a parameter by Name and ask for the specified value 
     * *dev is -1 if type unknown, otherwise, device type
     * **fast is a device, and will be set if possible.
     */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "cpdefs.h"
#include "fteext.h"
#include "ifsim.h"
#include "iferrmsg.h"
#include "suffix.h"

int
INPaName(parm,val,ckt,dev,devnam,fast,sim,dataType,selector)
    char *parm;     /* the name of the parameter to set */
    IFvalue *val;   /* the parameter union containing the value to set */
    GENERIC *ckt;   /* the circuit this device is a member of */
    int *dev;       /* the device type code to the device being parsed */
    char *devnam;   /* the name of the device */
    GENERIC **fast;  /* direct pointer to device being parsed */
    IFsimulator *sim;       /* the simulator data structure */
    int *dataType;  /* the datatype of the returned value structure */
    IFvalue *selector;  /* data sub-selector for questions */

{
    int error;  /* int to store evaluate error return codes in */
    int i;

    /* find the instance - don't know about model, so use null there,
     * otherwise pass on as much info as we have about the device
     * (name, type, direct pointer) - the type and direct pointer
     * WILL be set on return unless error is not OK
     */
    error = (*(sim->findInstance))(ckt,dev,fast,devnam,(GENERIC *)NULL,
            (char *)NULL);
    if(error) return(error);

    /* now find the parameter - hunt through the parameter tables for
     * this device type and look for a name match of an 'ask'able
     * parameter.
     */
    for(i=0;i<(*(*(sim->devices)[*dev]).numInstanceParms);i++) {
        if(strcmp(parm,
                ((*(sim->devices)[*dev]).instanceParms[i].keyword))==0 &&
                (((*(sim->devices)[*dev]).instanceParms[i].dataType)&IF_ASK)) {
            /* found it, so we ask the question using the device info we got
             * above and put the results in the IFvalue structure our caller
             * gave us originally
             */
            error = (*(sim->askInstanceQuest))(ckt,*fast,
                    (*(sim->devices)[*dev]).instanceParms[i].id,val,
                    selector);
            if(dataType) *dataType = 
                    (*(sim->devices)[*dev]).instanceParms[i].dataType;
            return(error);
        }
    }
    return(E_BADPARM);
}
