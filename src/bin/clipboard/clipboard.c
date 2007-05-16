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

#include <stdio.h>
#include <math.h>
#include "geom.h"
#include "mibload.h"
#include "clipboard.mib"
#include "clip.h"
#include "mg.h"
#include "mgx11.h"

void cut_callback(Widget , XtPointer,
	XmPushButtonCallbackStruct *);
void copy_callback(Widget , XtPointer,
	XmPushButtonCallbackStruct *);
void paste_callback(Widget , XtPointer,
	XmPushButtonCallbackStruct *);
void delete_callback(Widget , XtPointer,
	XmPushButtonCallbackStruct *);


char  obj_name[1000];
Geom *theObject;
int   clipfull;

/* The selected visual */
Visual *visual;
Colormap colormap;
int bitdepth;

void find_visual(Display *dpy)
{
  int result;

#if !MGX11
  visual = DefaultVisual(dpy, DefaultScreen(dpy));
  colormap = DefaultColormap(dpy, DefaultScreen(dpy));
  bitdepth = DefaultDepth(dpy, DefaultScreen(dpy));
#else
  result = mgx11_getvisual(dpy, &visual, &colormap, &bitdepth);

  if (result == MG_X11VISFAIL) {
     fprintf(stderr,
             "X11 MG driver currently supports only 24, 16, 8, and 1 bit\n"
             "\tdeep displays.  No supported display depth was found.\n"
             "\tPlease mail the output of \"xdpyinfo\" to\n"
             "\tsoftware@geomview.org\n");
     exit(1);
   }
   else
     if (result == MG_X11VISPRIVATE) {
        fprintf(stderr,
                "Not enough colors available. Using private colormap.\n");
     }
     else
       return;
#endif
}

int main(int argc, char **argv)
{
  XtAppContext	App;
  Widget TopLevel,
    MainWindow,
    CutButton,
    CopyButton,
    PasteButton,
    DeleteButton;
  

  int		no_mi;
  Arg		args[20];
  int		n = 0;

  mib_Widget   *MainForm;
  static String	fallbacks[] = {
	"myprog*Foreground:			gray20", /*15*/
	"myprog*Background:			gray70", /*80*/
	"myprog*XmTextField.background:	       DeepSkyBlue",
	"myprog*fontList:\
	-adobe-helvetica-medium-r-normal--14-100-100-100-p-76-iso8859-1",
	NULL};

  no_mi = 0;				/* no motif interface if no_mi = 1 */

  clipfull = 0;
  obj_name[0] = '\0';


  /* initialize application top level widget */

  TopLevel = XtVaAppInitialize(&App, "myprog", NULL, 0,
	&argc, argv, fallbacks, NULL);

  find_visual(XtDisplay(TopLevel));

  /* configure resize policy of window */

  XtVaSetValues(TopLevel, XmNminWidth, 272, XmNminHeight, 172, NULL);

  if (bitdepth != 24) {
    XtSetArg(args[n], XmNvisual, visual); n++;
    XtSetArg(args[n], XmNdepth, bitdepth); n++;
    XtSetArg(args[n], XmNcolormap, colormap); n++;
  }

  /* create the application main window widget */

  MainWindow = XtCreateManagedWidget("MainWindow",
	xmMainWindowWidgetClass, TopLevel, args, n);

  /* load the interface via the mib library */

  MainForm = mib_load_interface(MainWindow,
	Root, MI_FROMSTRING);
/*  MainForm = mib_load_interface(MainWindow, "mib/clipboard.mib", MI_FROMFILE);
*/

  if (MainForm == NULL)
    exit(0);

  /* Set widget pointers to null before trying to find their instances */

  CutButton  = NULL;
  CopyButton  = NULL;
  PasteButton  = NULL;
  DeleteButton  = NULL;

  /* Do similarly when trying to find a widget named "CutButton" */

  if (!(CutButton = XtNameToWidget(MainForm->me, "CutButton")))
    no_mi = 1;
  if (!(CopyButton = XtNameToWidget(MainForm->me, "CopyButton")))
    no_mi = 1;
  if (!(PasteButton = XtNameToWidget(MainForm->me, "PasteButton")))
    no_mi = 1;
  if (!(DeleteButton = XtNameToWidget(MainForm->me, "DeleteButton")))
    no_mi = 1;
  if (!XtNameToWidget(MainForm->me, "DrawingArea"))
    no_mi = 1;

  /* if we found both widgets then add a callback to the button. This
     is called whenever the user clicks the button. If there was no
     button and/or no text box then we avoid adding the callback. */

  if (!no_mi)
  {
    XtAddCallback(CutButton, XmNactivateCallback, (XtCallbackProc)cut_callback, NULL);
    XtAddCallback(CopyButton, XmNactivateCallback, (XtCallbackProc)copy_callback, NULL);
    XtAddCallback(PasteButton, XmNactivateCallback, (XtCallbackProc)paste_callback, NULL);
    XtAddCallback(DeleteButton, XmNactivateCallback, (XtCallbackProc)delete_callback, NULL);
  }


  /* Bring the application window up on the screen. */

  XtRealizeWidget(TopLevel);

  /* initialize visual clipboard  - mg library stuff (gl or vanilla-x)*/

  clipboard_init(MainForm->me);

  /* Begin main Intrinsics event loop */

  XtAppMainLoop (App);

  return 0;
}

void cut_callback(Widget w, XtPointer data, XmPushButtonCallbackStruct *cbs)
{
  char tmp;
  int  count;
  IOBFILE *infile;

  fprintf(stdout,"(echo (real-id target))\n");
  fflush(stdout);
  tmp = '\0';
  while (tmp != '\"')
    tmp = (char)fgetc(stdin);
  tmp = '\0';
  count = 0;
  while (tmp != '\"')
  {
    tmp = (char)fgetc(stdin);
    obj_name[count] = tmp;
    count++;
  }

  obj_name[count-1] = '\0';
   
  fprintf(stdout,"(write geometry - target)\n");
  fflush(stdout);
  infile = iobfileopen(stdin);
  theObject = GeomFLoad(infile, obj_name);
  iobfileclose(infile);

  fprintf(stdout,"(delete target)\n");
  fflush(stdout);

  clipfull = 1;
  redraw(w);

}
void copy_callback(Widget w, XtPointer data, XmPushButtonCallbackStruct *cbs)
{
  char tmp;
  int  count;
  IOBFILE *infile;

  fprintf(stdout,"(echo (real-id target))\n");
  fflush(stdout);
  tmp = '\0';
  while (tmp != '\"')
    tmp = (char)fgetc(stdin);
  tmp = '\0';
  count = 0;
  while (tmp != '\"')
  {
    tmp = (char)fgetc(stdin);
    obj_name[count] = tmp;
    count++;
  }

  obj_name[count-1] = '\0';

  fprintf(stdout,"(write geometry - target)");
  fflush(stdout);
  infile = iobfileopen(stdin);
  theObject = GeomFLoad(infile, obj_name);
  iobfileclose(infile);

  clipfull = 1;
  redraw(w);

}

void paste_callback(Widget w, XtPointer data, XmPushButtonCallbackStruct *cbs)
{
  if (clipfull == 0)
    return;

  fprintf(stdout,"(new-geometry %s {",obj_name);
  GeomFSave(theObject, stdout, obj_name);
  fprintf(stdout,"})");
  fflush(stdout);

}

void delete_callback(Widget w, XtPointer data, XmPushButtonCallbackStruct *cbs)
{
  fprintf(stdout,"(delete target)\n");
  fflush(stdout);
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
