/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher
**********/

/*
 * The main routine for spice 3 and nutmeg.
 */

#include "spice.h"
#include "misc.h"
#include "ifsim.h"
#include "inpdefs.h"
#include "iferrmsg.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "ftedev.h"
#include "ftedebug.h"
#include "const.h"
#include <setjmp.h>

#include <sys/types.h>

#ifdef HAS_GETPW
#include <pwd.h>
#endif

#ifdef HAS_UNIX_SIGS
#include <signal.h>
#endif

#ifndef HAS_BSDRUSAGE
#ifdef HAS_FTIME
#include <sys/timeb.h>
#endif
#endif

#include "patchlev.h"
#include "suffix.h"

int Patch_Level = PATCHLEVEL;
char *ft_rawfile = "rawspice.raw";

char *errRtn;
char *errMsg;
char *cp_program;

bool ft_servermode = false;
bool ft_intrpt = false;     /* Set by the (void) signal handlers. */
bool ft_setflag = false;    /* Don't abort after an interrupt. */

struct variable *(*if_getparam)( );

#ifdef BATCH

DISPDEVICE device[ ] = {
	{"", 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0 }
	};
DISPDEVICE *dispdev = device;
bool ft_nopage = false;
bool ft_nomod = false;
bool ft_parsedb = false;
bool ft_evdb = false;
bool ft_vecdb = false;
bool ft_grdb = false;
bool ft_gidb = false;
bool ft_controldb = false;
bool ft_asyncdb = false;

bool ft_debug = false;
bool ft_batchmode = true;
bool ft_nospiceadd = true;
bool ft_simdb = false;
static char *usage = "Usage: %s [ - ] [ -r rawfile ] [ file ... ]\n";
FILE *cp_curin = 0;
FILE *cp_curout = 0;
FILE *cp_curerr = 0;
FILE *cp_in = 0;
FILE *cp_out = 0;
FILE *cp_err = 0;
bool gi_endpause = true;

bool ft_listprint = true;
bool ft_optsprint = false;
bool ft_nodesprint = false;
bool ft_acctprint = false;

#  ifndef SPICE2
struct plot *plot_list = NULL;
/* int raw_prec = -1; */
int plot_num = 1;
#  endif

#else

bool ft_batchmode = false;
jmp_buf jbuf;
static char *usage = 
"Usage: %s [-] [-b] [-i] [-s] [-n] [-o outfile] [-r rawfile]\n\
\t[-t term] [file ...]\n";
struct options *exitoption;
struct options *helpoption;
#endif

static started = false;
IFsimulator *ft_sim = 0;

#ifdef MSDOS
/* XXX Can this be moved elsewhere? */
int _stack = 8000;
#endif

#ifndef HAS_BSDRUSAGE
#ifdef HAS_FTIME
extern struct timeb timebegin;		/* for use w/ ftime */
#endif
#endif

extern IFsimulator SIMinfo;

#ifdef SIMULATOR

bool ft_nutmeg = false;

#ifndef BATCH
extern struct comm spcp_coms[ ];
struct comm *cp_coms = spcp_coms;
#endif
extern int OUTpBeginPlot(), OUTpData(), OUTwBeginPlot(), OUTwReference();
extern int OUTwData(), OUTwEnd(), OUTendPlot(), OUTbeginDomain();
extern int OUTendDomain(), OUTstopnow(), OUTerror(), OUTattributes();
static IFfrontEnd nutmeginfo = {
    IFnewUid,
    IFdelUid,
    OUTstopnow,
    seconds,
    OUTerror,
    OUTpBeginPlot,
    OUTpData,
    OUTwBeginPlot,
    OUTwReference,
    OUTwData,
    OUTwEnd,
    OUTendPlot,
    OUTbeginDomain,
    OUTendDomain,
    OUTattributes
};

#else

bool ft_nutmeg = true;
extern struct comm nutcp_coms[ ];
struct comm *cp_coms = nutcp_coms;
static IFfrontEnd nutmeginfo;
/* XXX */
/* ARGSUSED */ int if_run(t, w, s, b) char *t, *w, *b; wordlist *s;
        { return (0); }
/* ARGSUSED */ int if_sens_run(t, w, s, b) char *t, *w, *b; wordlist *s;
        { return (0); }
/* ARGSUSED */ void if_dump(ckt, fp) char *ckt; FILE *fp; { }
/* ARGSUSED */ char * if_inpdeck(deck, tab) struct line *deck; char **tab;
        { return ((char *) 0); }
/* ARGSUSED */ int if_option(ckt, name, type, value) char *name, *ckt, *value;
        int type; { }
/* ARGSUSED */ void if_cktfree(ckt, tab) char *ckt, *tab; { }
/* ARGSUSED */ void if_setndnames(line) char *line; { }
/* ARGSUSED */ char * if_errstring(code) { return ("spice error"); }
/* ARGSUSED */ void if_setparam(ckt, name, param, val) 
        char *ckt, *name, *param;  struct variable *val; { }
/* ARGSUSED */
bool
if_tranparams(ckt, start, stop, step) struct circ *ckt;
        double *start, *stop, *step;
{ return (false); }
/* ARGSUSED */ struct variable *if_getstat(n, c) char *n, *c; { return (NULL);}

#endif

char *hlp_filelist[] = { "spice", 0 };

void
main(ac, av)
    char **av;
{
    char	**tv;
    int		tc, i;
    int		err;
    bool	gotone = false;
    char	*p;
    char	*cmd_line_term = 0, term_1stch;
    int		error2;

#ifdef BATCH
    bool	st = false;
#else

#ifdef HAS_GETPW
    struct passwd *pw;
#endif
    char	buf[BSIZE_SP];
    bool	readinit = true, rflag = false, ciprefix();
    bool	istty = true, iflag = false, qflag = false;
    bool	gdata = true;
    FILE	*fp;

#endif

    /* MFB tends to jump to 0 on errors.  This tends to catch it. */
    if (started) {
        fprintf(cp_err, "main: Internal Error: jump to zero\n");
        fatal();
    }
    started = true;

#ifdef HAS_MAC_ARGCARGV
    ac = initmac(&av);
#endif
    ivars( );

    cp_in = stdin;
    cp_out = stdout;
    cp_err = stderr;

#ifdef BATCH

    cp_curin = stdin;
    cp_curout = stdout;
    cp_curerr = stderr;

#else

#ifdef MALLOCTRACE
    mallocTraceInit("malloc.out");
#endif
#ifdef HAS_ISATTY
    istty = (bool) isatty(fileno(stdin));
#endif

#endif /* ~ BATCH */

    init_time( );

    err = SIMinit(&nutmeginfo,&ft_sim);
    if(err != OK) {
        ft_sperror(err,"SIMinit");
        exit(EXIT_BAD);
    }
    cp_program = ft_sim->simulator;

#ifdef HAS_GETPID
    srandom(getpid());
#else
    srandom(17);
#endif

    tv = av;
    tc = ac;

    /* Pass 1 -- get options. */
    while (--tc > 0) {
        tv++;
        if (**tv == Spice_OptChar)    /* Option argument. */
            switch ((*tv)[1]) {

#ifndef BATCH
	    case '\0':  /* No raw file. */
                gdata = false;
                break;

	    case 'b':   /* Batch mode. */
	    case 'B':
                ft_batchmode = true;
                break;

	    case 's':   /* Server mode. */
	    case 'S':
                ft_servermode = true;
                break;

	    case 'i':   /* Interactive mode. */
	    case 'I':
                iflag = true;
                break;

	    case 'q':   /* No command completion. */
	    case 'Q':
                qflag = true;
                break;

	    case 'n':   /* Don't read .spiceinit. */
	    case 'N':
                readinit = false;
                break;

	    case 't':   /* Terminal type. */
	    case 'T':
                if (tc > 1) {
                    tc--;
                    tv++;
		    cmd_line_term = *tv;
		    term_1stch = **tv;
                    **tv = Spice_OptChar;
                } else {
                    fprintf(cp_err, usage, 
                        cp_program);
                    exit(EXIT_BAD);
                }
                break;
	    case 'r':   /* The rawfile. */
	    case 'R':
                if (tc > 1) {
                    tc--;
                    tv++;
                    cp_vset("rawfile", VT_STRING, *tv);
                    **tv = Spice_OptChar;
                }
                rflag = true;
                break;

#else /* if BATCH */
	    case 'r':   /* The rawfile. */
	    case 'R':
                if (tc > 1) {
                    tc--;
                    tv++;
		    ft_rawfile = copy(*tv);
                    **tv = Spice_OptChar;
                } else {
		    fprintf(cp_err, usage, cp_program);
		    exit(EXIT_BAD);
		}
                break;
#endif

	    case 'o':   /* Output file. */
	    case 'O':
                if (tc > 1) {
                    tc--;
                    tv++;
                    if (!(freopen(*tv, "w", stdout))) {
                        perror(*tv);
                        exit(EXIT_BAD);
                    }
                    **tv = Spice_OptChar;
                } else {
                    fprintf(cp_err, usage, av[0]);
                    exit(EXIT_BAD);
                }
                break;

	    default:
                fprintf(cp_err, "Error: bad option %s\n", *tv);
                fprintf(cp_err, usage, cp_program);
                exit(EXIT_BAD);
            }
    }

#ifdef SIMULATOR
    if_getparam = spif_getparam;
#else
    if_getparam = nutif_getparam;
#endif

#ifndef BATCH

    if ((!iflag && !istty) || ft_servermode)
        ft_batchmode = true;
    if ((iflag && !istty) || qflag)
        cp_nocc = true;
    if (ft_servermode)
        readinit = false;
    if (!istty || ft_batchmode)
        out_moremode = false;

    /* Would like to do this later, but cpinit evals commands */
    init_rlimits( );

    /* Have to initialize cp now. */
    ft_cpinit();

    /* To catch interrupts during .spiceinit... */
    if (setjmp(jbuf) == 1) {
        fprintf(cp_err, "Warning: error executing .spiceinit.\n");
        if (!ft_batchmode)
            goto bot;
    }

#  ifdef HAS_UNIX_SIGS
    /* Set up (void) signal handling */
    if (!ft_batchmode) {
        (void) signal(SIGINT, ft_sigintr);
        (void) signal(SIGFPE, sigfloat);
#    ifdef SIGTSTP
        (void) signal(SIGTSTP, sigstop);
#    endif
    }
    /* Set up (void) signal handling for fatal errors. */
    (void) signal(SIGILL, sigill);

#    ifdef SIGBUS
    (void) signal(SIGBUS, sigbus);
#    endif
#    ifdef SIGSEGV
    (void) signal(SIGSEGV, sigsegv);
#    endif
#    ifdef SIGSYS
    (void) signal(SIGSYS, sig_sys);
#    endif

#  endif /* UNIX_SIGS */

#  ifdef HAS_GETPW
    if (readinit) {
        /* Try to source either .spiceinit or ~/.spiceinit. */
        if (access(".spiceinit", 0) == 0)
            inp_source(".spiceinit");
        else {
            pw = getpwuid(getuid());
            (void) strcpy(buf, pw->pw_dir);
            (void) strcat(buf, "/.spiceinit");
            if (access(buf, 0) == 0)
                inp_source(buf);
        }
    }
#  else /* ~ HAS_GETPW */
    /* Try to source the file "spice.rc" in the current directory.  */
    if (readinit) {
        if ((fp = fopen("spice.rc", "r")) != NULL) {
            (void) fclose(fp);
            inp_source("spice.rc");
        }
    }
#  endif /* ~ HAS_GETPW */

    if (cmd_line_term) {
	*cmd_line_term = term_1stch; /* XXX oh, gross!
				      *	First char got squashed scanning the
				      * command line
				      */
	cp_vset("term", VT_STRING, cmd_line_term);
    }

    if (!ft_batchmode) {
        DevInit( );
	com_version(NULL);
	if (News_File && *News_File) {
	    fp = fopen(cp_tildexpand(News_File), "r");
	    if (fp) {
		while (fgets(buf, BSIZE_SP, fp))
		    fputs(buf, stdout);
		(void) fclose(fp);
	    }
	}
    }


bot:

    /* Pass 2 -- get the filenames. If we are spice, then this means
     * build a circuit for this file. If this is in server mode, don't
     * process any of these args.
     */

#  ifdef HAS_LONGJUMP
    if (setjmp(jbuf) == 1)
        goto evl;
#  endif
#endif /* ~ BATCH */

    cp_interactive = false;
    err = 0;

#ifdef SIMULATOR
    if (!ft_servermode && !ft_nutmeg) {
        FILE *file = NULL, *tp = NULL;
        char *tempfile = NULL, buf[BSIZE_SP], *smktemp();

        for (tv = av + 1, i = 0; *tv; tv++)
            if (**tv != Spice_OptChar)
                i++;
        if (i == 1) {
            for (tv = av + 1, i = 0; *tv; tv++)
                if (**tv != Spice_OptChar)
                    break;
            if (!(file = fopen(*tv, "r"))) {
                perror(*tv);
                i = 0;
		if (ft_batchmode)
		    exit(EXIT_BAD);
            }
        } else if (i) {
            tempfile = smktemp("sp");
            if (!(file = fopen(tempfile, "w+"))) {
                perror(tempfile);
                exit(EXIT_BAD);
            }
            for (tv = av + 1, i = 0; *tv; tv++)
                if (**tv != Spice_OptChar) {
                    if (!(tp = fopen(*tv, "r"))) {
                        perror(*tv);
			err = 1;
                        break;
                    }
                    while ((i = fread(buf, 1, BSIZE_SP, tp)) > 0)
                        (void) fwrite(buf, i, 1, file);
                    (void) fclose(tp);
                }
            (void) fseek(file, (long) 0, 0);
        }
        if (file && (!err || !ft_batchmode)) {
            inp_spsource(file, false, tempfile ? (char *) NULL : *tv);
            gotone = true;
        }
#  ifdef HAS_UNLINK
	if (tempfile)
	    (void) unlink(tempfile);
#  endif
	if (ft_batchmode && err)
	    exit(EXIT_BAD);
    }
#endif

#ifndef BATCH
    if (ft_nutmeg && gdata) {
        /* Read in the rawfiles */
        for (av++; *av; av++)
            if (**av != Spice_OptChar) {
                ft_loadfile(*av);
                gotone = true;
            }
        if (!gotone)
            ft_loadfile(ft_rawfile);
    }
#endif

#ifdef SIMULATOR
    if (!gotone && ft_batchmode && !ft_nutmeg)
        inp_spsource(stdin, false, (char *) NULL);

#  ifndef BATCH
evl:
    if (ft_batchmode) {
        /* If we get back here in batch mode then something is
         * wrong, so exit.
         */
        bool st = false;

        (void) setjmp(jbuf);

        if (st == true)
            exit(EXIT_BAD);
        st = true;
        if (ft_servermode) {
            if (ft_curckt == NULL) {
                fprintf(cp_err, "Error: no circuit loaded!\n");
                exit(EXIT_BAD);
            }
            if (ft_dorun(""))
		exit(EXIT_BAD);
            exit(EXIT_NORMAL);
        }
        /* If -r is specified, then we don't bother with the dot
         * cards. Otherwise, we use wrd_run, but we are careful
         * not to save too much.
         */
        cp_interactive = false;
        if (rflag) {
            ft_dotsaves();
            error2 = ft_dorun(ft_rawfile);
            if (ft_cktcoms(true) || error2)
		exit(EXIT_BAD);
        } else {
            if (ft_savedotargs()) {
		error2 = ft_dorun(NULL);
		if (ft_cktcoms(false) || error2)
		    exit(EXIT_BAD);
	    } else {
		fprintf(stderr,
"Note: No \".plot\", \".print\", or \".fourier\" lines; no simulations run\n");
		exit(EXIT_BAD);
	    }
        }
    } else {
        (void) setjmp(jbuf);
        cp_interactive = true;
	while (cp_evloop((char *) NULL) == 1) ;
    }

#  else /* if BATCH */

    if (st == true)
	exit(EXIT_BAD);
    st = true;
    if (ft_servermode) {
	if (ft_curckt == NULL) {
	    fprintf(cp_err, "Error: no circuit loaded!\n");
	    exit(EXIT_BAD);
	}
    }
#    ifdef SPICE2
    if (ft_savedotargs( )) {
	error2 = ft_dorun(NULL);
	if (ft_cktcoms(false) || error2)
	    exit(EXIT_BAD);
    } else {
	fprintf(stderr,
"Note: No \".plot\", \".print\", or \".fourier\" lines; no simulations run\n");
	exit(EXIT_BAD);
    }
#    else
    ft_dotsaves( );
    error2 = ft_dorun(ft_rawfile);
    if (ft_cktcoms(false) || error2)
	exit(EXIT_BAD);

#    endif

#  endif
#else

evl:
    /* Nutmeg "main" */
    (void) setjmp(jbuf);
    cp_interactive = true;
    while (cp_evloop((char *) NULL) == 1) ;

#endif

    exit(EXIT_NORMAL);
}

/* allocate space for global constants in 'CONST.h' */

double CONSTroot2;
double CONSTvt0;
double CONSTKoverQ;
double CONSTe;
IFfrontEnd *SPfrontEnd = NULL;

int
SPIinit(frtEnd,description)
    IFfrontEnd *frtEnd;
    IFsimulator **description;
{

    SPfrontEnd = frtEnd;
    *description = &SIMinfo;
    CONSTroot2 = sqrt(2.);
    CONSTvt0 = CONSTboltz * (27 /* deg c */ + CONSTCtoK ) / CHARGE;
    CONSTKoverQ = CONSTboltz / CHARGE;
    CONSTe = exp((double)1.0);
    return(OK);
}

/* XXX SIMinit and SPIinit ?? */

int SIMinit(frontEnd,simulator)
    IFfrontEnd *frontEnd;
    IFsimulator **simulator;
{
    return(SPIinit(frontEnd,simulator));
}

#ifdef BATCH
/*
 * Incredibly Ugly
 */

/* Now some misc junk that we need to fake */

bool cp_nocc = true;
bool cp_debug = false;
int cp_maxhistlength = 0;
char cp_chars[128];

MFBHalt( ) { }

#ifndef SPICE2
/* ARGSUSED */ char *cp_tildexpand(s) char *s; { return (s); }
/* ARGSUSED */ struct dvec *vec_fromplot(w, p) char *w; struct plot *p;
        { return (NULL); }
void vec_new() {}
void plot_new() {}
void plot_setcur() {}
/* ARGSUSED */ struct plot *plot_alloc(name) char *name; { return (NULL); }
struct plot *plot_cur = NULL;
/* ARGSUSED */ void plot_docoms(wl) wordlist *wl; {}
void out_init() {}

void
out_printf(fmt, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10)
    char *fmt, *s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10;
{
    printf(fmt, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10);
}

/* ARGSUSED */ void out_send(s) char *s; {}
char out_pbuf[1];
/* ARGSUSED */ struct dvec *vec_get(word) char *word; { return (NULL); }
#endif

/* ARGSUSED */ int cp_usrset(v, i) struct variable *v; bool i; {return(US_OK);}
void cp_pushcontrol() {}
void cp_popcontrol() {}
/* ARGSUSED */ void cp_addkword(class, word) int class; char *word; {}
struct circ *ft_circuits = 0, *ft_curckt = 0;
/* ARGSUSED */ char *cp_kwswitch(c, t) int c; char *t; { return (NULL); }
/* ARGSUSED */ bool ft_bpcheck(r, i) struct plot *r; int i; { return (true); }
/* ARGSUSED */ void cp_ccon(b) bool b; {}
/* ARGSUSED */ wordlist *cp_cctowl(stuff) char *stuff; { return NULL; }

int currentgraph = 0;
/* ARGSUSED */ void Input(p, q) char *p, *q; {}
/* ARGSUSED */ void DevSwitch(i) int i; {}
/* ARGSUSED */ int *CopyGraph(i) int i; {}
/* ARGSUSED */ void DestroyGraph(i) int i; {}
/* ARGSUSED */ void NewViewport(i) int i; {}

/* ARGSUSED */ void cp_remkword() { }
/* ARGSUSED */ void cp_resetcontrol() { }

bool gr_init() {}
void gr_redraw() {}
/* ARGSUSED */ void gr_iplot(pl) struct plot *pl; {}
/* ARGSUSED */ void gr_end_iplot(pl) struct plot *pl; {}
/* ARGSUSED */ void gr_pmsg(text, more) char *text; bool more; {}
/* ARGSUSED */ void gr_clean(intr) bool intr; {}
/* ARGSUSED */ int cp_evloop(string) char *string; { return (0); }
void ft_graf() {}
void ft_trquery() {}
/* ARGSUSED */ void ft_newcirc(ckt) struct circ *ckt; {}
void cp_doquit() { exit(0); }
/* ARGSUSED */ void cp_usrvars(v1, v2) struct variable **v1, **v2; { return; }
/* ARGSUSED */ struct variable * cp_enqvar(word) char *word; { return (NULL); }
/* ARGSUSED */ void cp_ccom(w, b, e) wordlist *w; char *b; bool e; { return; }
/* ARGSUSED */ void com_resume( ) { return; }
/* ARGSUSED */ int if_sens_run( ) { return 0; }
/* ARGSUSED */ int ft_xgraph( ) { return 0; }
/* ARGSUSED */ int gr_resize( ) { return 0; }

/* This is from options.c */

/* Extract the .option cards from the deck... */

struct line *
inp_getopts(deck)
    struct line *deck;
{
    struct line *last = NULL, *opts = NULL, *dd, *next = NULL;

    for (dd = deck->li_next; dd; dd = next) {
        next = dd->li_next;
        if (ciprefix(".opt", dd->li_line)) {
            inp_casefix(dd->li_line);
            if (last)
                last->li_next = dd->li_next;
            else
                deck->li_next = dd->li_next;
            dd->li_next = opts;
            opts = dd;
        } else
            last = dd;
    }
    return (opts);
}

#ifndef SPICE2

/* This is from dotcards.c -- we don't want to include the whole file. */

static wordlist *gettoks();

/* Extract all the .save cards */

void
ft_dotsaves()
{
    wordlist *com, *wl = NULL;
    char *s;

    if (!ft_curckt) /* Shouldn't happen. */
        return;

    for (com = ft_curckt->ci_commands; com; com = com->wl_next) {
        if (ciprefix(".save", com->wl_word)) {
            s = com->wl_word;
            (void) gettok(&s);
            wl = wl_append(wl, gettoks(s));
        }
    }

    com_save(wl);
    return;
}

/* This is also from dotcards.c.  Stripped down version
 * for "rawfile only mode": execute .options acct and .options list,
 * and issue a warning message about any other output "dot" lines.
 */

int
ft_cktcoms(terse)
    bool terse;
{
    wordlist *coms, *command;
    char *plottype, *s;
    struct dvec *v;
    static wordlist twl = { "col", NULL, NULL } ;
    int i;

    if (!ft_curckt)
        return;
    cp_interactive = false;

    /* Circuit name */
    fprintf(cp_out, "Circuit: %s\nDate: %s\n\n", ft_curckt->ci_name,
            datestring());
    fprintf(cp_out, "\n");

    /* Listing if necessary... */
    if (ft_listprint)
        inp_list(cp_out, ft_curckt->ci_deck, ft_curckt->ci_options,
                LS_DECK);

    for (coms = ft_curckt->ci_commands; coms; coms = coms->wl_next) {
	if (ciprefix(".print", s) || ciprefix(".plot", s) ||
	    ciprefix(".four", s))
	{
	    fprintf(stderr,
	    "Note: \".plot\", \".print\", and .\"fourier\" lines ignored\n");
	    fprintf(stderr, "      (rawfile only)\n");
	    break;
	}
    }

    /* And finally the accounting info. */
    if (ft_acctprint) {
        static wordlist ww = { "everything", NULL, NULL } ;
        com_rusage(&ww);
    } else
        com_rusage((wordlist *) NULL);

    (void) putc('\n', cp_out);
    return;

bad:    fprintf(cp_err, "Internal Error: ft_cktcoms: bad commands\n");
    return;
}

static wordlist *
gettoks(s)
    char *s;
{
    char *t, *r, buf[64];
    wordlist *wl = NULL, *end = NULL;
    bool iflag;

    while (t = gettok(&s)) {
        if (*t == '(' /* ) */) {
            /* This is a (upper, lower) thing -- ignore. */
            continue;
        } else if (!index(t, '(' /*)*/ )) {
            if (end) {
                end->wl_next = alloc(struct wordlist);
                end->wl_next->wl_prev = end;
                end = end->wl_next;
            } else
                wl = end = alloc(struct wordlist);
            end->wl_word = copy(t);
        } else if (!index(t, ',')) {
            iflag = ((*t == 'i') || (*t == 'I')) ? true : false;
            while (*t != '(' /*)*/)
                t++;
            t++;
            for (r = t; *r && *r != /*(*/ ')'; r++)
                ;
            *r = '\0';
            if (end) {
                end->wl_next = alloc(struct wordlist);
                end->wl_next->wl_prev = end;
                end = end->wl_next;
            } else
                wl = end = alloc(struct wordlist);
            if (iflag) {
                (void) sprintf(buf, "%s#branch", t);
                t = buf;
            }
            end->wl_word = copy(t);
        } else {
            /* The painful case... */
            while (*t != '(' /*)*/)
                t++;
            t++;
            for (r = t; *r && *r != ','; r++)
                ;
            *r = '\0';
            if (end) {
                end->wl_next = alloc(struct wordlist);
                end->wl_next->wl_prev = end;
                end = end->wl_next;
            } else
                wl = end = alloc(struct wordlist);
            end->wl_word = copy(t);
            t = r + 1;
            for (r = t; *r && *r != /*(*/ ')'; r++)
                ;
            *r = '\0';
            if (end) {
                end->wl_next = alloc(struct wordlist);
                end->wl_next->wl_prev = end;
                end = end->wl_next;
            } else
                wl = end = alloc(struct wordlist);
            end->wl_word = copy(t);
        }
    }
    return (wl);
}

#endif

#endif
