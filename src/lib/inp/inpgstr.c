/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/*
 * Get string input token from 'line', and return a pointer to it in 'token'
 */

#include "spice.h"
#include "misc.h"
#include <stdio.h>
#include "util.h"
#include "iferrmsg.h"
#include "inpdefs.h"
#include "suffix.h"

int
INPgetStr(line, token, gobble)
  char **line;
  char **token;
  int gobble;			/* eat non-whitespace trash AFTER token? */
{
  char *point;
  char separator = '\0';

  /* Scan along throwing away garbage characters. */
  for (point = *line; *point != '\0'; point++) {
    if ( (*point == ' ') ||
	 (*point == '\t') ||
	 (*point == '=') ||
	 (*point == '(') ||
	 (*point == ')') ||
	 (*point == ',') ) continue;
    break;
  }
  if (*point == '"') {
    separator = '"';
    point++;
  } else if (*point == '\'') {
    separator = '\'';
    point++;
  }
  /* mark beginning of token */
  *line = point;
  /* now find all good characters */
  for (point = *line; *point != '\0'; point++) {
    if ( (*point == ' ') ||
	 (*point == '\t') ||
	 (*point == '=') ||
	 (*point == '(') ||
	 (*point == ')') ||
	 (*point == ',') ||
	 (*point == separator) ) break;
  }

  /* Create token */
  *token = (char *) MALLOC(1 + point - *line);
  if (!*token)
    return (E_NOMEM);
  (void) strncpy(*token, *line, point - *line);
  *(*token + (point - *line)) = '\0';
  *line = point;

  /* Gobble garbage to next token. */
  if (separator && **line == separator) {
    (*line)++;	/* Skip one closing separator */
  }
  for (; **line != '\0'; (*line)++) {
    if(**line == ' ') continue;
    if(**line == '\t') continue;
    if((**line == '=') && gobble) continue;
    if((**line == ',') && gobble) continue;
    break;
  }
  return (OK);
}
