/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 * Copyright (C) 2006-2007 Claus-Justus Heine
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

#if HAVE_CONFIG_H
# include "config.h"
#endif

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

#include "mibload.h"
#include "../common/drawer.h"
#include "../common/lang.h"
#include "gvui.h"
static /* prefix to ``char Root[]...'' */
#include "interface/Credits.mib"

extern Display *dpy;
extern char builddate[], buildinfo1[], buildinfo2[];
extern char *buildinfographics;
extern Pixmap geomicon;

/* private methods and variables */
/*****************************************************************************/

static void add_text(Widget, char *[]);
/*static char geometry[40];*/

static char *credits[] = {
  NULL,
  " ",
  "Geomview is an interactive program for viewing and manipulating",
  "geometric objects, originally written by staff members of the Geometry",
  "Center at the University of Minnesota, starting in 1991. It can be",
  "used as a standalone viewer for static objects or as a display engine",
  "for other programs which produce dynamically changing geometry.",
  " ",
  "Many people have contributed to Geomview, among them are:",
  " ",
  "Original Geomview Kernel + GL version:",
  "Stuart Levy, Tamara Munzner, Mark Phillips",
  " ",
  "X Window System Version:",
  "Daeron Meyer, Timothy Rowley",
  " ",
  "NeXTSTEP Version:",
  "Daniel Krech, Scott Wisdom",
  " ",
  "Important Modifications:",
  "Celeste Fowler, Charlie Gunn, Nathaniel Thurston", 
  " ",
  "Bug-fixes, maintenance:",
  "Claus-Justus Heine",
  " ",
  "Further:",
  "Steve Anderson, Rex Dieter, Todd Kaplan, Mario Lopez,",
  "Mark Meuer, Steve Robbins, Lloyd Wood and others.",
  " ",
  "Copyright 1992-1998 The Geometry Center",
  "Copyright 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips",
  "Copyright 2005-2007 Claus-Justus Heine",
  "http://www.geomview.org/",
  " ",
  "Geomview is free software; you can redistribute it and/or modify it",
  "under the terms of the GNU Lesser General Public License as published",
  "by the Free Software Foundation; either version 2, or (at your option)",
  "You should have received a copy of the GNU Lesser General Public",
  "License along with Geomview; see the file COPYING.  If not, write to",
  "the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA,",
  "or visit http://www.gnu.org.",
  " ",
  "Compiled:",
  buildinfo1,
  buildinfo2,
  NULL
};

/*****************************************************************************/

void ui_load_creditspanel()
{
  Widget     creditsform,
	     creditslist,
	     HideButton;

  mib_Widget *creditsload;
  static char Credits[] = "Credits";

/*****************************************************************************/

  ui_make_panel_and_form(Credits, Root, True, True, &creditsform);

/*creditsload = mib_load_interface(creditsform, "interface/Credits.mib",
		MI_FROMFILE);*/
  creditsload = mib_load_interface(creditsform, Root,
		MI_FROMSTRING);
 
  XtManageChild(creditsform);

/*****************************************************************************/

  HideButton = mib_find_name(creditsload, "HideButton")->me;
  XtAddCallback(HideButton, XmNactivateCallback, (XtCallbackProc) ui_hide,
			(XtPointer) Credits);

  creditslist = mib_find_name(creditsload, "CreditsList")->me;
  add_text(creditslist, credits);
}


/*****************************************************************************/

static void add_text(Widget clist, char *text[])
{
  int line = 0;
  XmString str;
  char first[128];

  sprintf(first, "Geomview %.80s %.20s", geomview_version, buildinfographics);
  text[0] = first;
  while (text[line])
  {
   str = XmStringCreateSimple(text[line]);
   XmListAddItemUnselected(clist, str, 0);
   XmStringFree(str);
   line++;
  }
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
