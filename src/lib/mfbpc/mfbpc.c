/**********
Copyright 1991 Regents of the University of California.  All rights reserved.
**********/

/*
 * Frame buffer for the IBM PC using MSC graphics library interface.
 */

#include "spice.h"
#include "util.h"
#include "mfb.h"
#include <dos.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <string.h>
#include <graph.h>
#include <graph.h>

#include "suffix.h"

#define print(S, C) {							      \
	in.h.ah = 0x05;							      \
	for (count = C, string = S; count > 0; count -= 1) {		      \
		in.h.dl = *string++;					      \
		int86(0x21, &in, &in);					      \
	}								      \
}

#define FINI_PRINTER "\033@\n\r\n"

static char *mfbpc_buffer;
static int mfbpc_height;
static int mfbpc_width;
static int mfbpc_double;

MFB *MFBCurrent;

static short config_try[ ] = {
	_VRES16COLOR,
	_VRES2COLOR,
	_ERESCOLOR,
	_ERESNOCOLOR,
	_HRESBW
	};

struct videoconfig config;

static unsigned short line_styles[ ] = {
	0xffff, 0xaaaa, 0x7777, 0xcccc,
	0xf0f0, 0xfc10, 0x7575, 0x3f11
	};

unsigned char c_map[95][7] = {
    { 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x20 },
    { 0x50, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x50, 0xf8, 0x50, 0x50, 0xf8, 0x50 },
    { 0x70, 0xa8, 0xa0, 0x70, 0x28, 0xa8, 0x70 },
    { 0xf8, 0xc8, 0x10, 0x20, 0x40, 0x98, 0x98 },
    { 0x40, 0xa0, 0xa0, 0x40, 0xa8, 0x90, 0x68 },
    { 0x60, 0x60, 0xc0, 0x00, 0x00, 0x00, 0x00 },
    { 0x08, 0x10, 0x20, 0x20, 0x20, 0x10, 0x08 },
    { 0x80, 0x40, 0x20, 0x20, 0x20, 0x40, 0x80 },
    { 0x20, 0xa8, 0x70, 0x20, 0x70, 0xa8, 0x20 },
    { 0x00, 0x20, 0x20, 0xf8, 0x20, 0x20, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x60, 0x60, 0xc0 },
    { 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x60 },
    { 0x00, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00 },
    { 0x70, 0x88, 0x98, 0xa8, 0xc8, 0x88, 0x70 },
    { 0x20, 0x60, 0x20, 0x20, 0x20, 0x20, 0x70 },
    { 0x70, 0x88, 0x08, 0x10, 0x20, 0x40, 0xf8 },
    { 0x70, 0x88, 0x08, 0x30, 0x08, 0x88, 0x70 },
    { 0x10, 0x30, 0x50, 0x90, 0xf8, 0x10, 0x10 },
    { 0xf8, 0x80, 0xf0, 0x08, 0x08, 0x88, 0x70 },
    { 0x30, 0x40, 0x80, 0xf0, 0x88, 0x88, 0x70 },
    { 0xf8, 0x08, 0x10, 0x10, 0x20, 0x20, 0x20 },
    { 0x70, 0x88, 0x88, 0x70, 0x88, 0x88, 0x70 },
    { 0x70, 0x88, 0x88, 0x78, 0x08, 0x10, 0x60 },
    { 0x00, 0x00, 0x60, 0x60, 0x00, 0x60, 0x60 },
    { 0x00, 0x00, 0x60, 0x60, 0x00, 0x60, 0x60 },
    { 0x00, 0x10, 0x20, 0x40, 0x20, 0x10, 0x00 },
    { 0x00, 0x00, 0xf8, 0x00, 0xf8, 0x00, 0x00 },
    { 0x00, 0x40, 0x20, 0x10, 0x20, 0x40, 0x00 },
    { 0x70, 0x88, 0x10, 0x20, 0x20, 0x00, 0x20 },
    { 0x70, 0x88, 0xb8, 0xa8, 0xb8, 0x80, 0x70 },
    { 0x70, 0x88, 0x88, 0xf8, 0x88, 0x88, 0x88 },
    { 0xf0, 0x88, 0x88, 0xf0, 0x88, 0x88, 0xf0 },
    { 0x70, 0x88, 0x80, 0x80, 0x80, 0x88, 0x70 },
    { 0xf0, 0x48, 0x48, 0x48, 0x48, 0x48, 0xf0 },
    { 0xf8, 0x80, 0x80, 0xf0, 0x80, 0x80, 0xf8 },
    { 0xf8, 0x80, 0x80, 0xf0, 0x80, 0x80, 0x80 },
    { 0x70, 0x88, 0x80, 0x80, 0x98, 0x88, 0x70 },
    { 0x88, 0x88, 0x88, 0xf8, 0x88, 0x88, 0x88 },
    { 0x70, 0x20, 0x20, 0x20, 0x20, 0x20, 0x70 },
    { 0x08, 0x08, 0x08, 0x08, 0x08, 0x88, 0x70 },
    { 0x88, 0x90, 0xa0, 0xc0, 0xa0, 0x90, 0x88 },
    { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xf8 },
    { 0x88, 0xd8, 0xa8, 0x88, 0x88, 0x88, 0x88 },
    { 0x88, 0x88, 0xc8, 0xa8, 0x98, 0x88, 0x88 },
    { 0x70, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70 },
    { 0xf0, 0x88, 0x88, 0xf0, 0x80, 0x80, 0x80 },
    { 0x70, 0x88, 0x88, 0x88, 0xa8, 0x90, 0x68 },
    { 0xf0, 0x88, 0x88, 0xf0, 0xa0, 0x90, 0x88 },
    { 0x70, 0x88, 0x80, 0x70, 0x08, 0x88, 0x70 },
    { 0xf8, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 },
    { 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70 },
    { 0x88, 0x88, 0x88, 0x88, 0x50, 0x50, 0x20 },
    { 0x88, 0x88, 0x88, 0x88, 0xa8, 0xd8, 0x88 },
    { 0x88, 0x88, 0x50, 0x20, 0x50, 0x88, 0x88 },
    { 0x88, 0x88, 0x50, 0x20, 0x20, 0x20, 0x20 },
    { 0xf8, 0x08, 0x10, 0xf8, 0x40, 0x80, 0xf8 },
    { 0x38, 0x20, 0x20, 0x20, 0x20, 0x20, 0x38 },
    { 0x00, 0x80, 0x40, 0x20, 0x10, 0x08, 0x00 },
    { 0xe0, 0x20, 0x20, 0x20, 0x20, 0x20, 0xe0 },
    { 0x20, 0x50, 0x88, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x30, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x70, 0x08, 0x78, 0x88, 0x78 },
    { 0x80, 0x80, 0xf0, 0x88, 0x88, 0x88, 0xf0 },
    { 0x00, 0x00, 0x70, 0x88, 0x80, 0x80, 0x78 },
    { 0x08, 0x08, 0x78, 0x88, 0x88, 0x88, 0x78 },
    { 0x00, 0x00, 0x70, 0x88, 0xf0, 0x80, 0x70 },
    { 0x30, 0x48, 0x40, 0xe0, 0x40, 0x40, 0x40 },
    { 0x00, 0x70, 0x88, 0x88, 0x78, 0x08, 0x70 },
    { 0x80, 0x80, 0xf0, 0x88, 0x88, 0x88, 0x88 },
    { 0x20, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20 },
    { 0x08, 0x00, 0x08, 0x08, 0x08, 0x88, 0x70 },
    { 0x80, 0x80, 0x88, 0x90, 0xe0, 0x90, 0x88 },
    { 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 },
    { 0x00, 0x00, 0xd0, 0xa8, 0xa8, 0xa8, 0xa8 },
    { 0x00, 0x00, 0xb0, 0xc8, 0x88, 0x88, 0x88 },
    { 0x00, 0x00, 0x70, 0x88, 0x88, 0x88, 0x70 },
    { 0x00, 0xf0, 0x88, 0x88, 0xf0, 0x80, 0x80 },
    { 0x00, 0x70, 0x88, 0x88, 0x78, 0x08, 0x08 },
    { 0x00, 0x00, 0xb0, 0xc8, 0x80, 0x80, 0x80 },
    { 0x00, 0x00, 0x78, 0x80, 0x70, 0x08, 0xf0 },
    { 0x20, 0x20, 0xf8, 0x20, 0x20, 0x20, 0x18 },
    { 0x00, 0x00, 0x88, 0x88, 0x88, 0x88, 0x70 },
    { 0x00, 0x00, 0x88, 0x88, 0x88, 0x50, 0x20 },
    { 0x00, 0x00, 0x88, 0x88, 0xa8, 0xa8, 0x50 },
    { 0x00, 0x00, 0x88, 0x50, 0x20, 0x50, 0x88 },
    { 0x00, 0x88, 0x88, 0x50, 0x20, 0x40, 0x80 },
    { 0x00, 0x00, 0xf8, 0x10, 0x70, 0x40, 0xf8 },
    { 0x08, 0x10, 0x10, 0x20, 0x10, 0x10, 0x08 },
    { 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 },
    { 0x80, 0x40, 0x40, 0x20, 0x40, 0x40, 0x80 },
    { 0x68, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8 }
    };

static initialized = 0, graphics_mode_on = 0;
static short graphics_mode = 0;
static void (interrupt far *hc_vect)( );
static void (interrupt far mfbpc_hardcopy)( );


#define NSTYLES	(NUMELEMS(line_styles))

MFB *
MFBOpen(name, device, error)
    char *name, *device;
    int *error;
{
    static MFB	onlyone;
    MFB		*thismfb;
    int		i;

    thismfb = &onlyone;

    MFBCurrent = thismfb;

    thismfb->fileDesc = fileno(stdout);

    if (!initialized) {
	for (i = 0; i < NUMELEMS(config_try); i++) {
	    if (_setvideomode(config_try[i])) {
		    break;
	    }
	}

	if (i < NUMELEMS(config_try)) {
	    initialized = 1;
#ifdef WANT_PCHARDCOPY
	    hc_vect = _dos_getvect(5);
#endif
	    _getvideoconfig(&config);

	    mfbpc_height = config.numypixels;
	    mfbpc_width = config.numxpixels;
	    if (mfbpc_height < 240) {
		mfbpc_double = 1;
		mfbpc_buffer = malloc(mfbpc_height + mfbpc_height + 50);
	    } else {
		mfbpc_double = 0;
		mfbpc_buffer = malloc(mfbpc_height + 50);
	    }

	    graphics_mode = config_try[i];
	    _setvideomode(_DEFAULTMODE);
	    /*pc_init_hardcopy(&config);*/
	} else {
	    printf("Can't initialize graphics.\n");
	    if (error)
		*error = MFBBADOPT;
	    return 0;
	}
    }

    if (error)
        *error = MFBOK;

    return (thismfb);
}

int
MFBInitialize( )
{
    _setvideomode(graphics_mode);
    _clearscreen(_GCLEARSCREEN);
#ifdef WANT_PCHARDCOPY
    _dos_setvect(5, mfbpc_hardcopy);
#endif
    return (MFBOK);
}

MFBClearScreen( )
{
    _clearscreen(_GCLEARSCREEN);
    _settextposition(1, 1);
}

MFBHome( )
{
    _settextposition(1, 1);
}

MFBClearEol( )
{
    printf("            ");
}

MFBSetColor(color)
	int	color;
{

    if (color > 0) {
	color = (color % (config.numcolors - 1)) + 1;
	_setcolor((short) (config.numcolors - color));
    } else
	_setcolor(0);

    return (MFBOK);
}

/* Don't ever want to do this */

int
MFBFlood()
{
    return (MFBOK);
}

int
MFBSetTextMode(dest)
{
    return (MFBOK);
}

int
MFBSetALUMode(mode)
{
    return (MFBOK);
}

int
MFBSetLineStyle(style)
{
    _setlinestyle(line_styles[style % NSTYLES]);
    return (MFBOK);
}

int
MFBLine(fromx, fromy, tox, toy)
	int fromx, fromy;
	int tox, toy;
{
    _moveto((short) fromx, (short) (config.numypixels - fromy));
    _lineto((short) tox, (short) (config.numypixels - toy));
    return (MFBOK);
}

int
MFBText(text, x, y, degrees)
    char *text;
    int x, y, degrees;
{
    char	c, *s;
    short	x0, y0;
    int		i, j;
    unsigned char	bits, p;

    y = (config.numypixels - y) - 8;
    for (s = text; s && *s; s++) {
	if (*s > 127)
		*s -= 128;
	c = *s - 33;
	if (c >= 0 && c < 96) {
	    y0 = y;
	    for (i = 0; i < 7; i++) {
		x0 = x;
		p = 0x80;
		bits = c_map[c][i];
		for (j = 0; j < 7; j++) {
		    if (p & bits)
			_setpixel(x0, y0);
		    p = p >> 1;
		    x0 += 1;
		}
		y0 += 1;
	    }
	}
	if (!degrees)
	    x += 7;
	else
	    y += 8;
    }

#ifdef notdef
    short	tx, ty;

    if (!text)
	return MFBOK;

    tx = x * config.numtextcols / config.numxpixels;
    ty = (config.numypixels - y) * config.numtextrows / config.numypixels;
    _settextposition(ty, tx);
    _outtext(text);

    printf("\"%s\" @ %d,%d // %d\n", text, x, y, degrees);
    if (!degrees)
	    _outtext(text);
    else {
	static char lb[ ] = "x\n\0";
	struct rccoord curp;
	for (s = text; *s; s++) {
	    lb[0] = *s;
	    _outtext(lb);
	    curp = _gettextposition( );
	    _settextposition((short) x, (short) curp.col);
	}
    }
#endif

    return (MFBOK);
}

int
MFBDrawLineTo(x, y)
	int x, y;
{
    _lineto((short) x, (short) (config.numypixels - y));
    return (MFBOK);
}

int
MFBUpdate()
{
    /* Do nothing */
    return (MFBOK);
}

int
MFBBox(left, bottom, right, top)
{
    /* We don't use this in nutmeg, really. */
    return (MFBOK);
}

int
MFBDefineColor(red, green, blue, num)
{
    /* No-op. */
    return (MFBOK);
}

int
MFBDefineLineStyle(num, mask)
{
    /*
    if ((num < 0) || (num >= NSTYLES))
        return (MFBBADLST);
    styles[num] = mask;
    */
    return (MFBOK);
}

int
MFBClose()
{
    int i;

    /* Reset the hardcopy interrupt vector */
#ifdef WANT_PCHARDCOPY
    _dos_setvect(5, hc_vect);
#endif

    /* Reset the graphics mode */
    _setvideomode(_DEFAULTMODE);

    graphics_mode_on = 0;

#ifdef notdef
    printf("x = %d, y = %d, colors = %d, def = %d\n",
	(int) config.numxpixels, (int) config.numypixels,
	(int) config.numcolors, (int) graphics_mode);
    printf("cx = %d, cy = %d\n", config.numtextrows, config.numtextcols);
#endif

    return (MFBOK);
}

int
MFBHalt()
{
    return (MFBClose());
}

/* Print a MFB error string... */

static struct errinfo {
    int er_num;
    char *er_string;
} errstrings[] = {
    { MFBOK,        "" } ,
    { MFBBADENT,        "Unknown terminal type" } ,
    { MFBBADMCE,        "Bad mfbcap entry" } ,
    { MFBBADLST,        "Illegal line style" } ,
    { MFBBADFST,        "Illegal fill style" } ,
    { MFBBADCST,        "Illegal color style" } ,
    { MFBBADTM1,        "No destructive text mode" } ,
    { MFBBADTM2,        "No overstrike text mode" } ,
    { MFBNODFLP,        "No definable line styles" } ,
    { MFBNODFFP,        "No definable fill styles" } ,
    { MFBNODFCO,        "No definable colors" } ,
    { MFBNOBLNK,        "No blinkers" } ,
    { MFBBADDEV,        "Can't open or close device" } ,
    { MFBBADOPT,        "Can't access or set device status" } ,
    { MFBNOMASK,        "No definable read or write mask" } ,
    { MFBBADWRT,        "Error in write" } ,
    { MFBNOPNT,     "No pointing device" } ,
    { MFBNOMEM,     "Out of memory" } ,
    { 0,            NULL }
} ;

char *
MFBError(num)
{
    struct errinfo *ei;
    static char buf[32];

    for (ei = errstrings; ei->er_string; ei++)
        if (ei->er_num == num)
            break;
    if (ei->er_string)
        return (ei->er_string);
    else {
        sprintf(buf, "Unknown error # %d", num);
        return (buf);
    }
}

/* The mfb struct will not contain anything valid. */

int
MFBInfo(num)
{
    int	result;
    int goback = 0;

    if (!initialized) {
	MFBOpen( );
	goback = 1;
    }

    switch (num) {
        case MAXX:
            result = config.numxpixels;
	    break;
        case MAXY:
            result = config.numypixels;
	    break;
        case MAXCOLORS:
            result = config.numcolors - 1;
	    break;
        case MAXLINESTYLES:
            result = NSTYLES;
	    break;
        case TEXTROTATABLE:
            result = 0; /* It can, but it's too ugly now... */
	    break;
        case FONTHEIGHT:
	    result = 8;
	    break;
        case FONTWIDTH:
	    result = 7;
	    break;
        case DEFLINEPATTERN:
            result = 1;
	    break;
        case DESTRUCTIVETEXT:
            result = 1;
	    break;
        case OVERSTRIKETEXT:
            result = 0;
	    break;

        default:
            result = -1;
    }

    if (goback)
	MFBClose( );

    return result;
}

/*ARGSUSED*/
void
MFBArc(x,y,r,astart,astop,s)
    int x;      /* x coordinate of center */
    int y;      /* y coordinate of center */
    int r;      /* radius of arc */
    int astart;       /* initial angle ( +x axis = 0 degrees ) */
    int astop;  /* final angle ( +x axis = 0 degrees ) */
    int s;      /* number of segments in a 360 degree arc */
    /*
     * Notes:
     *    Draws an arc of radius r and center at (x,y) beginning at
     *    angle astart (in degrees) and ending at astop
     */
{
    short	left, right, top, bottom;
    short	x0, y0, x1, y1;

    y = config.numypixels - y;

    left = x - r;
    right = x + r;
    top = y - r;
    bottom = y + r;

    x0 = x + r * cos((double) astart * M_PI / 180.0);
    y0 = x + r * cos((double) astart * M_PI / 180.0);
    x1 = x + r * cos((double) astop * M_PI / 180.0);
    y1 = x + r * cos((double) astop * M_PI / 180.0);

    _arc(left, top, right, bottom, x0, y0, x1, y1);

}

void (interrupt far mfbpc_hardcopy)( )
{
    static char	*string;
    static int	count;
    static int	i, j, k, l;
    static char	*buf;
    static int	groupwidth;
    static unsigned short a;
    static union REGS in;

    print("\r\n\0333\030", 5);

    mfbpc_buffer[0] = '\r';
    mfbpc_buffer[1] = '\n';
    mfbpc_buffer[2] = '\033';
    mfbpc_buffer[3] = '\K';
    if (!mfbpc_double) {
	mfbpc_buffer[4] = mfbpc_height % 256;
	mfbpc_buffer[5] = mfbpc_height / 256;
    } else {
	mfbpc_buffer[4] = (2 * mfbpc_height) % 256;
	mfbpc_buffer[5] = (2 * mfbpc_height) / 256;
    }

    buf = mfbpc_buffer + 6;

    for (i = 0; i < mfbpc_width; i += 8) {
	groupwidth = mfbpc_width - i;
	if (groupwidth > 8)
	    groupwidth = 8;
	if (mfbpc_double) {
            for (j = 0; j < mfbpc_height + mfbpc_height; j += 1)
	        buf[j] = 0;
	} else {
            for (j = 0; j < mfbpc_height; j += 1)
	        buf[j] = 0;
	}
	for (j = 0; j < groupwidth; j += 1) {
	    l = 0;
	    for (k = 0; k < mfbpc_height; k += 1) {
		a = _getpixel((short) mfbpc_width - i - j - 1, (short) k);
		if (a)
		    buf[l] |= 0x80 >> j;
		if (mfbpc_double) {
		    l += 1;
		    if (a) {
			buf[l] |= 0x80 >> j;
		    }
		}
		l += 1;
	    }
	}

	if (!mfbpc_double) {
	    print(mfbpc_buffer, mfbpc_height + 6);
	} else {
	    print(mfbpc_buffer, mfbpc_height + mfbpc_height + 6);
	}
    }

    print(FINI_PRINTER, sizeof(FINI_PRINTER) - 1);

}
