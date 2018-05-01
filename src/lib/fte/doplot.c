/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Plotting routines
 */

#include "spice.h"
#include "fteinput.h"
#include "ftedbgra.h"
#include "cpdefs.h"
#include "fteconst.h"
#include "ftedefs.h"
#include "ftedev.h"
#include "ftedata.h"
#include "fteparse.h"
#include "fteext.h"
#include "suffix.h"

static char *getword();
static bool getflag();
static double *getlims();
static void compress();
static void xtend();
static bool plotit();
extern double *ft_SMITHminmax( );
extern void ft_xgraph();

static bool sameflag;
#ifdef HAS_MFB
char *MFBHardcopy = (char *) NULL;      /* hack for MFB hardcopy */
#endif /* HAS_MFB */


/* asciiplot file name ... [xlimit] xhi xlo] [vs xname] */

void
com_asciiplot(wl)   
    wordlist *wl;
{
    (void) plotit(wl, (char *) NULL, "lpr");
    return;
}

/* xgraph file plotargs
 */

void
com_xgraph(wl)    
    wordlist *wl;
{
    char *fname;
    char buf[BSIZE_SP];
    bool tempf = false;
    char *devtype;

#ifdef HAS_WAIT
    if (wl) {
        fname = wl->wl_word;
        wl = wl->wl_next;
    }
    if (!wl) {
	return;
    }
    if (cieq(fname, "temp") || cieq(fname, "tmp")) {
        fname = smktemp("xg");
        tempf = true;
    }

    (void) plotit(wl, fname, "xgraph");

#ifdef HAS_UNLINK
/* Leave temp file sitting around so xgraph can grab it from background.
    if (tempf)
        (void) unlink(fname);
*/
#endif
#else /* NOT HAS_WAIT */
    fprintf(cp_err, "Sorry: xgraph not available.\n");
#endif

    return;
}

/* hardcopy file plotargs, or 'hardcopy file' -- with no other args this
 * prompts the user for a window to dump to a plot file. XXX no it doesn't.
 */

void
com_hardcopy(wl)    
    wordlist *wl;
{
    char *buf2, *prompt( );
    wordlist *process( );
    char *fname;
    char buf[BSIZE_SP], device[BSIZE_SP];
    bool tempf = false;
    char *devtype;
    char format[513];
    int	printed;
    int hc_button;
    int foundit;

    if (!cp_getvar("hcopydev", VT_STRING, device))
        *device = '\0';

    if (wl) {
	hc_button = 0;
        fname = wl->wl_word;
        wl = wl->wl_next;
    } else {
	hc_button = 1;
        fname = smktemp("hc");
        tempf = true;
    }

    if (!cp_getvar("hcopydevtype", VT_STRING, buf)) {
        devtype = "plot5";
    } else {
        devtype = buf;
    }

/* enable screen plot selection for these display types */

    foundit = 0;

#ifdef HAS_X_
    if (!wl && hc_button) {

        REQUEST request;
        RESPONSE response;
        GRAPH *tempgraph;
        
        request.option = click_option;
        Input(&request, &response);

        if (response.option == error_option) return;

	if (response.reply.graph) {

	    if (DevSwitch(devtype)) return;
	    tempgraph = CopyGraph(response.reply.graph);
	    tempgraph->devdep = fname;
	    if (NewViewport(tempgraph)) {
	      DevSwitch(NULL);
	      return;
	    }
	    gr_resize(tempgraph);
	    gr_redraw(tempgraph);
	    DestroyGraph(tempgraph->graphid);
	    DevSwitch(NULL);
	    foundit = 1;
	}
    }

#endif


    if (!foundit) {

	if (!wl) {
	    outmenuprompt("which variable ? ");
	    if ((buf2 = prompt(cp_in)) == (char *) -1)	/* XXXX Sick */
		return;
	    wl = alloc(struct wordlist);
	    wl->wl_word = buf2;
	    wl->wl_next = NULL;
	    wl = process(wl);
	}

#ifdef HAS_MFB
	MFBHardcopy = fname;
#endif /* HAS_MFB */

	if (DevSwitch(devtype)) return;

	if (!wl || !plotit(wl, fname, (char *) NULL)) {
	    printf("com_hardcopy: graph not defined\n");
	    DevSwitch(NULL);    /* remember to switch back */
	    return;
	}

	DevSwitch(NULL);

    }

    printed = 0;

#ifdef HAS_SYSTEM
    if (*device) {
#ifdef SYSTEM_PLOT5LPR
      if (!strcmp(devtype, "plot5") || !strcmp(devtype, "MFB")) {
	if (!cp_getvar("lprplot5", VT_STRING, format))
		strcpy(format, SYSTEM_PLOT5LPR);
        (void) sprintf(buf, format, device, fname);
        fprintf(cp_out, "Printing %s on the %s printer.\n", fname, device);
        (void) system(buf);
	printed = 1;
      }
#endif
#ifdef SYSTEM_PSLPR
      if (!printed && !strcmp(devtype, "postscript")) {
        /* note: check if that was a postscript printer XXX */
	if (!cp_getvar("lprps", VT_STRING, format))
		strcpy(format, SYSTEM_PSLPR);
        (void) sprintf(buf, format, device, fname);
        fprintf(cp_out, "Printing %s on the %s printer.\n", fname, device);
        (void) system(buf);
	printed = 1;
      }
#endif
    }
#endif
    if (!printed) {
      if (!strcmp(devtype, "plot5")) {
        fprintf(cp_out,
	    "The file \"%s\" may be printed with the Unix \"plot\" command,\n",
                fname);
        fprintf(cp_out,
	    "\tor by using the '-g' flag to the Unix lpr command.\n");
      } else if (!strcmp(devtype, "postscript")) {
        fprintf(cp_out,
	    "The file \"%s\" may be printed on a postscript printer.\n",
	    fname);
      } else if (!strcmp(devtype, "MFB")) {
	fprintf(cp_out,
		"The file \"%s\" may be printed on a MFB device.\n",
		fname);
      }
    }

#ifdef HAS_UNLINK
    if (tempf && *device)
        (void) unlink(fname);
#endif

    return;
}

/* plot name ... [xl[imit]] xlo xhi] [yl[imit ylo yhi] [vs xname] */

void
com_plot(wl)
    wordlist *wl;
{
    (void) plotit(wl, (char *) NULL, (char *) NULL);
    return;
}

/* The common routine for all plotting commands. This does hardcopy
 * and graphics plotting.
 */

static wordlist *wl_root;

static bool
plotit(wl, hcopy, devname)
    wordlist *wl;
    char *hcopy, *devname;
{
    /* All these things are static so that "samep" will work. */
    static double *xcompress = NULL, *xindices = NULL;
    static double *xlim = NULL, *ylim = NULL;
    static double *xdelta = NULL, *ydelta = NULL;
    static char *xlabel = NULL, *ylabel = NULL, *title = NULL;
    static bool nointerp = false;
    static GRIDTYPE gtype = GRID_LIN;
    static PLOTTYPE ptype = PLOT_LIN;

    bool gfound = false, pfound = false, oneval = false;
    double *dd, ylims[2], xlims[2];
    struct pnode *n, *names;
    struct dvec *dv, *d = NULL, *vecs = NULL, *lv, *lastvs = NULL;
    char *xn;
    int i, j, xt;
    double tt, mx, my, rad;
    wordlist *wwl, *tw;
    char cline[BSIZE_SP], buf[BSIZE_SP], *pname;

    int newlen;
    struct dvec *v, *newv_scale;
    double *newdata, *newscale;
    double tstep, tstart, tstop, ttime;
    double dummy;

    if (!wl)
	return false;
    wl_root = wl;

    /* First get the command line, without the limits. */
    wwl = wl_copy(wl);
    (void) getlims(wwl, "xl", 2);
    (void) getlims(wwl, "xlimit", 2);
    (void) getlims(wwl, "yl", 2);
    (void) getlims(wwl, "ylimit", 2);
    (void) sprintf(cline, "plot %s", wl_flatten(wwl));

    wl_free(wwl);

    /* Now extract all the parameters. */

    /* In case the parameter is the first on the line, we need a
     * "buffer" word...
     */
    tw = alloc(struct wordlist);
    wl->wl_prev = tw;
    tw->wl_next = wl;
    wl = tw;
    tw->wl_word = "";

    sameflag = getflag(wl, "samep");

    if (!sameflag || !xlim) {
        xlim = getlims(wl, "xl", 2);
        if (!xlim)
            xlim = getlims(wl, "xlimit", 2);
    } else {
        (void) getlims(wl, "xl", 2);
        (void) getlims(wl, "xlimit", 2);
    }

    if (!sameflag || !ylim) {
        ylim = getlims(wl, "yl", 2);
        if (!ylim)
            ylim = getlims(wl, "ylimit", 2);
    } else {
        (void) getlims(wl, "yl", 2);
        (void) getlims(wl, "ylimit", 2);
    }

    if (!sameflag || !xcompress) {
        xcompress = getlims(wl, "xcompress", 1);
        if (!xcompress)
            xcompress = getlims(wl, "xcomp", 1);
    } else {
        (void) getlims(wl, "xcompress", 1);
        (void) getlims(wl, "xcomp", 1);
    }

    if (!sameflag || !xindices) {
        xindices = getlims(wl, "xindices", 2);
        if (!xindices)
            xindices = getlims(wl, "xind", 2);
    } else {
        (void) getlims(wl, "xindices", 2);
        (void) getlims(wl, "xind", 2);
    }

    if (!sameflag || !xdelta) {
        xdelta = getlims(wl, "xdelta", 1);
        if (!xdelta)
            xdelta = getlims(wl, "xdel", 1);
    } else {
        (void) getlims(wl, "xdelta", 1);
        (void) getlims(wl, "xdel", 1);
    }
    if (!sameflag || !ydelta) {
        ydelta = getlims(wl, "ydelta", 1);
        if (!ydelta)
            ydelta = getlims(wl, "ydel", 1);
    } else {
        (void) getlims(wl, "ydelta", 1);
        (void) getlims(wl, "ydel", 1);
    }
    
    /* Get the grid type and the point type.  Note we can't do if-else
     * here because we want to catch all the grid types.
     */
    if (getflag(wl, "lingrid")) {
        if (gfound)
            fprintf(cp_err, 
                "Warning: too many grid types given\n");
        else {
            gtype = GRID_LIN;
            gfound = true;
        }
    }
    if (getflag(wl, "loglog")) {
        if (gfound)
            fprintf(cp_err, 
                "Warning: too many grid types given\n");
        else {
            gtype = GRID_LOGLOG;
            gfound = true;
        }
    }
    if (getflag(wl, "nogrid")) {
        if (gfound)
            fprintf(cp_err, 
                "Warning: too many grid types given\n");
        else {
            gtype = GRID_NONE;
            gfound = true;
        }
    }
    if (getflag(wl, "linear")) {
        if (gfound)
            fprintf(cp_err, 
                "Warning: too many grid types given\n");
        else {
            gtype = GRID_LIN;
            gfound = true;
        }
    }
    if (getflag(wl, "xlog")) {
        if (gfound)
            fprintf(cp_err, 
                "Warning: too many grid types given\n");
        else {
            gtype = GRID_XLOG;
            gfound = true;
        }
    }
    if (getflag(wl, "ylog")) {
        if (gfound)
            fprintf(cp_err, 
                "Warning: too many grid types given\n");
        else {
            gtype = GRID_YLOG;
            gfound = true;
        }
    }
    if (getflag(wl, "polar")) {
        if (gfound)
            fprintf(cp_err, 
                "Warning: too many grid types given\n");
        else {
            gtype = GRID_POLAR;
            gfound = true;
        }
    }
    if (getflag(wl, "smith")) {
        if (gfound)
            fprintf(cp_err, 
                "Warning: too many grid types given\n");
        else {
            gtype = GRID_SMITH;
            gfound = true;
        }
    }
    if (getflag(wl, "smithgrid")) {
        if (gfound)
            fprintf(cp_err, 
                "Warning: too many grid types given\n");
        else {
            gtype = GRID_SMITHGRID;
            gfound = true;
        }
    }

    if (!sameflag && !gfound) {
        if (cp_getvar("gridstyle", VT_STRING, buf)) {
            if (eq(buf, "lingrid"))
                gtype = GRID_LIN;
            else if (eq(buf, "loglog"))
                gtype = GRID_LOGLOG;
            else if (eq(buf, "xlog"))
                gtype = GRID_XLOG;
            else if (eq(buf, "ylog"))
                gtype = GRID_YLOG;
            else if (eq(buf, "smith"))
                gtype = GRID_SMITH;
            else if (eq(buf, "smithgrid"))
                gtype = GRID_SMITHGRID;
            else if (eq(buf, "polar"))
                gtype = GRID_POLAR;
            else if (eq(buf, "nogrid"))
                gtype = GRID_NONE;
            else {
                fprintf(cp_err,
                    "Warning: strange grid type %s\n",
                    buf);
                gtype = GRID_LIN;
            }
            gfound = true;
        } else
            gtype = GRID_LIN;
    }

    /* Now get the point type.  */

    if (getflag(wl, "linplot")) {
        if (pfound)
            fprintf(cp_err, 
                "Warning: too many plot types given\n");
        else {
            ptype = PLOT_LIN;
            pfound = true;
        }
    }
    if (getflag(wl, "combplot")) {
        if (pfound)
            fprintf(cp_err, 
                "Warning: too many plot types given\n");
        else {
            ptype = PLOT_COMB;
            pfound = true;
        }
    }
    if (getflag(wl, "pointplot")) {
        if (pfound)
            fprintf(cp_err, 
                "Warning: too many plot types given\n");
        else {
            ptype = PLOT_POINT;
            pfound = true;
        }
    }

    if (!sameflag && !pfound) {
        if (cp_getvar("plotstyle", VT_STRING, buf)) {
            if (eq(buf, "linplot"))
                ptype = PLOT_LIN;
            else if (eq(buf, "combplot"))
                ptype = PLOT_COMB;
            else if (eq(buf, "pointplot"))
                ptype = PLOT_POINT;
            else {
                fprintf(cp_err,
                    "Warning: strange plot type %s\n",
                    buf);
                ptype = PLOT_LIN;
            }
            pfound = true;
        } else
            ptype = PLOT_LIN;
    }

    if (!sameflag || !xlabel)
        xlabel = getword(wl, "xlabel");
    else
        (void) getword(wl, "xlabel");
    if (!sameflag || !ylabel)
        ylabel = getword(wl, "ylabel");
    else
        (void) getword(wl, "ylabel");
    if (!sameflag || !title)
        title = getword(wl, "title");
    else
        (void) getword(wl, "title");

    if (!sameflag)
        nointerp = getflag(wl, "nointerp");
    else if (getflag(wl, "nointerp"))
        nointerp = true;

    wl = wl->wl_next;
    if (!wl) {
        fprintf(cp_err, "Error: no vectors given\n");
        return (false);
    }

    wl->wl_prev = NULL;

    /* Now parse the vectors.  We have a list of the form
     * "a b vs c d e vs f g h".  Since it's a bit of a hassle for
     * us to parse the vector boundaries here, we do this -- call
     * ft_getpnames() without the check flag, and then look for 0-length
     * vectors with the name "vs"...  This is a sort of a gross hack,
     * since we have to check for 0-length vectors ourselves after
     * evaulating the pnodes...
     */

    names = ft_getpnames(wl, false);
    if (names == NULL)
        return (false);

    /* Now evaluate the names. */
    for (n = names, lv = NULL; n; n = n->pn_next) {
        if (n->pn_value && (n->pn_value->v_length == 0) &&
                eq(n->pn_value->v_name, "vs")) {
            if (!lv) {
                fprintf(cp_err, "Error: misplaced vs arg\n");
                return (false);
            } else {
                if (!(n = n->pn_next)) {
                    fprintf(cp_err,
                        "Error: missing vs arg\n");
                    return (false);
                }
                dv = ft_evaluate(n);
                if (!dv)
                    return (false);
                if (lastvs)
                    lv = lastvs->v_link2;
                else
                    lv = vecs;
                while (lv) {
                    lv->v_scale = dv;
                    lastvs = lv;
                    lv = lv->v_link2;
                }
            }
            continue;
        }
        dv = ft_evaluate(n);
        if (!dv)
            return (false);
        if (!d)
            vecs = dv;
        else
            d->v_link2 = dv;
        for (d = dv; d->v_link2; d = d->v_link2)
            ;
        lv = dv;
    }
    free_pnode(names);
    d->v_link2 = NULL;

    /* Now check for 0-length vectors. */
    for (d = vecs; d; d = d->v_link2)
        if (!d->v_length) {
            fprintf(cp_err, "Error: %s: no such vector\n",
                    d->v_name);
            return (false);
        }
    
    /* If there are higher dimensional vectors, transform them into a
     * family of vectors.
     */
    for (d = vecs, lv = NULL; d; d = d->v_link2) {
        if (d->v_numdims > 1) {
	    if (lv)
		lv->v_link2 = vec_mkfamily(d);
	    else
		vecs = lv = vec_mkfamily(d);
            while (lv->v_link2)
                lv = lv->v_link2;
            lv->v_link2 = d->v_link2;
            d = lv;
        } else {
            lv = d;
	}
    }

    /* Now fill in the scales for vectors who aren't already fixed up. */
    for (d = vecs; d; d = d->v_link2)
        if (!d->v_scale) {
            if (d->v_plot->pl_scale)
                d->v_scale = d->v_plot->pl_scale;
            else
                d->v_scale = d;
        }


    /* See if the log flag is set anywhere... */
    if (!gfound) {
        for (d = vecs; d; d = d->v_link2)
            if (d->v_scale && (d->v_scale->v_gridtype == GRID_XLOG))
                gtype = GRID_XLOG;
        for (d = vecs; d; d = d->v_link2)
            if (d->v_gridtype == GRID_YLOG) {
                if ((gtype == GRID_XLOG) ||
                        (gtype == GRID_LOGLOG))
                    gtype = GRID_LOGLOG;
                else
                    gtype = GRID_YLOG;
            }
        for (d = vecs; d; d = d->v_link2)
            if (d->v_gridtype == GRID_SMITH || d->v_gridtype == GRID_SMITHGRID
		|| d->v_gridtype == GRID_POLAR)
	    {
                gtype = d->v_gridtype;
                break;
            }
    }

    /* See if there are any default plot types...  Here, like above, we
     * don't do entirely the best thing when there is a mixed set of
     * default plot types...
     */
    if (!sameflag && !pfound) {
        ptype = PLOT_LIN;
        for (d = vecs; d; d = d->v_link2)
            if (d->v_plottype != PLOT_LIN) {
                ptype = d->v_plottype;
                break;
            }
    }

    /* Check and see if this is pole zero stuff. */
    if ((vecs->v_type == SV_POLE) || (vecs->v_type == SV_ZERO))
        oneval = true;
    
    for (d = vecs; d; d = d->v_link2)
        if (((d->v_type == SV_POLE) || (d->v_type == SV_ZERO)) !=
                oneval ? 1 : 0) {
            fprintf(cp_err,
"Error: plot must be either all pole-zero or contain no poles or zeros\n");
            return (false);
        }
    
    if ((gtype == GRID_POLAR) || (gtype == GRID_SMITH
	|| gtype == GRID_SMITHGRID))
    {
        oneval = true;
    }

    /* If we are plotting scalars, make sure there is enough
     * data to fit on the screen.
     */
    
    for (d = vecs; d; d = d->v_link2)
        if (d->v_length == 1)
            xtend(d, d->v_scale->v_length);

    /* Now patch up each vector with the compression and the index
     * selection.
     */
    if (xcompress || xindices) {
        for (d = vecs; d; d = d->v_link2) {
            compress(d, xcompress, xindices);
            d->v_scale = vec_copy(d->v_scale);
            compress(d->v_scale, xcompress, xindices);
        }
    }
    
    /* Transform for smith plots */
    if (gtype == GRID_SMITH) {
	double	re, im, rex, imx;
	double	r, i, x;
	struct dvec **prevvp, *n;
	int	j;

	prevvp = &vecs;
	for (d = vecs; d; d = d->v_link2) {
	    if (d->v_flags & VF_PERMANENT) {
		n = vec_copy(d);
		n->v_flags &= ~VF_PERMANENT;
		n->v_link2 = d->v_link2;
		d = n;
		*prevvp = d;
	    }
	    prevvp = &d->v_link2;

	    if (isreal(d)) {
		fprintf(cp_err,
		    "Warning: plotting real data \"%s\" on a smith grid\n",
		    d->v_name);

		for (j = 0; j < d->v_length; j++) {
		    r = d->v_realdata[j];
		    d->v_realdata[j] = (r - 1) / (r + 1);
		}
	    } else {
		for (j = 0; j < d->v_length; j++) {
		    /* (re - 1, im) / (re + 1, im) */

		    re = realpart(d->v_compdata + j);
		    im = imagpart(d->v_compdata + j);

		    rex = re + 1;
		    imx = im;
		    re = re - 1;

		    /* (re, im) / (rex, imx) */
		    x = 1 - (imx / rex) * (imx / rex);
		    r = re / rex + im / rex * imx / rex;
		    i = im / rex - re / rex * imx / rex;

		    realpart(d->v_compdata + j) = r / x;
		    imagpart(d->v_compdata + j) = i / x;
		}
	    }
	}
    }

    /* Figure out the proper x- and y-axis limits. */
    if (ylim) {
#ifdef notdef
	if (gtype == GRID_SMITH) {
	    if (xlim) {
		SMITH_tfm(xlim[0], ylim[0], &dummy, &ylims[0]);
		SMITH_tfm(xlim[1], ylim[1], &dummy, &ylims[1]);
	    } else {
		SMITH_tfm(0.0, ylim[0], &dummy, &ylims[0]);
		SMITH_tfm(0.0, ylim[1], &dummy, &ylims[1]);
	    }
	} else {
	}
#endif
	    ylims[0] = ylim[0];
	    ylims[1] = ylim[1];
    } else if (oneval) {
        ylims[0] = HUGE;
        ylims[1] = - ylims[0];
        for (d = vecs; d; d = d->v_link2) {
#ifdef notdef
	    if (gtype == GRID_SMITH) {
		dd = ft_SMITHminmax(d, true);
		if( dd[0] < 0.0 )
		    dd[0] *= 1.1;
		else
		    dd[0] *= 0.9;
		if( dd[1] >= 0.0 )
		    dd[1] *= 1.1;
		else
		    dd[1] *= 0.9;
	    } else
#endif
		dd = ft_minmax(d, true);
            if (dd[0] < ylims[0])
                ylims[0] = dd[0];
            if (dd[1] > ylims[1])
                ylims[1] = dd[1];
        }
    } else {
        ylims[0] = HUGE;
        ylims[1] = - ylims[0];
        for (d = vecs; d; d = d->v_link2) {
            dd = ft_minmax(d, true);
            if (dd[0] < ylims[0])
                ylims[0] = dd[0];
            if (dd[1] > ylims[1])
                ylims[1] = dd[1];
        }

	/* XXX */
	for (d = vecs; d; d = d->v_link2) {
	    if (d->v_flags & VF_MINGIVEN)
		if (ylims[0] < d->v_minsignal)
		    ylims[0] = d->v_minsignal;
	    if (d->v_flags & VF_MAXGIVEN)
		if (ylims[1] > d->v_maxsignal)
		    ylims[1] = d->v_maxsignal;
	}
    }

    if (xlim) {
#ifdef notdef
	if (gtype == GRID_SMITH) {
	    if (ylim) {
		SMITH_tfm(xlim[0], ylim[0], &xlims[0], &dummy);
		SMITH_tfm(xlim[1], ylim[1], &xlims[1], &dummy);
	    } else {
		SMITH_tfm(xlim[0], 0.0, &xlims[0], &dummy);
		SMITH_tfm(xlim[1], 0.0, &xlims[1], &dummy);
	    }
	} else {
	}
#endif
	    xlims[0] = xlim[0];
	    xlims[1] = xlim[1];
    } else if (oneval) {
        xlims[0] = HUGE;
        xlims[1] = - xlims[0];
        for (d = vecs; d; d = d->v_link2) {
#ifdef notdef
	    if (gtype == GRID_SMITH) {
		dd = ft_SMITHminmax(d, false);
		if( dd[0] < 0.0 )
		    dd[0] *= 1.1;
		else
		    dd[0] *= 0.9;
		if( dd[1] >= 0.0 )
		    dd[1] *= 1.1;
		else
		    dd[1] *= 0.9;
	    } else
#endif
		dd = ft_minmax(d, false);

            if (dd[0] < xlims[0])
                xlims[0] = dd[0];
            if (dd[1] > xlims[1])
                xlims[1] = dd[1];
        }
    } else {
        xlims[0] = HUGE;
        xlims[1] = - xlims[0];
        for (d = vecs; d; d = d->v_link2) {
            dd = ft_minmax(d->v_scale, true);
            if (dd[0] < xlims[0])
                xlims[0] = dd[0];
            if (dd[1] > xlims[1])
                xlims[1] = dd[1];
        }
        for (d = vecs; d; d = d->v_link2) {
            if (d->v_scale->v_flags & VF_MINGIVEN)
                if (xlims[0] < d->v_scale->v_minsignal)
                    xlims[0] = d->v_scale->v_minsignal;
            if (d->v_scale->v_flags & VF_MAXGIVEN)
                if (xlims[1] > d->v_scale->v_maxsignal)
                    xlims[1] = d->v_scale->v_maxsignal;
        }
    }

    /* Do some coercion of the limits to make them reasonable. */
    if ((xlims[0] == 0) && (xlims[1] == 0)) {
        xlims[0] = -1.0;
        xlims[1] = 1.0;
    }
    if ((ylims[0] == 0) && (ylims[1] == 0)) {
        ylims[0] = -1.0;
        ylims[1] = 1.0;
    }
    if (xlims[0] > xlims[1]) {
        tt = xlims[1];
        xlims[1] = xlims[0];
        xlims[0] = tt;
    }
    if (ylims[0] > ylims[1]) {
        tt = ylims[1];
        ylims[1] = ylims[0];
        ylims[0] = tt;
    }
    if (xlims[0] == xlims[1]) {
        xlims[0] *= (xlims[0] > 0) ? 0.9 : 1.1;
        xlims[1] *= (xlims[1] > 0) ? 1.1 : 0.9;
    }
    if (ylims[0] == ylims[1]) {
	    /* || fabs(ylims[0])/(ylims[1]-ylims[0]) > 1.0e9
	    || fabs(ylims[1])/(ylims[1]-ylims[0]) > 1.0e9) */
        ylims[0] *= (ylims[0] > 0) ? 0.9 : 1.1;
        ylims[1] *= (ylims[1] > 0) ? 1.1 : 0.9;
    }

#ifdef notdef
    /* Now shrink the limits very slightly -- this helps prevent round-off
     * error from doing bad things.
     */
    if (gtype != GRID_LOGLOG && gtype != GRID_XLOG
	    && gtype != GRID_POLAR && gtype != GRID_SMITH)
    {
        tt = xlims[1] - xlims[0];
        xlims[0] += tt * 0.001;
        xlims[1] -= tt * 0.001;
    }
    if (gtype != GRID_LOGLOG && gtype != GRID_YLOG
	    && gtype != GRID_POLAR && gtype != GRID_SMITH) {
        tt = ylims[1] - ylims[0];
        ylims[0] += tt * 0.001;
        ylims[1] -= tt * 0.001;
    }
#endif

    if ((xlims[0] <= 0.0) && ((gtype == GRID_XLOG) ||
            (gtype == GRID_LOGLOG))) {
        fprintf(cp_err, 
            "Error: X values must be > 0 for log scale\n");
        return (false);
    }
    if ((ylims[0] <= 0.0) && ((gtype == GRID_YLOG) ||
            (gtype == GRID_LOGLOG))) {
        fprintf(cp_err, 
            "Error: Y values must be > 0 for log scale\n");
        return (false);
    }

    /* Fix the plot limits for smith and polar grids. */
    if ((!xlim || !ylim) && (gtype == GRID_POLAR)) {
        /* (0,0) must be in the center of the screen. */
        mx = (fabs(xlims[0]) > fabs(xlims[1])) ? fabs(xlims[0]) :
                fabs(xlims[1]);
        my = (fabs(ylims[0]) > fabs(ylims[1])) ? fabs(ylims[0]) :
                fabs(ylims[1]);
        rad = (mx > my) ? mx : my;
        /* rad = sqrt(mx * mx + my * my); */
        xlims[0] = - rad;
        xlims[1] = rad;
        ylims[0] = - rad;
        ylims[1] = rad;
    } else if ((!xlim || !ylim) && (gtype == GRID_SMITH
        || gtype == GRID_SMITHGRID))
    {
#ifdef notdef
	/* Let the user zoom in */
        mx = (fabs(xlims[0]) > fabs(xlims[1])) ? fabs(xlims[0]) :
                fabs(xlims[1]);
        my = (fabs(ylims[0]) > fabs(ylims[1])) ? fabs(ylims[0]) :
                fabs(ylims[1]);
        rad = (mx > my) ? mx : my;
	/* XXX */
        xlims[0] = - rad;
        xlims[1] = rad;
        ylims[0] = - rad;
        ylims[1] = rad;
#endif
        xlims[0] = -1.0;
        xlims[1] = 1.0;
        ylims[0] = -1.0;
        ylims[1] = 1.0;
    }

    /* We don't want to try to deal with smith plots for asciiplot. */
    if (devname && eq(devname, "lpr")) {
        /* check if we should (can) linearize */
        if (!(!ft_curckt || !ft_curckt->ci_ckt ||
            strcmp(ft_curckt->ci_name, plot_cur->pl_title) ||
            !if_tranparams(ft_curckt, &tstart, &tstop, &tstep) ||
            ((tstop - tstart) * tstep <= 0.0) ||
            ((tstop - tstart) < tstep) ||
            !plot_cur || !plot_cur->pl_dvecs ||
            !plot_cur->pl_scale ||
            !isreal(plot_cur->pl_scale) ||
            !ciprefix("tran", plot_cur->pl_typename))) {

          newlen = (tstop - tstart) / tstep + 1.5;

          newscale = (double *) tmalloc(newlen * sizeof(double));

          newv_scale = alloc(struct dvec);
          newv_scale->v_flags = vecs->v_scale->v_flags;
          newv_scale->v_type = vecs->v_scale->v_type;
          newv_scale->v_gridtype = vecs->v_scale->v_gridtype;
          newv_scale->v_length = newlen;
          newv_scale->v_name = copy(vecs->v_scale->v_name);
          newv_scale->v_realdata = newscale;

          for (i = 0, ttime = tstart; i < newlen; i++, ttime += tstep)
            newscale[i] = ttime;

          for (v = vecs; v; v= v->v_link2) {
            newdata = (double *) tmalloc(newlen * sizeof (double));

            if (!ft_interpolate(v->v_realdata, newdata,
                v->v_scale->v_realdata, v->v_scale->v_length,
            newscale, newlen, 1)) {
            fprintf(cp_err,
                "Error: can't interpolate %s\n", v->v_name);
            return(false);
            }

            tfree(v->v_realdata);
            v->v_realdata = newdata;

            /* Why go to all this trouble if agraf ignores it? */
            nointerp = true;
          }

          vecs->v_scale = newv_scale;

        }
        ft_agraf(xlims, ylims, vecs->v_scale, vecs->v_plot, vecs, 
            xdelta ? *xdelta : 0.0, ydelta ? *ydelta : 0.0,
            ((gtype == GRID_XLOG) || (gtype == GRID_LOGLOG)),
            ((gtype == GRID_YLOG) || (gtype == GRID_LOGLOG)),
            nointerp);
        return (true);
    }

    /* See if there is one type we can give for the y scale... */
    for (j = vecs->v_type, d = vecs->v_link2; d; d = d->v_link2)
        if (d->v_type != j) {
            j = SV_NOTYPE;
            break;
        }

    if (devname && eq(devname, "xgraph")) {
	/* Interface to XGraph-11 Plot Program */
	ft_xgraph(xlims, ylims, hcopy,
	    title ? title : vecs->v_plot->pl_title,
	    xlabel ? xlabel : ft_typabbrev(vecs->v_scale->v_type),
	    ylabel ? ylabel : ft_typabbrev(j),
	    gtype, ptype, vecs);
	return (true);
    }
    for (d = vecs, i = 0; d; d = d->v_link2)
        i++;

    /* Figure out the X name and the X type.  This is sort of bad... */
    xn = vecs->v_scale->v_name;
    xt = vecs->v_scale->v_type;

    pname = plot_cur->pl_typename;

    if (!gr_init(xlims, ylims, (oneval ? (char *) NULL : xn),
            title ? title : vecs->v_plot->pl_title, hcopy, i,
            xdelta ? *xdelta : 0.0, ydelta ? *ydelta : 0.0, gtype,
            ptype, xlabel, ylabel, xt, j, pname, cline))
        return (false);

    /* Now plot all the graphs. */
    for (d = vecs; d; d = d->v_link2)
        ft_graf(d, oneval ? (struct dvec *) NULL : d->v_scale, false);

    gr_clean();
#ifdef HAS_MFB
    if (!strcmp(dispdev->name, "MFB"))
	MFBHalt( );
#endif
    return (true);
}

/* This routine gets parameters from the command line, which are of the
 * form "name number ..." It returns a pointer to the parameter values.
 */

static double *
getlims(wl, name, number)
    wordlist *wl;
    char *name;
    int number;
{
    double *d, *td;
    wordlist *beg, *wk;
    char *ss;
    int n;

    for (beg = wl; beg; beg = beg->wl_next) {
        if (eq(beg->wl_word, name)) {
            if (beg == wl) {
                fprintf(cp_err,
			"Syntax error: looking for plot parameters \"%s\".\n",
			name);
                return (NULL);
            }
            wk = beg;
            if (number) {
                d = (double *) tmalloc(sizeof (double) *
                        number);
                for (n = 0; n < number; n++) {
                    wk = wk->wl_next;
                    if (!wk) {
                        fprintf(cp_err,
                            "Syntax error: not enough parameters for \"%s\".\n",
			    name);
                        return (NULL);
                    }
                    ss = wk->wl_word;
                    td = ft_numparse(&ss, false);
                    if (td == NULL)
                        goto bad;
                    d[n] = *td;
                }
            } else
                /* Minor hack... */
                d = (double  *) 1;

            if (beg->wl_prev)
                beg->wl_prev->wl_next = wk->wl_next;
            if (wk->wl_next) {
                wk->wl_next->wl_prev = beg->wl_prev;
                wk->wl_next = NULL;
            }
	    if (beg != wl_root)
		wl_free(beg);
            return (d);
        }
    }
    return (NULL);
bad:
    fprintf(cp_err, "Syntax error: bad parameters for \"%s\".\n", name);
    return (NULL);
}

/* Return a parameter of the form "xlabel foo" */

static char *
getword(wl, name)
    char *name;
    wordlist *wl;
{
    wordlist *beg;
    char *s;

    for (beg = wl; beg; beg = beg->wl_next) {
        if (eq(beg->wl_word, name)) {
            if ((beg == wl) || !beg->wl_next) {
                fprintf(cp_err,
			"Syntax error: looking for plot keyword at \"%s\".\n",
			name);
                return (NULL);
            }
            s = copy(beg->wl_next->wl_word);
            beg->wl_prev->wl_next = beg->wl_next->wl_next;
            if (beg->wl_next->wl_next)
                beg->wl_next->wl_next->wl_prev = beg->wl_prev;
            beg->wl_next->wl_next = NULL;
            wl_free(beg);
            return (s);
        }
    }
    return (NULL);
}

/* Check for and remove a one-word keyword. */

static bool
getflag(wl, name)
    wordlist *wl;
    char *name;
{
    while (wl) {
        if (eq(wl->wl_word, name)) {
            if (wl->wl_prev)
                wl->wl_prev->wl_next = wl->wl_next;
            if (wl->wl_next)
                wl->wl_next->wl_prev = wl->wl_prev;
            return (true);
        }
        wl = wl->wl_next;
    }
    return (false);
}

/* Extend a data vector to length by replicating the
 * last element, or truncate it if it is too long.
 */

static void
xtend(v, length)
    struct dvec *v;
    int length;
{
    int i;
    complex c, *oc;
    double d, *od;

    if (v->v_length == length)
        return;
    if (v->v_length > length) {
        v->v_length = length;
        return;
    }
    if (isreal(v)) {
        od = v->v_realdata;
        v->v_realdata = (double *) tmalloc(length * sizeof (double));
        for (i = 0; i < v->v_length; i++)
            v->v_realdata[i] = od[i];
        d = od[--i];
        while (i < length)
            v->v_realdata[i++] = d;
        tfree(od);
    } else {
        oc = v->v_compdata;
        v->v_compdata = (complex *) tmalloc(length * sizeof (complex));
        for (i = 0; i < v->v_length; i++) {
            realpart(&v->v_compdata[i]) = realpart(&oc[i]);
            imagpart(&v->v_compdata[i]) = imagpart(&oc[i]);
        }
        realpart(&c) = realpart(&oc[--i]);
        imagpart(&c) = imagpart(&oc[i]);
        while (i < length) {
            realpart(&v->v_compdata[i]) = realpart(&c);
            imagpart(&v->v_compdata[i++]) = imagpart(&c);
        tfree(oc);
        }
    }
    v->v_length = length;
    return;
}

/* Collapse every *xcomp elements into one, and use only the elements
 * between xind[0] and xind[1].
 */

static void
compress(d, xcomp, xind)
    struct dvec *d;
    double *xcomp, *xind;
{
    int cfac, ihi, ilo, newlen, i;
    int sz = isreal(d) ? sizeof (double) : sizeof (complex);
    double *dd;
    complex *cc;

    if (xind) {
        ilo = (int) xind[0];
        ihi = (int) xind[1];
        if ((ilo <= ihi) && (ilo > 0) && (ilo < d->v_length) &&
                (ihi > 1) && (ihi <= d->v_length)) {
            newlen = ihi - ilo;
            dd = (double *) tmalloc(newlen * sz);
            cc = (complex *) dd;
            if (isreal(d)) {
                bcopy((char *) (d->v_realdata + ilo),
                        (char *) dd, newlen * sz);
                tfree(d->v_realdata);
                d->v_realdata = dd;
            } else {
                bcopy((char *) (d->v_compdata + ilo),
                        (char *) cc, newlen * sz);
                tfree(d->v_compdata);
                d->v_compdata = cc;
            }
            d->v_length = newlen;
        }
    }

    if (xcomp) {
        cfac = (int) *xcomp;
        if ((cfac > 1) && (cfac < d->v_length)) {
            for (i = 0; i * cfac < d->v_length; i++)
                if (isreal(d))
                    d->v_realdata[i] = 
                            d->v_realdata[i * cfac];
                else
                    d->v_compdata[i] = 
                            d->v_compdata[i * cfac];
            d->v_length = i;
        }
    }
    return;
}
