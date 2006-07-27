/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Geometry Technologies, Inc.
 *
 * This file is part of Geomview.
 * 
 * Geomview is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * Geomview is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with Geomview; see the file COPYING.  If not, write
 * to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
 * USA, or visit http://www.gnu.org.
 */

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Geometry Technologies, Inc.";
#endif

/* Author: Timothy Rowley */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <tcl.h>
#include <tk.h>

static char **fonts;
static int numfonts;

static char *standard_fonts[] = {
    "Charter",
    "Courier",
    "Helvetica",
    "New century schoolbook",
    "Times",
    NULL
};

static char *weights[] = {
    "Black",
    "Bold",
    "Demibold",
    "Medium",
    NULL
};

static char *slants[] = {
    "Italic",
    "Oblique",
    "Roman",
    NULL
};

static int sizes[] = { 18, 24, 36, 48, 60, 72, 84, 96, 108, 120, 132, 144, 156, 68, 180, 192, 204, 216, 228, 240,  -1};

#define TABLE_SIZE 200

static void
insertFont(char *fname)
{
    int i;
    if (islower(fname[0]))
	fname[0] = toupper(fname[0]);
    for (i=0; i<numfonts; i++)
    {
	if (!strcmp(fonts[i], fname))
	    return;
    }
    fonts[i] = (char *)malloc(strlen(fname)+1);
    numfonts++;
    strcpy(fonts[i], fname);
}

static int
compare(const void *s1, const void *s2)
{
    return(strcmp(*(char **)s1, *(char **)s2));
}

int
getSizesCmd(ClientData clientData, Tcl_Interp *interp,
	    int argc, const char **argv)
{
    int i;
    char s[64];
    for (i=0; sizes[i] != -1; i++)
    {
	sprintf(s, "%d", sizes[i]);
	Tcl_AppendElement(interp, s);
    }
    return TCL_OK;
}

int
getWeightsCmd(ClientData clientData, Tcl_Interp *interp,
	      int argc, const char **argv)
{
    int i;
    for (i=0; weights[i] != NULL; i++)
	Tcl_AppendElement(interp, weights[i]);
    return TCL_OK;
}
	
int
getSlantsCmd(ClientData clientData, Tcl_Interp *interp,
	     int argc, const char **argv)
{
    int i;
    for (i=0; slants[i] != NULL; i++)
	Tcl_AppendElement(interp, slants[i]);
    return TCL_OK;
}
	
int
getFontsCmd(ClientData clientData, Tcl_Interp *interp,
	    int argc, const char **argv)
{
    int num, i;
    char **fontnames;
    char *prev="ping!", *current;

    fontnames = XListFonts(Tk_Display(clientData), 
			   "-*-*-*-*-*-*-0-0-*-*-*-0-iso8859-1", 500, &num);
    fonts = (char **)malloc(TABLE_SIZE*sizeof(char *));
    numfonts = 0;
    for (i=0; i<TABLE_SIZE; i++)
	fonts[i] = NULL;
    for (i=0; i<num; i++)
    {
	strtok(fontnames[i], "-");
	current = strtok(NULL, "-");
	if (strcmp(prev, current))
	    insertFont(current);
	prev = current;
    }
    XFreeFontNames(fontnames);
    for (i=0; standard_fonts[i]!= NULL; i++)
	insertFont(standard_fonts[i]);
    qsort(fonts, numfonts, sizeof(char *), compare);
    for (i=0; i<numfonts; i++)
	Tcl_AppendElement(interp, fonts[i]);
    return TCL_OK;
}


/* Expects font, weight, slant, size */

int
handleFontCmd(ClientData clientData, Tcl_Interp *interp,
	      int argc, const char **argv)
{
    char **fontnames;
    int i, num;
    char s[256];

    if (argc != 5) {
	Tcl_SetResult(interp, "usage: handleFont font weight slant size",
		      TCL_STATIC);
	return TCL_ERROR;
    }

    /* Try out different fontnames */
    for (i=0; fonts[i] != NULL; i++)
	if (strcmp(argv[1], fonts[i]))
	{
	    sprintf(s, "-*-%s-%s-%c-*-*-*-%d-75-75-*-*-iso8859-1", 
		    fonts[i], argv[2], argv[3][0], atoi(argv[4])*10);
	    fontnames = XListFonts(Tk_Display(clientData), s, 100, &num);
	    if (num == 0)
		sprintf(s, "doDelete family %s", fonts[i]);
	    else
		sprintf(s, "doInsert family %s", fonts[i]);
	    Tcl_Eval(interp, s);
	    XFreeFontNames(fontnames);
	}

    if (!strcmp(argv[1], "VECTOR"))
    {
	/* Disable all style options */
	Tcl_Eval(interp, ".weight delete 0 end");
	Tcl_Eval(interp, ".slant delete 0 end");
	Tcl_Eval(interp, ".size delete 0 end");
	sprintf(s, "changeFont -*-Helvetica-Medium-R-*-*"
		"-*-140-75-75-*-*-iso8859-1");
	Tcl_Eval(interp, s);
	return TCL_OK;
    }

    /* Try out different weights */
    for (i=0; weights[i] != NULL; i++)
    {
	sprintf(s, "-*-%s-%s-%c-*-*-*-%d-75-75-*-*-iso8859-1", 
		argv[1], weights[i], argv[3][0], atoi(argv[4])*10);
	fontnames = XListFonts(Tk_Display(clientData), s, 100, &num);
	if (num == 0)
	    sprintf(s, "doDelete weight %s", weights[i]);
	else
	    sprintf(s, "doInsert weight %s", weights[i]);
	Tcl_Eval(interp, s);
	XFreeFontNames(fontnames);
    }

    /* Try out different slants */
    for (i=0; slants[i] != NULL; i++)
    {
	sprintf(s, "-*-%s-%s-%c-*-*-*-%d-75-75-*-*-iso8859-1", 
		argv[1], argv[2], slants[i][0], atoi(argv[4])*10);
	fontnames = XListFonts(Tk_Display(clientData), s, 100, &num);
	if (num == 0)
	    sprintf(s, "doDelete slant %s", slants[i]);
	else
	    sprintf(s, "doInsert slant %s", slants[i]);
	Tcl_Eval(interp, s);
	XFreeFontNames(fontnames);
    }

    /* try out different point sizes */
    for (i=0; sizes[i] != -1; i++)
    {
	sprintf(s, "-*-%s-%s-%c-*-*-*-%d-75-75-*-*-iso8859-1", 
		argv[1], argv[2], argv[3][0], sizes[i]*10);
	fontnames = XListFonts(Tk_Display(clientData), s, 100, &num);
	if (num == 0)
	    sprintf(s, "doDelete size %d", sizes[i]);
	else
	    sprintf(s, "doInsert size %d", sizes[i]);
	Tcl_Eval(interp, s);
	XFreeFontNames(fontnames);
    }

    /* set font */
    sprintf(s, "changeFont \"-*-%s-%s-%c-*-*-*-180-75-75-*-*-iso8859-1\"",
	    argv[1], argv[2], argv[3][0]);
    Tcl_Eval(interp, s);
    return TCL_OK;
}


/* Expects font, weight, slant, size, string */
/* This is the guts of the program -- it does the actual
   conversion of a string into a QUAD */

int
scanFontCmd(ClientData clientData, Tcl_Interp *interp,
	    int argc, const char **argv)
{
    char s[256];
    Pixmap pixmap;
    XFontStruct *font_info;
    Display *d = Tk_Display(clientData);
    int width, height;
    GC gc;
    XImage *xi;
    int x, y, pixel, current, start = 0;

    if (argc != 6) {
	Tcl_SetResult(interp, "usage: scanFont font weight slant size string",
		      TCL_STATIC);
	return TCL_ERROR;
    }

    /* Get font */
    sprintf(s, "-*-%s-%s-%c-*-*-*-%d-75-75-*-*-iso8859-1", 
	    argv[1], argv[2], argv[3][0], atoi(argv[4])*10);
    font_info = XLoadQueryFont(d, s);
    width = XTextWidth(font_info, argv[5], strlen(argv[5])) +
		font_info->max_bounds.rbearing;
    height = font_info->ascent + font_info->descent + 1;

    /* Set up for drawing */
    gc = XCreateGC(d, RootWindow(d, 0), 0, NULL);
    pixmap = XCreatePixmap(d, RootWindow(d, 0), width, height, 
			   DefaultDepth(d, 0));
    XFillRectangle(d, pixmap, gc, 0, 0, width, height);
    XSetForeground(d, gc, 1);
    XSetBackground(d, gc, 0);
    XSetFont(d, gc, font_info->fid);

    /* Do drawing */
    XDrawString(d, pixmap, gc, 0, font_info->ascent, argv[5], strlen(argv[5]));

    /* Get image out */
    xi = XGetImage(d, pixmap, 0, 0, width, height, AllPlanes, XYPixmap);
    printf("QUAD\n");
    current = 0;
    for (x=0; x<width; x++)
    {
	for (y=0; y<height; y++)
	{
	    pixel = XGetPixel(xi, x, y);
	    if (current && (!pixel)) /* finished run of pixels */
	    {
		printf("%d %d 0  %d %d 0  %d %d 0  %d %d 0\n", 
		       x, height-start, x, height-y, x+1, height-y, 
		       x+1, height-start);
		current = 0;
	    }
	    if ((!current) && pixel) /* start run of pixels */
	    {
		current = 1;
		start = y;
	    }
	}
    }
    fflush(stdout);

    /* Free up everything */
    XFreeFont(d, font_info);
    XFreeGC(d, gc);
    XFreePixmap(d, pixmap);
    XDestroyImage(xi);

    return TCL_OK;
}

/************************************************/

int labeler_init(Tcl_Interp *interp)
{
    Tk_Window mainw = Tk_MainWindow(interp);

    Tcl_CreateCommand(interp, "handleFont", handleFontCmd, (ClientData) mainw,
		      (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(interp, "scanFont", scanFontCmd, (ClientData) mainw,
		      (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(interp, "getWeights", getWeightsCmd, (ClientData) mainw,
		      (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(interp, "getSlants", getSlantsCmd, (ClientData) mainw,
		      (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(interp, "getSizes", getSizesCmd, (ClientData) mainw,
		      (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(interp, "getFonts", getFontsCmd, (ClientData) mainw,
		      (Tcl_CmdDeleteProc *) NULL);
    return TCL_OK;
}

