/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1986 Wayne A. Christopher, U. C. Berkeley CAD Group 
**********/

/*
 * Definitions for the help system.
 */

#ifdef HAS_X10
#  ifndef RootWindow
#    define RootWindow
#    include <X/Xlib.h>
#  endif
#endif

#ifdef HAS_X11
#  include <X11/Intrinsic.h>
#endif

typedef struct fplace {
    char *filename;
    long fpos;
    FILE *fp;
} fplace;

typedef struct button {
    char *text;
    fplace *tag;        /* Why isn't used for anything? */
    int x;
    int y;
    int width;
    int height;
} button;

struct hlp_index {
    char subject[64];
    long fpos;
};

typedef struct toplink {
    char *description;          /* really the subject */
    fplace *place;
    struct toplink *next;
    button button;
} toplink;

typedef struct topic {
    char *subject;
    char *title;
    fplace *place;
    wordlist *text;
    char *chartext;
    toplink *subtopics;
    toplink *seealso;
    int xposition;
    int yposition;
    struct topic *parent;
    struct topic *children;
    struct topic *next;
    struct topic *winlink;
    struct topic *readlink;
    int numlines;
    int maxcols;
    int curtopline;
#ifdef HAS_X10
    Window win;
    int lines;
    int cols;
    int xpix;
    int ypix;
    int sublabypos;
    int salabypos;
    button but_quit;
    button but_delete;
    button but_next;
    button but_prev;
#endif /*HAS_X10*/
#ifdef HAS_X11
    Widget shellwidget, formwidget, titlewidget, buttonwidget,
        textwidget, seelabelwidget, sublabelwidget, seeboxwidget, subboxwidget;
#endif
} topic;

typedef struct handle {
    topic *parent;
    toplink *result;
} handle;

#define REG_FONT        "timrom12"
#define BOLD_FONT       "timrom12b"
#define ITALIC_FONT     "timrom12i"
#define TITLE_FONT      "accordb"
#define BUTTON_FONT     "6x10"

#define X_INCR          20
#define Y_INCR          20
#define BORDER_WIDTH    3
#define INT_BORDER      10
#define BUTTON_XPAD 4
#define BUTTON_YPAD 2

#define START_XPOS  100
#define START_YPOS  100

/* If the MAX_LINES and SCROLL_INCR are different, it is very confusing... */

#define MIN_COLS    40
#define MAX_COLS    90

#define MAX_LINES   25
#define SCROLL_INCR 25

#define BS_LEFT     0
#define BS_CENTER   1
#define BS_UNIF     2

/* External symbols. */

/* help.c */

extern void hlp_main();
extern char *hlp_directory;
extern FILE *hlp_fopen();
extern fplace *findglobalsubject();

/* readhelp.c */

extern topic *hlp_read();
extern void hlp_free();
extern void hlp_pathfix();
extern long findsubject();
extern bool hlp_approvedfile();

/* provide.c */

extern void hlp_provide();
extern bool hlp_usex;

/* xdisplay.c */

extern bool hlp_xdisplay();
extern void hlp_xclosedisplay();
extern toplink *hlp_xhandle();
extern void hlp_killfamily();
extern void hlp_fixchildren();
extern void hlp_xkillwin();
extern void hlp_xwait();
extern char *hlp_regfontname;
extern char *hlp_boldfontname;
extern char *hlp_italicfontname;
extern char *hlp_titlefontname;
extern char *hlp_buttonfontname;
extern int hlp_initxpos;
extern int hlp_initypos;
extern int hlp_buttonstyle;
/*
extern bool hlp_using_x;
*/
extern char *hlp_displayname;

/* textdisplay.c */

extern bool hlp_tdisplay();
extern toplink *hlp_thandle();
extern void hlp_tkillwin();
extern int hlp_width;

