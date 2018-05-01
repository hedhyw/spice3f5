/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
**********/

/* from FILENAME.txt, make FILENAME.idx */

#include "spice.h"
#include "misc.h"
#include <stdio.h>
#include "cpdefs.h"
#include "hlpdefs.h"
#include "suffix.h"

#ifdef HAS_MAC_ARGCARGV
#include <console.h>
#endif

main(argc, argv)
int argc;
char *argv[];
{

    FILE *fp;
    FILE *wfp;
    char buf[BSIZE_SP];
    long fpos;
    char subject[BSIZE_SP];
    struct hlp_index indexitem;
    char *pos;

#ifdef HAS_MAC_ARGCARGV
    argc = ccommand(&argv);
#endif
    while (--argc) {
      if (!(fp = fopen(argv[argc], "r"))) {
        perror(argv[argc]);
        continue;
      }
      strcpy(buf, argv[argc]);
      if (((pos = rindex(buf, '.')) == 0) ||
            strcmp(pos, ".txt")) {
        fprintf(stderr, "%s does not end in .txt\n", buf);
        continue;
      }
      *++pos = 'i'; *++pos = 'd'; *++pos = 'x';
      if (!(wfp = fopen(buf, "wb"))) {
        perror(buf);
        continue;
      }
      fpos = 0;
      while (fgets(buf, sizeof(buf), fp)) {
        if (!strncmp(buf, "SUBJECT: ", 9)) {
          strcpy(subject, &buf[9]);
          subject[strlen(subject) - 1] = '\0';  /* get rid of '\n' */
          strncpy(indexitem.subject, subject, 64);  /* zero out end */
          indexitem.fpos = fpos;
          fwrite(&indexitem, sizeof(struct hlp_index), 1, wfp);
        }
        fpos = ftell(fp);
      }
    }
    exit(0);
}
