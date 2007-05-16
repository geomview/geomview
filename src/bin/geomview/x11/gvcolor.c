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
#include "mibwidgets.h"
#include "mg.h"
#include "../common/drawer.h"
#include "../common/lights.h"
#include "../common/ui.h"
#include "gvui.h"
static /* prefix to ``char Root[]...'' */
#include "interface/Color.mib"

extern Display *dpy;
extern GC       mib_gc;
extern Pixmap   geomicon;

/* public methods */
/*****************************************************************************/

void ui_config_colorpanel(char *, Color *, int, int, int);

/* private methods and variables */
/*****************************************************************************/

extern Pixel ui_RGB(Colormap cmap, int permanent, float r, float g, float b);
static void reset_color(Widget, XtPointer, XmAnyCallbackStruct *);
static void rgb_sliders(Widget, XtPointer, XmScaleCallbackStruct *);
static void set_rgb_sliders(Widget, XtPointer, XmAnyCallbackStruct *);
static void tog_cmodel(Widget, XtPointer, XmAnyCallbackStruct *);
static void show_settings(int changed);
static void set_rgbtest();
static Widget shell,
/*temp,*/
	      RGBText,
	      HSVText,
	      CModelButton,
	      RGBTest;

static struct cslider {
  char *slidername;
  char *labelname;
  Widget slider;
  Widget label;
} sliders[] = {
  { "RScale", "RLabel" },
  { "GScale", "GLabel" },
  { "BScale", "BLabel" }
};

static Window testwin;
static Color oldc;
static Color rgb, hsv;
static enum Colormodel { CM_RGB, CM_HSV } colormodel = CM_RGB;
static int myval, colortarget;

/*****************************************************************************/

void ui_load_colorpanel()
{
	int		n;

  Widget     colorform,
	     DoneButton,
	     CancelButton;

  mib_Widget *colorload;
  static char Colors[] = "Color";

/*****************************************************************************/

  shell = ui_make_panel_and_form(Colors, Root, False, True, &colorform);

/*colorload = mib_load_interface(colorform, "interface/Color.mib",
		MI_FROMFILE);*/
  colorload = mib_load_interface(colorform, Root,
		MI_FROMSTRING);
 
  XtManageChild(colorform);

/*****************************************************************************/

  DoneButton = mib_find_name(colorload, "DoneButton")->me;
  CancelButton = mib_find_name(colorload, "CancelButton")->me;
  XtAddCallback(DoneButton, XmNactivateCallback, (XtCallbackProc) ui_hide,
		Colors);
  XtAddCallback(CancelButton, XmNactivateCallback, (XtCallbackProc) reset_color,
		NULL);

  for(n = 0; n < COUNT(sliders); n++) {
    struct cslider *cs = &sliders[n];
    cs->slider = mib_find_name(colorload, cs->slidername)->me;
    cs->label = mib_find_name(colorload, cs->labelname)->me;
    XtAddCallback(cs->slider, XmNdragCallback, (XtCallbackProc) rgb_sliders,
			(XtPointer)(long)n);
    XtVaSetValues(cs->slider, XmNmaximum, 255, NULL);
  }

  RGBText = mib_find_name(colorload, "RGBText")->me;
  XtAddCallback(RGBText, XmNactivateCallback, (XtCallbackProc) set_rgb_sliders,
		(XtPointer) CM_RGB);
  HSVText = mib_find_name(colorload, "HSVText")->me;
  XtAddCallback(HSVText, XmNactivateCallback, (XtCallbackProc) set_rgb_sliders,
		(XtPointer) CM_HSV);

  CModelButton = mib_find_name(colorload, "ColorModel")->me;
  XtAddCallback(CModelButton, XmNactivateCallback,
		(XtCallbackProc)tog_cmodel, (XtPointer)NULL);

/*****************************************************************************/

  RGBTest = mib_find_name(colorload, "ColorTest")->me;

  tog_cmodel(NULL, NULL, NULL);	/* Toggle cmodel button for consistent layout */

  rgb.r = 0.0; rgb.g = 0.0; rgb.b = 0.0;
  oldc.r = 0.0; oldc.g = 0.0; oldc.b = 0.0;

  show_settings(0);

}


/*****************************************************************************/

void ui_show_colorpanel()
{
  if(!XtIsRealized(shell)) {
    XSetWindowAttributes swa;

    XtRealizeWidget(shell);

    swa.colormap = gvcolormap;
    swa.backing_store = NotUseful;
    swa.background_pixel = swa.border_pixel = ui_RGB(gvcolormap, 0, 1., 1., 1.);
    testwin = XCreateWindow(dpy, XtWindow(RGBTest), 1, 1, 63-2, 95-2, 0,
		gvbitdepth, InputOutput, gvvisual,
		CWColormap|CWBackPixel|CWBorderPixel|CWBackingStore, &swa);
    XMapRaised(dpy, testwin);
  }

  XRaiseWindow(dpy, XtWindow(shell));

  set_rgbtest(); /* Raise color test rectangle and set its color */
}

/*****************************************************************************/

static void reset_color(Widget w, XtPointer data, XmAnyCallbackStruct *cbs)
{
  int changed = memcmp(&rgb, &oldc, sizeof(Color));
  rgb = oldc;
  rgb2hsv(&rgb, &hsv);
  if(changed)
    show_settings(1);
  ui_hide(w, "Color", NULL);
}

/*****************************************************************************/

static void tog_cmodel(Widget w, XtPointer data, XmAnyCallbackStruct *cbs)
{
  static char *labels[] = { "RGB", "HSV" };	/* Indexed by CM_ colormodel */
  char *abc = NULL, label[2];
  XmString lbl;
  int i;

  colormodel = (colormodel == CM_RGB) ? CM_HSV : CM_RGB;
  abc = labels[colormodel];

  lbl = XmStringCreateSimple(abc);
  XtVaSetValues(CModelButton, XmNlabelString, lbl, NULL);
  XmStringFree(lbl);

  for(i = 0; i < 3; i++) {
    label[0] = abc[i];
    label[1] = '\0';
    lbl = XmStringCreateSimple(label);
    XtVaSetValues(sliders[i].label, XmNlabelString, lbl, NULL);
    XmStringFree(lbl);
  }

  show_settings(0);
}
    

static void set_rgbtest()
{
  if (XtIsRealized(shell))
  {
    XSetWindowBackground(dpy, testwin, ui_RGB(gvcolormap, 0, rgb.r, rgb.g, rgb.b));
    XMapRaised(dpy, testwin);
    XClearWindow(dpy, testwin);
  }
}

/*****************************************************************************/

static void show_settings(int changed)
{
  char str[100];
  float *cmc = (colormodel == CM_RGB) ? &rgb.r : &hsv.r;
  int i;

  sprintf(str, "%.3f %.3f %.3f", rgb.r, rgb.g, rgb.b);
  XmTextFieldSetString(RGBText, str);
  sprintf(str, "%.3f %.3f %.3f", hsv.r, hsv.g, hsv.b);
  XmTextFieldSetString(HSVText, str);
  set_rgbtest();

  for(i = 0; i < 3; i++) {
    int v = 255 * cmc[i];
    XmScaleSetValue(sliders[i].slider, v<0 ? 0 : v>255 ? 255 : v);
  }

  if(changed)
    drawer_color(colortarget, (DrawerKeyword)myval, &rgb);
}

/*****************************************************************************/

static void set_rgb_sliders(Widget w, XtPointer data, XmAnyCallbackStruct *cbs)
{
  char *str;
  Color c;
  int ok = 0;

  str = XmTextFieldGetString(w);

  if (3 == sscanf(str,"%f %f %f", &c.r, &c.g, &c.b))
  {
    ok = 1;
    switch((int)(long)data) {
    case CM_RGB:
	rgb = c;
	rgb2hsv(&rgb, &hsv);
	break;
    case CM_HSV:
	hsv = c;
	hsv2rgb(&hsv, &rgb);
	break;
    }
  }

  show_settings(ok);
  XtFree(str);
}

/*****************************************************************************/

static void rgb_sliders(Widget w, XtPointer data, XmScaleCallbackStruct *cbs)
{
  float *cmc = (colormodel == CM_RGB) ? &rgb.r : &hsv.r;

  /* Set {rgb,hsv}.{r,g,b} according to current color model and callback-data */
  cmc[(int)(long)data] = ((float)cbs->value)/255.0;
  if(colormodel == CM_RGB)
    rgb2hsv(&rgb, &hsv);
  else
    hsv2rgb(&hsv, &rgb);

  show_settings(1);
}

/*****************************************************************************/

void ui_config_colorpanel(char *name, Color *old, int index, int val,
				int id)
{
  rgb = *old;
  oldc = *old;
  rgb2hsv(&rgb, &hsv);

  myval = val;
  colortarget = id;

  XtVaSetValues(shell, XmNtitle, name, NULL);
  show_settings(0);
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
