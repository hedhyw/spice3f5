/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Hong J. Park, Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "bsim1def.h"
#include "suffix.h"

/* routine to calculate equivalent conductance and total terminal 
 * charges
 */

void
B1mosCap(ckt,vgd,vgs,vgb,
	args,
	/*
	GateDrainOverlapCap,GateSourceOverlapCap, GateBulkOverlapCap,
	capbd,capbs,
        cggb,cgdb,cgsb,
	*/
	cbgb,cbdb,cbsb,cdgb,cddb,cdsb,
        gcggbPointer,gcgdbPointer,gcgsbPointer,gcbgbPointer,gcbdbPointer,
        gcbsbPointer,gcdgbPointer,gcddbPointer,gcdsbPointer,
        gcsgbPointer,gcsdbPointer,gcssbPointer,qGatePointer,qBulkPointer,
        qDrainPointer,qSourcePointer)
    register CKTcircuit *ckt;
    double vgd;
    double vgs;
    double vgb;
    double args[8];
    /*
    double GateDrainOverlapCap;
    double GateSourceOverlapCap;
    double GateBulkOverlapCap;
    double capbd;
    double capbs;
    double cggb;
    double cgdb;
    double cgsb;
    */
    double cbgb;
    double cbdb;
    double cbsb;
    double cdgb;
    double cddb;
    double cdsb;
    double *gcggbPointer;
    double *gcgdbPointer;
    double *gcgsbPointer;
    double *gcbgbPointer;
    double *gcbdbPointer;
    double *gcbsbPointer;
    double *gcdgbPointer;
    double *gcddbPointer;
    double *gcdsbPointer;
    double *gcsgbPointer;
    double *gcsdbPointer;
    double *gcssbPointer;
    double *qGatePointer;
    double *qBulkPointer;
    double *qDrainPointer;
    double *qSourcePointer;

{
    double qgd;
    double qgs;
    double qgb;
    double ag0;

    ag0 = ckt->CKTag[0];
    /* compute equivalent conductance */
    *gcdgbPointer = (cdgb - args[0]) * ag0;
    *gcddbPointer = (cddb + args[3] + args[0]) * ag0;
    *gcdsbPointer = cdsb * ag0;
    *gcsgbPointer = -(args[5] + cbgb + cdgb + args[1]) * ag0;
    *gcsdbPointer = -(args[6] + cbdb + cddb ) * ag0;
    *gcssbPointer = (args[4] + args[1] - 
        (args[7] + cbsb + cdsb )) * ag0;
    *gcggbPointer = (args[5] + args[0] +
        args[1] + args[2] ) * ag0;
    *gcgdbPointer = (args[6] - args[0]) * ag0;
    *gcgsbPointer = (args[7] - args[1]) * ag0;
    *gcbgbPointer = (cbgb - args[2]) * ag0;
    *gcbdbPointer = (cbdb - args[3]) * ag0;
    *gcbsbPointer = (cbsb - args[4]) * ag0;
 
    /* compute total terminal charge */
    qgd = args[0] * vgd;
    qgs = args[1] * vgs;
    qgb = args[2] * vgb;
    *qGatePointer = *qGatePointer + qgd + qgs + qgb;
    *qBulkPointer = *qBulkPointer - qgb;
    *qDrainPointer = *qDrainPointer - qgd;
    *qSourcePointer = -(*qGatePointer + *qBulkPointer + *qDrainPointer);

}


