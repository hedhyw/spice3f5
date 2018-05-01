/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
**********/

/*
 * This code is courtesy of Earl Faneuf (dec-elmer!faneuf).  It is for
 * spawning shells and editors under VMS.  Also in this file are a few
 * useful UNIX-type routines.
 */

#include "spice.h"
#include <stdio.h>
#include "suffix.h"

#ifdef HAS_VMSHACK

/*-----------------SYSTEM emulation-------------------------------------
 *
 *  Executes the VMS command in string command_line.
 *  The command may ask the terminal for further input.
 *  After the VMS command exits, control is returned to the calling
 *  function.
 *
 *  Beware: This call is painfully slow on a loaded system
 *
 *  for VAX-VMS 4-Nov-85  J. Faricelli
 *  modified    2-Dec-85  J. Faricelli (allow command to ask for input)
 *
 *----------------------------------------------------------------------*/

int     system (command_line)
char   *command_line;
{
#include descrip        /* VMS descriptors header file */
#include ssdef          /* VMS system call header file */

    static  $DESCRIPTOR (spawn_line, "");
    int     lib$spawn ();
    int     spawn_flags, spawn_rc, i;
    unsigned int    len;
    char   *string;

    if (command_line == NULL)
    command_line = " ";
 /* Create local storage for command line */
    len = strlen (command_line);
    string = (char *) tmalloc (len);
    (void) strncpy (string, command_line, len);

 /* VMS doesn't grok a newline at the end of the string, so don't send that to
    spawn (make it a blank). */
 /* command_line[len-1] is end of string (remember, index is 0..len-1) */
    if (command_line[len - 1] == '\n')
    {
    string[len - 1] = ' ';
    }

 /* Prepare descriptor for call to spawn */
    spawn_line.dsc$w_length = len;
    spawn_line.dsc$a_pointer = string;
    spawn_flags = 0;
    spawn_rc = lib$spawn (&spawn_line, &0, &0, &spawn_flags);
    if (spawn_rc != SS$_NORMAL)
    {
    fprintf (stderr, "Internal error in VMS emulation of system() call\n\
Error during spawn of subprocess:\n\
VMS return code was: %d\n", spawn_rc);
    free (string);
    return (-1);
    }
    free (string);
    return (0);
}

#include <rms.h>

int
unlink(name)
char *name;
{
    struct FAB fblock;
    int status;

    fblock = cc$rms_fab;

    fblock.fab$l_fna = name;
    fblock.fab$b_fns = strlen(name);

    status = sys$erase(&fblock);

    if (status != RMS$_NORMAL) {
    return(status);
    }

    return(0);
}


#else

/* Some systems don't like empty files... */

#ifndef LINT

static int ducks() { return (0); }

#endif

#endif

