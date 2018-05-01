/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1982 Giles C. Billingsley, Kenneth H. Keller
Author: 1985 Wayne A. Christopher
**********/

#ifndef HAS_PCTERM
/*
 * mfb.h
 *
 * sccsid "@(#)mfb.h    1.9  9/3/83"
 *
 *     MFB is a graphics package that was developed by the integrated
 * circuits group of the Electronics Research Laboratory and the
 * Department of Electrical Engineering and Computer Sciences at
 * the University of California, Berkeley, California.  The programs
 * in MFB are available free of charge to any interested party.
 * The sale, resale, or use of these program for profit without the
 * express written consent of the Department of Electrical Engineering
 * and Computer Sciences, University of California, Berkeley, California,
 * is forbidden.
 */
 
#ifdef vms

#include <types.h>
#include <timeb.h>
#include <ttdef.h>

#else

#include <sys/types.h>

#ifdef HAS_FTIME
#include <sys/timeb.h>
#endif

#ifdef HAS_STAT
#include <sys/stat.h>
#endif

#ifdef HAS_BSDTTY
#include <sgtty.h>
#else
#ifdef HAS_SYSVTTY
#include <termio.h>
#else
#ifdef HAS_POSIXTTY
#include <termios.h>
#endif
#endif
#endif

#endif

#ifdef HAS_UNIX_SIGS
#include <signal.h>
#endif

#include <stdio.h>

/*
 * By define DEBUG we enable the MFBCounters routine for measuring bandwidth
 */
#define DEBUG

#define BUFSIZE 4096
#define UNINITIALIZED -1
#define MFBPOLYGONBUFSIZE 600
#define MFBMAXPOLYGONVERTICES 300
#define MFBmax(a,b) (a > b ? a : b)
#define MFBmin(a,b) (a < b ? a : b)
#define MFBSwapInt(f1,f2) {int f3; f3 = f1; f1 = f2; f2 = f3;}

#define TTY 't'
#define HCOPY   'r'

#ifdef vms
#define TTYBUFSIZE 800
#else
#define TTYBUFSIZE 4096
#endif
#ifdef vms
#define void int
#endif

char gEntBuf[BUFSIZE];

/************************************************************************/
/*                                  */
/*           MFB structure definition.          */
/*                                  */
/************************************************************************/

#ifndef BOOL_ALREADY_TYPEDEFED  /* X has a typedef Bool also */
typedef enum {false, true} Bool;
#endif /*not BOOL_ALREADY_TYPEDEFED*/

struct mfb_window {
    int left;
    int right;
    int top;
    int bottom;
    double length,width;
    };

typedef struct mfb_window WINDOW;
typedef struct mfb_window VIEWPORT;

struct mfbpath {
    int nvertices;
    int *xy;
    };

typedef struct mfbpath MFBPOLYGON;
typedef struct mfbpath MFBPATH;

#ifndef vms
struct mfbremttyb {

#ifdef HAS_BSDTTY
    struct sgttyb oldttyb;
    struct sgttyb newttyb;
#else
#ifdef HAS_SYSVTTY
    struct termio oldttyb;
    struct termio newttyb;
#else
#ifdef HAS_POSIXTTY
    struct termios oldttyb;
    struct termios newttyb;
#else
    int oldttyb, newttyb;
#endif
#endif
#endif

    };

struct mfbremstat {
    int graphttyw;      /* old mode bits of graphics device */
    int kybrdttyw;      /* old mode bits of standard input/output */
    struct stat graphstat;  /* old fstats of graphics device */
    struct stat kybrdstat;  /* old fstats of standard input/output */
    };

typedef struct mfbremttyb MFBSAVETTYB;
typedef struct mfbremstat MFBSAVESTAT;
#endif

struct mfbformatstrs {
    char *startSequence;        /* first transmitted sequence */
    char *endSequence;          /* last transmitted sequence */
    char *initLineStyles;       /* initialize line styles */
    char *initFillPatterns;     /* initialize fill styles */
    char *initColorStyles;      /* initialize color styles */

    char *vltEntry;         /* define color in VLT */
    char *setForegroundColor;       /* set current foreground color */
    char *screenFlood;          /* flood screen to current color */

    char *channelMaskSet;       /* set write mask */
    char *readMaskSet;          /* set read mask */

    char *enablePointingDevice;     /* initialize pointing device */
    char *enableRubberBanding;      /* turn on rubber banding */
    char *disablePointingDevice;    /* disable pointing device and cursor */
    char *disableRubberBanding;     /* turn off rubber banding */
    char *readPointingDevice;       /* wait and read pointing device */
    char *formatPointingDevice;     /* decode format for pointing device */

    char *keyboardStart;        /* initailize keyboard */
    char *keyboardEnd;          /* terminate keyboard input */
    char *keyboardBackspace;        /* keyboard backspace sequence */
    char *audio;            /* ring the terminals bell */

    char *lineDefineStart;      /* begin defining a line pattern */
    char *lineDefineFormat;     /* define bit array of line pattern */
    char *lineDefineEnd;        /* terminate line pattern definition */
    char *setLineStyle;         /* set current line style */
    char *setSolidLineStyle;        /* set current line style to solid */
    char *movePenSequence;      /* move current graphics position */
    char *drawLineSequence;     /* draw a line in current style */
    char *drawSolidLineSequence;    /* draw a solid line */
    char *drawLineToSequence;       /* move and draw current position */
    char *drawSolidLineToSequence;  /* move and draw solid line */

    char *drawBoxSequence;      /* draw box in current style */
    char *drawSolidBoxSequence;     /* draw a solid box */

    char *beginPlygnSequence;       /* begin polygon in cur. fill style */
    char *beginSolidPlygnSequence;  /* begin solid polygon */
    char *sendPlygnVertex;      /* define one point in polygon */
    char *endPlygnSequence;     /* terminate polygon sequence */

    char *drawCircleSequence;       /* draw a circle in solid line style */

    char *rotateTextSequence;       /* rotate graphic text */
    char *graphicsTextStart;        /* begin graphic text */
    char *graphicsTextEnd;      /* terminate graphic text */
    char *replaceON;            /* turn on destructive text mode */
    char *overstrikeON;         /* turn on overstriking text mode */
    char *writePixel;           /* write one pixel in current color */

    char *setALUEOR;            /* set ALU mode to EOR */
    char *setALUNOR;            /* set ALU mode to NOR */
    char *setALUOR;         /* set ALU mode to OR */
    char *setALUJAM;            /* set ALU mode to JAM or REPLACE */

    char *blinkerON;            /* make a color blink */
    char *blinkerOFF;           /* turn off a blinking layer */

    char *rastCopyStart;        /* begin raster copy sequence */
    char *rastCopyEnd;          /* terminate raster copy sequence */
    char *rastCopyDest;         /* define raster copy destionation */
    char *rastCopySource;       /* define raster copy source area */

    char *fillDefineStart;      /* begin defining a fill style */
    char *fillDefineFormat;     /* define bit array of row/column */
    char *fillDefineEnd;        /* terminate fill style definition */
    char *setFillPattern;       /* set current fill pattern */
    char *setSolidFillPattern;      /* set current fill pattern to solid */
    };


struct mfb {

            /*
             * INTEGERS FIELDS
             */
/* used by mfbgncode */ int lastX,lastY;    /* for Tektronix encoding */
            int X,Y,Z,T;        /* parameter list */

/* Resolution */    int maxX;       /* horizontal resolution */
            int maxY;       /* vertical resolution */
            int maxColors;      /* maximum number of colors */
            int minOffScreenX;  /* left of off screen memory */
            int minOffScreenY;  /* bottom of off screen mem. */
            int offScreenDX;    /* length of off screen mem. */
            int offScreenDY;    /* width of off screen mem. */

/* Video Layer Table */ int maxIntensity;   /* max RGB or LS intensity */
            int lengthOfVLT;    /* number of bit planes */

/* Pointing Device */   int buttonMask[12]; /* returned button masks */
            int numberOfButtons;    /* 12 maximum */

/* Keyboard Control */  int keyboardYOffset;
            int keyboardXOffset;

/* Line Drawing */  int lineDefineLength;   /* number of bytes in array */
            int maxLineStyles;  /* number of line styles */

/* Text font */     int fontHeight;     /* font height in pixels */
            int fontWidth;      /* font width in pixels */
            int fontXOffset;
            int fontYOffset;

/* Blinkers */      int maxBlinkers;    /* number of blinkers */

/* Fill Patterns */ int fillDefineHeight;   /* number of byte rows */
            int fillDefineWidth;    /* number of byte columns */
            int maxFillPatterns;    /* number of fill patterns */

/* Current variables */ int cursorColor1Id; /* blinked cursor color ID */
            int cursorColor2Id; /* unblinked cursor color ID */
                int fgColorId;      /* cur. foreground color ID */
            int fillPattern;    /* cur. fill pattern ID */
            int lineStyle;      /* cur. line style ID */
            int channelMask;    /* cur. write mask */
            int readMask;       /* cur. read mask */
            int textMode;       /* text mode (1=dest, 0=rep) */
            int numBlinkers;    /* cur. number of blinkers */
            int stipplePattern[8];  /* cur. stipple pattern */

            int fileDesc;       /* desc. for graphics device */
            int numTtyBuffer;   /* cur. chars in output queue */

#ifdef DEBUG
/* bandwidth test */    int nChars;
            int nBoxes,sumBoxArea;
            int nLines,sumLineLength;
#endif


                /*
             * POINTERS
             */
/* for tty's only */    char *name;     /* device name */
            char *terminalName; /* name of MFBCAP entry */

/* I/O routines */  int (*dsply_getchar)();
            int (*kybrd_getchar)();
            int (*dsply_ungetchar)();
            int (*kybrd_ungetchar)();
            int (*outchar)();
            int (*outstr)();


            /*
             * BOOLEANS
             */
                Bool initializedBool;

/* Video Layer Table */ Bool vltBool;
            Bool vltUseHLSBool;

/* Channel Mask */  Bool channelMaskBool;
            Bool readMaskBool;

/* Pointing Device */   Bool PointingDeviceBool;
            Bool buttonsBool;
            Bool readImmediateBool;

/* Keyboard Control */  Bool keyboardBool;

/* Line Drawing */  Bool linePatternDefineBool;
            Bool reissueLineStyleBool;

/* Polygon Drawing */   Bool filledPlygnBool;

/* Text font */     Bool textPositionableBool;
            Bool textRotateBool;
            Bool replaceTextBool;
            Bool overstrikeTextBool; 

/* Blinkers */      Bool blinkersBool;

/* Raster Copy */   Bool rastCopyBool;
            Bool rastRSCSFBool;

/* Fill Patterns */ Bool fillPtrnDefineBool;
            Bool fillDefineRowMajorBool;

/* for tty's only */    Bool litout;
            Bool raw;
            Bool ttyIsSet;


    /*
     * STRUCTURES
     */
    struct  mfbformatstrs strings;      /* format strings */

    WINDOW  currentWindow;          /* current window */
    VIEWPORT    currentViewport;        /* current viewport */

#ifndef vms
    /* graphics device ttyb struct */
    MFBSAVETTYB graphTtyb;

    /* keyboard ttyb struct if graphics device does not have a kybrd */
    MFBSAVETTYB kybrdTtyb;

    /* tty status ints */
    MFBSAVESTAT oldstat;
#endif


    /*
     * CHARACTERS
     */
    char deviceType;            /* TTY=tty, HCOPY=hard copy */
    char strBuf[BUFSIZE];       /* storage for format strings */
    char ttyBuffer[TTYBUFSIZE];     /* tty output buffer */
    };

typedef struct mfb MFB;

extern MFB *MFBCurrent;         /* global FB structure */

extern struct err {
    int errnum;
    char    *message;
        };

/* Define Macros for general program usage. */
#define TRUE 1
#define FALSE 0
#define ON 1
#define OFF 0


/* MFBSetALUMode defines */
#define MFBALUJAM   0
#define MFBALUOR    1
#define MFBALUNOR   2
#define MFBALUEOR   3


/* ERROR DEFINES */
#define MFBOK       1   /* Successful return */
#define MFBBADENT   -10 /* Unknown terminal type */
#define MFBBADMCF   -20 /* Can't open mfbcap file */
#define MFBMCELNG   -30 /* MFBCAP entry too long */
#define MFBBADMCE   -40 /* Bad mfbcap entry */
#define MFBINFMCE   -50 /* Infinite mfbcap entry */
#define MFBBADTTY   -60 /* stdout not in /dev */
#define MFBBADLST   -70 /* Illegal line style */
#define MFBBADFST   -80 /* Illegal fill style */
#define MFBBADCST   -90 /* Illegal color style */
#define MFBBADTM1   -100    /* No destructive text */
#define MFBBADTM2   -110    /* No overstriking text */
#define MFBNODFLP   -120    /* No definable line styles */
#define MFBNODFFP   -130    /* No definable fill styles */
#define MFBNODFCO   -140    /* No definable colors */
#define MFBNOBLNK   -150    /* No blinkers */
#define MFBTMBLNK   -160    /* Too many blinkers */
#define MFBBADDEV   -180    /* Can't open or close device */
#define MFBBADOPT   -190    /* Can't access or set device stat */
#define MFBNOMASK   -170    /* No definable read or write mask */
#define MFBBADWRT   -200    /* Error in write */
#define MFBPNTERR   -210    /* Error in pointing device */
#define MFBNOPTFT   -220    /* No format for pointing device */
#define MFBNOPNT    -230    /* No pointing device */
#define MFBNORBND   -240    /* No Rubberbanding */
#define MFBBADALU   -250    /* Cannot set ALU mode */


/*
 *  defines for MFBInfo
 */
#define MAXX            1   /* max x coordinate */
#define MAXY            2   /* max y coordinate */
#define MAXCOLORS       3   /* max number of colors */
#define MAXINTENSITY        4   /* max color intensity */
#define MAXFILLPATTERNS     5   /* max number of fill patterns */
#define MAXLINESTYLES       6   /* max number of line styles */
#define MAXBLINKERS     7   /* max number of blinkers */
#define POINTINGDEVICE      8   /* Bool: terminal has pointing device */
#define POINTINGBUTTONS     9   /* Bool: pointing device has buttons */
#define NUMBUTTONS      10  /* number of pointing device buttons */
#define BUTTON1         11  /* button value returned by button 1 */
#define BUTTON2         12  /* button value returned by button 2 */
#define BUTTON3         13  /* button value returned by button 3 */
#define BUTTON4         14  /* button value returned by button 4 */
#define BUTTON5         15  /* button value returned by button 5 */
#define BUTTON6         16  /* button value returned by button 6 */
#define BUTTON7         17  /* button value returned by button 7 */
#define BUTTON8         18  /* button value returned by button 8 */
#define BUTTON9         19  /* button value returned by button 9 */
#define BUTTON10        20  /* button value returned by button 10 */
#define BUTTON11        21  /* button value returned by button 11 */
#define BUTTON12        22  /* button value returned by button 12 */
#define TEXTPOSITIONALBE    30  /* Bool: accurately positionable text */
#define TEXTROTATABLE       31  /* Bool: rotateable text */
#define FONTHEIGHT      32  /* font height in pixels */
#define FONTWIDTH       33  /* font width in pixels */
#define FONTXOFFSET     34  /* font x offset in pixels */
#define FONTYOFFSET     35  /* font y offset in pixels */
#define DESTRUCTIVETEXT     36  /* Bool: text can be destructive */
#define OVERSTRIKETEXT      37  /* Bool: text can be overstrike */
#define VLT         38  /* Bool: terminal has VLT */
#define BLINKERS        39  /* Bool: terminal has blinkers */
#define FILLEDPOLYGONS      40  /* Bool: terminal has filled polygons */
#define DEFFILLPATTERNS     41  /* Bool: defineable fill patterns */
#define DEFCHANNELMASK      42  /* Bool: defineable write mask */
#define DEFLINEPATTERN      43  /* Bool: defineable line styles */
#define CURFGCOLOR      44  /* current foreground color */
#define CURFILLPATTERN      45  /* current fill pattern */
#define CURLINESTYLE        46  /* current line style */
#define CURCHANNELMASK      47  /* current write mask */
#define CURREADMASK     48  /* current read mask */
#define NUMBITPLANES        49  /* number of bit planes */
#define DEFREADMASK     50  /* Bool: definable read mask */
#define RASTERCOPY      51  /* Bool: terminal has raster copy */
#define OFFSCREENX      52  /* left value of off screen memory */
#define OFFSCREENY      53  /* bottom value of off screen memory */
#define OFFSCREENDX     54  /* length of off screen memory */
#define OFFSCREENDY     55  /* width of off screen memory */


extern int MFBPutchar();
extern int MFBPutstr();
extern int MFBGetchar();
extern int MFBUngetchar();
extern int OldGetchar();
extern int OldUngetchar();
extern int MFBUpdate();
extern int MFBFlushInput();
extern void MFBAudio();
extern void SetCurrentMFB();

#ifdef DEBUG
extern void MFBZeroCounters();
extern void MFBCounters();
#endif

extern MFB *MFBOpen();
extern int MFBInfo();
extern int MFBInitialize();
extern int MFBInitTTY();
extern int MFBClose();
extern int MFBHalt();
extern int MFBSetLineStyle();
extern int MFBSetFillPattern();
extern int MFBSetChannelMask();
extern int MFBSetReadMask();
extern int MFBSetColor();
extern int MFBSetTextMode();
extern int MFBSetCursorColor();
extern int MFBSetBlinker();
extern int MFBSetRubberBanding();
extern int MFBDefineColor();
extern int MFBDefineFillPattern();
extern int MFBDefineLineStyle();
extern void MFBMoveTo();
extern void MFBDrawLineTo();
extern void MFBDrawPath();
extern void MFBLine();
extern void MFBBox();
extern void MFBArc();
extern void MFBCircle();
extern void MFBFlash();
extern void MFBPolygon();
extern void MFBText();
extern void MFBFlood();
extern void MFBPixel();
extern void MFBRasterCopy();
extern MFBPATH *MFBArcPath();
extern MFBPOLYGON *MFBEllipse();
extern Bool MFBLineClip();
extern MFBPATH *MFBClipArc();
extern int MFBPoint();
extern char *MFBKeyboard();
extern char *MFBError();
extern void MFBSetViewport();
extern void MFBSetWindow();
extern void MFB_Y_Intercept();
extern void MFB_X_Intercept();
extern void MFBPolygonClip();
extern void MFBNaiveBoxFill();
extern void MFBMore();
extern int MFBScaleX();
extern int MFBScaleY();
extern int MFBDescaleX();
extern int MFBDescaleY();


/*  All MFBCAP defines must be less than zero so they will not clash
 *  with the ASCII character set.
 */
#define MFBCAPOK    -10
#define MFBCAPNOFORMAT  -20
#define MFBCAPSYNTAXERR -30


/* mfbcap field evaluation */
#define mfbarg( cp, Reg, reg, val ) {					      \
    val = (int)(*cp);							      \
    ++cp;								      \
    if(val == '%'){							      \
	switch(*cp){							      \
	case 'X':							      \
		val = MFBCurrent->X;					      \
	    break;							      \
	case 'Y':							      \
		val = MFBCurrent->Y;					      \
	    break;							      \
	case 'Z':							      \
		val = MFBCurrent->Z;					      \
	    break;							      \
	case 'T':							      \
		val = MFBCurrent->T;					      \
	    break;							      \
	case 'R':							      \
		val = Reg;						      \
	    break;							      \
	case 'r':							      \
		val = reg;						      \
	    break;							      \
	case 'C':							      \
		val = MFBCurrent->fgColorId;				      \
	    break;							      \
	case 'F':							      \
		val = MFBCurrent->fillPattern;				      \
	    break;							      \
	case 'L':							      \
		val = MFBCurrent->lineStyle;				      \
	    break;							      \
	default:							      \
		--cp;							      \
	    break;							      \
        }								      \
        ++cp;								      \
    } else if(val == '#'){						      \
        int iarg;							      \
	val = 0;							      \
        while( (iarg = ((int)(*cp) - 060)) >= 0 && (iarg < 10) ) {	      \
            cp++; val *= 10; val += iarg;				      \
            }								      \
    }									      \
}

#else /* if it is a PCTERM */

/*
 * MFB emulation package for the IBM PC.
 */

#include <stdio.h>

typedef struct __foo {
	int fileDesc;
} MFB;

extern MFB *MFBCurrent;

extern int MFBdebug;

#define TRUE 1
#define FALSE 0
#define ON 1
#define OFF 0

/* MFBSetALUMode defines */
#define	MFBALUJAM 	0
#define MFBALUOR	1
#define	MFBALUNOR	2
#define	MFBALUEOR	3

/* ERROR DEFINES */
#define MFBOK 		1	/* Successful return */
#define	MFBBADENT	-10	/* Unknown terminal type */
#define	MFBBADMCF	-20	/* Can't open mfbcap file */
#define	MFBMCELNG	-30	/* MFBCAP entry too long */
#define	MFBBADMCE	-40	/* Bad mfbcap entry */
#define	MFBINFMCE	-50	/* Infinite mfbcap entry */
#define	MFBBADTTY	-60	/* stdout not in /dev */
#define	MFBBADLST	-70	/* Illegal line style */
#define	MFBBADFST	-80	/* Illegal fill style */
#define	MFBBADCST	-90	/* Illegal color style */
#define	MFBBADTM1	-100	/* No destructive text */
#define	MFBBADTM2	-110	/* No overstriking text */
#define	MFBNODFLP	-120	/* No definable line styles */
#define	MFBNODFFP	-130	/* No definable fill styles */
#define	MFBNODFCO	-140	/* No definable colors */
#define	MFBNOBLNK	-150	/* No blinkers */
#define	MFBTMBLNK	-160	/* Too many blinkers */
#define	MFBBADDEV	-180	/* Can't open or close device */
#define	MFBBADOPT	-190	/* Can't access or set device stat */
#define	MFBNOMASK	-170	/* No definable read or write mask */
#define	MFBBADWRT	-200	/* Error in write */
#define	MFBPNTERR	-210	/* Error in pointing device */
#define	MFBNOPTFT	-220	/* No format for pointing device */
#define	MFBNOPNT	-230	/* No pointing device */
#define	MFBNORBND	-240	/* No Rubberbanding */
#define	MFBBADALU	-250	/* Cannot set ALU mode */
#define MFBNOMEM	-260	/* Out of memory */

/* Defines for MFBInfo */
#define MAXX			1	/* max x coordinate */
#define MAXY			2	/* max y coordinate */
#define MAXCOLORS		3	/* max number of colors */
#define MAXINTENSITY		4	/* max color intensity */
#define MAXFILLPATTERNS		5	/* max number of fill patterns */
#define MAXLINESTYLES		6	/* max number of line styles */
#define MAXBLINKERS		7	/* max number of blinkers */
#define POINTINGDEVICE		8	/* Bool: terminal has pointing device */
#define POINTINGBUTTONS		9	/* Bool: pointing device has buttons */
#define NUMBUTTONS		10	/* number of pointing device buttons */
#define BUTTON1			11	/* button value returned by button 1 */
#define BUTTON2			12	/* button value returned by button 2 */
#define BUTTON3			13	/* button value returned by button 3 */
#define BUTTON4			14	/* button value returned by button 4 */
#define BUTTON5			15	/* button value returned by button 5 */
#define BUTTON6			16	/* button value returned by button 6 */
#define BUTTON7			17	/* button value returned by button 7 */
#define BUTTON8			18	/* button value returned by button 8 */
#define BUTTON9			19	/* button value returned by button 9 */
#define BUTTON10		20	/* button value returned by button 10 */
#define BUTTON11		21	/* button value returned by button 11 */
#define BUTTON12		22	/* button value returned by button 12 */
#define TEXTPOSITIONALBE	30	/* Bool: accurately positionable text */
#define TEXTROTATABLE		31	/* Bool: rotateable text */
#define FONTHEIGHT		32	/* font height in pixels */
#define FONTWIDTH		33	/* font width in pixels */
#define FONTXOFFSET		34 	/* font x offset in pixels */
#define FONTYOFFSET		35	/* font y offset in pixels */
#define DESTRUCTIVETEXT		36	/* Bool: text can be destructive */
#define OVERSTRIKETEXT		37	/* Bool: text can be overstrike */
#define VLT			38	/* Bool: terminal has VLT */
#define BLINKERS		39	/* Bool: terminal has blinkers */
#define FILLEDPOLYGONS		40	/* Bool: terminal has filled polygons */
#define DEFFILLPATTERNS		41	/* Bool: defineable fill patterns */
#define DEFCHANNELMASK		42	/* Bool: defineable write mask */
#define DEFLINEPATTERN		43	/* Bool: defineable line styles */
#define CURFGCOLOR		44	/* current foreground color */
#define CURFILLPATTERN		45	/* current fill pattern */
#define CURLINESTYLE		46	/* current line style */
#define CURCHANNELMASK		47	/* current write mask */
#define CURREADMASK		48	/* current read mask */
#define NUMBITPLANES		49	/* number of bit planes */
#define DEFREADMASK		50	/* Bool: definable read mask */
#define RASTERCOPY		51	/* Bool: terminal has raster copy */
#define	OFFSCREENX		52	/* left value of off screen memory */
#define	OFFSCREENY		53	/* bottom value of off screen memory */
#define OFFSCREENDX		54	/* length of off screen memory */
#define OFFSCREENDY		55	/* width of off screen memory */

extern MFB *MFBOpen();
extern char *MFBError();
extern char *MFBKeyboard();
extern void MFBArc();

#endif
