/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1986 Wayne A. Christopher, U. C. Berkeley CAD Group 
**********/

/*
 * The main routine for the help system in stand-alone mode.
 */

#include "spice.h"

#ifdef HAS_BSDDIRS
#  include <sys/types.h>
#  include <sys/dir.h>
#else
#  ifdef HAS_SYSVDIRS
#    include <sys/types.h>
#    include <dirent.h>
#    ifndef direct
#      define direct dirent
#    endif
#  endif
#endif

#ifdef HAS_GETPW
#  include <pwd.h>
#endif

#include "cpdefs.h"
#include "hlpdefs.h"
#include "suffix.h"

#ifdef HAS_X10
Display *X_display = (Display *) NULL;
#endif

#ifdef HAS_X11
Widget toplevel;
#endif

FILE *cp_in, *cp_out, *cp_err;

/* dummy declaration so CP.a doesn't pull in lexical.o and other objects */
bool cp_interactive = false;

char *hlp_filelist[] = { "spice", 0 };

/* ARGSUSED */
void
main(ac, av)
    int ac;
    char **av;
{
    wordlist *wl = NULL;

#ifdef HAS_X11
    char *displayname;
    /* grrr, Xtk forced contortions */
    char *argv[2];
    int argc = 2;
    char buf[512];
#endif

    ivars( );

    cp_in = stdin;
    cp_out = stdout;
    cp_err = stderr;

#ifdef HAS_X11

    if (cp_getvar("display", VT_STRING, buf)) {
      displayname = buf;
    } else if (!(displayname = getenv("DISPLAY"))) {
      fprintf(stderr, "Can't open X display.");
      goto out;
    }

    argv[0] = "nutmeg";
    argv[1] = displayname;
    /* initialize X toolkit */
    toplevel = XtInitialize("nutmeg", "Nutmeg", NULL, 0, &argc, argv);

#endif

out:
    if (ac > 1)
        wl = wl_build(av + 1);
    hlp_main(Help_Path, wl);

#ifdef HAS_X11
    if (hlp_usex) {
	printf("Hit control-C when done.\n");		/* sigh */
	XtMainLoop();
    }
#endif

    exit(EXIT_NORMAL);
}

void
fatal(s)
    char *s;
{
    fprintf(stderr, "fatal error: %s\n", s);
    exit(1);
}

void
cp_printword(s)
    char *s;
{
    printf("%s", s);
    return;
}

/* ARGSUSED */
bool
cp_getvar(n, t, r)
    char *n, *r;
    int t;
{
    return (false);
}

char *
cp_tildexpand(s)
	char	*s;
{
	return tilde_expand(s);
}
