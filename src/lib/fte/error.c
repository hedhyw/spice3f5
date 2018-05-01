/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 *
 * Print out in more detail what a floating point error was.
 */

#include "spice.h"
#include "cpdefs.h"
#include "ftedefs.h"
#ifdef HAS_UNIX_SIGS
#include <signal.h>
#endif
#include "suffix.h"

/* global error message buffer */
char ErrorMessage[1024];

#ifdef HAS_VAX_FPERRORS

static char *fperrs[] = {
            "FPE 0",
            "Integer overflow",
            "Integer divide by zero",
            "Floating overflow",
            "Floating / decimal divide by zero",
            "Floating underflow",
            "Decimal overflow",
            "Subscript out of range",
            "Floating overflow fault",
            "Divide by zero floating fault",
            "Floating underflow fault"
        } ;

static int nfperrs = 11;

void
fperror(mess, code)
    char *mess;
{
    if ((code < 0) || (code >= nfperrs))
        fprintf(cp_err, "%s: Unknown floating point error (# %d).\n", 
            mess, code);
    else
        fprintf(cp_err, "%s: %s.\n", mess, fperrs[code]);
    return;
}

#else

/*ARGSUSED*/
void
fperror(mess, code)
    char *mess;
{
    fprintf(cp_err, "%s: floating point exception.\n", mess);
    return;
}

#endif

/* Print a spice error message. */

void
ft_sperror(code, mess)
    char *mess;
{
    fprintf(cp_err, "%s: %s\n", mess, if_errstring(code));
    return;
}

void
fatal()
{
    cp_ccon(false);
#ifdef HAS_UNIX_SIGS
#ifdef FTEDEBUG
#ifdef SIGQUIT
    (void) signal(SIGQUIT, SIG_DFL);
    (void) kill(getpid(), SIGQUIT);
#endif
#endif
#endif
    exit(EXIT_BAD);
}

/* These error messages are from internal consistency checks. */
internalerror(message)
char *message;
{

    fprintf(stderr, "internal error:  %s\n", message);

}

/* These errors are from external routines like fopen. */
externalerror(message)
char *message;
{

    fprintf(stderr, "external error:  %s\n", message);

}
