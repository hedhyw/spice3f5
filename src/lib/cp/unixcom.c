/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Routines to do execution of unix commands.
 */

#include "spice.h"
#include "cpdefs.h"

#ifdef HAS_VFORK

/* The only reason this exists is efficiency */

#  ifdef HAS_BSDDIRS
#    include <sys/types.h>
#    include <sys/dir.h>
#  else

#    ifdef HAS_SYSVDIRS
#      include <sys/types.h>
#      include <dirent.h>
#      ifndef direct
#        define direct dirent
#      endif
#    endif
#  endif

#include <sys/file.h>
#include <sys/wait.h>
#include <signal.h>
#include "suffix.h"

static bool myopendir();
static bool tryexec();
static char *myreaddir();
static int hash();

struct hashent {
    char *h_name;
    char *h_path;
    struct hashent *h_next;
} ;

#define HASHSIZE 256

static struct hashent *hashtab[HASHSIZE];
static char *dirbuffer;
static int dirlength, dirpos;

/* Create the hash table for the given search path. pathlist is a : seperated
 * list of directories. If docc is true, then all the commands found are
 * added to the command completion lists.
 */

void
cp_rehash(pathlist, docc)
    char *pathlist;
    bool docc;
{
    register int i;
    struct hashent *hh, *ht;
    char buf[BSIZE_SP], pbuf[BSIZE_SP], *curpath;
    DIR *pdir;
    struct direct *entry;

    /* First clear out the old hash table. */
    for (i = 0; i < HASHSIZE; i++) {
        for (hh = hashtab[i]; hh; hh = ht) {
            ht = hh->h_next;
            /* Don't free any of the other stuff -- it is too
             * strange.
             */
            tfree(hh);
        }
	hashtab[i] = NULL;
    }

    while (pathlist && *pathlist) {
        /* Copy one path to buf. We have to make sure that the path
         * is a full path name.
         */
        if (*pathlist == '/')
            i = 0;
        else {
#ifdef HAS_GETWD
            (void) getwd(buf);
#else
#  ifdef HAS_GETCWD
            (void) getcwd(buf, sizeof(buf));
#  else
	    *buf = 0;
#  endif
#endif
            i = strlen(buf);
        }
        while (*pathlist && (*pathlist != ':'))
            buf[i++] = *pathlist++;
        while (*pathlist == ':')
            pathlist++;
        buf[i] = '\0';

        curpath = copy(buf);
        if (!(pdir = opendir(curpath)))
            continue;
        while (entry = readdir(pdir)) {
            (void) strcpy(pbuf, curpath);
            (void) strcat(pbuf, "/");
            (void) strcat(pbuf, entry->d_name);
            /* Now we could make sure that it is really an
             * executable, but that is too slow
	     * (as if "we" really cared).
             */
            hh = alloc(struct hashent);
            hh->h_name = copy(entry->d_name);
            hh->h_path = curpath;
            i = hash(entry->d_name);
            /* Make sure this goes at the end, with
             * possible duplications of names.
             */
            if (hashtab[i]) {
                ht = hashtab[i];
                while (ht->h_next)
                    ht = ht->h_next;
                ht->h_next = hh;
            } else
                hashtab[i] = hh;
            if (docc) {
                /* Add to completion hash table. */
                cp_addcomm(entry->d_name, (long) 0, (long) 0, (long) 0,
                        (long) 0);
            }
        }
	closedir(pdir);
    }
    return;
}

/* The return value is false if no command was found, and true if it was. */

bool
cp_unixcom(wl)
    wordlist *wl;
{
    int i;
    register struct hashent *hh;
    register char *name;
    char **argv;
    char buf[BSIZE_SP];

    if (!wl)
        return (false);
    name = wl->wl_word;
    argv = wl_mkvec(wl);
    if (cp_debug) {
        printf("name: %s, argv: ", name);
        wl_print(wl, stdout);
        printf(".\n");
    }
    if (index(name, '/'))
        return (tryexec(name, argv));
    i = hash(name);
    for (hh = hashtab[i]; hh; hh = hh->h_next) {
        if (eq(name, hh->h_name)) {
            (void) sprintf(buf, "%s/%s", hh->h_path, hh->h_name);
            if (tryexec(buf, argv))
                return (true);
        }
    }
    return (false);
}

static bool
tryexec(name, argv)
    char *name;
    char *argv[];
{
#  ifdef HAS_INTWAITSTATUS
    int status;
#  else
    union wait status;
#  endif
    int pid, j;
    SIGNAL_TYPE (*svint)( ), (*svquit)( ), (*svtstp)( );

    pid = vfork( );
    if (pid == 0) {
	fixdescriptors();
        (void) execv(name, argv);
        (void) _exit(120);  /* A random value. */
        /* NOTREACHED */
    } else {
	svint = signal(SIGINT, SIG_DFL);
	svquit = signal(SIGQUIT, SIG_DFL);
	svtstp = signal(SIGTSTP, SIG_DFL);
        do {
            j = wait(&status);
        } while (j != pid);
	(void) signal(SIGINT, (SIGNAL_FUNCTION) svint);
	(void) signal(SIGQUIT, (SIGNAL_FUNCTION) svquit);
	(void) signal(SIGTSTP, (SIGNAL_FUNCTION) svtstp);
    }
    if (WTERMSIG(status) == 0 && WEXITSTATUS(status) == 120)
    /*if ((status.w_termsig == 0) && (status.w_retcode == 120)) */
	return (false);
    else
	return (true);
}

static int
hash(str)
    register char *str;
{
    register int i = 0;

    while (*str)
        i += *str++;
    return (i % HASHSIZE);
}

/* Debugging. */

void
cp_hstat()
{
    struct hashent *hh;
    int i;

    for (i = 0; i < HASHSIZE; i++)
        for (hh = hashtab[i]; hh; hh = hh->h_next)
            fprintf(cp_err, "i = %d, name = %s, path = %s\n",
                    i, hh->h_name, hh->h_path);
    return;
}

#ifdef notdef
/*** The person who wrote this should be strung up ***/
/* Some strange stuff... Don't ever free the dir buffer, because we keep
 * pointers into it in the command hash table.
 */

static bool
myopendir(path)
    char *path;
{
    struct stat stbuf;
    int i;

    if (!path)
        return (0);
    if (stat(path, &stbuf))
        return (false);
    if ((i = open(path, O_RDONLY)) == -1)
        return (0);
    dirbuffer = tmalloc(stbuf.st_size);
    if (read(i, dirbuffer, stbuf.st_size) != stbuf.st_size) {
        fprintf(cp_err, "Error: bad read on directory %s\n", path);
        return (0);
    }
    dirlength = stbuf.st_size;
    dirpos = 0;
    return (1);
}

static char *
myreaddir()
{
    struct direct *dp;

    /* Advance us to the next valid directory entry. */
    for (;;) {
        dp = (struct direct *) &dirbuffer[dirpos];
        if (dirpos >= dirlength)
            return (NULL);
        while (dp->d_ino == 0) {
            dirpos += dp->d_reclen;
            goto x; /* Ack... */
        }
        break;
x:      ;
    }
    dirpos += dp->d_reclen;
    return (dp->d_name);
}
#  endif

#else

void
cp_rehash(pathlist, docc)
    char *pathlist;
    bool docc;
{ }

bool
cp_unixcom(wl) 
    wordlist *wl;
{
    char *s = wl_flatten(wl);

#  ifdef HAS_SYSTEM
    if (system(s))
        return (false);
    else
        return (true);
#  else
    return (false);
#  endif
}

#endif

