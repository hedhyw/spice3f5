/*
 *	Macintosh Think C version 5.1
 */

/* ----- System capabilities */
#define HAS_FLAT_INCLUDES
#define HAS_QUICKDRAW
#define HAS_MACCLOCK
#define HAS_LOCALTIME
#define HAS_NO_ERFC

#define HAS_MAC_ARGCARGV

#define HAS_LIMITS_H			/* limits.h */
#define HAS_FLOAT_H				/* float.h */
#define HAS_ASCII		/* eighth bit of a character is not used */
#define HAS_CLEARERR		/* clearerr( ), should be in stdio	*/
#define HAS_CTYPE		/* <ctype.h>, iswhite( ), etc.		*/
#define HAS_MEMAVL
#define HAS_ENVIRON		/* getenv( )				*/
#define HAS_ISATTY		/* isatty( )				*/
#define HAS_LONGJUMP		/* setjmp( ), longjmp( )		*/
#define HAS_NO_ATRIGH_DECL	/* if asinh( ) is not in math.h		*/
#define HAS_NO_IEEE_LOGB	/* no logb( ) and scalb( ) functions	*/
#define HAS_NO_IEEE_LOGB_DECL	/* logb( ) and scalb( ) not in math.h	*/
#define HAS_QSORT		/* qsort( ) exists			*/
#define HAS_STDLIB		/* #include <stdlib.h> for libc defs	*/
#define HAS_STRCHR		/* strchr( ) instead of index( )	*/
#define HAS_SYSTEM		/* system( ), execute system command	*/
#define HAS_UNIX_SIGS		/* signal( ), kill( )			*/
#define HAS_UNLINK		/* unlink( ), for removing files	*/
#define HAS_VPERROR		/* perror( ) defined by standard '.h's	*/

/* ----- String or character constants */
#define DIR_PATHSEP	":"		/* subdirectory separator	*/
#define TEMPFORMAT	"temp:spice%s%d"


/* ----- Weird definitions */
#define SIGNAL_FUNCTION __sig_func
#define SIMULATOR		/* There should be a better way */

