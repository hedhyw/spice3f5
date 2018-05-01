/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
**********/

/*
 * Directory emulation
 */

#include "spice.h"
#include "stdio.h"
#include "misc.h"

#ifdef HAS_DOSDIRS
#include <direct.h>

#include <fcntl.h>

struct dos_dir {
	char	d_name[8];
	char	d_ext[3];
	char	d_attr;
	char	d_fill[10];
	short	d_time, d_date;
	short	d_ino;
	long	d_size;
};

DIR *
opendir(path)
    char *path;
{
    static int i;

    i = open(path, O_RDONLY);
    if (i == -1)
        return (NULL);
    else
        return ((DIR *) &i);
}

struct direct *
readdir(i)
    DIR *i;
{
    static struct direct d;
    static struct dos_dir dd;

    if (read(*i, &dd, sizeof (struct dos_dir)) < sizeof (struct dos_dir))
        return (NULL);
    else {
	strncpy(d.d_name, dd.d_name, sizeof(dd.d_name));
	d.d_name[sizeof(dd.d_name)] = 0;
	strcat(d.d_name, ".");
	strcat(d.d_name, dd.d_ext);
	d.d_namelen = strlen(d.d_name);
	d.d_ino = dd.d_ino;
	d.d_reclen = sizeof(d);	/* XXX */
        return (&d);
    }
}

void
closedir(i)
    DIR *i;
{
    (void) close (*i);
    return;
}

#else
int Dummy_Symbol;
#endif
