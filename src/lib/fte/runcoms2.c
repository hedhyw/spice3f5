/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Circuit simulation commands.
 */

#include "spice.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "ftedev.h"
#include "ftedebug.h"
#include "ftedata.h"
#include "suffix.h"

extern FILE *rawfileFp;
extern bool rawfileBinary;
extern struct dbcomm *dbs;

/* Continue a simulation. If there is non in progress, this is the
 * equivalent of "run".
 */

/* ARGSUSED */

/* This is a hack to tell iplot routine to redraw the grid and initialize
	the display device
 */

bool resumption = false;

void
com_resume(wl)
    wordlist *wl;
{
    struct dbcomm *db;
    int err;

    if (ft_curckt->ci_inprogress == false) {
        fprintf(cp_err, "Note: run starting\n");
        com_run((wordlist *) NULL);
        return;
    }
    ft_curckt->ci_inprogress = true;
    ft_setflag = true;

    reset_trace( );
    for ( db = dbs, resumption = false; db; db = db->db_next )
	if( db->db_type == DB_IPLOT || db->db_type == DB_IPLOTALL ) {
		resumption = true;
	}
    err = if_run(ft_curckt->ci_ckt, "resume", (wordlist *) NULL,
            ft_curckt->ci_symtab);
    if (err == 1) {
        /* The circuit was interrupted somewhere. */
#ifdef HAS_MFB
	if(!strcmp(dispdev->name, "MFB" ))
		MFBHalt();
#endif
        fprintf(cp_err, "simulation interrupted\n");
    } else if (err == 2) {
#ifdef HAS_MFB
	if(!strcmp(dispdev->name, "MFB" ))
		MFBHalt();
#endif
        fprintf(cp_err, "simulation aborted\n");
        ft_curckt->ci_inprogress = false;
    } else
        ft_curckt->ci_inprogress = false;
    return;
}

/* Throw out the circuit struct and recreate it from the deck.  This command
 * should be obsolete.
 */

/* ARGSUSED */
void
com_rset(wl)
    wordlist *wl;
{
    struct variable *v, *next;

    if (ft_curckt == NULL) {
        fprintf(cp_err, "Error: there is no circuit loaded.\n");
        return;
    }

    if_cktfree(ft_curckt->ci_ckt, ft_curckt->ci_symtab);
    for (v = ft_curckt->ci_vars; v; v = next) {
	next = v->va_next;
	tfree(v);
    }
    ft_curckt->ci_vars = NULL;

    inp_dodeck(ft_curckt->ci_deck, ft_curckt->ci_name, (wordlist *) NULL,
            true, ft_curckt->ci_options, ft_curckt->ci_filename);
    return;
}
