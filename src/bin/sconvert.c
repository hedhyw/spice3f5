/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Main routine for sconvert.
 */

#include "spice.h"
#include <stdio.h>
#include "cpdefs.h"
#include "ftedefs.h"
#include "ftedata.h"
#include "suffix.h"

FILE *cp_in = 0;
FILE *cp_out = 0;
FILE *cp_err = 0;
FILE *cp_curin = 0;
FILE *cp_curout = 0;
FILE *cp_curerr = 0;
int cp_maxhistlength;
bool cp_debug = false;
char cp_chars[128];
bool cp_nocc = true;
bool ft_parsedb = false;
struct circ *ft_curckt = NULL;

char *cp_program = "sconvert";

/* doesn't get used, but some unused routine in some file references it */
char out_pbuf[BSIZE_SP];

static void oldwrite();
static struct plot *oldread();
static char *fixdate();

void
main(ac, av)
    char **av;
{
    char *sf, *af;
    char buf[BSIZE_SP];
    char t, f;
    struct plot *pl;
    int i;
    char *infile;
    char *outfile;
    FILE *fp;
    cp_in = stdin;
    cp_out = stdout;
    cp_err = stderr;
    cp_curin = stdin;
    cp_curout = stdout;
    cp_curerr = stderr;

    switch (ac) {
        case 5: 
            sf = av[2];
            af = av[4];
            f = *av[1];
            t = *av[3];
            break;

        case 3:
            f = *av[1];
            t = *av[2];
            /* This is a pain, but there is no choice */
            sf = infile = smktemp("scin");
            af = outfile = smktemp("scout");
            if (!(fp = fopen(infile, "w"))) {
                perror(infile);
                exit(EXIT_BAD);
            }
            while (i = fread(buf, 1, sizeof(buf), stdin))
                (void) fwrite(buf, 1, i, fp);
            (void) fclose(fp);
            break;

        case 1: printf("Input file: ");
            (void) fflush(stdout);
            (void) gets(buf);
            sf = copy(buf);
            printf("Input type: ");
            (void) fflush(stdout);
            (void) gets(buf);
            f = buf[0];
            printf("Output file: ");
            (void) fflush(stdout);
            (void) gets(buf);
            af = copy(buf);
            printf("Output type: ");
            (void) fflush(stdout);
            (void) gets(buf);
            t = buf[0];
            break;
        default:
            fprintf(cp_err, 
                "Usage: %s fromtype fromfile totype tofile,\n",
                cp_program);
            fprintf(cp_err, "\twhere types are o, b, or a\n");
            fprintf(cp_err, 
                "\tor, %s fromtype totype, used as a filter.\n",
                cp_program);
            exit(EXIT_BAD);
    }
    switch(f) {
        case 'o' :
        pl = oldread(sf);
        break;

        case 'b' :
        case 'a' :
        pl = raw_read(sf);
        break;

        default:
        fprintf(cp_err, "Types are o, a, or b\n");
        exit(EXIT_BAD);
    }
    if (!pl)
        exit(EXIT_BAD);

    switch(t) {
        case 'o' :
        oldwrite(af, false, pl);
        break;

        case 'b' :
        raw_write(af, pl, false, true);
        break;

        case 'a' :
        raw_write(af, pl, false, false);
        break;

        default:
        fprintf(cp_err, "Types are o, a, or b\n");
        exit(EXIT_BAD);
    }
    if (ac == 3) {
        /* Gotta finish this stuff up */
        if (!(fp = fopen(outfile, "r"))) {
            perror(outfile);
            exit(EXIT_BAD);
        }
        while (i = fread(buf, 1, sizeof(buf), fp))
            (void) fwrite(buf, 1, i, stdout);
        (void) fclose(fp);
        (void) unlink(infile);
        (void) unlink(outfile);
    }
    exit(EXIT_NORMAL);
}

#define tfread(ptr, siz, nit, fp)   if (fread((char *) (ptr), (siz), \
                        (nit), (fp)) != (nit)) { \
                fprintf(cp_err, "Error: unexpected EOF\n"); \
                    return (NULL); }

#define tfwrite(ptr, siz, nit, fp)  if (fwrite((char *) (ptr), (siz), \
                        (nit), (fp)) != (nit)) { \
                    fprintf(cp_err, "Write error\n"); \
                    return; }

static struct plot *
oldread(name)
    char *name;
{
    struct plot *pl;
    char buf[BSIZE_SP];
    struct dvec *v, *end = NULL;
    short nv;       /* # vars */
    long np;        /* # points/var. */
    long i, j;
    short a;        /* The magic number. */
    float f1, f2;
    FILE *fp;

    if (!(fp = fopen(name, "r"))) {
        perror(name);
        return (NULL);
    }
    pl = alloc(struct plot);
    tfread(buf, 1, 80, fp);
    buf[80] = '\0';
    for (i = strlen(buf) - 1; (i > 1) && (buf[i] == ' '); i--)
        ;
    buf[i + 1] = '\0';
    pl->pl_title = copy(buf);

    tfread(buf, 1, 16, fp);
    buf[16] = '\0';
    pl->pl_date = copy(fixdate(buf));

    tfread(&nv, sizeof (short), 1, fp);

    tfread(&a, sizeof (short), 1, fp);
    if (a != 4)
        fprintf(cp_err, "Warning: magic number 4 is wrong...\n");

    for (i = 0; i < nv; i++) {
        v = alloc(struct dvec);
        if (end)
            end->v_next = v;
        else
            pl->pl_scale = pl->pl_dvecs = v;
        end = v;
        tfread(buf, 1, 8, fp);
        buf[8] = '\0';
        v->v_name = copy(buf);
    }
    for (v = pl->pl_dvecs; v; v = v->v_next) {
        tfread(&a, sizeof (short), 1, fp);
        v->v_type = a;
    }

    /* If the first output variable is type FREQ then there is complex
     * data, otherwise the data is real.
     */
    i = pl->pl_dvecs->v_type;
    if ((i == SV_FREQUENCY) || (i == SV_POLE) || (i == SV_ZERO))
        for (v = pl->pl_dvecs; v; v = v->v_next)
            v->v_flags |= VF_COMPLEX;
    else 
        for (v = pl->pl_dvecs; v; v = v->v_next)
            v->v_flags |= VF_REAL;

    /* Check the node indices -- this shouldn't be a problem ever. */
    for (i = 0; i < nv; i++) {
        tfread(&a, sizeof(short), 1, fp);
        if (a != i + 1) 
            fprintf(cp_err, "Warning: output %d should be %d\n",
                a, i);
    }
    tfread(buf, 1, 24, fp);
    buf[24] = '\0';
    pl->pl_name = copy(buf);
    /* Now to figure out how many points of data there are left in
     * the file. 
     */
    i = ftell(fp);
    (void) fseek(fp, (long) 0, 2);
    j = ftell(fp);
    (void) fseek(fp, i, 0);
    i = j - i;
    if (i % 8) {    /* Data points are always 8 bytes... */
        fprintf(cp_err, "Error: alignment error in data\n");
        (void) fclose(fp);
        return (NULL);
    }
    i = i / 8;
    if (i % nv) {
        fprintf(cp_err, "Error: alignment error in data\n");
        (void) fclose(fp);
        return (NULL);
    }
    np = i / nv;

    for (v = pl->pl_dvecs; v; v = v->v_next) {
        v->v_length = np;
        if (isreal(v)) {
            v->v_realdata = (double *) tmalloc(sizeof (double)
                    * np);
        } else {
            v->v_compdata = (complex *) tmalloc(sizeof (complex)
                    * np);
        }
    }
    for (i = 0; i < np; i++) {
        /* Read in the output vector for point i. If the type is
         * complex it will be float and we want double.
         */
        for (v = pl->pl_dvecs; v; v = v->v_next) {
            if (v->v_flags & VF_REAL) {
                tfread(&v->v_realdata[i], sizeof (double),
                        1, fp);
            } else {
                tfread(&f1, sizeof (float), 1, fp);
                tfread(&f2, sizeof (float), 1, fp);
                realpart(&v->v_compdata[i]) = f1;
                imagpart(&v->v_compdata[i]) = f2;
            }
        }
    }
    (void) fclose(fp);
    return (pl);
}

static void
oldwrite(name, app, pl)
    char *name;
    bool app;
    struct plot *pl;
{
    short four = 4, k;
    struct dvec *v;
    float f1, f2, zero = 0.0;
    char buf[80];
    int i, j, tp = VF_REAL, numpts = 0, numvecs = 0;
    FILE *fp;

    if (!(fp = fopen(name, app ? "a" : "w"))) {
        perror(name);
        return;
    }

    for (v = pl->pl_dvecs; v; v = v->v_next) {
        if (v->v_length > numpts)
            numpts = v->v_length;
        numvecs++;
        if (iscomplex(v))
            tp = VF_COMPLEX;
    }

    /* This may not be a good idea... */
    if (tp == VF_COMPLEX)
        pl->pl_scale->v_type = SV_FREQUENCY;

    for (i = 0; i < 80; i++)
        buf[i] = ' ';
    for (i = 0; i < 80; i++)
        if (pl->pl_title[i] == '\0')
            break;
        else
            buf[i] = pl->pl_title[i];
    tfwrite(buf, 1, 80, fp);

    for (i = 0; i < 80; i++)
        buf[i] = ' ';
    for (i = 0; i < 16; i++)
        if (pl->pl_date[i] == '\0')
            break;
        else
            buf[i] = pl->pl_date[i];
    tfwrite(buf, 1, 16, fp);

    tfwrite(&numvecs, sizeof (short), 1, fp);
    tfwrite(&four, sizeof (short), 1, fp);

    for (v = pl->pl_dvecs; v; v = v->v_next) {
        for (j = 0; j < 80; j++)
            buf[j] = ' ';
        for (j = 0; j < 8; j++)
            if (v->v_name[j] == '\0')
                break;
            else
                buf[j] = v->v_name[j];
        tfwrite(buf, 1, 8, fp);
    }

    for (v = pl->pl_dvecs; v; v = v->v_next) {
        j = (short) v->v_type;
        tfwrite(&j, sizeof (short), 1, fp);
    }

    for (k = 1; k < numvecs + 1; k++)
        tfwrite(&k, sizeof (short), 1, fp);
    for (j = 0; j < 80; j++)
        buf[j] = ' ';
    for (j = 0; j < 24; j++)
        if (pl->pl_name[j] == '\0')
            break;
        else
            buf[j] = pl->pl_name[j];
    tfwrite(buf, 1, 24, fp);
    for (i = 0; i < numpts; i++) {
        for (v = pl->pl_dvecs; v; v = v->v_next) {
            if ((tp == VF_REAL) && isreal(v)) {
                if (i < v->v_length) {
            tfwrite(&v->v_realdata[i], sizeof (double), 1, fp);
                } else {
    tfwrite(&v->v_realdata[v->v_length - 1], sizeof (double), 1, fp);
                }
            } else if ((tp == VF_REAL) && iscomplex(v)) {
                if (i < v->v_length)
                    f1 = realpart(&v->v_compdata[i]);
                else
                    f1 = realpart(&v->v_compdata[v-> v_length - 1]);
                tfwrite(&f1, sizeof (double), 1, fp);
            } else if ((tp == VF_COMPLEX) && isreal(v)) {
                if (i < v->v_length)
                    f1 = v->v_realdata[i];
                else
                    f1 = v->v_realdata[v->v_length - 1];
                tfwrite(&f1, sizeof (float), 1, fp);
                tfwrite(&zero, sizeof (float), 1, fp);
            } else if ((tp == VF_COMPLEX) && iscomplex(v)) {
                if (i < v->v_length) {
                    f1 = realpart(&v->v_compdata[i]);
                    f2 = imagpart(&v->v_compdata[i]);
                } else {
                    f1 = realpart(&v->v_compdata[v-> v_length - 1]);
                    f2 = imagpart(&v->v_compdata[v-> v_length - 1]);
                }
                tfwrite(&f1, sizeof (float), 1, fp);
                tfwrite(&f2, sizeof (float), 1, fp);
            }
        }
    }

    (void) fclose(fp);
    return;
}

static char *
fixdate(date)
    char *date;
{
    char buf[20];
    int i;

    (void) strcpy(buf, date);
    for (i = 17; i > 8; i--)
        buf[i] = buf[i - 1];
    buf[8] = ' ';
    buf[18] = '\0';
    return (copy(buf));
}

void cp_pushcontrol() { }
void cp_popcontrol() { }
void out_init() { }
void cp_doquit() { exit(0); }
/* ARGSUSED */ void cp_usrvars(v1, v2) struct variable **v1, **v2; { return; }
/* ARGSUSED */ int cp_evloop(s) char *s; { return (0); }
/* ARGSUSED */ void cp_ccon(o) bool o; { }
/* ARGSUSED */ char *if_errstring(c) int c; { return ("error"); }
#ifndef out_printf
/* VARARGS1 ARGSUSED */ void out_printf(fmt, args) char *fmt; { }
#endif
/* ARGSUSED */ void out_send(string) char *string; {}
/* ARGSUSED */ struct variable * cp_enqvar(word) char *word; { return (NULL); }
/* ARGSUSED */ struct dvec *vec_get(word) char *word; { return (NULL); }
/* ARGSUSED */ void cp_ccom(w, b, e) wordlist *w; char *b; bool e; { return; }
/* ARGSUSED */ int cp_usrset(v, i) struct variable *v; bool i;{return(US_OK); }

int disptype;
void XtDispatchEvent(pev) char *pev; {}
