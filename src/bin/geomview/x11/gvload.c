/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
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
#include "gvui.h"
static /* prefix to ``char Root[]...'' */
#include "interface/Input.mib"

#include "X11/IntrinsicP.h" /* for XtMoveWidget */

extern Display *dpy;
extern Widget TopLevel;

/* private methods and variables */
/*****************************************************************************/

static Widget shell, loadform;
static Widget FileBrowserButton, HideButton, OKButton, TextField;
/*static int    pos_later = 0, posx = 0, posy = 0;
  static char geometry[40];*/

static void do_load(Widget w, XtPointer data, XmAnyCallbackStruct *cbs);
static void action_load(Widget, XEvent *, String *, Cardinal *);
static XtActionsRec actions[1] = { { "GVLoad", action_load } };
static String loadtransl = "<KeyPress>Return: GVLoad()";
/*****************************************************************************/

void ui_load_loadpanel()
{
  mib_Widget *loadload;
  XtTranslations LoadTransl;
  static char Load[] = "Load";

/*****************************************************************************/

  XtAppAddActions(App, actions, 1);

  shell = ui_make_panel_and_form(Load, Root, False, True, &loadform);

/*loadload = mib_load_interface(loadform, "interface/MiniLoad.mib",
                MI_FROMFILE);*/
  loadload = mib_load_interface(loadform, Root,
                MI_FROMSTRING);
 
  XtManageChild(loadform);

  FileBrowserButton = mib_find_name(loadload, "FileBrowser")->me;
  HideButton =        mib_find_name(loadload, "Cancel")->me;
  OKButton =          mib_find_name(loadload, "OK")->me;
  TextField =         mib_find_name(loadload, "FileName")->me;


  XtAddCallback(HideButton, XmNactivateCallback, (XtCallbackProc)ui_hide,
		(XtPointer) Load);
  XtAddCallback(OKButton, XmNactivateCallback, (XtCallbackProc)do_load,
		(XtPointer) NULL);
  XtAddCallback(FileBrowserButton, XmNactivateCallback,
			(XtCallbackProc)ui_show,
			(XtPointer)"Files");

  XtVaSetValues(TextField, XmNblinkRate, 0, NULL);

  LoadTransl = XtParseTranslationTable(loadtransl);
  XtOverrideTranslations(TextField, LoadTransl);

}


/*****************************************************************************/

void ui_show_loadpanel()
{
  Window root, child;
  int x, y, wx, wy, posx = 0, posy = 0;
  unsigned int mask;

  XtManageChild(loadform);

  if(XQueryPointer(dpy, DefaultRootWindow(dpy), &root, &child,
		&x, &y, &wx, &wy, &mask)) {
	posx = (x -= 20) < 0 ? 0 : x;
	posy = (y -= 20) < 0 ? 0 : y;
  }
  XtMoveWidget(shell, posx, posy);

  XtSetMappedWhenManaged(loadform, True);
  XtRealizeWidget(shell);
  XRaiseWindow(dpy, XtWindow(shell));
  XtSetKeyboardFocus(loadform, TextField);

  XmTextFieldSetSelection(TextField, 0, 9999, last_event_time);
}

/*****************************************************************************/

static void do_load(Widget w, XtPointer data, XmAnyCallbackStruct *cbs)
{
  char *str = XmTextGetString(TextField);
  if(str != NULL && str[0] != '\0')
    gv_load( str, NULL );
  ui_hide(w, "Load", NULL);
}


static void action_load(Widget w, XEvent *event, String *argv, Cardinal *argc)
{
  do_load(w, NULL, NULL);
}
