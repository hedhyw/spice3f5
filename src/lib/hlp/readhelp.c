/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1986 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * The main routine for the help system in stand-alone mode.  We have three
 * versions of this code currently -- one for UNIX, one for MS-DOS, and one
 * for VMS.
 */

#include "spice.h"
#include "misc.h"
#include "cpstd.h"
#include "hlpdefs.h"

#ifdef notdef

#  ifdef HAS_BSDDIRS
#    include <sys/types.h>
#    include <sys/dir.h>
#    include <sys/stat.h>
#  else
#    ifdef HAS_SYSVDIRS
#      include <sys/types.h>
#      include <dirent.h>
#      ifndef direct
#        define direct dirent
#      endif
#    endif
#  endif
/* XXX */
#  ifdef vms
#    include <descrip.h>
#    include <rmsdef.h>
#  endif /*VMS*/

#endif

#include "suffix.h"

static char *getsubject();
static toplink *getsubtoplink();
static void sortlist(), tlfree();
static int sortcmp();

static topic *alltopics = NULL;

static fplace *copy_fplace();

topic *
hlp_read(place)
fplace *place;
{
    char buf[BSIZE_SP];
    topic *top = alloc(topic);
    toplink *topiclink;
    toplink *tl, *tend = NULL;
    wordlist *wl, *end = NULL;
    int i, fchanges;
    char *s;
    bool mof;

    if (!place)
	return 0;

    top->place = copy_fplace(place);

    /* get the title */
    if (!place->fp) place->fp = hlp_fopen(place->filename);
    if (!place->fp) return(NULL);
    fseek(place->fp, place->fpos, 0);
    (void) fgets(buf, BSIZE_SP, place->fp);    /* skip subject */
    (void) fgets(buf, BSIZE_SP, place->fp);
    for (s = buf; *s && (*s != '\n'); s++)
        ;
    *s = '\0';
    top->title = copy(&buf[7]);     /* don't copy "TITLE: " */

    /* get the text */
    /* skip to TEXT: */
    while (fgets(buf, BSIZE_SP, place->fp)) {
      if (!strncmp("TEXT: ", buf, 6)) break;
      if (*buf = '\0' ||
        !strncmp("SEEALSO: ", buf, 9) ||
        !strncmp("SUBTOPIC: ", buf, 10)) {
        /* no text */
        top->text = NULL;
        goto endtext;
      }
    }
    mof = true;
    while (mof && !strncmp("TEXT: ", buf, 6)) {
        for (s = &buf[6], fchanges = 0; *s && (*s != '\n'); s++)
            if (((s[0] == '\033') && s[1]) ||
                    ((s[0] == '_') && (s[1] == '\b')))
                fchanges++;
        *s = '\0';
        wl = alloc(wordlist);
        wl->wl_word = copy(&buf[6]);
        if (end)
            end->wl_next = wl;
        else
            top->text = wl;
        wl->wl_prev = end;
        end = wl;
        top->numlines++;
        if ((i = strlen(&buf[6]) - fchanges) > top->maxcols)
            top->maxcols = i;
        mof = fgets(buf, BSIZE_SP, place->fp) == NULL ? false : true;
    }
endtext:

    /* get subtopics */
    while(mof && !strncmp("SUBTOPIC: ", buf, 10)) {
        s = &buf[10];
        /* process tokens within line, updating pointer */
	while (*s) {
          if (topiclink = getsubtoplink(&s)) {
            if (tend)
              tend->next = topiclink;
            else
              top->subtopics = topiclink;
            tend = topiclink;
          }
	}
        mof = fgets(buf, BSIZE_SP, place->fp) == NULL ? false : true;
    }

    /* get see alsos */
    tend = NULL;
    while(mof && !strncmp("SEEALSO: ", buf, 9)) {
        s = &buf[9];
        /* process tokens within line, updating pointer */
	while (*s) {
          if (topiclink = getsubtoplink(&s)) {
            if (tend)
              tend->next = topiclink;
            else
              top->seealso = topiclink;
            tend = topiclink;
          }
	}
        mof = fgets(buf, BSIZE_SP, place->fp) == NULL ? false : true;
    }

        /* Now we have to fill in the subjects
        for the seealsos and subtopics. */
    for (tl = top->seealso; tl; tl = tl->next)
        tl->description = getsubject(tl->place);
    for (tl = top->subtopics; tl; tl = tl->next)
        tl->description = getsubject(tl->place);

    sortlist(&top->seealso);
    /* sortlist(&top->subtopics); It looks nicer if they
				    are in the original order */

    top->readlink = alltopics;
    alltopics = top;

    return (top);
}

/* *ss is of the form filename:subject */
static toplink *getsubtoplink(ss)
char **ss;
{
    toplink *tl;
    char *tmp, *s, *t;
    char subject[BSIZE_SP];

    if (!**ss) return(NULL);

    s = *ss;

    tl = alloc(toplink);
    if (tmp = index(s, ':')) {
      tl->place = alloc(fplace);
      tl->place->filename = strncpy(
            calloc(1, (unsigned) (sizeof (char) * (tmp - s + 1))),
            s, (tmp - s));
      tl->place->filename[tmp - s] = '\0';
      strtolower(tl->place->filename);

      /* see if filename is on approved list */
      if (!hlp_approvedfile(tl->place->filename)) {
        tfree(tl->place);
        tfree(tl);
        /* skip up to next comma or newline */
        while (*s && *s != ',' && *s != '\n') s++;
        while (*s && (*s == ',' || *s == ' ' || *s == '\n')) s++;
        *ss = s;
        return(NULL);
      }

      tl->place->fp = hlp_fopen(tl->place->filename);
      for (s = tmp + 1, t = subject; *s && *s != ',' && *s != '\n'; s++) {
        *t++ = *s;
      }
      *t = '\0';
      tl->place->fpos = findsubject(tl->place->filename, subject);
      if (tl->place->fpos == -1) {
        tfree(tl->place);
        tfree(tl);
        while (*s && (*s == ',' || *s == ' ' || *s == '\n')) s++;
        *ss = s;
        return(NULL);
      }
    } else {
      fprintf(stderr, "bad filename:subject pair %s\n", s);
      /* skip up to next free space */
      while (*s && *s != ',' && *s != '\n') s++;
      while (*s && (*s == ',' || *s == ' ' || *s == '\n')) s++;
      *ss = s;
      tfree(tl->place);
      tfree(tl);
      return(NULL);
    }
    while (*s && (*s == ',' || *s == ' ' || *s == '\n')) s++;
    *ss = s;
    return(tl);
}

/* returns a file position, -1 on error */
long
findsubject(filename, subject)
    char *filename, *subject;
{

    FILE *fp;
    char buf[BSIZE_SP];
    struct hlp_index indexitem;

    if (!filename) {
	return -1;
    }

    /* open up index for filename */
    sprintf(buf, "%s%s%s.idx", hlp_directory, DIR_PATHSEP, filename);
    hlp_pathfix(buf);
    if (!(fp = fopen(buf, "rb"))) {
      perror(buf);
      return(-1);
    }

    while(fread((char *) &indexitem, sizeof (struct hlp_index), 1, fp)) {
      if (!strncmp(subject, indexitem.subject, 64)) {
        fclose(fp);
        return(indexitem.fpos);
      }
    }

    fclose(fp);
    return(-1);

}

static char *
getsubject(place)
    fplace *place;
{
    char buf[BSIZE_SP], *s;

    if (!place->fp) place->fp = hlp_fopen(place->filename);
    if (!place->fp) return(NULL);

    fseek(place->fp, place->fpos, 0);
    (void) fgets(buf, BSIZE_SP, place->fp);
    for (s = buf; *s && (*s != '\n'); s++)
        ;
    *s = '\0';
    return (copy(&buf[9]));     /* don't copy "SUBJECT: " */
}

static void
sortlist(tlp)
    toplink **tlp;
{
    toplink **vec, *tl;
    int num = 0, i;

    for (tl = *tlp; tl; tl = tl->next)
        num++;
    if (!num)
        return;
    vec = (toplink **) tmalloc(sizeof (toplink *) * num);
    for (tl = *tlp, i = 0; tl; tl = tl->next, i++)
        vec[i] = tl;
    (void) qsort((char *) vec, num, sizeof (toplink *), sortcmp);
    *tlp = vec[0];
    for (i = 0; i < num - 1; i++)
        vec[i]->next = vec[i + 1];
    vec[i]->next = NULL;
    tfree(vec);
    return;
}

static int
sortcmp(tlp1, tlp2)
    toplink **tlp1, **tlp2;
{
    return (strcmp((*tlp1)->description, (*tlp2)->description));
}

void
hlp_free()
{
    topic *top, *nt = NULL;

    for (top = alltopics; top; top = nt) {
        nt = top->readlink;
        tfree(top->title);
        tfree(top->place);
        wl_free(top->text);
        tlfree(top->subtopics);
        tlfree(top->seealso);
        tfree(top);
    }
    alltopics = NULL;
    return;
}

static void
tlfree(tl)
    toplink *tl;
{
    toplink *nt = NULL;

    while (tl) {
        tfree(tl->description);
        tfree(tl->place->filename);
        tfree(tl->place);
        /* Don't free the button stuff... */
        nt = tl->next;
        tfree(tl);
        tl = nt;
    }
    return;
}

static fplace *
copy_fplace(place)
fplace *place;
{
    fplace *newplace;

    newplace = (fplace *) malloc(sizeof(fplace));
    newplace->filename = copy(place->filename);
    newplace->fpos = place->fpos;
    newplace->fp = place->fp;

    return(newplace);
}
