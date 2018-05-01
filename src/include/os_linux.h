/*
 *  Linux
 */
#include "os_unix.h"

#define WANT_X11		/* If the X11 Window System can work	*/
#define HAS_LIMITS_H			/* limits.h */
#define HAS_FLOAT_H				/* float.h */
#define HAS_STRCHR		/* strchr( ) instead of index( )	*/
#define HAS_STDLIB		/* #include <stdlib.h> for libc defs	*/
#define HAS_VPERROR		/* perror( ) defined by standard '.h's	*/
#define HAS_ATRIGH		/* acosh( ), asinh( ), atanh( )         */
#define HAS_BCOPY		/* bcopy( ), bzero( )			*/
#define HAS_BSDRANDOM		/* srandom( ) and random( )		*/
#define HAS_BSDRUSAGE		/* getrusage( )				*/
#define HAS_DUP2		/* dup2(a, b) for shifting file descrs. */
#define HAS_ENVIRON		/* getenv( )				*/
#define HAS_GETWD		/* getwd(buf)				*/
#define HAS_STRINGS		/* use <strings.h> instead of <string.h> */
#define HAS_SYSVTTY
#define HAS_SYSVDIRS
