/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1986 Wayne A. Christopher, U. C. Berkeley CAD Group 
**********/

#include "spice.h"
#include "cpstd.h"
#include "hlpdefs.h"
#include "suffix.h"

#ifdef HAS_X10
extern Display *X_display;  /* so we can switch back, ick, ick, ick J.H. */
#endif

char *hlp_boldfontname = BOLD_FONT;
char *hlp_regfontname = REG_FONT;
char *hlp_italicfontname = ITALIC_FONT;
char *hlp_titlefontname = TITLE_FONT;
char *hlp_buttonfontname = BUTTON_FONT;
char *hlp_displayname = NULL;
int hlp_initxpos = START_XPOS;
int hlp_initypos = START_YPOS;
int hlp_buttonstyle = BS_LEFT;

#ifdef HAS_X10

static topic *topics = NULL;
static Display *xdisplay = NULL;
static Cursor cursor, waitcur;
static FontInfo *regfont, *boldfont, *italicfont, *titlefont, *buttonfont;
static Color fore, back, border;
static int lineheight;

#define SUB_TOPICS  "Sub-Topics:"
#define SEE_ALSO    "See Also:"

#include "help.cur"
#include "helpmask.cur"

#include "wait.cur"
#include "waitmask.cur"

static void drawtext(), puttitle();
static void redraw();
static void drawbutton(), calcpos();
static button *findbutton();

/* Create a new window... */

bool
hlp_xdisplay(top)
    topic *top;
{
    int width, yoff, y;
    toplink *tl;

    if (!xdisplay) {
        /* Gotta init everything. */
        if (!(xdisplay = XOpenDisplay(hlp_displayname))) {
            fprintf(stderr, "Error: can't open display\n");
            return (false);
        }
        if (!(cursor = XCreateCursor(help_width, help_height,
                help_bits, helpmask_bits, help_x_hot,
                help_y_hot, BlackPixel, WhitePixel, GXcopy))) {
            fprintf(stderr, "Error: can't create cursor\n");
            return (false);
        }
        if (!(waitcur = XCreateCursor(wait_width, wait_height,
                wait_bits, waitmask_bits, wait_x_hot, 
                wait_y_hot, BlackPixel, WhitePixel, GXcopy))) {
            fprintf(stderr, "Error: can't create cursor\n");
            return (false);
        }
        if (!(regfont = XOpenFont(hlp_regfontname))) {
            fprintf(stderr, "Note: can't open font %s\n",
                    hlp_regfontname);
	    if (!(regfont = XOpenFont("fixed"))) {
		fprintf(stderr, "Error: can't open font \"fixed\"\n");
		return (false);
	    }
        }
        if (!(boldfont = XOpenFont(hlp_boldfontname))) {
            fprintf(stderr, "Note: can't open font %s\n",
                    hlp_boldfontname);
	    boldfont = regfont;
        }
        if (!(italicfont = XOpenFont(hlp_italicfontname))) {
            fprintf(stderr, "Note: can't open font %s\n",
                    hlp_italicfontname);
	    italicfont = boldfont;
        }
        if (!(titlefont = XOpenFont(hlp_titlefontname))) {
            fprintf(stderr, "Note: can't open font %s\n",
                    hlp_titlefontname);
	    titlefont = boldfont;
        }
        if (!(buttonfont = XOpenFont(hlp_buttonfontname))) {
            fprintf(stderr, "Note: can't open font %s\n",
                    hlp_buttonfontname);
	    buttonfont = boldfont;
        }
        fore.pixel = BlackPixel;
        back.pixel = WhitePixel;
        border.pixel = BlackPixel;

        lineheight = (regfont->height > boldfont->height) ?
                regfont->height : boldfont->height;
        lineheight = (lineheight > italicfont->height) ? lineheight :
                italicfont->height;
    }

    XSetDisplay(xdisplay);

    if (!top->parent) {
        top->xposition = hlp_initxpos;
        top->yposition = hlp_initypos;
    } else {
        top->xposition = top->parent->xposition + X_INCR;
        top->yposition = top->parent->yposition + Y_INCR;
    }
    top->lines = (top->numlines < MAX_LINES) ? top->numlines : MAX_LINES;
    top->cols = (top->maxcols < MAX_COLS) ? top->maxcols : MAX_COLS;

    width = (regfont->width + boldfont->width + italicfont->width) / 3;

    top->xpix = ((top->cols > MIN_COLS) ? top->cols : MIN_COLS) * width +
            2 * INT_BORDER;
    top->ypix = top->lines * lineheight + 2 * INT_BORDER +
            titlefont->height;

    /* Better figure out the positions of the buttons... */
    for (tl = top->seealso; tl; tl = tl->next) {
        tl->button.text = tl->description;
        tl->button.tag = tl->place;
        if (!tl->button.text)
            tl->button.text = "<unknown>";
    }
    for (tl = top->subtopics; tl; tl = tl->next) {
        tl->button.text = tl->description;
        tl->button.tag = tl->place;
        if (!tl->button.text)
            tl->button.text = "<unknown>";
    }
    calcpos(top->subtopics, top->xpix);
    calcpos(top->seealso, top->xpix);

    yoff = top->ypix - INT_BORDER;

    if (top->subtopics) {
        yoff += titlefont->height * 2;
        top->sublabypos = yoff - titlefont->height * 3 / 2;
        y = yoff;
        for (tl = top->subtopics; tl; tl = tl->next) {
            tl->button.y += y;
            if (yoff < tl->button.y + tl->button.height)
                yoff = tl->button.y + tl->button.height;
        }
    }

    if (top->seealso) {
        top->salabypos = yoff + titlefont->height / 2;
        yoff += titlefont->height * 2; 
        y = yoff;
        for (tl = top->seealso; tl; tl = tl->next) {
            tl->button.y += y;
            if (yoff < tl->button.y + tl->button.height)
                yoff = tl->button.y + tl->button.height;
        }
    }

    top->ypix = yoff + INT_BORDER;

    top->but_quit.text = "Quit Help";
    top->but_quit.x = top->xpix - INT_BORDER -
            XStringWidth(top->but_quit.text, buttonfont, 0, 0);
    top->but_quit.y = -1;

    top->but_delete.text = "Delete Window";
    top->but_delete.x = top->but_quit.x - XStringWidth(top->but_delete.
            text, buttonfont, 0, 0) - INT_BORDER;
    top->but_delete.y = -1;
            
    top->but_prev.text = "Prev Page";
    top->but_prev.x = top->but_delete.x - XStringWidth(top->but_prev.
            text, buttonfont, 0, 0) - INT_BORDER;
    top->but_prev.y = -1;
            
    top->but_next.text = "Next Page";
    top->but_next.x = top->but_prev.x - XStringWidth(top->but_next.
            text, buttonfont, 0, 0) - INT_BORDER;
    top->but_next.y = -1;
            
    if (!(top->win = XCreateWindow(RootWindow, top->xposition,
            top->yposition, top->xpix, top->ypix, BORDER_WIDTH,
            BlackPixmap, WhitePixmap))) {
        fprintf(stderr, "Error: can't create window\n");
        return (false);
    }
    XDefineCursor(top->win, cursor);
    XStoreName(top->win, top->title);
    XMapWindow(top->win);

    redraw(top, 0, 0, top->xpix, top->ypix);

    XSelectInput(top->win, ButtonPressed  | ExposeRegion | ExposeWindow);

    top->winlink = topics;
    topics = top;

    return (true);
}

/* need to switch back old display pointer J.H. */
void
hlp_xclosedisplay()
{
    XCloseDisplay(xdisplay);
    xdisplay = NULL;
    XSetDisplay(X_display);
}

/* Deal with the windows on the screen...  Strange things may happen if there
 * is also a plot window...
 */

toplink *
hlp_xhandle(parent)
    topic **parent;
{
    XEvent event;
    XKeyOrButtonEvent *ev = (XKeyOrButtonEvent *) &event;
    XExposeEvent *xev = (XExposeEvent *) &event;
    topic *top;
    button *but;
    toplink *tl;

    if (!topics) {
        *parent = NULL;
        return (NULL);
    }
    for (;;) {
        XSetDisplay(xdisplay);
        XNextEvent(&event);

        for (top = topics; top; top = top->winlink)
            if (top->win == event.window)
                break;
        if (!top) {
            /* Probably it's one we just destroyed. */
            continue;
        }
        *parent = top;

        switch (event.type) {
            case 0:
            break;
        
            case ExposeRegion:
            case ExposeWindow:
            redraw(top, xev->x, xev->y, xev->width, xev->height);
            break;

            case ButtonPressed:
            if ((but = findbutton(top, ev->x, ev->y))) {
                if (but == &top->but_quit) {
                    *parent = NULL; 
                    return (NULL); 
                } else if (but == &top->but_delete) {
                    return (NULL);
                } else if (but == &top->but_next) {
                    if (top->curtopline + MAX_LINES >=
                            top->numlines) {
                        XFeep(0);
                        break;
                    }
                    top->curtopline += SCROLL_INCR;
                    XClear(top->win);
                    redraw(top, 0, 0, top->xpix,
                            top->ypix);
                    break;
                } else if (but == &top->but_prev) {
                    if (top->curtopline == 0) {
                        XFeep(0);
                        break;
                    }
                    top->curtopline -= SCROLL_INCR;
                    if (top->curtopline < 0)
                        top->curtopline = 0;
                    XClear(top->win);
                    redraw(top, 0, 0, top->xpix,
                            top->ypix);
                    break;
                }
                for (tl = top->subtopics; tl; tl = tl->next)
                    if (but == &tl->button)
                        return (tl);
                for (tl = top->seealso; tl; tl = tl->next)
                    if (but == &tl->button)
                        return (tl);
                fprintf(stderr, "Ack, no matching button.\n");
            }
            break;
        
            default:
            fprintf(stderr, "Strange event type %d\n", event.type);
            break;
        }
    }
}

/* Kill a window. */

void
hlp_xkillwin(top)
    topic *top;
{
    topic *last;

    if (top == topics)
        topics = top->winlink;
    else {
        for (last = topics; last->winlink; last = last->winlink)
            if (last->winlink == top) {
                last->winlink = top->winlink;
                break;
            }
        if (!last->winlink) {
            fprintf(stderr, "window not in list!!\n");
            return;
        }
    }
    XDestroyWindow(top->win);
    XFlush();
    return;
}

static void
puttitle(top)
    topic *top;
{
    XPixSet(top->win, 0, 0, top->xpix, titlefont->height + 1, BlackPixel);
    XText(top->win, INT_BORDER, 0, top->title, strlen(top->title),
            titlefont->id, WhitePixel, BlackPixel);
    return;
}

/* Print text.  This is kind of tough, since we have to use different fonts
 * depending on whether we have bold, italic, or roman.
 */ 

static void
drawtext(top, first, last)
    topic *top;
    int first, last;
{
    int i, curxpos, curypos;
    wordlist *wl;
    char *s, *t, save;
    FontInfo *ft = regfont, *nft = regfont;

    if (first < 0)
        first = 0;
    if (last >= top->lines)
        last = top->lines - 1;
    for (i = first, wl = top->text; i && wl; i--)
        wl = wl->wl_next;
    for (i = top->curtopline; i && wl; i--)
        wl = wl->wl_next;
    for (i = first; (i <= last) && wl; i++, wl = wl->wl_next) {
        curxpos = INT_BORDER;
        curypos = i * lineheight + INT_BORDER + titlefont->height;
        for (s = wl->wl_word; *s; ) {
            for (t = s; *t; t++) {
                if ((t[0] == '\033') && (t[1] == 'G')) {
                    nft = boldfont;
                    break;
                } else if ((t[0] == '\033') && (t[1] == 'H')) {
                    nft = regfont;
                    break;
                } else if (t[0] == '\033') {
                    nft = ft;
                    break;
                } else if ((t[0] == '_') && (t[1] == '\b')) {
                    nft = italicfont;
                    break;
                } else if (ft == italicfont) {
                    nft = regfont;
                    t++;
                    break;
                }
            }
            /* Now print everything between s and t (but not *t) */
            if (ft == boldfont)
                XText(top->win, curxpos, curypos - 2, s, t - s,
                    ft->id, BlackPixel, WhitePixel);
            else
                XText(top->win, curxpos, curypos, s, t - s,
                    ft->id, BlackPixel, WhitePixel);
            /* This is gross... */
            save = *t;
            *t = '\0';
            curxpos += XStringWidth(s, ft, 0, 0);
            *t = save;
            if (*t && (ft != italicfont))
                s = t + 2;
            else
                s = t;
            ft = nft;
        }
    }
    if (top->curtopline + MAX_LINES < top->numlines)
        XText(top->win, top->xpix / 2, MAX_LINES * lineheight +
                INT_BORDER + titlefont->height, "- more -", 8,
                buttonfont->id, BlackPixel, WhitePixel);
    else
        XText(top->win, top->xpix / 2, MAX_LINES * lineheight +
                INT_BORDER + titlefont->height, "        ", 8,
                buttonfont->id, BlackPixel, WhitePixel);
    return;
}

/* This draws a button in the button font, with a box around it. */

static void
drawbutton(top, but)
    topic *top;
    button *but;
{
    but->height = buttonfont->height + 6;
    if ((hlp_buttonstyle != BS_UNIF) || (but->width == 0))
        but->width = XStringWidth(but->text, buttonfont, 0, 0) + 6;

    /* This should look sort of neat... */
    XPixSet(top->win, but->x + 1, but->y + 1, but->width - 1,
            but->height - 2, BlackPixel);
    XPixSet(top->win, but->x + 2, but->y + 2, but->width - 3,
            but->height - 4, WhitePixel);
    XText(top->win, but->x + 3, but->y + 3, but->text, strlen(but->text),
            buttonfont->id, BlackPixel, WhitePixel);
    return;
}

#define within(but, xp, yp) (((xp) >= (but)->x) && ((xp) <= (but)->x + \
        (but)->width) &&  ((yp) >= (but)->y) && ((yp) <= (but)->y + \
        (but)->height))

static button *
findbutton(top, x, y)
    topic *top;
    int x, y;
{
    toplink *tl;

    if (within(&top->but_quit, x, y))
        return (&top->but_quit);
    if (within(&top->but_delete, x, y))
        return (&top->but_delete);
    if (within(&top->but_next, x, y) && (top->numlines > MAX_LINES))
        return (&top->but_next);
    if (within(&top->but_prev, x, y) && (top->numlines > MAX_LINES))
        return (&top->but_prev);
    for (tl = top->subtopics; tl; tl = tl->next)
        if (within(&tl->button, x, y))
            return (&tl->button);
    for (tl = top->seealso; tl; tl = tl->next)
        if (within(&tl->button, x, y))
            return (&tl->button);
    return (NULL);
}

static void
calcpos(tls, width)
    toplink *tls;
    int width;
{
    toplink *tl;
    int maxwidth = 0, ncols, nrows, nbuts = 0, x, y;

    if (!tls)
        return;
    for (tl = tls; tl; tl = tl->next) {
        tl->button.width = XStringWidth(tl->button.text, buttonfont,
                0, 0) + 6;
        tl->button.height = buttonfont->height + 6;
        if (tl->button.width + BUTTON_XPAD > maxwidth)
            maxwidth = tl->button.width + BUTTON_XPAD;
        nbuts++;
    }
    if (hlp_buttonstyle == BS_UNIF)
        for (tl = tls; tl; tl = tl->next)
            tl->button.width = maxwidth - BUTTON_XPAD;
    ncols = width / maxwidth;
    if (!ncols) {
        fprintf(stderr, "Help, button too big!!\n");
        return;
    }
    if (ncols > nbuts)
        ncols = nbuts;
    maxwidth = width / ncols;
    nrows = nbuts / ncols;
    if (nrows * ncols < nbuts)
        nrows++;
    
    for (tl = tls, x = y = 0; tl; tl = tl->next) {
        if (x == ncols) {
            fprintf(stderr, "Ack, too many buttons\n");
            return;
        }
        if (hlp_buttonstyle == BS_CENTER)
            tl->button.x = x * maxwidth + BUTTON_XPAD + (maxwidth -
                    tl->button.width) / 2;
        else
            tl->button.x = x * maxwidth + BUTTON_XPAD;
        tl->button.y = y * (buttonfont->height + 6 + BUTTON_YPAD) +
                BUTTON_YPAD;
        if (++y == nrows) {
            y = 0;
            x++;
        }
    }
    return;
}

#define sq_overlap(x1, y1, w1, h1, x2, y2, w2, h2)      \
    (!((((x1) < (x2)) && ((x1) + (w1) < (x2)) ||        \
    ((x1) > (x2) + (w2)) && ((x1) + (w1) > (x2) + (w2))) || \
    (((y1) < (y2)) && ((y1) + (h1) < (y2)) ||       \
    ((y1) > (y2) + (h2)) && ((y1) + (h1) > (y2) + (h2)))))
    

static void
redraw(top, x, y, w, h)
    topic *top;
    int x, y, w, h;
{
    int y1 = (y - titlefont->height) / lineheight - 1;
    int y2 = (y + h - titlefont->height) / lineheight + 1;
    toplink *tl;

    if (y < titlefont->height) {
        puttitle(top);
        drawbutton(top, &top->but_quit);
        drawbutton(top, &top->but_delete);
        if (top->numlines > MAX_LINES) {
            drawbutton(top, &top->but_next);
            drawbutton(top, &top->but_prev);
        }
    }
    drawtext(top, y1, y2);
    for (tl = top->subtopics; tl; tl = tl->next)
        if (sq_overlap(x, y, w, h, tl->button.x, tl->button.y,
                tl->button.width, tl->button.height))
            drawbutton(top, &tl->button);
    for (tl = top->seealso; tl; tl = tl->next)
        if (sq_overlap(x, y, w, h, tl->button.x, tl->button.y,
                tl->button.width, tl->button.height))
            drawbutton(top, &tl->button);
    if (top->subtopics)
        XText(top->win, INT_BORDER, top->sublabypos,
            SUB_TOPICS, strlen(SUB_TOPICS),
            titlefont->id, BlackPixel, WhitePixel);
    if (top->seealso)
        XText(top->win, INT_BORDER, top->salabypos,
            SEE_ALSO, strlen(SEE_ALSO), 
            titlefont->id, BlackPixel, WhitePixel);
    return;
}

void
hlp_xwait(top, on)
    topic *top;
    bool on;
{
    XDefineCursor(top->win, on ? waitcur : cursor);
    XFlush();
    return;
}

#else

#ifndef HAS_X11
/* ARGSUSED */ bool hlp_xdisplay(top) topic *top; { return (false); }
/* ARGSUSED */ void hlp_xkillwin(top) topic *top; { }
#endif

/* ARGSUSED */ void hlp_xwait(top, on) topic *top; bool on; { }
void hlp_xclosedisplay() {}
toplink * hlp_xhandle(pp) topic **pp; { *pp = NULL; return (NULL); }

#endif

