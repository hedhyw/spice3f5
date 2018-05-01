/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
**********/

/*
 * String functions
 */

#include "spice.h"
#include "stdio.h"
#include "misc.h"
#include "suffix.h"

int
prefix(p, s)
    register char *p, *s;
{
    while (*p && (*p == *s))
        p++, s++;
    if (!*p)
        return (true);
    else
        return (false);
}

/* Create a copy of a string. */

char *
copy(str)
    char *str;
{
    char *p;
    
    if (p = tmalloc(strlen(str) + 1))
	    (void) strcpy(p, str);
    return(p);
}

/* Determine whether sub is a substring of str. */
/* Like strstr( ) XXX */

int
substring(sub, str)
    register char *str, *sub;
{
    char *s, *t;

    while (*str) {
        if (*str == *sub) {
	    t = str;
            for (s = sub; *s; s++) {
                if (!*t || (*s != *t++))
                    break;
            }
            if (*s == '\0')
                return (true);
        }
        str++;
    }
    return (false);
}

/* Append one character to a string. Don't check for overflow. */
/* Almost like strcat( ) XXX */

void
appendc(s, c)
    char *s, c;
{
    while (*s)
        s++;
    *s++ = c;
    *s = '\0';
    return;
}

/* Try to identify an integer that begins a string. Stop when a non-
 * numeric character is reached.
 */
/* Like atoi( ) XXX */

int
scannum(str)
    char *str;
{
    int i = 0;

    while(isdigit(*str))
        i = i * 10 + *(str++) - '0';
    return(i);
}

/* Case insensitive str eq. */
/* Like strcasecmp( ) XXX */

int
cieq(p, s)
    register char *p, *s;
{
    while (*p) {
        if ((isupper(*p) ? tolower(*p) : *p) !=
            (isupper(*s) ? tolower(*s) : *s))
            return(false);
        p++;
        s++;
    }
    return (*s ? false : true);
}

/* Case insensitive prefix. */

int
ciprefix(p, s)
    register char *p, *s;
{
    while (*p) {
        if ((isupper(*p) ? tolower(*p) : *p) !=
            (isupper(*s) ? tolower(*s) : *s))
            return(false);
        p++;
        s++;
    }
    return (true);
}

void
strtolower(str)
    char	*str;
{
    if (str)
	while (*str) {
	    *str = tolower(*str);
	    str++;
	}
}

char *
gettok(s)
    char **s;
{
    char buf[BSIZE_SP];
    int i = 0;
    char c;
    int paren;

    paren = 0;
    while (isspace(**s))
        (*s)++;
    if (!**s)
        return (NULL);
    while ((c = **s) && !isspace(c)) {
	if (c == '('/*)*/)
	    paren += 1;
	else if (c == /*(*/')')
	    paren -= 1;
	else if (c == ',' && paren < 1)
	    break;
        buf[i++] = *(*s)++;
    }
    buf[i] = '\0';
    while (isspace(**s) || **s == ',')
        (*s)++;
    return (copy(buf));
}

#ifndef HAS_INDEX

#ifndef index
char *
index(s, c)
    register char *s;
    register char c;
{
    while ((*s != c) && (*s != '\0'))
        s++;
    if (*s == '\0')
        return ((char *) 0);
    else
        return (s);
}
#endif

#ifndef rindex
char *
rindex(s, c)
    register char *s;
    register char c;
{
    register char *t;

    for (t = s; *t != '\0'; t++);
    while ((*t != c) && (t != s))
        t--;
    if (t == s)
        return ((char *) 0);
    else
        return (t);
}
#endif

#endif

#ifndef HAS_BCOPY

#ifndef bcopy
void
bcopy(from, to, num)
    register char *from, *to;
    register int num;
{
    while (num-- > 0)
        *to++ = *from++;
    return;
}
#endif

#ifndef bzero
/* can't declare void here, because we've already used it in this file */
/* and haven't declared it void before the use */
int
bzero(ptr, num)
    register char *ptr;
    register int num;
{
    while (num-- > 0)
        *ptr++ = '\0';
    return (0);
}

#endif
#endif
