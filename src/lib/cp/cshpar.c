/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * The main entry point for cshpar.
 */

#include "spice.h"
#include "misc.h"
#include "cpdefs.h"

#ifdef HAS_BSDTTY
#include <sgtty.h>
#else
#ifdef HAS_SYSVTTY
#include <termio.h>
#else
#ifdef HAS_POSIXTTY
#include <termios.h>
#endif
#endif
#endif

#ifdef HAS_GETPW
#include <pwd.h>
#endif
#ifdef HAS_UNIX_SIGS
#include <signal.h>
#endif
#ifdef HAS_WAIT2
#include <sys/wait.h>
#endif

#include "suffix.h"

/* Things go as follows:
 * (1) Read the line and do some initial quoting (by setting the 8th bit),
 *  and command ignoring. Also deal with command completion.
 * (2) Do history substitutions. (!, ^)
 * (3) Do alias substitution.
 * 
 * In front.c these things get done:
 * (4) Do variable substitution. ($varname)
 * (5) Do backquote substitution. (``)
 * (6) Do globbing. (*, ?, [], {}, ~)
 * (7) Do io redirection.
 */

static bool fileexists();
void fixdescriptors();
static void pwlist();

bool cp_debug = false;
char cp_gt = '>';
char cp_lt = '<';
char cp_amp = '&';

FILE *cp_in;
FILE *cp_out;
FILE *cp_err;

/* These are the fps that cp_ioreset resets the cp_* to.  They are changed
 * by the source routines.
 */

FILE *cp_curin = NULL;
FILE *cp_curout = NULL;
FILE *cp_curerr = NULL;

wordlist *
cp_parse(string)
    char *string;
{
    wordlist *wlist;

    wlist = cp_lexer(string);

    if (!string)
        cp_event++;

    if (!wlist || !wlist->wl_word)
        return (wlist);

    pwlist(wlist, "Initial parse");

    wlist = cp_histsubst(wlist);
    if (!wlist || !wlist->wl_word)
        return (wlist);
    pwlist(wlist, "After history substitution");
    if (cp_didhsubst) {
        wl_print(wlist, stdout);
        (void) putc('\n', stdout);
    }

    /* Add the word list to the history. */
    if (*wlist->wl_word)
        cp_addhistent(cp_event - 1, wlist);

    wlist = cp_doalias(wlist);
    pwlist(wlist, "After alias substitution");

    if (string && cp_lastone) {
        /* Don't put this one in... */
        cp_lastone = cp_lastone->hi_prev;
        if (cp_lastone)
            cp_lastone->hi_next = NULL;
    }

    pwlist(wlist, "Returning ");
    return (wlist);
}

static void
pwlist(wlist, name)
    wordlist *wlist;
    char *name;
{
    wordlist *wl;

    if (!cp_debug)
        return;
    fprintf(cp_err, "%s : [ ", name);
    for (wl = wlist; wl; wl = wl->wl_next)
        fprintf(cp_err, "%s ", wl->wl_word);
    fprintf(cp_err, "]\n");
    return;
}

/* This has to go somewhere... */

void
com_echo(wlist)
    wordlist *wlist;
{
    bool nl = true;

    if (wlist && eq(wlist->wl_word, "-n")) {
        wlist = wlist->wl_next;
        nl = false;
    }

    while (wlist) {
        fputs(cp_unquote(wlist->wl_word), cp_out);
        if (wlist->wl_next)
            fputs(" ", cp_out);
        wlist = wlist->wl_next;
    }
    if (nl)
        fputs("\n", cp_out);
}

/* This routine sets the cp_{in,out,err} pointers and takes the io
 * directions out of the command line.
 */

wordlist *
cp_redirect(wl)
    wordlist *wl;
{
    bool gotinput = false, gotoutput = false, goterror = false;
    bool app = false, erralso = false;
    wordlist *w, *bt, *nw;
    char *s;
    FILE *tmpfp;

    w = wl->wl_next;    /* Don't consider empty commands. */
    while (w) {
        if (*w->wl_word == cp_lt) {
            bt = w;
            if (gotinput) {
                fprintf(cp_err, 
                "Error: ambiguous input redirect.\n");
                goto error;
            }
            gotinput = true;
            w = w->wl_next;
            if (w == NULL) {
                fprintf(cp_err, 
                "Error: missing name for input.\n");
                return (NULL);
            }
            if (*w->wl_word == cp_lt) {
                /* Do reasonable stuff here... */
            } else {
                tmpfp = fopen(cp_unquote(w->wl_word), "r");
                if (!tmpfp) {
                    perror(w->wl_word);
                    goto error;
                } else
                    cp_in = tmpfp;
            }
#ifdef CPDEBUG
            if (cp_debug)
                fprintf(cp_err, "Input file is %s...\n",
                    w->wl_word);
#endif
            bt->wl_prev->wl_next = w->wl_next;
            if (w->wl_next)
                w->wl_next->wl_prev = bt->wl_prev;
            nw = w->wl_next;
            w->wl_next = NULL;
            w = nw;
            wl_free(bt);
        } else if (*w->wl_word == cp_gt) {
            bt = w;
            if (gotoutput) {
                fprintf(cp_err, 
                "Error: ambiguous output redirect.\n");
                goto error;
            }
            gotoutput = true;
            w = w->wl_next;
            if (w == NULL) {
                fprintf(cp_err, 
                "Error: missing name for output.\n");
                return (NULL);
            }
            if (*w->wl_word == cp_gt) {
                app = true;
                w = w->wl_next;
                if (w == NULL) {
                    fprintf(cp_err, 
                    "Error: missing name for output.\n");
                    return (NULL);
                }
            }
            if (*w->wl_word == cp_amp) {
                erralso = true;
                if (goterror) {
                    fprintf(cp_err, 
                "Error: ambiguous error redirect.\n");
                    return (NULL);
                }
                goterror = true;
                w = w->wl_next;
                if (w == NULL) {
                    fprintf(cp_err, 
                    "Error: missing name for output.\n");
                    return (NULL);
                }
            }
            s = cp_unquote(w->wl_word);
            if (cp_noclobber && fileexists(s)) {
                fprintf(stderr, "Error: %s: file exists\n", s);
                goto error;
            }
            if (app)
                tmpfp = fopen(s, "a");
            else
                tmpfp = fopen(s, "w+");
            if (!tmpfp) {
                perror(w->wl_word);
                goto error;
            } else {
                cp_out = tmpfp;
                out_isatty = false;
            }
#ifdef CPDEBUG
            if (cp_debug)
                fprintf(cp_err, "Output file is %s... %s\n",
                    w->wl_word, app ? "(append)" : "");
#endif
            bt->wl_prev->wl_next = w->wl_next;
            if (w->wl_next)
                w->wl_next->wl_prev = bt->wl_prev;
            w = w->wl_next;
            if (w)
                w->wl_prev->wl_next = NULL;
            wl_free(bt);
            if (erralso)
                cp_err = cp_out;
        } else
            w = w->wl_next;
    }
    return (wl);

error:  wl_free(wl);
    return (NULL);
}

/* Reset the cp_* FILE pointers to the standard ones.  This is tricky, since
 * if we are sourcing a command file, and io has been redirected from inside
 * the file, we have to reset it back to what it was for the source, not for
 * the top level.  That way if you type "foo > bar" where foo is a script,
 * and it has redirections of its own inside of it, none of the output from
 * foo will get sent to stdout...
 */

void
cp_ioreset()
{
    if (cp_in != cp_curin) {
        if (cp_in)
            (void) fclose(cp_in);
        cp_in = cp_curin;
    }
    if (cp_out != cp_curout) {
        if (cp_out)
            (void) fclose(cp_out);
        cp_out = cp_curout;
    }
    if (cp_err != cp_curerr) {
        if (cp_err)
            (void) fclose(cp_err);
        cp_err = cp_curerr;
    }

    /*** Minor bug here... */
    out_isatty = true;
    return;
}

static bool
fileexists(name)
    char *name;
{
#ifdef HAS_ACCESS
    if (access(name, 0) == 0)
        return (true);
#endif
    return (false);
}

#ifdef HAS_SYSTEM

/* Fork a shell. */

void
com_shell(wl)
    wordlist *wl;
{
    char *com, *shell = NULL;
    long pid, r;
#ifdef HAS_UNIX_SIGS
    static SIGNAL_TYPE (*svint)( ), (*svquit)( ), (*svtstp)( );
#endif

#ifdef HAS_ENVIRON
    shell = getenv("SHELL");
    if (shell == NULL)
        shell = "/bin/csh";
#endif

        cp_ccon(false);

#ifdef HAS_VFORK
	/* XXX Needs to switch process groups.  Also, worry about suspend */
	/* Only bother for efficiency */
        pid = vfork();
        if (pid == 0) {
            fixdescriptors();
	    if (wl == NULL) {
		(void) execl(shell, shell, 0);
		_exit(99);
	    } else {
		com = wl_flatten(wl);
		(void) execl("/bin/sh", "sh", "-c", com, 0);
	    }
        } else {
	    /* XXX Better have all these signals */
	    svint = signal(SIGINT, SIG_DFL);
	    svquit = signal(SIGQUIT, SIG_DFL);
	    svtstp = signal(SIGTSTP, SIG_DFL);
	    /* XXX Sig on proc group */
            do {
                r = wait((union wait *) NULL);
            } while ((r != pid) && pid != -1);
	    (void) signal(SIGINT, (SIGNAL_FUNCTION) svint);
	    (void) signal(SIGQUIT, (SIGNAL_FUNCTION) svquit);
	    (void) signal(SIGTSTP, (SIGNAL_FUNCTION) svtstp);
        }
#else
    /* Easier to forget about changing the io descriptors. */
    if (wl) {
        com = wl_flatten(wl);
        (void) system(com);
    } else
        (void) system(shell);
#endif

    return;
}

#else

/* ARGSUSED */
void
com_shell(wl)
    wordlist *wl;
{
    fprintf(cp_err, "Error: No shell available.\n");
    return;
}

#endif

/* Do this only right before an exec, since we lose the old std*'s. */

void
fixdescriptors()
{
    if (cp_in != stdin)
        (void) dup2(fileno(cp_in), fileno(stdin));
    if (cp_out != stdout)
        (void) dup2(fileno(cp_out), fileno(stdout));
    if (cp_err != stderr)
        (void) dup2(fileno(cp_err), fileno(stderr));
    return;
}

/* ARGSUSED */
void
com_rehash(wl)
    wordlist *wl;
{
    char *s;

    if (!cp_dounixcom) {
        fprintf(cp_err, "Error: unixcom not set.\n");
        return;
    }
    s = getenv("PATH");
    if (s)
        cp_rehash(s, true);
    else
        fprintf(cp_err, "Error: no PATH in environment.\n");
    return;
}

#ifdef HAS_CHDIR
void
com_chdir(wl)
    wordlist *wl;
{
    char *s;
    struct passwd *pw;
    extern struct passwd *getpwuid( );
    char localbuf[257];
    int copied = 0;

    s = NULL;

    if (wl == NULL) {

#ifdef HAS_ENVIRON
	s = getenv("HOME");
#endif

#ifdef HAS_GETPW
	if (s == NULL) {
	    pw = getpwuid(getuid());
	    if (pw == NULL) {
		fprintf(cp_err, "Can't get your password entry\n");
		return;
	    }           
	    s = pw->pw_dir;
	}
#endif
    } else {
        s = cp_unquote(wl->wl_word);
	copied = 1;
    }

#ifdef HAS_DOSDIRS
    if (s && *s && s[1] == ':') {
	char	buf[1000];
	sprintf(buf, "%c:", *s);
	system(buf);
	strcpy(s, s + 2);
    }
#endif

    if (*s && chdir(s) == -1)
        perror(s);

    if (copied)
	tfree(s);

#ifdef HAS_GETCWD
    if (s = getcwd(localbuf, sizeof(localbuf)))
	    printf("current directory: %s\n", s);
    else
	    fprintf(cp_err, "Can't get current working directory.\n");
#endif

    return;

}

#else

void
com_chdir(wl)
    wordlist *wl;
{
    fprintf(cp_err, "Error: 'chdir' no available.\n");
    return;
}

#endif

/* This is a truly evil thing */

void
com_strcmp(wl)
    wordlist *wl;
{
    char *var, *s1, *s2;
    int i;

    var = wl->wl_word;
    s1 = cp_unquote(wl->wl_next->wl_word);
    s2 = cp_unquote(wl->wl_next->wl_next->wl_word);
    
    i = strcmp(s1, s2);

    cp_vset(var, VT_NUM, (char *) &i);
    return;
}

