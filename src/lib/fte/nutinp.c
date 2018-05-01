/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher
**********/

/*
 * For dealing with nutmeg input decks and command scripts
 */

#include "spice.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "ftedata.h"
#include "fteinp.h"
#include "suffix.h"

/* The routine to source a spice input deck. We read the deck in, take out
 * the front-end commands, and create a CKT structure. Also we filter out
 * the following lines: .save, .width, .four, .print, and .plot, to perform
 * after the run is over.
 */

void
inp_nutsource(fp, comfile, filename)
    FILE *fp;
    bool comfile;
    char *filename;
{
    struct line *deck, *dd, *ld;
    struct line *realdeck, *options;
    char *tt, name[BSIZE_SP], *s, *t;
    bool nosubckts, commands = false;
    wordlist *wl = NULL, *end = NULL;
    wordlist *controls = NULL;
    FILE *lastin, *lastout, *lasterr;
    
    inp_readall(fp, &deck);
    if (!deck)
        return;

    realdeck = inp_deckcopy(deck);

    if (!comfile) {
        /* Save the title before INPgetTitle gets it. */
        tt = copy(deck->li_line);
        if (!deck->li_next)
            fprintf(cp_err, "Warning: no lines in deck...\n");
    }
    (void) fclose(fp);

    /* Now save the IO context and start a new control set...  After
     * we are done with the source we'll put the old file descriptors 
     * back.  I guess we could use a FILE stack, but since this routine
     * is recursive anyway...
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
     * the lines .control and .endc, unless comfile
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
            if ((dd->li_line[0] == '*') &&
                    (dd->li_line[1] != '#')) {
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
                    "Warning: redundant .control line\n");
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
                    "Warning: misplaced .endc line\n");
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
                if (eq(s, ".width") || ciprefix(".four", s) ||
                        eq(s, ".plot") || 
                        eq(s, ".print") ||
                        eq(s, ".save")) {
                    if (end) {
                        end->wl_next = alloc(struct wordlist);
                        end->wl_next->wl_prev = end;
                        end = end->wl_next;
                    } else
                        wl = end = alloc(struct wordlist);
                    end->wl_word = copy(dd->li_line);
                    ld->li_next = dd->li_next;
                    tfree(dd->li_line);
                    tfree(dd);
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
            nutinp_dodeck(deck, tt, wl, false, options, filename);
        }

        /* Now that the deck is loaded, do the commands... */
        if (controls) {
            for (end = wl = wl_reverse(controls); wl;
                    wl = wl->wl_next)
                (void) cp_evloop(wl->wl_word);
            wl_free(end);
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

void
nutcom_source(wl)
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
        /* There are several files -- put them into a temp file...  */
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
        inp_nutsource(fp, true, tempfile ? (char *) NULL : wl->wl_word);
    else
        inp_nutsource(fp, false, tempfile ? (char *) NULL : wl->wl_word);
    cp_interactive = inter;
#ifdef HAS_UNLINK
    if (tempfile)
        (void) unlink(tempfile);
#endif
    return;
}

void
nutinp_source(file)
    char *file;
{
    static struct wordlist wl = { NULL, NULL, NULL } ;

    wl.wl_word = file;
    nutcom_source(&wl);
    return;
}


/* This routine is cut in half here because com_rset has to do what follows
 * also. End is the list of commands we execute when the job is finished:
 * we only bother with this if we might be running in batch mode, since
 * it isn't much use otherwise.
 */

void
nutinp_dodeck(deck, tt, end, reuse, options, filename)
    struct line *deck;
    char *tt;
    wordlist *end;
    bool reuse;
    struct line *options;
    char *filename;
{
    struct circ *ct;
    struct line *dd;
    char *ckt, *tab, *s;
    struct variable *eev = NULL;
    wordlist *wl;
    bool noparse, ii;

    /* This was "ifdef notdef"-ed out, so I tossed it */
}
