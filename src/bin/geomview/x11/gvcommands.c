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
#include "../common/ui.h"
#include "../common/comm.h"
#include "gvui.h"
static /* prefix to ``char Root[]...'' */
#include "interface/Commands.mib"
#include "bitmaps/up_arrow"
#include "bitmaps/down_arrow"

extern HandleOps CommandOps;
extern Display *dpy;
extern Pixmap   geomicon;

/* private methods and variables */
/*****************************************************************************/

static void command_input(Widget, XtPointer, XmAnyCallbackStruct *);
static void command_scroll(Widget, XtPointer, XmListCallbackStruct *);
static void command_up(Widget, XtPointer, XmAnyCallbackStruct *);
static void command_down(Widget, XtPointer, XmAnyCallbackStruct *);

static int num_items;
/*static int pos_later = 0, posx = 0, posy = 0;
  static char geometry[40];*/


/*****************************************************************************/

void ui_load_commandspanel()
{
  int	     depth;
  Widget     commandsform,
	     commscroll,
	     commtext,
	     HideButton,
	     temp;

  Pixel      fg, bg;
  Pixmap     button_pix;
  mib_Widget *commandsload;
  static char Commands[] = "Commands";

/*****************************************************************************/

  ui_make_panel_and_form(Commands, Root, False, True, &commandsform);

/*commandsload = mib_load_interface(commandsform, "interface/Commands.mib",
		MI_FROMFILE);*/
  commandsload = mib_load_interface(commandsform, Root,
		MI_FROMSTRING);
 
  XtManageChild(commandsform);

/*****************************************************************************/

  HideButton = mib_find_name(commandsload, "HideButton")->me;
  XtAddCallback(HideButton, XmNactivateCallback, (XtCallbackProc) ui_hide,
		(XtPointer)Commands);

  commtext = mib_find_name(commandsload, "CommandText")->me;
  commscroll = mib_find_name(commandsload, "CommandScroll")->me;
  XtAddCallback(commtext, XmNactivateCallback, (XtCallbackProc) command_input,
		(XtPointer) commscroll);
  XtAddCallback(commscroll, XmNbrowseSelectionCallback,
			(XtCallbackProc) command_scroll, (XtPointer) commtext);

  temp = mib_find_name(commandsload, "UpButton")->me;
  XtVaGetValues(temp, XmNforeground, &fg,
			XmNbackground, &bg, XmNdepth, &depth, NULL);
  button_pix = XCreatePixmapFromBitmapData(dpy, DefaultRootWindow(dpy),
		(char *)up_arrow_bits, up_arrow_width, up_arrow_height, fg, bg, depth);
  XtVaSetValues(temp, XmNlabelType, XmPIXMAP,
			XmNlabelPixmap, button_pix, NULL);
  XtAddCallback(temp, XmNactivateCallback, (XtCallbackProc) command_up,
		(XtPointer) commscroll);

  temp = mib_find_name(commandsload, "DownButton")->me;
  XtVaGetValues(temp, XmNforeground, &fg,
			XmNbackground, &bg, XmNdepth, &depth, NULL);
  button_pix = XCreatePixmapFromBitmapData(dpy, DefaultRootWindow(dpy),
		(char *)down_arrow_bits, down_arrow_width, down_arrow_height,
		fg, bg, depth);
  XtVaSetValues(temp, XmNlabelType, XmPIXMAP,
			XmNlabelPixmap, button_pix, NULL);
  XtAddCallback(temp, XmNactivateCallback, (XtCallbackProc) command_down,
		(XtPointer) commscroll);


  num_items = 0;
}


/*****************************************************************************/

static void command_input(Widget w, XtPointer data, XmAnyCallbackStruct *cbs)
{
  Widget commscroll = (Widget)data;
  XmString addlist;
  char *str;

  str = XmTextFieldGetString(w);
  if (!strlen(str))
  {
    XtFree(str);
    return;
  }
  comm_object(str, &CommandOps, NULL, NULL, COMM_NOW);

  addlist = XmStringCreate(str, XmSTRING_DEFAULT_CHARSET);
  XtFree(str);

  XmListAddItemUnselected(commscroll, addlist, 0);
  num_items++;
}

/*****************************************************************************/

static void command_scroll(Widget w, XtPointer data, XmListCallbackStruct *cbs)
{
  Widget commtext = (Widget)data;

  char *temp;

  if (!XmStringGetLtoR(cbs->item, XmSTRING_DEFAULT_CHARSET, &temp))
    return;

  if (!*temp)
    return;

  XmTextFieldSetString(commtext, temp);
  XtFree(temp);

}

/*****************************************************************************/

static void command_up(Widget w, XtPointer data, XmAnyCallbackStruct *cbs)
{
  Widget commscroll = (Widget)data;
  int *poslist, poscount, pos;

  if (!XmListGetSelectedPos(commscroll, &poslist, &poscount))
  {
    if (!num_items)
      return;
    XmListSelectPos(commscroll, 0, True);
  }
  if (!poscount)
    return;
  pos = poslist[0]-1;
  if (pos<0)
    pos = num_items-1;
  if (pos<0)
    pos = 0;
  XmListSelectPos(commscroll, pos, True);
}

/*****************************************************************************/

static void command_down(Widget w, XtPointer data, XmAnyCallbackStruct *cbs)
{
  Widget commscroll = (Widget)data;
  int *poslist, poscount, pos;

  if (!XmListGetSelectedPos(commscroll, &poslist, &poscount))
  {
    if (!num_items)
      return;
    XmListSelectPos(commscroll, 0, True);
  }
  if (!poscount)
    return;
  pos = poslist[0]+1;
  if (pos>num_items)
    pos = 1;

  XmListSelectPos(commscroll, pos, True);
}

/*****************************************************************************/
