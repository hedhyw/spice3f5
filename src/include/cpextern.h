/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1986 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Definitions for all external symbols in CP.
 */

#ifndef CPEXTERN_H
#define CPEXTERN_H

/* alias.c */

extern struct alias *cp_aliases;
extern void com_alias();
extern void com_unalias();
extern void cp_paliases();
extern void cp_setalias();
extern void cp_unalias();
extern wordlist *cp_doalias();

/* backquote.c */

extern char cp_back;
extern wordlist *cp_bquote();

/* complete.c */

extern bool cp_nocc;
extern bool cp_comlook();
extern char *cp_kwswitch();
extern void cp_addcomm();
extern void cp_addkword();
extern void cp_ccom();
extern void cp_ccon();
extern void cp_ccrestart();
extern void cp_remcomm();
extern void cp_remkword();
extern wordlist *cp_cctowl();

/* cshpar.c */

extern FILE *cp_in;
extern FILE *cp_out;
extern FILE *cp_err;
extern FILE *cp_curin;
extern FILE *cp_curout;
extern FILE *cp_curerr;
extern bool cp_debug;
extern char cp_amp;
extern char cp_gt;
extern char cp_lt;
extern void com_chdir();
extern void com_echo();
extern void com_strcmp();
extern void com_rehash();
extern void com_shell();
extern void cp_ioreset();
extern wordlist *cp_redirect();
extern wordlist *cp_parse();

/* front.c */

extern bool cp_cwait;
extern bool cp_dounixcom;
extern char *cp_csep;
extern int cp_evloop();
extern void com_cdump();
extern void cp_resetcontrol();
extern void cp_toplevel();
extern void cp_popcontrol();
extern void cp_pushcontrol();

/* glob.c */

extern bool cp_globmatch();
extern char *cp_tildexpand();
extern char cp_cbrac;
extern char cp_ccurl;
extern char cp_comma;
extern char cp_huh;
extern char cp_obrac;
extern char cp_ocurl;
extern char cp_star;
extern char cp_til;
extern wordlist *cp_doglob();

/* history.c */

extern bool cp_didhsubst;
extern char cp_bang;
extern char cp_hat;
extern int cp_maxhistlength;
extern struct histent *cp_lastone;
extern void com_history();
extern void cp_addhistent();
extern void cp_hprint();
extern wordlist *cp_histsubst();

/* lexical.c */

extern FILE *cp_inp_cur;
extern bool cp_bqflag;
extern bool cp_interactive;
extern char *cp_altprompt;
extern char *cp_promptstring;
extern char cp_hash;
extern int cp_event;
extern wordlist *cp_lexer();
extern int inchar();

/* modify.c */

extern char cp_chars[];
extern void cp_init();

/* output.c */

extern char out_pbuf[];
extern bool out_moremode;
extern bool out_isatty;
extern void out_init();
#ifndef out_printf
/* don't want to declare it if we have #define'ed it */
extern void out_printf();
#endif
extern void out_send();

/* quote.c */

extern char *cp_unquote();
extern void cp_quoteword();
extern void cp_striplist();
extern void cp_wstrip();

/* spawn.c */

#ifdef HAS_VMSHACK
extern int system();
#endif /*HAS_VMSHACK*/

/* unixcom.c */

extern bool cp_unixcom();
extern void cp_hstat();
extern void cp_rehash();

/* variable.c */

extern bool cp_getvar();
extern bool cp_ignoreeof;
extern bool cp_noclobber;
extern bool cp_noglob;
extern bool cp_nonomatch;
extern char cp_dol;
extern void com_set();
extern void com_unset();
extern void com_shift();
extern void cp_remvar();
extern void cp_vprint();
extern void cp_vset();
extern wordlist *cp_variablesubst();
extern wordlist *cp_varwl();
extern struct variable *cp_setparse();

/* var2.c */
wordlist *vareval();

/* cpinterface.c etc -- stuff CP needs from FTE */

extern bool cp_istrue();
extern bool cp_oddcomm();
extern void cp_doquit();
extern void cp_periodic();
extern void ft_cpinit();
extern struct comm *cp_coms;
extern double *ft_numparse();
extern char *cp_program;
extern bool ft_nutmeg;
extern struct variable *cp_enqvar();
extern void cp_usrvars();
extern int cp_usrset();
extern void fatal();

#endif
