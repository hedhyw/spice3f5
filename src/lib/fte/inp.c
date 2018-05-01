/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher
**********/

/*
 * Stuff for dealing with spice input decks and command scripts, and
 * the listing routines.
 */

#include "spice.h"
#include "cpdefs.h"
#include "inpdefs.h"
#include "ftedefs.h"
#include "ftedata.h"
#include "fteinp.h"
#include "suffix.h"

static char *readline();
static bool doedit();

/* Do a listing. Use is listing [expanded] [logical] [physical] [deck] */

void
com_listing(wl)
    wordlist *wl;
{
    int type = LS_LOGICAL;
    bool expand = false;
    char *s;

    if (ft_curckt) {
        while (wl) {
            s = wl->wl_word;
            switch (*s) {
                case 'l':
                case 'L':
                    type = LS_LOGICAL;
                    break;
                case 'p':
                case 'P':
                    type = LS_PHYSICAL;
                    break;
                case 'd':
                case 'D':
                    type = LS_DECK;
                    break;
                case 'e':
                case 'E':
                    expand = true;
                    break;
                default:
                    fprintf(cp_err,
                    "Error: bad listing type %s\n", s);
            }
            wl = wl->wl_next;
        }
        if (type != LS_DECK)
            fprintf(cp_out, "\t%s\n\n", ft_curckt->ci_name);
        inp_list(cp_out, expand ? ft_curckt->ci_deck :
                ft_curckt->ci_origdeck, ft_curckt->ci_options,
                type);
    } else
        fprintf(cp_err, "Error: no circuit loaded.\n");
    return;
}

static char *
upper(string)
    register char *string;
{

    static char buf[BSIZE_SP];
    register char *s;

    if (string) {
	strncpy(buf, string, BSIZE_SP - 1);
	buf[BSIZE_SP - 1] = 0;
	inp_casefix(buf);
    } else {
	strcpy(buf, "<null>");
    }

    return buf;

}

/* Provide an input listing on the specified file of the given
 * card deck.  The listing should be of either LS_PHYSICAL or LS_LOGICAL
 * or LS_DECK lines as specified by the type parameter.
 */

void
inp_list(file, deck, extras, type)
    FILE *file;
    struct line *deck, *extras;
    int type;
{
    struct line *here;
    struct line *there;
    struct line *tmp, *next;
    bool renumber;
    bool useout = (file == cp_out);
    int i = 1;

    if (useout)
        out_init();
    (void) cp_getvar("renumber", VT_BOOL, (char *) &renumber);
    if (type == LS_LOGICAL) {
top1:       for (here = deck; here; here = here->li_next) {
            if (renumber)
                here->li_linenum = i;
            i++;
            if (ciprefix(".end", here->li_line) &&
                    !isalpha(here->li_line[4]))
                continue;
            if (*here->li_line != '*') {
                if (useout) {
                    sprintf(out_pbuf, "%6d : %s\n",
                            here->li_linenum,
                            upper(here->li_line));
            out_send(out_pbuf);
/*                    out_printf("%6d : %s\n",
                            here->li_linenum,
                            upper(here->li_line)); */
                } else
                    fprintf(file, "%6d : %s\n",
                            here->li_linenum,
                            upper(here->li_line));
                if (here->li_error) {
                    if (useout) {
                        out_printf("%s\n",
                                here->li_error);
                    } else
                        fprintf(file, "%s\n",
                            here->li_error, file);
                }
            }
        }
        if (extras) {
            deck = extras;
            extras = NULL;
            goto top1;
        }
        if (useout) {
/*            out_printf("%6d : .end\n", i); */
            sprintf(out_pbuf, "%6d : .end\n", i);
        out_send(out_pbuf);
        } else
            fprintf(file, "%6d : .end\n", i);
    } else if ((type == LS_PHYSICAL) || (type == LS_DECK)) {
top2:       for (here = deck; here; here = here->li_next) {
            if ((here->li_actual == NULL) || (here == deck)) {
                if (renumber)
                    here->li_linenum = i;
                i++;
                if (ciprefix(".end", here->li_line) &&
                        !isalpha(here->li_line[4]))
                    continue;
                if (type == LS_PHYSICAL) {
                    if (useout) {
                        sprintf(out_pbuf, "%6d : %s\n",
                            here->li_linenum,
                            upper(here->li_line));
            out_send(out_pbuf);
                    } else
                        fprintf(file, "%6d : %s\n",
                            here->li_linenum,
                            upper(here->li_line));
                } else {
                    if (useout)
                        out_printf("%s\n",
                            upper(here->li_line));
                    else
                        fprintf(file, "%s\n",
                            upper(here->li_line));
                }
                if (here->li_error && (type == LS_PHYSICAL)) {
                    if (useout)
                        out_printf("%s\n",
                            here->li_error);
                    else
                        fprintf(file, "%s\n",
                            here->li_error);
                }
            } else {
                for (there = here->li_actual; there;
                        there = there->li_next) {
                    there->li_linenum = i++;
                    if (ciprefix(".end", here->li_line) &&
                            isalpha(here->li_line[4]))
                        continue;
                    if (type == LS_PHYSICAL) {
                        if (useout) {
                            sprintf(out_pbuf, "%6d : %s\n",
                                there->li_linenum,
                                upper(there->li_line));
                out_send(out_pbuf);
                        } else
                            fprintf(file, "%6d : %s\n",
                                there->li_linenum,
                                upper(there->li_line));
                    } else {
                        if (useout)
                            out_printf("%s\n",
                                upper(there->li_line));
                        else
                            fprintf(file, "%s\n",
                                upper(there->li_line));
                    }
                    if (there->li_error && 
                            (type == LS_PHYSICAL)) {
                        if (useout)
                            out_printf("%s\n",
                            there->li_error);
                        else
                            fprintf(file, "%s\n",
                            there->li_error);
                    }
                }
                here->li_linenum = i;
            }
        }
        if (extras) {
            deck = extras;
            extras = NULL;
            goto top2;
        }
        if (type == LS_PHYSICAL) {
            if (useout) {
                sprintf(out_pbuf, "%6d : .end\n", i);
                out_send(out_pbuf);
            } else
                fprintf(file, "%6d : .end\n", i);
        } else {
            if (useout)
                out_printf(".end\n");
            else
                fprintf(file, ".end\n");
        }
    } else
        fprintf(cp_err, "inp_list: Internal Error: bad type %d\n", 
                type);
    return;
}

/* The routine to source a spice input deck. We read the deck in, take out
 * the front-end commands, and create a CKT structure. Also we filter out
 * the following cards: .save, .width, .four, .print, and .plot, to perform
 * after the run is over.
 */

void
inp_spsource(fp, comfile, filename)
    FILE *fp;
    bool comfile;
    char *filename;
{
    struct line *deck, *dd, *ld;
    struct line *realdeck, *options;
    char *tt, name[BSIZE_SP], *s, *t;
    bool nosubckts, commands = false;
    wordlist *wl = NULL, *end = NULL, *wl_first = NULL;
    wordlist *controls = NULL;
    FILE *lastin, *lastout, *lasterr;
    char c;

    inp_readall(fp, &deck);
    if (!deck)
        return;
    
    if (!comfile)
        options = inp_getopts(deck);

    realdeck = inp_deckcopy(deck);

    if (!comfile) {
        /* Save the title before INPgetTitle gets it. */
        tt = copy(deck->li_line);
        if (!deck->li_next)
            fprintf(cp_err, "Warning: no lines in input\n");
    }
    (void) fclose(fp);

    /* Now save the IO context and start a new control set.  After
     * we are done with the source we'll put the old file descriptors 
     * back.  I guess we could use a FILE stack, but since this routine
     * is recursive anyway.
     */
    lastin = cp_curin;
    lastout = cp_curout;
    lasterr = cp_curerr;
    cp_curin = cp_in;
    cp_curout = cp_out;
    cp_curerr = cp_err;

    cp_pushcontrol();

    /* We should now go through the deck and execute front-end 
     * commands and remove them. Front-end commands are enclosed by
     * the cards .control and .endc, unless comfile
     * is true, in which case every line must be a front-end command.
     * There are too many problems with matching the first word on
     * the line.
     */
    ld = deck;
    if (comfile) {
        /* This is easy. */
        for (dd = deck; dd; dd = ld) {
            ld = dd->li_next;
            if ((dd->li_line[0] == '*') && (dd->li_line[1] != '#'))
                continue;
            if (!ciprefix(".control", dd->li_line) &&
                    !ciprefix(".endc", dd->li_line))
                if (dd->li_line[0] == '*')
                    (void) cp_evloop(dd->li_line + 2);
                else
                    (void) cp_evloop(dd->li_line);
            tfree(dd->li_line);
            tfree(dd);
        }   
    } else {
        for (dd = deck->li_next; dd; dd = ld->li_next) {
	    for (s = dd->li_line; (c = *s) && c <= ' '; s++)
		;
            if (c == '*' && (s != deck->li_line || s[1] != '#')) {
                ld = dd;
                continue;
            }
            (void) strncpy(name, dd->li_line, BSIZE_SP);
            for (s = name; *s && isspace(*s); s++)
                ;
            for (t = s; *t && !isspace(*t); t++)
                ;
            *t = '\0';

            if (ciprefix(".control", dd->li_line)) {
                ld->li_next = dd->li_next;
                tfree(dd->li_line);
                tfree(dd);
                if (commands)
                    fprintf(cp_err, 
                    "Warning: redundant .control card\n");
                else
                    commands = true;
            } else if (ciprefix(".endc", dd->li_line)) {
                ld->li_next = dd->li_next;
                tfree(dd->li_line);
                tfree(dd);
                if (commands)
                    commands = false;
                else
                    fprintf(cp_err, 
                    "Warning: misplaced .endc card\n");
            } else if (commands || prefix("*#", dd->li_line)) {
                wl = alloc(struct wordlist);
                if (controls) {
                    wl->wl_next = controls;
                    controls->wl_prev = wl;
                    controls = wl;
                } else
                    controls = wl;
                if (prefix("*#", dd->li_line))
                    wl->wl_word = copy(dd->li_line + 2);
                else
                    wl->wl_word = dd->li_line;
                ld->li_next = dd->li_next;
                tfree(dd);
            } else if (!*dd->li_line) {
                /* So blank lines in com files don't get
                 * considered as circuits.
                 */
                ld->li_next = dd->li_next;
                tfree(dd->li_line);
                tfree(dd);
            } else {
                inp_casefix(s);
                inp_casefix(dd->li_line);
                if (eq(s, ".width") || ciprefix(".four", s) || eq(s, ".plot")
			|| eq(s, ".print") || eq(s, ".save")
                        || eq(s, ".op") || eq(s, ".tf"))
		{
                    if (end) {
                        end->wl_next = alloc(struct wordlist);
                        end->wl_next->wl_prev = end;
                        end = end->wl_next;
                    } else
                        wl_first = end = alloc(struct wordlist);
                    end->wl_word = copy(dd->li_line);

		    if (!eq(s, ".op") && !eq(s, ".tf")) {
			ld->li_next = dd->li_next;
			tfree(dd->li_line);
			tfree(dd);
		    } else
			ld = dd;
                } else
                    ld = dd;
            }
        }
        if (deck->li_next) {
            /* There is something left after the controls. */
            fprintf(cp_out, "\nCircuit: %s\n\n", tt);

            /* Now expand subcircuit macros. Note that we have to 
             * fix the case before we do this but after we 
             * deal with the commands.
             */
            if (!cp_getvar("nosubckt", VT_BOOL, (char *) 
                    &nosubckts))
                deck->li_next = inp_subcktexpand(deck->
                        li_next);
            deck->li_actual = realdeck;
            inp_dodeck(deck, tt, wl_first, false, options, filename);
        }

        /* Now that the deck is loaded, do the commands */
        if (controls) {
            for (end = wl = wl_reverse(controls); wl;
                    wl = wl->wl_next)
                (void) cp_evloop(wl->wl_word);
            /*wl_free(end);*/
        }
    }

    /* Now reset everything.  Pop the control stack, and fix up the IO
     * as it was before the source.
     */
    cp_popcontrol();

    cp_curin = lastin;
    cp_curout = lastout;
    cp_curerr = lasterr;
    return;
}

/* This routine is cut in half here because com_rset has to do what follows
 * also. End is the list of commands we execute when the job is finished:
 * we only bother with this if we might be running in batch mode, since
 * it isn't much use otherwise.
 */

void
inp_dodeck(deck, tt, end, reuse, options, filename)
    struct line *deck;
    char *tt;
    wordlist *end;
    bool reuse;
    struct line *options;
    char *filename;
{
    struct circ *ct;
    struct line *dd;
    char *ckt, *s;
    INPtables *tab;
    struct variable *eev = NULL;
    wordlist *wl;
    bool noparse, ii;

    /* First throw away any old error messages there might be and fix
     * the case of the lines.
     */
    for (dd = deck; dd; dd = dd->li_next) {
        if (dd->li_error) {
            tfree(dd->li_error);
            dd->li_error = NULL;
        }
    }
    if (reuse) {
        ct = ft_curckt;
    } else {
        if (ft_curckt) {
            ft_curckt->ci_devices = cp_kwswitch(CT_DEVNAMES, 
                    (char *) NULL);
            ft_curckt->ci_nodes = cp_kwswitch(CT_NODENAMES, 
                    (char *) NULL);
        }
        ft_curckt = ct = alloc(struct circ);
    }
    (void) cp_getvar("noparse", VT_BOOL, (char *) &noparse);
    if (!noparse)
        ckt = if_inpdeck(deck, &tab);
    else
        ckt = NULL;

    out_init();
    for (dd = deck; dd; dd = dd->li_next)
        if (dd->li_error) {
	    char *p, *q;
	    p = dd->li_error;
	    do {
		q = index(p, '\n');
		if (q)
		    *q = 0;

		if (p == dd->li_error)
		    out_printf("Error on line %d : %s\n\t%s\n",
			dd->li_linenum, dd->li_line, dd->li_error);
		else
		    out_printf("\t%s\n", p);

		if (q)
		    *q++ = '\n';
		p = q;
	    } while (p && *p);
	}

    /* Add this circuit to the circuit list. If reuse is true then 
     * use the ft_curckt structure.
     */

    if (!reuse) {
#ifdef notdef
	/* Unused time-waster. */
        for (dd = deck->li_next; dd; dd = dd->li_next)
            if_setndnames(dd->li_line);
#endif

        /* Be sure that ci_devices and ci_nodes are valid */
        ft_curckt->ci_devices = cp_kwswitch(CT_DEVNAMES, 
                (char *) NULL);
        (void) cp_kwswitch(CT_DEVNAMES, ft_curckt->ci_devices);
        ft_curckt->ci_nodes = cp_kwswitch(CT_NODENAMES, (char *) NULL);
        (void) cp_kwswitch(CT_NODENAMES, ft_curckt->ci_nodes);
        ft_newcirc(ct);
        /* ft_setccirc(); */ ft_curckt = ct;
    }
    ct->ci_name = tt;
    ct->ci_deck = deck;
    ct->ci_options = options;
    if (deck->li_actual)
        ct->ci_origdeck = deck->li_actual;
    else
        ct->ci_origdeck = ct->ci_deck;
    ct->ci_ckt = ckt;
    ct->ci_symtab = tab;
    ct->ci_inprogress = false;
    ct->ci_runonce = false;
    ct->ci_commands = end;
    if (filename)
        ct->ci_filename = copy(filename);
    else
        ct->ci_filename = NULL;

    if (!noparse) {
        for (; options; options = options->li_next) {
            for (s = options->li_line; *s && !isspace(*s); s++)
                ;
            ii = cp_interactive;
            cp_interactive = false;
            wl = cp_lexer(s);
            cp_interactive = ii;
            if (!wl || !wl->wl_word || !*wl->wl_word)
                continue;
            if (eev)
                eev->va_next = cp_setparse(wl);
            else
                ct->ci_vars = eev = cp_setparse(wl);
            while (eev->va_next)
                eev = eev->va_next;
        }
        for (eev = ct->ci_vars; eev; eev = eev->va_next) {
	    static int one = 1;
            switch (eev->va_type) {
                case VT_BOOL:
                if_option(ct->ci_ckt, eev->va_name, 
                    eev->va_type, &one);
                break;
                case VT_NUM:
                if_option(ct->ci_ckt, eev->va_name, 
                    eev->va_type, (char *) &eev->va_num);
                break;
                case VT_REAL:
                if_option(ct->ci_ckt, eev->va_name, 
                    eev->va_type, (char *) &eev->va_real);
                break;
                case VT_STRING:
                if_option(ct->ci_ckt, eev->va_name, 
                    eev->va_type, eev->va_string);
                break;
	    }
        }
    }

    cp_addkword(CT_CKTNAMES, tt);
    return;
}

/* Edit and re-load the current input deck.  Note that if these commands are
 * used on a non-unix machine, they will leave spice.tmp junk files lying
 * around.
 */

void
com_edit(wl)
    wordlist *wl;
{
    char *filename;
    FILE *fp;
    bool inter, permfile;
    char buf[BSIZE_SP];

    inter = cp_interactive;
    cp_interactive = false;
    if (wl) {
        if (!doedit(wl->wl_word)) {
            cp_interactive = inter;
            return;
        }
        if (!(fp = inp_pathopen(wl->wl_word, "r"))) {
            perror(wl->wl_word);
            cp_interactive = inter;
            return;
        }
        inp_spsource(fp, false, wl->wl_word);
    } else {
        /* If there is no circuit yet, then create one */
        if (ft_curckt && ft_curckt->ci_filename) {
            filename = ft_curckt->ci_filename;
            permfile = true;
        } else {
            filename = smktemp("sp");
            permfile = false;
        }
        if (ft_curckt && !ft_curckt->ci_filename) {
            if (!(fp = fopen(filename, "w"))) {
                perror(filename);
                cp_interactive = inter;
                return;
            }
            inp_list(fp, ft_curckt->ci_deck, ft_curckt->ci_options,
                LS_DECK);
            fprintf(cp_err,
        "Warning: editing a temporary file -- circuit not saved\n");
            (void) fclose(fp);
        } else if (!ft_curckt) {
            if (!(fp = fopen(filename, "w"))) {
                perror(filename);
                cp_interactive = inter;
                return;
            }
            fprintf(fp, "SPICE 3 test deck\n");
            (void) fclose(fp);
        }
        if (!doedit(filename)) {
            cp_interactive = inter;
            return;
        }

        if (!(fp = fopen(filename, "r"))) {
            perror(filename);
            cp_interactive = inter;
            return;
        }
        inp_spsource(fp, false, permfile ? filename : (char *) NULL);
        (void) fclose(fp);
#ifdef HAS_UNLINK
        if (ft_curckt && !ft_curckt->ci_filename)
            (void) unlink(filename);
#endif
    }

    cp_interactive = inter;

    /* note: default is to run circuit after successful edit */

    fprintf(cp_out, "run circuit? ");
    fflush(cp_out);
    (void) gets(buf);
    if (buf[0] != 'n') {
      fprintf(cp_out, "running circuit\n");
      com_run(NULL);
    }

    return;
}

static bool
doedit(filename)
    char *filename;
{
#ifdef HAS_SYSTEM
    char buf[BSIZE_SP], buf2[BSIZE_SP], *editor;

    if (cp_getvar("editor", VT_STRING, buf2)) {
        editor = buf2;
    } else {
        if (!(editor = getenv("EDITOR"))) {
            if (Def_Editor && *Def_Editor)
	        editor = Def_Editor;
	    else
	        editor = "/usr/ucb/vi";
	}
    }
    (void) sprintf(buf, "%s %s", editor, filename);
    return (system(buf) ? false : true);
#else
    fprintf(stderr, "Editor not available\n");
    return false;
#endif
}

void
com_source(wl)
    wordlist *wl;
{
    FILE *fp, *tp;
    char buf[BSIZE_SP];
    bool inter;
    char *tempfile = NULL;
    wordlist *owl = wl;
    int i;

    inter = cp_interactive;
    cp_interactive = false;
    if (wl->wl_next) {
        /* There are several files -- put them into a temp file  */
        tempfile = smktemp("sp");
        if (!(fp = inp_pathopen(tempfile, "w+"))) {
            perror(tempfile);
            cp_interactive = true;
            return;
        }
        while (wl) {
            if (!(tp = inp_pathopen(wl->wl_word, "r"))) {
                perror(wl->wl_word);
                (void) fclose(fp);
                cp_interactive = true;
#ifdef HAS_UNLINK
                (void) unlink(tempfile);
#endif
                return;
            }
            while ((i = fread(buf, 1, BSIZE_SP, tp)) > 0)
                (void) fwrite(buf, 1, i, fp);
            (void) fclose(tp);
            wl = wl->wl_next;
        }
        (void) fseek(fp, (long) 0, 0);
    } else
        fp = inp_pathopen(wl->wl_word, "r");
    if (fp == NULL) {
        perror(wl->wl_word);
        cp_interactive = true;
        return;
    }

    /* Don't print the title if this is a .spiceinit file. */
    if (ft_nutmeg || substring(".spiceinit", owl->wl_word)
            || substring("spice.rc", owl->wl_word))
        inp_spsource(fp, true, tempfile ? (char *) NULL : wl->wl_word);
    else
        inp_spsource(fp, false, tempfile ? (char *) NULL : wl->wl_word);
    cp_interactive = inter;
#ifdef HAS_UNLINK
    if (tempfile)
        (void) unlink(tempfile);
#endif
    return;
}

void
inp_source(file)
    char *file;
{
    static struct wordlist wl = { NULL, NULL, NULL } ;

    wl.wl_word = file;
    com_source(&wl);
    return;
}
