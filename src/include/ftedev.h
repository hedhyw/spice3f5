/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1987 Jeffrey M. Hsu
**********/

/*
    $Header: FTEdevice.h,v 1.2 88/11/17 02:35:31 quarles Exp $

    The display device structure.
*/

typedef struct {
    char *name;
    int minx, miny;
    int width, height;      /* in screen coordinate system */
    int numlinestyles, numcolors;   /* number supported */
    int (*Init)();
    int (*NewViewport)();
    int (*Close)();
    int (*Clear)();
    int (*DrawLine)();
    int (*Arc)();
    int (*Text)();
    int (*DefineColor)();
    int (*DefineLinestyle)();
    int (*SetLinestyle)();
    int (*SetColor)();
    int (*Update)();
/*  int (*NDCtoScreen)(); */
    int (*Track)();
    int (*MakeMenu)();
    int (*MakeDialog)();
    int (*Input)();
    int (*DatatoScreen)();
} DISPDEVICE;

extern DISPDEVICE *dispdev;
