/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Circuit debugging commands.
 */

#include "spice.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "ftedebug.h"
#include "ftedata.h"
#include "fteinp.h"
#include "suffix.h"

/* ARGSUSED */
void
com_state(wl)
    wordlist *wl;
{
    if (!ft_curckt) {
        fprintf(cp_err, "Error: no circuit loaded.\n");
        return;
    }
    fprintf(cp_out, "Current circuit: %s\n", ft_curckt->ci_name);
    if (!ft_curckt->ci_inprogress) {
        fprintf(cp_out, "No run in progress.\n");
        return;
    }
    fprintf(cp_out, "Type of run: %s\n", plot_cur->pl_name);
    fprintf(cp_out, "Number of points so far: %d\n",
            plot_cur->pl_scale->v_length);
    fprintf(cp_out, "(That's all this command does so far)\n");
    return;
}

/* ARGSUSED */
void
com_dump(wl)
    wordlist *wl;
{
    if (!ft_curckt || !ft_curckt->ci_ckt) {
        fprintf(cp_err, "Error: no circuit loaded.\n");
        return;
    }
    if_dump(ft_curckt->ci_ckt, cp_out);
    return;
}

