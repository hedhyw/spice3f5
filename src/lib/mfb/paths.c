/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
**********/
/*
 * paths.c
 *
 * Written by John Ousterhout.  Modified for VMS by Giles Billingsley.
 * sccsid "@(#)paths.c  1.9  9/3/83"
 *
 * This file contains routines that a) implement a path mechanism, whereby
 * several places may be searched for files, and b) provide a defaulting
 * mechanism for file name extensions.
 *
 *     KIC is a graphics editor that was developed by the integrated
 * circuits group of the Electronics Research Laboratory and the
 * Department of Electrical Engineering and Computer Sciences at
 * the University of California, Berkeley, California.  The program
 * KIC is available free of charge to any interested party.
 * The sale, resale, or use of this program for profit without the
 * express written consent of the Department of Electrical Engineering
 * and Computer Sciences, University of California, Berkeley, California,
 * is forbidden.
 */

#include "spice.h"
#include <stdio.h>
#ifndef vms
#include <pwd.h>
#endif
#include <ctype.h>
#include "suffix.h"

#ifdef TEST
main(){
    char buf[100];
    int i,j,k,l,n;
    FILE *POpen();
    char *PGetPath();
    char *prealname;
#ifdef vms
    PSetPath(" ~ ~.foo dma1:[kic.cells] ");
#else
    PSetPath(" . ~gilesb/mfb/src ");
#endif
    printf("PATH? %s\n",PGetPath());
    for(;;){
    scanf("%s",buf);
    POpen(buf, "r", NULL, &prealname);
    printf("%s\n",prealname);
    }
    }
#endif

#define FALSE   0
#define TRUE    1
#define TILDA   '~'

/* Library routines */

char *strcpy(), *strncpy(), *index();

/* The following string holds the current path,which consists of a bunch
 * of directory names separated by spaces.
 */

#define PATHSIZE 180 
static char path[PATHSIZE];

int
PConvertTilde(psource, pdest, size)
char **psource;         /* Pointer to a pointer to the source string */
char **pdest;           /* Pointer to a pointer to the dest. string */
int *size;          /* Pointer to no. bytes available at pdest */

/*-----------------------------------------------------------------------------
 *  This routine converts tilde notation into standard directory names.
 *
 *  Results:
 *  If the conversion was done successfully, then TRUE is returned.
 *  If a user name couldn't be found in the password file, then
 *  FALSE is returned.
 *
 *  Side Effects:
 *  If the first character of the string indicated by psource is a
 *  tilde ("~") then the subsequent user name is converted to a login
 *  directory name and stored in the string indicated by pdest.  Then
 *  remaining characters in the file name at psource are copied to
 *  pdest and both pointers are updated.  Upon return, psource points
 *  to the terminating character in the source file name and pdest
 *  points to the next character after the last character in the file
 *  name at that location.  If a tilde cannot be converted because the
 *  user name cannot be found, psource is still advanced past the current
 *  entry, but pdest is unaffected.  At most size characters will be
 *  stored at pdest, and the size is decremented by the number of
 *  characters we actually stored.
 *-----------------------------------------------------------------------------
 */

    {
    register char *ps, *pd;

#ifndef vms
    struct passwd *passwd, *getpwnam();
#endif

    char username[35];
    int i, length;

    ps = *psource;
    if (*ps == TILDA){
    /* Copy the user name into a string (at most 34 characters), then
     * read the password file entry for the user, then grab out the
     * login directory.
     */

#ifdef vms
    ps++;
    if(getenv("HOME") == NULL)
#else
    pd = username;
    for (i=0; ; i++)
        {
        *pd = *++ps;
        if (isspace(*pd) || (*pd=='\0') || (*pd=='/')) break;
        if (i < 34) pd++;
        }
    *pd = '\0';
    passwd = getpwnam(username);

    /* If the entry can't be found, skip the source entry and return */

    if (passwd == NULL)
#endif
        {
        while ((*ps != '\0') && !isspace(*ps)) ps++;
        *psource = ps;
        return FALSE;
        }

#ifdef vms
    length = strlen(getenv("HOME"));
    /* remove closing bracket */
    --length;
    if (length > *size) length = *size;
    (void) strncpy(*pdest, getenv("HOME"), length);
    *size -= length;
    pd = *pdest+length;
#else
    length = strlen(passwd->pw_dir);
    if (length > *size) length = *size;
    (void) strncpy(*pdest, passwd->pw_dir, length);
    *size -= length;
    pd = *pdest+length;
#endif
    }
    else pd = *pdest;

    /* Copy the rest of the directory name from the source to the dest. */

    while ((*ps != '\0') && !isspace(*ps))
    if (*size > 0)
        {
        *pd++ = *ps++;
        (*size)--;
        }
    else ps++;
#ifdef vms
    /* insert closing bracket for tilda expansion */
    if (**psource == TILDA) {
    if (*size > 0) {
        *pd++ = ']';
        (*size)--;
        }
    else
        *(pd - 1) = ']';
    }
#endif
    *psource = ps;
    *pdest = pd;
    return TRUE;
    }

int
PSetPath(string)
char *string;           /* Pointer to a string that is to become
                 * the new fle search path.  Must consist
                 * of one or more directory names separated
                 * by white space.  ~ notation is ok.
                 */

/*-----------------------------------------------------------------------------
 *  PSetPath sets up the current search path.
 *
 *  Results:
 *  FALSE is returned if one or more of the paths contained a tilde
 *  notation that couldn't be converted.  Otherwise TRUE is returned.
 *
 *  Side Effects:
 *  The string is stored as the current path, and all entries with
 *  tilde notation are converted to non-tilde notation.  Tilde entries
 *  that cannot be converted are ignored.  Note:  only PATHSIZE total
 *  bytes of path are stored, after tilde conversion.  Excess bytes
 *  are truncated.
 *-----------------------------------------------------------------------------
 */

    {
    int result, spaceleft;
    char *p;

    if(string[0] == '\0')
      return(path[0] = '\0');
    result = TRUE;
    spaceleft = PATHSIZE-1;
    p = path;
    while (*string != '\0') {
    if (spaceleft <= 0) break;
    while (isspace(*string)) string++;
    if (!PConvertTilde(&string, &p, &spaceleft)) result = FALSE;
    else if (spaceleft-- > 0) *p++ = ' ';
    }
    *p = '\0';
    return result;
    }

char *
PGetPath()

/*-----------------------------------------------------------------------------
 *  This routine merely returns a pointer to the current path.
 *
 *  Results:
 *  The address of the current path (with all tildes expanded).
 *
 *  Side Effects:   None.
 *-----------------------------------------------------------------------------
 */

    {
    return path;
    }

FILE *
POpen(file, mode, ext, prealname)
char *file;         /* Name of the file to be opened. */
char *mode;         /* The file mode, as given to fopen. */
char *ext;          /* The default extension fo the file, or
                 * NULL if there is to be no default extension.
                 */
char **prealname;       /* Pointer to a location that will be filled
                 * in with the address of the real name of
                 * the file that was successfully opened.
                 * If NULL, then nothing is stored.
                 */

/*-----------------------------------------------------------------------------
 *  This routine does a file lookup using the current path and
 *  supplying a default extension.
 *
 *  Results:
 *  A pointer to a FILE, or NULL if the file couldn't be found.
 *
 *  Side Effects:
 *  If the file name doesn't contain the first character of ext
 *  then ext is appended to the file name, and this routine is
 *  called recursively with the new name.  If that succeeeds,
 *  then we are done.  Otherwise, if the first character of the
 *  file name is "~" or "/" then we try to look up the file with
 *  the original name, doing tilde expansion of course.  If the
 *  first character isn't one of those two characters, we go through
 *  the current path trying to look up the file once for each path
 *  entry by prepending the path entry to the original file name.
 *  This concatenated name is stored in a static string and made
 *  available to the caller through prealname if the open succeeds.
 *  Note: the static string will be trashed on the next call to this
 *  routine.  Also, note that no individual file name is allowed to
 *  be more than NAMESIZE characters long.  Excess characters are lost.
 *-----------------------------------------------------------------------------
 */

    {
#define NAMESIZE 80
    static char realname[NAMESIZE];
    char extendedname[NAMESIZE], *p, *p2;
    int length, spaceleft;
    FILE *f;

    if(path[0] == '\0' && file[0] != TILDA)
      return(fopen(file,mode));

    /* See if we must supply a default extension.  If so, then do it
     * and call this routine recursively.
     */

    if (prealname != NULL) *prealname = realname;
    if (ext != NULL)
    if (index(file, ext[0]) == 0)
        {
        length = strlen(file);
        if (length >= NAMESIZE) length = NAMESIZE-1;
        (void) strncpy(extendedname, file, length);
        p = &extendedname[length];
        length = NAMESIZE-1-length;
        if (length > 0) (void) strncpy(p, ext, length);
        extendedname[NAMESIZE-1] = '\0';
        f = POpen(extendedname, mode, (char *) NULL, prealname);
        if (f != NULL) return f;
        }

    /* OK, the default extension either wasn't necessary or didn't work.
     * Now try the original name.  If it starts with a ~ or /, look it
     * up directly.
     */

#ifdef vms
    if ((index(file,':') != 0) || (index(file,'[') != 0))
#else
    if (file[0] == '/')
#endif
    {
    (void) strncpy(realname, file, NAMESIZE-1);
    realname[NAMESIZE-1] = '\0';
    return fopen(realname, mode);
    }

#ifndef vms
    if (file[0] == TILDA)
    {
    p = realname;
    length = NAMESIZE-1;
    if (!PConvertTilde(&file, &p, &length)) return NULL;
    *p = '\0';
    return fopen(realname, mode);
    }
#endif

    /* Last, but not least, try going through the path. */

    p = path;
    while (*p != '\0')
    {
    spaceleft = NAMESIZE-1;
    p2 = realname;
    while (isspace(*p)) p++;
#ifdef vms
    if(*p == '.' && (isspace(*(p+1)) || *(p+1) == '\0')){
        ++p;
        if((f = fopen(file, mode)) != NULL)
        return f;
        else
        continue;
        }
#endif
    while ((*p != '\0') && !isspace(*p))
        if (spaceleft-- > 0) *p2++ = *p++;
        else p++;
#ifndef vms
    if (spaceleft-- > 0) *p2++ = '/';
#endif
    if (spaceleft > 0) (void) strncpy(p2, file, spaceleft);
    realname[NAMESIZE-1] = '\0';
    f = fopen(realname, mode);
    if (f != NULL) return f;
    }
    return NULL;
    }

#ifdef notdef
/*
 * VMS and the Masscomp MC-500 do not have index() in the string library.
 */
#ifdef mc500
char *index(s,c)
    char *s,c;
    {
    char buf[2];
    int i,strcspn();
    buf[0] = c;
    buf[1] = NULL;
    i = strcspn(s,buf);
    if(i != strlen(s))
    return( &s[i] );
    else
    return( NULL );
    }
#endif
#ifdef vms
char *index(s,c)
    char *s,c;
    {
    char buf[2];
    int i,strcspn();
    buf[0] = c;
    buf[1] = NULL;
    i = strcspn(s,buf);
    if(i != strlen(s))
    return( &s[i] );
    else
    return( NULL );
    }
#endif
#endif


