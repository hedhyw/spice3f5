/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
**********/
/*
 * vmsio.c
 *
 * Written by Giles Billingsley, UC Berkeley.
 * sccsid "@(#)vmsio.c	1.9  9/3/83"
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
 *
 * Routines to replace stty on VMS.
 *
 *          dev_init(buf,size)   initialize this package
 *          dev_getchar()        input a char (no-echo, 1 char buff)
 *                               from stdin.
 *          dev_printf()         output a string of n chars with no
 *				 hacking of special characters to stdout
 *          dev_getc()           input a char (no-echo, 1 char buff)
 *          dev_write()          output a string of n chars with no
 *				 hacking of special characters
 */

#include "spice.h"

#ifdef vms

#include ssdef
#include descrip
#include iodef
#include rms
#include "mfb.h"
#include "suffix.h"

static unsigned rfunc;
static unsigned wfunc;
static int dev_channel;
static int initialized = 0;

/*
 * Test routine for vmsio.c
 *
 * main(){
 *     int c;
 *     dev_init();
 *     c = dev_getchar();
 *     printf("character = %d\n",c);
 *     c = dev_getchar();
 *     printf("character = %d\n",c);
 *     c = dev_getchar();
 *     printf("character = %d\n",c);
 *     c = dev_getchar();
 *     printf("character = %d\n",c);
 *     c = dev_getchar();
 *     printf("character = %d\n",c);
 *     dev_printf("this is a test",14);
 *     }
 */


/*									*/
/*	DEV_INIT - initializes global variables in DISPLAY		*/
/*									*/
dev_init() {
    struct dsc$descriptor_s com_desc;
    char *cp = "TT";
    int status;

    if(initialized)
	return;
    initialized = 1;
    com_desc.dsc$w_length = strlen( cp );
    com_desc.dsc$a_pointer = cp;
    com_desc.dsc$b_class = DSC$K_CLASS_S;
    com_desc.dsc$b_dtype = DSC$K_DTYPE_T;
    wfunc = IO$_WRITEVBLK | IO$M_NOFORMAT;
    rfunc = IO$_READVBLK | IO$M_NOECHO;
    status = sys$assign(&com_desc,&dev_channel,0,0);
    if(status != SS$_NORMAL){
        printf("SYS$ASSIGN failed.  Status = %d\n",status);
        exit(1);
        }
    }


/*									*/
/*	DEV_OPEN - initializes global variables in DISPLAY.		*/
/*	open and assign the 'channel' to device 'name' (stdio by	*/
/*	default).							*/
/*									*/
dev_open(name,channel)
    char *name;
    int *channel;
    {
    struct dsc$descriptor_s com_desc;
    char *cp = "TT";
    int status;

    /* initialize stdio */
    dev_init();
    /* open the standard channel by default */
    if(name == NULL || *name == NULL){
	/* stdout by default */
        com_desc.dsc$w_length = strlen( cp );
        com_desc.dsc$a_pointer = cp;
	}
    else{
        com_desc.dsc$w_length = strlen( name );
        com_desc.dsc$a_pointer = name;
	}
    com_desc.dsc$b_class = DSC$K_CLASS_S;
    com_desc.dsc$b_dtype = DSC$K_DTYPE_T;
    wfunc = IO$_WRITEVBLK | IO$M_NOFORMAT;
    rfunc = IO$_READVBLK | IO$M_NOECHO;
    status = sys$assign(&com_desc,channel,0,0);
    if(status != SS$_NORMAL){
        printf("SYS$ASSIGN failed.  Status = %d\n",status);
        exit(1);
        }
    }


/*									*/
/*	DEV_PRINTF - dumps buffer to standard terminal			*/
/*									*/
/*	It is necessary to have the length argument because the		*/
/*	character buffer may have embedded nulls.			*/
/*									*/
dev_printf(buffer,length)
    char *buffer;
    int length;
    {
    int status;
    int f_status;

    status = sys$qiow(6,dev_channel,wfunc,&f_status,0,0,buffer,length,0,0,0,0);
    if(status != SS$_NORMAL){
        printf("SYS$QIOW failed.  Status = %d\n",status);
        }
    return(status);
    }


/*									*/
/*  DEV_GETCHAR - get a character from the standard terminal		*/
/*									*/
dev_getchar(){
    char buf[10];
    int status;
    int f_status;

    status = sys$qiow(7,dev_channel,rfunc,&f_status,0,0,buf,1,0,0,0,0);
    return((int)buf[0]);
    }


/*									*/
/*	DEV_WRITE - dumps buffer to device channel			*/
/*									*/
/*	It is necessary to have the length argument because the		*/
/*	character buffer may have embedded nulls.			*/
/*									*/
dev_write(channel,buffer,length)
    char *buffer;
    int channel;
    int length;
    {
    int status;
    int f_status;

    status = sys$qiow(6,channel,wfunc,&f_status,0,0,buffer,length,0,0,0,0);
    if(status != SS$_NORMAL){
        printf("SYS$QIOW failed.  Status = %d\n",status);
        }
    return(status);
    }


/*									*/
/*  DEV_GETC - get a character from the device channel			*/
/*									*/
dev_getc(channel)
    int channel;
    {
    char buf[10];
    int status;
    int f_status;

    status = sys$qiow(7,channel,rfunc,&f_status,0,0,buf,1,0,0,0,0);
    return((int)buf[0]);
    }

#else
int Dummy_Symbol;
#endif
