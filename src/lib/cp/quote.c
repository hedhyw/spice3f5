/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 *
 * Various things for quoting words. If this is not ascii, quote and
 * strip are no-ops, so '' and \ quoting won't work. To fix this, sell
 * your IBM machine and buy a vax.
 */

#include "spice.h"
#include "cpdefs.h"
#include "suffix.h"

/* Strip all the 8th bits from a string (destructively). */

void
cp_wstrip(str)
    char *str;
{
    char	c, d;

    if (str)
	while (c = *str) {
	    d = strip(c);
	    if (c != d)
		    *str = d;
	    str++;
	}
    return;
}

/* Quote all characters in a word. */

void
cp_quoteword(str)
    char *str;
{
    if (str)
	while (*str) {
	    *str = quote(*str);
	    str++;
	}
    return;
}

/* Print a word (strip the word first). */

void
cp_printword(string, fp)
    char *string;
    FILE *fp;
{
    char *s;

    if (string)
        for (s = string; *s; s++)
            (void) putc((strip(*s)), fp);
    return;
}

/* (Destructively) strip all the words in a wlist. */

void
cp_striplist(wlist)
    wordlist *wlist;
{
    wordlist *wl;

    for (wl = wlist; wl; wl = wl->wl_next)
        cp_wstrip(wl->wl_word);
    return;
}

/* Remove the "" from a string. */

char *
cp_unquote(string)
    char *string;
{
    char *s;
    int l;
    if (string) {
	s = copy(string);

	if (*s == '"')
	    s++;

	l = strlen(s) - 1;
	if (s[l] == '"')
	    s[l] = '\0';
	return (s);
    } else
	return 0;
}
