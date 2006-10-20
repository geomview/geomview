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
#include <Xm/DialogS.h>
#include "../common/drawer.h"
#include "gvui.h"
#include "interface/LoadPaths.mib"

extern Display *dpy;
extern Widget TopLevel;

/* private methods and variables */
/*****************************************************************************/

static void load_selection(Widget, XtPointer,
			XmFileSelectionBoxCallbackStruct *);
static void select_path(Widget, XtPointer, XmListCallbackStruct *);
/*static void ui_update_filespanel();*/
static Widget shell, pathlist, filesform;
static char **dirp;

/*****************************************************************************/

void ui_load_filespanel()
{
  Widget     pathform,
	     temp;

  mib_Widget *pathload;
  XmString   xname;
  static char Files[] = "Files";

/*****************************************************************************/

  shell = ui_make_panel_and_form(Files, Root, True, False, NULL);

  filesform = XmCreateFileSelectionBox(shell, Files, NULL, 0);

  XtManageChild(filesform);
  XtSetMappedWhenManaged(filesform, True);
 
/*****************************************************************************/

  temp = XmFileSelectionBoxGetChild(filesform, XmDIALOG_HELP_BUTTON);
  XtUnmanageChild(temp);

  XtAddCallback(filesform, XmNokCallback, (XtCallbackProc) load_selection,
		(XtPointer) NULL);

  temp = XmFileSelectionBoxGetChild(filesform, XmDIALOG_CANCEL_BUTTON);

  xname = XmStringCreateSimple("Hide");
  XtVaSetValues(temp, XmNlabelString, xname, NULL);
  XmStringFree(xname);

  XtAddCallback(temp, XmNactivateCallback, (XtCallbackProc) ui_hide,
		(XtPointer) Files);

/*****************************************************************************/

/*pathload = mib_load_interface(filesform, "interface/LoadPaths.mib",
			MI_FROMFILE);*/
  pathload = mib_load_interface(filesform, Root,
			MI_FROMSTRING);
  pathform = pathload->me;

  pathlist = mib_find_name(pathload, "PathList")->me;
  XtAddCallback(pathlist, XmNbrowseSelectionCallback,
		(XtCallbackProc) select_path, (XtPointer) NULL);
  XtManageChild(pathform);
  XtManageChild(pathlist);
}


/*****************************************************************************/

void ui_show_filespanel()
{
  int		i;
  XmString	str;

  XmListDeleteAllItems(pathlist);
  dirp = getfiledirs();
  for (i=0; dirp[i] != NULL; i++)
  {
    str = XmStringCreateSimple(dirp[i]);
    XmListAddItemUnselected(pathlist, str, 0);
    XmStringFree(str);
  }
}

/*****************************************************************************/

static void select_path(Widget w, XtPointer data, XmListCallbackStruct *cbs)
{
  char        *temp, dirmask[200];
  XmString     dmask;

  if (!XmStringGetLtoR(cbs->item, XmSTRING_DEFAULT_CHARSET, &temp))
    return;

  if (!*temp)
    return;

  sprintf(dirmask,"%s/*", temp);
  XtFree(temp);
  dmask = XmStringCreate(dirmask, XmSTRING_DEFAULT_CHARSET);

  XmFileSelectionDoSearch(filesform, dmask);

  XmStringFree(dmask);
}

/*****************************************************************************/

static void load_selection(Widget w, XtPointer data,
		XmFileSelectionBoxCallbackStruct *cbs)
{

  char  *object;

  if (!XmStringGetLtoR(cbs->value, XmSTRING_DEFAULT_CHARSET, &object))
    return;

  if (!*object)
    return;

  gv_load(object, NULL);
  XtFree(object);
}

/*****************************************************************************/
