/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
**********/

/*
 *	IPSC860 SYS-V and derivative systems
 */

#include "os_unix.h"

#define HAS_SYSVTTY		/* <termio.h>				*/
#define HAS_SYSVDIRS		/* <dirent.h>				*/
#define HAS_SYSVRLIMIT
#define HAS_SYSVRUSAGE
#define HAS_STRCHR		/* strchr( ) instead of index( )	*/
#define HAS_LIMITS_H
#define HAS_ATRIGH		/* acosh( ), asinh( ), atanh( )         */

/* Kill off things not available on the nodes */
#undef HAS_POPEN
#undef HAS_SYSTEM
#undef HAS_SYSVRLIMIT
/* #define HAS_NO_IEEE_LOGB */	/* If no logb( ) and scalb( ) functions */

/* XXX This doesn't belong here */
#define index strchr
#define rindex strrchr
