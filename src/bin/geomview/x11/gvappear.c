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
#include <stdlib.h>
static /* prefix to ``char Root[]...'' */
#include "interface/Appearance.mib"

extern Display *dpy;
extern Pixmap   geomicon;

/* private methods and variables */
/*****************************************************************************/

static void revert_appearance(Widget, XtPointer, XmAnyCallbackStruct *);
void ap_toggle(Widget, XtPointer, XmToggleButtonCallbackStruct *);
static void ap_color(Widget, XtPointer, XmAnyCallbackStruct *);
static void list_callbacks(Widget, XtPointer, XmListCallbackStruct *);
static void text_callbacks(Widget, XtPointer, XmAnyCallbackStruct *);
static Widget NormalList,
  ShadeList,
  OverrideToggle,
  FaceToggle, FaceColor,
  EdgeToggle, EdgeColor,
  NormalToggle, NormalColor,
  BBoxToggle, BBoxColor,
  VectToggle,
  TexToggle,
  TexQToggle,
  ShadeLineToggle,
  ConcaveToggle,
  FacingNormalsToggle,
  BezDiceInput, NormalScaleInput,
  LineWidthInput;

static int  NormalListSet = -1, ShadeListSet = -1;
static int  FaceSet = 0, EdgeSet = 0, NormalSet = 0, BBoxSet = 0, VectSet = 0,
  TexSet = 0, TexQSet = 0,
  ConcaveSet = 0, FacingNormalsSet = 0, OverrideSet = 0;
static int  app_loaded = 0;
static char AppearancE[] = "Appearance";

/*****************************************************************************/

static struct awid {
  enum what { TOG, BTN, TXT, LST } type; 
  char *name;
  Widget *widp;
  void (*callback)();
  XtPointer cbdata;
} awidgets[] = {
  {BTN, "HideButton", NULL, ui_hide, AppearancE },
  {BTN, "RevButton", NULL, revert_appearance, NULL },
  {TOG, "OverrideToggle",&OverrideToggle,ap_toggle,(XtPointer)DRAWER_APOVERRIDE },
  {TOG, "TexToggle", &TexToggle,  ap_toggle, (XtPointer)DRAWER_TEXTUREDRAW },
  {TOG, "TexQToggle", &TexQToggle,  ap_toggle, (XtPointer)DRAWER_TEXTUREQUAL },
  {TOG, "ShadeLineToggle",&ShadeLineToggle,ap_toggle,(XtPointer)DRAWER_SHADELINES },
  {TOG, "ConcaveToggle",&ConcaveToggle,ap_toggle,(XtPointer)DRAWER_CONCAVE },
  {TOG, "FacingNormalsToggle",&FacingNormalsToggle,ap_toggle, (XtPointer)DRAWER_EVERT },
  {TOG, "VectToggle",&VectToggle, ap_toggle, (XtPointer)DRAWER_VECTDRAW },
  {TOG, "BBoxToggle",&BBoxToggle, ap_toggle, (XtPointer)DRAWER_BBOXDRAW },
  {TOG, "NormalToggle",&NormalToggle,ap_toggle,(XtPointer)DRAWER_NORMALDRAW },
  {TOG, "FaceToggle",&FaceToggle, ap_toggle, (XtPointer)DRAWER_FACEDRAW },
  {TOG, "EdgeToggle",&EdgeToggle, ap_toggle, (XtPointer)DRAWER_EDGEDRAW },
  {BTN, "BBoxColor",&BBoxColor,   ap_color, (XtPointer)DRAWER_BBOXCOLOR },
  {BTN, "NormalColor",&NormalColor,ap_color,(XtPointer)DRAWER_NORMALCOLOR },
  {BTN, "FaceColor",&FaceColor,   ap_color, (XtPointer)DRAWER_DIFFUSE },
  {BTN, "EdgeColor",&EdgeColor,   ap_color, (XtPointer)DRAWER_EDGECOLOR },
  {TXT, "DiceInput",&BezDiceInput, text_callbacks, (XtPointer)DRAWER_BEZDICE },
  {TXT, "NormInput",&NormalScaleInput, text_callbacks, (XtPointer)DRAWER_NORMSCALE },
  {TXT, "LineInput",&LineWidthInput, text_callbacks, (XtPointer)DRAWER_LINEWIDTH },
  {LST, "NormList",&NormalList,list_callbacks, (XtPointer)DRAWER_NORMALIZATION },
  {LST, "ShadeList",&ShadeList,list_callbacks, (XtPointer)DRAWER_SHADING },
};
 

void ui_load_appearancepanel()
{
  int	     i;
  Widget     appform;

  struct awid *a;
  mib_Widget *appload;

  /*****************************************************************************/

  ui_make_panel_and_form(AppearancE, Root, False, True, &appform);

  /*appload = mib_load_interface(appform, "interface/Appearance.mib",
    MI_FROMFILE);*/
  appload = mib_load_interface(appform, Root,
			       MI_FROMSTRING);
 
  XtManageChild(appform);

  /*****************************************************************************/

  NormalList   = mib_find_name(appload, "NormList")->me;
  ShadeList    = mib_find_name(appload, "ShadeList")->me;

  for(i = 0, a = awidgets; i < COUNT(awidgets); i++, a++) {
    Widget w = mib_find_name(appload, a->name)->me;
    if (a->widp)
      *a->widp = w;
    switch(a->type) {
    case BTN:
      XtAddCallback(w, XmNactivateCallback,
		    (XtCallbackProc) a->callback, a->cbdata);
      break;
    case TOG:
      XtAddCallback(w, XmNvalueChangedCallback,
		    (XtCallbackProc) a->callback, a->cbdata);
      break;
    case TXT:
      XtAddCallback(w, XmNactivateCallback,
		    (XtCallbackProc)a->callback, a->cbdata);
      XtAddCallback(w, XmNlosingFocusCallback,
		    (XtCallbackProc)a->callback, a->cbdata);
      break;
    case LST:
      XtAddCallback(w, XmNbrowseSelectionCallback,
		    (XtCallbackProc) a->callback, a->cbdata);
    }
  }

  ui_build_browser(NormalList, COUNT(norm), norm);
  ui_build_browser(ShadeList, COUNT(shades), shades);

  app_loaded = 1;
}


/*****************************************************************************/

static void revert_appearance(Widget w, XtPointer data,
			      XmAnyCallbackStruct *cbs)
{

  drawer_set_ap( GEOMID(uistate.targetgeom), NULL, NULL );

}

/*****************************************************************************/

static void ap_color(Widget w, XtPointer data, XmAnyCallbackStruct *cbs)
{
  ui_pickcolor((int)(long)data);
}

/*****************************************************************************/

void ap_toggle(Widget w, XtPointer data,
	       XmToggleButtonCallbackStruct *cbs)
{
  drawer_int(GEOMID(uistate.targetgeom), (DrawerKeyword)(long)data, cbs->set);
}

/*****************************************************************************/

static void set_toggle(Widget w, int *current, int val)
{
  if (*current != (val != 0)) {
    *current = (val != 0);
    XmToggleButtonSetState(w, *current, False);
  }
}

void set_ap_toggle(Widget w, int *current, Appearance *ap, int flagbit)
{
  set_toggle(w, current, (ap->flag & flagbit));
}

/*****************************************************************************/

void ui_target_appearancepanel(int id)
{

  if (!app_loaded)
    return;

  if (ISGEOM(id)) {
    DGeom      *dg;
    Appearance *ap;
    int         val;

    if ((dg = (DGeom *)drawer_get_object(id))) {
      ap = drawer_get_ap(id);

      set_ap_toggle(FaceToggle, &FaceSet, ap, APF_FACEDRAW);
      set_ap_toggle(EdgeToggle, &EdgeSet, ap, APF_EDGEDRAW);
      set_ap_toggle(NormalToggle, &NormalSet, ap, APF_NORMALDRAW);
      set_ap_toggle(VectToggle, &VectSet, ap, APF_VECTDRAW);
      set_ap_toggle(TexToggle, &TexSet, ap, APF_TEXTURE);
      set_ap_toggle(TexQToggle, &TexQSet, ap, APF_TXMIPMAP|APF_TXMIPINTERP|APF_TXLINEAR);
      set_ap_toggle(ConcaveToggle, &ConcaveSet, ap, APF_CONCAVE);
      set_ap_toggle(FacingNormalsToggle, &FacingNormalsSet, ap, APF_EVERT);

      set_toggle(OverrideToggle, &OverrideSet, uistate.apoverride);
      set_toggle(BBoxToggle, &BBoxSet, dg->bboxdraw);

      if ((val = (dg->normalization + 1)) != NormalListSet) {
	NormalListSet = val;
	XmListSelectPos(NormalList, val, False);
      }

      if ((val = (ap->shading - CONSTANTSHADE + 1)) != ShadeListSet) {
	ShadeListSet = val;
	XmListSelectPos(ShadeList, val, False);
      }

      ui_set_itext(BezDiceInput, ap->dice[0]);
      ui_set_ftext(NormalScaleInput, ap->nscale);
      ui_set_itext(LineWidthInput, ap->linewidth);

    }


  }
}

/*****************************************************************************/

static void list_callbacks(Widget w, XtPointer data, XmListCallbackStruct *cbs)
{
  DrawerKeyword val = (DrawerKeyword)(long)data;
  int offset = 0;

  switch (val)
    {
    case DRAWER_NORMALIZATION:
      NormalListSet = offset = cbs->item_position - 1;
      break;
    case DRAWER_SHADING:
      ShadeListSet = offset = cbs->item_position - 1 + CONSTANTSHADE;
      break;
    default:
      OOGLError(1, "Bogus browser callback.\n");
      return;
    }

  drawer_int(GEOMID(uistate.targetgeom), val, offset);
}

/*****************************************************************************/

static int
getfloat(char *str, float *fp)
{
  if (sscanf(str, "%f", fp))
    return 1;
  else
    return 0;
}

/*****************************************************************************/

static void ui_float(char *str, DrawerKeyword key, int id, float min, float max)
{
  float f;
  if (getfloat(str, &f)) {
    if (f < min) f = min;
    else if (f > max) f = max;
    drawer_float(id, key, f);
  } else {
    ui_select(id);
  }

}

/*****************************************************************************/

static int
getint(char *str, int *ip)
{
  char *after;
  *ip = strtol(str, &after, 0);
  return str != after;

}

/*****************************************************************************/

static void ui_int(char *str, DrawerKeyword key, int id, int min, int max)
{
  int i;
  if (getint(str, &i)) {
    if (i < min) i = min;
    else if (i > max) i = max;
    drawer_int(id, key, i);
  } else {
    ui_select(id);
  }

}

/*****************************************************************************/

static void text_callbacks(Widget w, XtPointer data, XmAnyCallbackStruct *cbs)
{
  DrawerKeyword val = (DrawerKeyword)(long)data;
  char *str;
  str = XmTextFieldGetString(w);
  XmTextFieldSetInsertionPosition(w, (XmTextPosition) 0);

  switch(val) {
  case DRAWER_BEZDICE:
    ui_int(str, val, GEOMID(uistate.targetgeom), 0, 999);
    break;
  case DRAWER_NORMSCALE:
    ui_float(str, val, GEOMID(uistate.targetgeom), 0.0, 999.0);
    break;
  case DRAWER_LINEWIDTH:
    ui_int(str, val, GEOMID(uistate.targetgeom), 1, 256);
    break;
  default:
    break;
  }

  XtFree(str);
}

/*****************************************************************************/

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
