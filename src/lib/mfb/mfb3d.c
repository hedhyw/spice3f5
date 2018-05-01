/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: -C- 1982 Giles C. Billingsley
**********/
/*
 * mfb3d.c
 *
 * sccsid "@(#)mfb3d.c  1.9  9/3/83"
 *
 *     MFB is a graphics package that was developed by the integrated
 * circuits group of the Electronics Research Laboratory and the
 * Department of Electrical Engineering and Computer Sciences at
 * the University of California, Berkeley, California.  The programs
 * in MFB are available free of charge to any interested party.
 * The sale, resale, or use of these program for profit without the
 * express written consent of the Department of Electrical Engineering
 * and Computer Sciences, University of California, Berkeley, California,
 * is forbidden.
 */


#include "spice.h"
#include "mfb.h"
#include "suffix.h"

#define MFBFORMAT   MFBCurrent->strings

/**************************************************************************
 *
 *                 MFB3D
 *
 *     MFB3D contains MFB routines for 3 dimension graphics.
 *
 *
 **************************************************************************/
