/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Spice-2 compatibility stuff for .plot, .print, .four, and .width.
 */

#include "spice.h"
#include "misc.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "ftedata.h"
#include "fteinp.h"
#include "suffix.h"

static wordlist *gettoks();
static void fixdotplot();
static void fixdotprint();
static char *fixem();
static bool setcplot();

/* Extract all the .save lines */

void
ft_dotsaves()
{
    wordlist *iline, *wl = NULL;
    char *s;

    if (!ft_curckt) /* Shouldn't happen. */
        return;

    for (iline = ft_curckt->ci_commands; iline; iline = iline->wl_next) {
        if (ciprefix(".save", iline->wl_word)) {
            s = iline->wl_word;
            (void) gettok(&s);
            wl = wl_append(wl, gettoks(s));
        }
    }

    com_save(wl);
    return;
}

/* Go through the dot lines given and make up a big "save" command with
 * all the node names mentioned. Note that if a node is requested for
 * one analysis, it is saved for all of them.
 */

static char *plot_opts[ ] = {
	"linear",
	"xlog",
	"ylog",
	"loglog"
	};

int
ft_savedotargs()
{
    wordlist *w, *wl = NULL, *iline, **prev_wl, *w_next;
    char *name;
    char *s;
    int some = 0;
    static wordlist all = { "all", NULL };
    int isaplot;
    int i;

    if (!ft_curckt) /* Shouldn't happen. */
        return 0;

    for (iline = ft_curckt->ci_commands; iline; iline = iline->wl_next) {
        s = iline->wl_word;
	if (ciprefix(".plot", s))
	    isaplot = 1;
	else
	    isaplot = 0;

        if (isaplot || ciprefix(".print", s)) {
            (void) gettok(&s);
            name = gettok(&s);

            if (!(w = gettoks(s))) {
                fprintf(cp_err, "Warning: no nodes given: %s\n",
                        iline->wl_word);
	    } else {
		if (isaplot) {
		    prev_wl = &w;
		    for (wl = w; wl; wl = w_next) {
			w_next = wl->wl_next;
			for (i = 0; i < NUMELEMS(plot_opts); i++) {
			    if (!strcmp(wl->wl_word, plot_opts[i])) {
				/* skip it */
				*prev_wl = w_next;
				tfree(wl);
				break;
			    }
			}
			if (i == NUMELEMS(plot_opts))
			    prev_wl = &wl->wl_next;
		    }
		}
		some = 1;
		com_save2(w, name);
	    }
        } else if (ciprefix(".four", s)) {
            (void) gettok(&s);
            (void) gettok(&s);
            if (!(w = gettoks(s)))
                fprintf(cp_err, "Warning: no nodes given: %s\n",
                        iline->wl_word);
	    else {
		some = 1;
		com_save2(w, "TRAN");	/* A hack */
	    }
        } else if (ciprefix(".op", s)) {
		some = 1;
		com_save2(&all, "OP");
        } else if (ciprefix(".tf", s)) {
		some = 1;
		com_save2(&all, "TF");
	}
    }

    return some;
}

/* Execute the .whatever lines found in the deck, after we are done running.
 * We'll be cheap and use cp_lexer to get the words... This should make us
 * spice-2 compatible.  If terse is true then there was a rawfile, so don't
 * print lots of junk.
 */

int
ft_cktcoms(terse)
    bool terse;
{
    wordlist *coms, *command, all;
    char *plottype, *s;
    struct dvec *v;
    static wordlist twl = { "col", NULL, NULL } ;
    struct plot *pl;
    int i, found;

    all.wl_next = NULL;
    all.wl_word = "all";

    if (!ft_curckt)
	return 1;
    if (!ft_curckt->ci_commands)
        goto nocmds;
    coms = ft_curckt->ci_commands;
    cp_interactive = false;

    /* Circuit name */
    fprintf(cp_out, "Circuit: %s\nDate: %s\n\n", ft_curckt->ci_name,
            datestring());
    fprintf(cp_out, "\n");

    /* Listing */
    if (ft_listprint)
	if (terse)
	    fprintf(cp_err, ".options: no listing, rawfile was generated.\n");
	else
	    inp_list(cp_out, ft_curckt->ci_deck, ft_curckt->ci_options,
                LS_DECK);

    /* If there was a .op line, then we have to do the .op output. */
    if (setcplot("op")) {
	if (terse) {
	    fprintf(cp_out, "OP information in rawfile.\n");
	} else {
	    fprintf(cp_out, "\nOperating point information:\n\n");
	    fprintf(cp_out, "\tNode\tVoltage\n");
	    fprintf(cp_out, "\t----\t-------\n");
	    for (v = plot_cur->pl_dvecs; v; v = v->v_next) {
		if (!isreal(v)) {
		    fprintf(cp_err, 
		"Internal error: op vector %s not real\n",
			    v->v_name);
		    continue;
		}
		if (v->v_type == SV_VOLTAGE)
		    fprintf(cp_out, "\t%s\t%s\n", v->v_name,
			printnum(v->v_realdata[0]));
	    }
	    fprintf(cp_out, "\n\tSource\tCurrent\n");
	    fprintf(cp_out, "\t------\t-------\n\n");
	    for (v = plot_cur->pl_dvecs; v; v = v->v_next)
		if (v->v_type == SV_CURRENT)
		    fprintf(cp_out, "\t%s\t%s\n", v->v_name,
			printnum(v->v_realdata[0]));
	    fprintf(cp_out, "\n");

	    if (!ft_nomod)
		com_showmod(&all);
	    com_show(&all);
	}
    }

    for (pl = plot_list; pl; pl = pl->pl_next) {
        if (ciprefix("tf", pl->pl_typename)) {
	    if (terse) {
		fprintf(cp_out, "TF information in rawfile.\n");
		break;
	    }
            plot_cur = pl;
	    fprintf(cp_out, "Transfer function information:\n");
	    com_print(&all);
	    fprintf(cp_out, "\n");
	}
    }

    /* Now all the '.' lines */
    while (coms) {
        command = cp_lexer(coms->wl_word);
        if (!command)
            goto bad;
        if (eq(command->wl_word, ".width")) {
            do {
                command = command->wl_next;
            } while (command && !ciprefix("out", command->wl_word));
            if (command) {
                s = index(command->wl_word, '=');
                if (!s || !s[1]) {
                    fprintf(cp_err, "Error: bad line %s\n", coms->wl_word);
                    coms = coms->wl_next;
                    continue;
                }
                i = atoi(++s);
                cp_vset("width", VT_NUM, (char *) &i);
            }
        } else if (eq(command->wl_word, ".print")) {
            if (terse) {
                fprintf(cp_out, 
            ".print line ignored since rawfile was produced.\n");
            } else {
                command = command->wl_next;
                if (!command) {
                    fprintf(cp_err, "Error: bad line %s\n", coms->wl_word);
                    coms = coms->wl_next;
                    continue;
                }
                plottype = command->wl_word;
                command = command->wl_next;
                fixdotprint(command);
                twl.wl_next = command;
		found = 0;
		for (pl = plot_list; pl; pl = pl->pl_next) {
		    if (ciprefix(plottype, pl->pl_typename)) {
			plot_cur = pl;
			com_print(&twl);
			fprintf(cp_out, "\n");
			found = 1;
		    }
		}
                if (!found)
                    fprintf(cp_err, "Error: .print: no %s analysis found.\n",
			plottype);
            }
        } else if (eq(command->wl_word, ".plot")) {
            if (terse) {
                fprintf(cp_out, 
		    ".plot line ignored since rawfile was produced.\n");
            } else {
                command = command->wl_next;
                if (!command) {
                    fprintf(cp_err, "Error: bad line %s\n",
                            coms->wl_word);
                    coms = coms->wl_next;
                    continue;
                }
                plottype = command->wl_word;
                command = command->wl_next;
                fixdotplot(command);
		found = 0;
		for (pl = plot_list; pl; pl = pl->pl_next) {
		    if (ciprefix(plottype, pl->pl_typename)) {
			plot_cur = pl;
			com_asciiplot(command);
			fprintf(cp_out, "\n");
			found = 1;
		    }
		}
                if (!found)
                    fprintf(cp_err, "Error: .plot: no %s analysis found.\n",
			plottype);
            }
        } else if (ciprefix(".four", command->wl_word)) {
            if (terse) {
                fprintf(cp_out, 
		    ".fourier line ignored since rawfile was produced.\n");
            } else if (setcplot("tran")) {
		com_fourier(command->wl_next);
		fprintf(cp_out, "\n\n");
	    } else
		fprintf(cp_err,
			"No transient data available for fourier analysis");
        } else if (!eq(command->wl_word, ".save")
		&& !eq(command->wl_word, ".op")
		&& !eq(command->wl_word, ".tf"))
	{
            goto bad;
	}
        coms = coms->wl_next;
    }

nocmds:
    /* Now the node table */
    if (ft_nodesprint)
        ;
    
    /* The options */
    if (ft_optsprint) {
        fprintf(cp_err, "Options:\n\n");
        cp_vprint();
        (void) putc('\n', cp_out);
    }

    /* And finally the accounting info. */
    if (ft_acctprint) {
        static wordlist ww = { "everything", NULL, NULL } ;
        com_rusage(&ww);
    } else
        com_rusage((wordlist *) NULL);

    (void) putc('\n', cp_out);
    return 0;

bad:    fprintf(cp_err, "Internal Error: ft_cktcoms: bad commands\n");
    return 1;
}

/* These routines make sure that the arguments to .plot and .print in
 * spice2 decks are acceptable to spice3. The things we look for are:
 *  trailing (a,b) in .plot -> xlimit a b
 *  vm(x) -> mag(v(x))
 *  vp(x) -> ph(v(x))
 *  v(x,0) -> v(x)
 *  v(0,x) -> -v(x)
 */

static void
fixdotplot(wl)
    wordlist *wl;
{
    char buf[BSIZE_SP], *s;
    double *d, d1, d2;

    while (wl) {
        wl->wl_word = fixem(wl->wl_word);

        /* Is this a trailing (a,b) ? Note that we require it to be
         * one word.
         */
        if (!wl->wl_next && (*wl->wl_word == '(')) /*)*/ {
            s = wl->wl_word + 1;
            d = ft_numparse(&s, false);
            if (*s != ',') {
                fprintf(cp_err, "Error: bad limits \"%s\"\n",
                        wl->wl_word);
                    return;
            }
            d1 = *d;
            s++;
            d = ft_numparse(&s, false);
            if ((*s != /*(*/ ')') || s[1]) {
                fprintf(cp_err, "Error: bad limits \"%s\"\n",
                        wl->wl_word);
                    return;
            }
            d2 = *d;
            tfree(wl->wl_word);
            wl->wl_word = copy("xlimit");
            wl->wl_next = alloc(struct wordlist);
            wl->wl_next->wl_prev = wl;
            wl = wl->wl_next;
            (void) strcpy(buf, printnum(d1));
            wl->wl_word = copy(buf);
            wl->wl_next = alloc(struct wordlist);
            wl->wl_next->wl_prev = wl;
            wl = wl->wl_next;
            (void) strcpy(buf, printnum(d2));
            wl->wl_word = copy(buf);
        }
        wl = wl->wl_next;
    }
    return;
}

static void
fixdotprint(wl)
    wordlist *wl;
{
    while (wl) {
        wl->wl_word = fixem(wl->wl_word);
        wl = wl->wl_next;
    }
    return;
}

static char *
fixem(string)
    char *string;
{
    char buf[BSIZE_SP], *s, *t, *ss = string;

    if (ciprefix("v(", string) && index(string, ',')) {
        for (s = string; *s && (*s != ','); s++)
            ;
        *s++ = '\0';
        for (t = s; *t && (*t != ')'); t++)
            ;
        *t = '\0';
        if (eq(s, "0"))
            (void) sprintf(buf, "v(%s)", string + 2);
        else if (eq(string + 2, "0"))
            (void) sprintf(buf, "-v(%s)", s);
        else
            (void) sprintf(buf, "v(%s)-v(%s)", string + 2, s);
        tfree(ss);
        string = copy(buf);
    } else if (ciprefix("vm(", string)) {
        for (s = string; *s && (*s != ')'); s++)
            ;
        *s = '\0';
        (void) sprintf(buf, "mag(v(%s))", string + 3);
        tfree(ss);
        string = copy(buf);
    } else if (ciprefix("vp(", string)) {
        for (s = string; *s && (*s != ')'); s++)
            ;
        *s = '\0';
        (void) sprintf(buf, "ph(v(%s))", string + 3);
        tfree(ss);
        string = copy(buf);
    } else if (ciprefix("vi(", string)) {
        for (s = string; *s && (*s != ')'); s++)
            ;
        *s = '\0';
        (void) sprintf(buf, "imag(v(%s))", string + 3);
        tfree(ss);
        string = copy(buf);
    } else if (ciprefix("vr(", string)) {
        for (s = string; *s && (*s != ')'); s++)
            ;
        *s = '\0';
        (void) sprintf(buf, "real(v(%s))", string + 3);
        tfree(ss);
        string = copy(buf);
    } else if (ciprefix("vdb(", string)) {
        for (s = string; *s && (*s != ')'); s++)
            ;
        *s = '\0';
        (void) sprintf(buf, "db(v(%s))", string + 4);
        tfree(ss);
        string = copy(buf);
    } else if (ciprefix("i(", string)) {
        for (s = string; *s && (*s != ')'); s++)
            ;
        *s = '\0';
        string += 2;
        (void) sprintf(buf, "%s#branch", string);
        tfree(ss);
        string = copy(buf);
    }
    return (string);
}

/* Don't bother with ccom strangeness here. */

static bool
setcplot(name)
    char *name;
{
    struct plot *pl;

    for (pl = plot_list; pl; pl = pl->pl_next) {
        if (ciprefix(name, pl->pl_typename)) {
            plot_cur = pl;
            return (true);
        }
    }
    return (false);
}

#ifdef notdef
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
            /* The painful case */
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

static wordlist *
gettoks(s)
	char *s;
{
    char	*t;
    char	*l, *r, *c;	/* left, right, center/comma */
    wordlist	*wl, *list, **prevp;


    list = NULL;
    prevp = &list;

    while (t = gettok(&s)) {
	if (*t == '(')
	    continue;
	l = rindex(t, '('/*)*/);
	if (!l) {
	    wl = alloc(struct wordlist);
	    wl->wl_word = copy(t);
	    *prevp = wl;
	    prevp = &wl->wl_next;
	    continue;
	}

	r = index(t, /*(*/')');

	c = index(t, ',');
	if (!c)
	    c = r;

	if (c)
	    *c = 0;

	wl = alloc(struct wordlist);

	if (*(l - 1) == 'i' || *(l - 1) == 'I') {
	    char buf[513];
	    sprintf(buf, "%s#branch", l + 1);
	    wl->wl_word = copy(buf);
	    c = r = NULL;
	} else
	    wl->wl_word = copy(l + 1);

	*prevp = wl;
	prevp = &wl->wl_next;

	if (c != r) {
	    *r = 0;
	    wl = alloc(struct wordlist);
	    wl->wl_word = copy(c + 1);
	    *prevp = wl;
	    prevp = &wl->wl_next;
	}
    }
    return list;
}
