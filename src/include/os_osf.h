/**********
Copyright 1992 Regents of the University of California.  All rights reserved.
**********/

/*
 *	OSF/1
 */

#define HAS_LIMITS_H
#define HAS_FLOAT_H
#define HAS_STDLIB

#include "os_unix.h"

#define HAS_ATRIGH		/* acosh( ), asinh( ), atanh( )         */
#define HAS_FTIME		/* ftime( ), <times.h>			*/
#define HAS_TERMCAP		/* tgetxxx( )				*/
#define HAS_VFORK		/* BSD-ism, should not be necessary	*/
#define HAS_INDEX		/* index( ) instead of strchr( )	*/
#define HAS_BCOPY		/* bcopy( ), bzero( )			*/
#define HAS_BSDRANDOM		/* srandom( ) and random( )		*/
#define HAS_BSDTTY		/* <sgtty.h>				*/
#define HAS_SYSVDIRS		/* <sys/dir.h>				*/
#define HAS_BSDRUSAGE		/* getrusage( )				*/
#define HAS_BSDRLIMIT		/* getrlimit( )				*/
#define HAS_BSDSOCKETS		/* <net/inet.h>, socket( ), etc.	*/
#define HAS_DUP2
#define HAS_GETWD		/* getwd(buf)				*/
#define HAS_INTWAITSTATUS	/* wait(3)  takes an int *, not a union */

#define complex	cmplx_struct
