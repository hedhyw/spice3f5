/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
**********/

/*
 *	MS-DOS using MSC-C 5.1
 */

#define     DIR_CWD		"."
#define     DIR_PATHSEP		"\\"
#define     DIR_TERM		'\\'
#define	    TEMPFORMAT		"%s%d.tmp"

#define HAS_ASCII
#define HAS_CHDIR		/* for tree filesystems, chdir( )	*/
#define HAS_CLEARERR		/* clearerr( ), should be in stdio	*/
#define HAS_CTYPE		/* <ctype.h>, iswhite( ), etc.		*/
#define HAS_DOSDIRS		/* Emulate opendir, etc.		*/
#define HAS_MEMAVL		/* Use _memavl( ) 			*/
#define HAS_ENVIRON
#define HAS_FTIME
#define HAS_GETCWD		/* getcwd(buf, size)			*/
#define HAS_LOCALTIME
#define HAS_LONGJUMP		/* setjmp( ), longjmp( )		*/
#define HAS_MINDATA
#define HAS_NOINLINE
#define HAS_NOVM
#define HAS_PCTERM
#define HAS_QSORT		/* qsort( )				*/
#define HAS_SHORTMACRO
#define HAS_STAT
#define HAS_STDLIB
#define HAS_STRCHR		/* strchr( ) instead of index( )	*/
#define HAS_SYSTEM
#define HAS_UNIX_SIGS
#define HAS_UNLINK
#define HAS_LIMITS_H
#define HAS_FLOAT_H
#define HAS_NO_IEEE_LOGB
#define HAS_NO_ERFC
#define HAS_BATCHSIM

#define WANT_PCHARDCOPY
#define WANT_MFB
