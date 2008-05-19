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
#include "gvui.h"

static /* prefix to ``char Root[]...'' */
#include "interface/Material.mib"

/* private methods and variables */
/*****************************************************************************/

static void matt_sliders(Widget, XtPointer, XmScaleCallbackStruct *);
static void matt_entries(Widget w, XtPointer data, XmAnyCallbackStruct *cbs);
extern void ap_toggle(Widget w, XtPointer data, XmToggleButtonCallbackStruct *cbs);
extern void set_ap_toggle(Widget w, int *current, Appearance *ap, int flagbit);

static Widget TranslucentToggle, TranslucencyList;

static int  TranslucencyListSet  = -1;
static int TranslucentToggleSet = -1;

static struct _slider {
  float min, max;
  DrawerKeyword val;
  int mtcode;
  char *slidername, *textname;
  Widget sliderw, textw;
  float lastval;
} sliders[] = {
  { 1.0, 128.0, DRAWER_SHININESS, MT_SHININESS, "ShininessScale", "ShininessText" },
  { 0.0, 1.0,   DRAWER_KS, MT_Ks,         "SpecularScale", "SpecularText" },
  { 0.0, 1.0,   DRAWER_KD, MT_Kd,         "DiffuseScale", "DiffuseText" },
  { 0.0, 1.0,   DRAWER_KA, MT_Ka,         "AmbientScale", "AmbientText" },
  { 0.0, 1.0,   DRAWER_ALPHA, MT_ALPHA,   "AlphaScale", "AlphaText" },
};

#define         S_SHINY         0
#define         S_SPECULAR      1
#define         S_DIFFUSE       2
#define         S_AMBIENT       3
#define         S_ALPHA         4

/*****************************************************************************/

static void
translucency_callback(Widget w, XtPointer data, XmListCallbackStruct *cbs)
{
  int offset = 0;

  if ((DrawerKeyword)(long)data != DRAWER_TRANSLUCENCY) {
    OOGLError(1, "Bogus browser callback.\n");
    return;
  }
  
  TranslucencyListSet = offset = cbs->item_position + BSPTREE_BLENDING;
  if (TranslucentToggleSet == 0) {
    offset = -offset;
  }
  
  drawer_int(GEOMID(uistate.targetgeom), DRAWER_TRANSLUCENCY, offset);
}

/*****************************************************************************/

static void
translucent_callback(Widget w, XtPointer data,
		     XmToggleButtonCallbackStruct *cbs)
{
  int offset;
  if ((DrawerKeyword)(long)data != DRAWER_TRANSLUCENCY) {
    OOGLError(1, "Bogus browser callback.\n");
    return;
  }
  TranslucentToggleSet = cbs->set;
  offset = TranslucentToggleSet ? TranslucencyListSet : -TranslucencyListSet;
  drawer_int(GEOMID(uistate.targetgeom), DRAWER_TRANSLUCENCY, offset);
}

/*****************************************************************************/

void ui_load_materialpanel()
{
  int        i;
  struct _slider *s;
  Widget     mattform,
    HideButton;

  mib_Widget *mattload;
  static char Material[] = "Material";

  /*****************************************************************************/

  ui_make_panel_and_form(Material, Root, False, True, &mattform);

  /*mattload = mib_load_interface(mattform, "interface/Material.mib",
    MI_FROMFILE);*/
  mattload = mib_load_interface(mattform, Root,
                                MI_FROMSTRING);
 
  XtManageChild(mattform);

  /*****************************************************************************/

  HideButton = mib_find_name(mattload, "HideButton")->me;
  TranslucentToggle = mib_find_name(mattload, "TranslucentToggle")->me;
  TranslucencyList = mib_find_name(mattload, "TranslucencyList")->me;
  XtAddCallback(HideButton, XmNactivateCallback, (XtCallbackProc) ui_hide,
                (XtPointer) Material);

  for (i = 0, s = sliders; i < COUNT(sliders); i++, s++) {
    s->sliderw = mib_find_name(mattload, s->slidername)->me;
    s->textw = mib_find_name(mattload, s->textname)->me;
    XtAddCallback(s->sliderw, XmNdragCallback,
                  (XtCallbackProc) matt_sliders, (XtPointer)(long) i);
    XtAddCallback(s->sliderw, XmNvalueChangedCallback,
                  (XtCallbackProc) matt_sliders, (XtPointer)(long) i);
    XtAddCallback(s->textw, XmNactivateCallback,
                  (XtCallbackProc) matt_entries, (XtPointer)(long) i);
    XtAddCallback(s->textw, XmNlosingFocusCallback,
                  (XtCallbackProc) matt_entries, (XtPointer)(long) i);
  }
  XtAddCallback(TranslucentToggle, XmNvalueChangedCallback,
                (XtCallbackProc)translucent_callback,
                (XtPointer)DRAWER_TRANSLUCENCY);
  ui_build_browser(TranslucencyList, COUNT(translucencies), translucencies);
  XtAddCallback(TranslucencyList, XmNbrowseSelectionCallback,
                (XtCallbackProc)translucency_callback,
                (XtPointer)DRAWER_TRANSLUCENCY);
}


/*****************************************************************************/

static void matt_sliders(Widget w, XtPointer data, XmScaleCallbackStruct *cbs)
{
  struct _slider *s = &(sliders[(int)(long)data]);
  float val = (float)cbs->value;
  float conv = ((s->max - s->min)/100.0)*val + s->min;

  drawer_float(GEOMID(uistate.targetgeom), s->val, conv);
  s->lastval = conv;
  ui_set_ftext(s->textw, conv);
}

static void matt_entries(Widget w, XtPointer data, XmAnyCallbackStruct *cbs)
{
  struct _slider *s = &sliders[(int)(long)data];
  char *str = XmTextFieldGetString(s->textw);
  char *estr;
  float val = strtod(str, &estr);

  if (*str != '\0' && *estr == '\0') {
    if (fabs(s->lastval - val) > .001) {
      XmScaleSetValue(s->sliderw, (int)(100.0*(val - s->min)/(s->max - s->min)));
      s->lastval = val;
    }
  } else {
    ui_set_ftext(s->textw, s->lastval);
  }
}

/*****************************************************************************/

void ui_target_materialpanel(int id)
{
  int         i, ival;
  double      val;
  int         conv;
  Appearance *ap;
  Material *mt = NULL;
  struct _slider *s;

  if (drawer_get_object(id) != NULL) {
    ap = drawer_get_ap(id);
    ApGet(ap, AP_MAT, &mt);
   
    set_ap_toggle(TranslucentToggle, &TranslucentToggleSet, ap, APF_TRANSP);
    if ((ival = (ap->translucency - BSPTREE_BLENDING + 1))
	!= TranslucencyListSet) {
      TranslucencyListSet = ival;
      XmListSelectPos(TranslucencyList, ival, False);
    }
    
    if (mt == NULL) {
      return;
    }
    for (i = 0; i < COUNT(sliders); i++) {
      s = &sliders[i];
      if (MtGet(mt, s->mtcode, &val) > 0) {
        conv = (int)(100.0*(val - s->min)/(s->max - s->min));
        XmScaleSetValue(s->sliderw, conv);
        ui_set_ftext(s->textw, val);
        s->lastval = val;
      }
    }
  }
}

/*****************************************************************************/

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
