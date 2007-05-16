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

#include "mg.h"
#include "../common/drawer.h"
#include "../common/ui.h"
#include "gvui.h"
static /* prefix to ``char Root[]...'' */
#include "interface/Tool.mib"
#include "X11/StringDefs.h"

#include "bitmaps/rotate"
#include "bitmaps/translate"
#include "bitmaps/scale"
#include "bitmaps/fly"
#include "bitmaps/zoom"
#include "bitmaps/orbit"
#include "bitmaps/stop"
#include "bitmaps/lookat"
#include "bitmaps/center"
#include "bitmaps/reset"

extern Display *dpy;
extern UIState uistate;
extern Pixmap  geomicon;

/* public methods */
/*****************************************************************************/

void   ui_load_toolpanel(void);
void   ui_tool_targetset(char *);
void   ui_tool_centerset(char *);
void   ui_tool_center_origin_set(int use_bbox);

/* private methods and variables */
/*****************************************************************************/

static void load_tool_bitmaps(mib_Widget *);
static void target_set(Widget, XtPointer, XmAnyCallbackStruct *);
static void center_set(Widget, XtPointer, XmAnyCallbackStruct *);
static void center_origin_set(Widget, XtPointer, XmAnyCallbackStruct *);

#if defined(__STDC__) || defined(__ANSI_CPP__)
#define BWH(name) (char *)name##_bits, name##_width, name##_height
#else
#define BWH(name) (char *)name/**/_bits, name/**/_width, name/**/_height
#endif

static struct toolbutton {
  char *name;
  char *bits;
  int width, height;
  int istool;
  XtPointer data;
  Widget button;
  Pixmap norm_pix, rev_pix;
} toolbuttons[] = {

  { "RotateButton",	BWH(rotate),	True,	OBJROTATE },
  { "TranslateButton",	BWH(translate),	True,	OBJTRANSLATE },
  { "ScaleButton",	BWH(scale),	True,	OBJSCALE },
  { "ZoomButton",	BWH(zoom),	True,	OBJZOOM },
  { "OrbitButton",	BWH(orbit),	True,	OBJORBIT },
  { "FlyButton",	BWH(fly),	True,	OBJFLY },
  { "StopButton",	BWH(stop),	False,	(XtPointer)ACTION_STOP },
  { "LookAtButton",	BWH(lookat),	False,	(XtPointer)ACTION_LOOK },
  { "CenterButton",	BWH(center),	False,	(XtPointer)ACTION_CENTER },
  { "ResetButton",	BWH(reset),	False,	(XtPointer)ACTION_RESET },
};

static struct toolbutton *last;
static Widget TargetText, CenterText, BBoxCenter;


/*****************************************************************************/

void ui_load_toolpanel()
{
  Widget     toolform,
	     HideButton,
	     MainButton;

  mib_Widget *toolload;
  static char Tools[] = "Tools";

/*****************************************************************************/

  ui_make_panel_and_form(Tools, Root, False, False, &toolform);

/*toolload = mib_load_interface(toolform, "interface/Tool.mib", MI_FROMFILE);*/
  toolload = mib_load_interface(toolform, Root, MI_FROMSTRING);
  XtVaSetValues(toolload->me, XmNresizePolicy, XmRESIZE_NONE, NULL);

  XtManageChild(toolform);

/*****************************************************************************/

  HideButton = mib_find_name(toolload, "HideButton")->me;
  MainButton = mib_find_name(toolload, "MainButton")->me;
  TargetText = mib_find_name(toolload, "TargetText")->me;
  CenterText = mib_find_name(toolload, "CenterText")->me;
  BBoxCenter = mib_find_name(toolload, "BBoxCenterToggle")->me;

  load_tool_bitmaps(toolload);
  XtAddCallback(HideButton, XmNactivateCallback, (XtCallbackProc) ui_hide,
			Tools);
  XtAddCallback(MainButton, XmNactivateCallback, (XtCallbackProc) ui_show,
			"Geomview");
  XtAddCallback(TargetText, XmNactivateCallback, (XtCallbackProc) target_set,
			NULL);
  XtAddCallback(CenterText, XmNactivateCallback, (XtCallbackProc) center_set,
			NULL);
  XtAddCallback(BBoxCenter, XmNvalueChangedCallback,
		(XtCallbackProc) center_origin_set,
		NULL);
}


/*****************************************************************************/

void ui_event_mode(char *mode)
{
  struct toolbutton *tb;
  int i;

  for(i = 0, tb = toolbuttons; i < COUNT(toolbuttons); i++, tb++) {
    if(tb->istool && tb->button!=NULL && !strcmp(mode, tb->data)) {
	XtVaSetValues(last->button, XmNlabelPixmap, last->norm_pix, NULL);
	XtVaSetValues(tb->button, XmNlabelPixmap, tb->norm_pix, NULL);
	XtVaSetValues(tb->button, XmNlabelPixmap, tb->rev_pix, NULL);
	last = tb;
    }
  }
}

/*****************************************************************************/

static void modetool_callback(Widget w, XtPointer data,
				XmPushButtonCallbackStruct *cbs)
{
  gv_event_mode( (char *)data );
}

/*****************************************************************************/

static void action_callback(Widget w, XtPointer data,
				XmPushButtonCallbackStruct *cbs)
{
  int val = (int)(long) data;
  int id = uistate.targetid;

  switch (val)
  {
    case ACTION_STOP:
      drawer_stop(NOID);
      break;
    case ACTION_LOOK:
      gv_look(GEOMID(uistate.targetgeom), CAMID(uistate.targetcam));
      break;
    case ACTION_CENTER:
      drawer_center(id);
      break;
    case ACTION_RESET:
      drawer_center(NOID);
      break;
  }
  ui_action(val);
}

/*****************************************************************************/

static void load_tool_bitmaps(mib_Widget *toolload)
{
  int    depth;
  Pixel  blue, red, white;
  struct toolbutton *tb;
  int i;
  Colormap cm;
  Screen *s = DefaultScreenOfDisplay(dpy);

  XtVaGetValues(toolload->me, XtNcolormap, &cm, NULL);
  blue =  ui_RGB(cm, 1, 0.0, 0.0, 1.0);
  red =   ui_RGB(cm, 1, 1.0, 0.0, 0.0);
  white = ui_RGB(cm, 1, 1.0, 1.0, 1.0);
  if((red == white || blue == white) && cm == DefaultColormapOfScreen(s)) {
	white = WhitePixelOfScreen(s);
	if(red == white) red = BlackPixelOfScreen(s);
	if(blue == white) blue = BlackPixelOfScreen(s);
  }
	

/*****************************************************************************/

  last = &toolbuttons[0];
  for(i = 0, tb = toolbuttons; i < COUNT(toolbuttons); i++, tb++) {
    Pixel colored = tb->istool ? blue : red;
    tb->button = mib_find_name(toolload, tb->name)->me;
    XtVaGetValues(tb->button, XmNdepth, &depth, NULL);
    tb->norm_pix = XCreatePixmapFromBitmapData(dpy, DefaultRootWindow(dpy),
		tb->bits, tb->width, tb->height, colored, white, depth);
    tb->rev_pix = XCreatePixmapFromBitmapData(dpy, DefaultRootWindow(dpy),
		tb->bits, tb->width, tb->height, white, colored, depth);

    XtVaSetValues(tb->button, XmNlabelType, XmPIXMAP,
			XmNlabelPixmap, tb==last ? tb->rev_pix : tb->norm_pix,
			XmNarmPixmap, tb->rev_pix, NULL);
    XtAddCallback(tb->button, XmNarmCallback,
		(XtCallbackProc)(tb->istool ? modetool_callback : action_callback),
		tb->data);
  }
}

/*****************************************************************************/

static void target_set(Widget w, XtPointer data, XmAnyCallbackStruct *cbs)
{
  char *str;
  int   id;

  str = XmTextFieldGetString(w);
  if ((id = drawer_idbyname(str)))
    set_ui_target_id(id);
  ui_select(uistate.targetid);
  XtFree(str);
}

/*****************************************************************************/

void ui_tool_targetset(char *target)
{
  char *str;

  if (TargetText == NULL)
    return;

  str = XmTextFieldGetString(TargetText);

  if (strcmp(target, str))
    XmTextFieldSetString(TargetText, target);

  XtFree(str);
}

/*****************************************************************************/

static void center_set(Widget w, XtPointer data, XmAnyCallbackStruct *cbs)
{
  char *str;
  int   id;

  str = XmTextFieldGetString(w);
  if ((id = drawer_idbyname(str)))
    set_ui_center(id);
  ui_select(uistate.centerid);
  XtFree(str);
}

/*****************************************************************************/

void ui_tool_centerset(char *center)
{
  char *str;

  if (CenterText == NULL)
    return;

  str = XmTextFieldGetString(CenterText);

  if (strcmp(center,str))
    XmTextFieldSetString(CenterText, center);

  XtFree(str);
}

/*****************************************************************************/

static void
center_origin_set(Widget w, XtPointer data, XmAnyCallbackStruct *cbs)
{
  set_ui_center_origin(XmToggleButtonGetState(w));
}

/*****************************************************************************/

void ui_tool_center_origin_set(int use_bbox_center)
{
  if (BBoxCenter == NULL)
    return;

  XmToggleButtonSetState(BBoxCenter, use_bbox_center != 0, False);
}

/*****************************************************************************/

