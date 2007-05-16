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
#include "interface/Lights.mib"

#define		MAXLIGHTS	8
#define		LIGHTINDEX      255


extern Display *dpy;
extern UIState uistate;
extern int light_count();
extern void set_light(int);
extern Pixmap geomicon;

/* private methods and variables */
/*****************************************************************************/

static void select_light(Widget, XtPointer, XmListCallbackStruct *);
static void intensity_light(Widget, XtPointer, XmScaleCallbackStruct *);
static void toggle_lights(Widget, XtPointer, XmToggleButtonCallbackStruct *);
static void color_light(Widget, XtPointer, XmAnyCallbackStruct *);
static void light_add(Widget, XtPointer, XmAnyCallbackStruct *);
static void light_del(Widget, XtPointer, XmAnyCallbackStruct *);
static Widget  ltoggle,
	       lintensity,
	       lightslist;
static char   *lights[MAXLIGHTS+1];
/*static int    pos_later = 0, posx = 0, posy = 0;
  static char   geometry[40];*/


/*****************************************************************************/

void ui_load_lightspanel()
{
  int	     n;
  Widget     lightsform,
	     lightsadd,
	     lightsdel,
	     ColButton,
	     HideButton;
  char	     buf[100];

  mib_Widget *lightsload;
  static char Lights[] = "Lights";

/*****************************************************************************/

  ui_make_panel_and_form(Lights, Root, True, True, &lightsform);

/*lightsload = mib_load_interface(lightsform, "interface/Lights.mib",
		MI_FROMFILE);*/
  lightsload = mib_load_interface(lightsform, Root,
		MI_FROMSTRING);
 
  XtManageChild(lightsform);

/*****************************************************************************/

  ColButton  = mib_find_name(lightsload, "ColorButton")->me;
  HideButton = mib_find_name(lightsload, "HideButton")->me;
  lightslist = mib_find_name(lightsload, "LightsList")->me;
  lintensity = mib_find_name(lightsload, "IntensityScale")->me;
  ltoggle    = mib_find_name(lightsload, "LightsToggle")->me;
  lightsadd  = mib_find_name(lightsload, "AddButton")->me;
  lightsdel  = mib_find_name(lightsload, "DeleteButton")->me;

  XtAddCallback(ColButton, XmNactivateCallback, (XtCallbackProc) color_light,
			(XtPointer) NULL);
  XtAddCallback(HideButton, XmNactivateCallback,
			(XtCallbackProc) ui_hide, (XtPointer) Lights);
  XtAddCallback(lightslist, XmNbrowseSelectionCallback,
			(XtCallbackProc) select_light, (XtPointer) &lintensity);
  XtAddCallback(lintensity, XmNdragCallback, (XtCallbackProc) intensity_light,
			(XtPointer) NULL);
  XtAddCallback(lintensity, XmNvalueChangedCallback,
			(XtCallbackProc) intensity_light, (XtPointer) NULL);
  XtAddCallback(ltoggle, XmNvalueChangedCallback, (XtCallbackProc) toggle_lights,
			(XtPointer) NULL);
  XtAddCallback(lightsadd, XmNactivateCallback, (XtCallbackProc) light_add,
			(XtPointer) NULL);
  XtAddCallback(lightsdel, XmNactivateCallback, (XtCallbackProc) light_del,
			(XtPointer) NULL);

/*****************************************************************************/
/* Initialize Panel Variables */

  lights[0] = "ambient";
  for (n = 1; n <= MAXLIGHTS; ++n)
  {
    sprintf(buf, "light %1d", n);
    lights[n] = strdup(buf);
  }
  ui_build_browser(lightslist, light_count(), lights);

}


/*****************************************************************************/

void ui_show_lightspanel()
{
  set_light(light_count() > 0 ? 1 : 0);
  XmListSelectPos(lightslist, uistate.current_light + 1, False);
}

/*****************************************************************************/

void set_light_display(int lightno)
{
  extern float light_intensity();

  XmScaleSetValue(lintensity, (int) (light_intensity()*100.0));
}

/*****************************************************************************/

void ui_light_button()
{
  int state = (uistate.lights_shown != 0);
  XmToggleButtonSetState(ltoggle, state, False);
}

/*****************************************************************************/

void ui_lights_changed()
{
  ui_build_browser(lightslist, light_count(), lights);
  set_light(uistate.current_light);
}

/*****************************************************************************/

static void select_light(Widget w, XtPointer data, XmListCallbackStruct *cbs)
{
  extern float light_intensity();
  Widget intensity = (Widget)*((Widget *)data);

  set_light(cbs->item_position-1);
  XmScaleSetValue(intensity, (int) (light_intensity()*100.0));
}

/*****************************************************************************/

static void color_light(Widget w, XtPointer data, XmAnyCallbackStruct *cbs)
{
  extern Color *light_color(); /* in clights.c */

  ui_config_colorpanel("Light Color", light_color(), LIGHTINDEX,
			DRAWER_LIGHTCOLOR, 0);
  ui_showpanel(P_COLOR, 1);
}

/*****************************************************************************/

static void light_add(Widget w, XtPointer data, XmAnyCallbackStruct *cbs)
{
  extern void add_light();

  if (light_count() > 8)
  {
    OOGLError(0, "Can't have more than 8 lights\n");
    return;
  }
  add_light();
}

/*****************************************************************************/

static void light_del(Widget w, XtPointer data, XmAnyCallbackStruct *cbs)
{
  extern void delete_light();

  if (uistate.current_light == 0)
    return;
  delete_light();
}

/*****************************************************************************/

static void toggle_lights(Widget w, XtPointer data,
				XmToggleButtonCallbackStruct *cbs)
{
  extern void light_edit_mode(int);

  light_edit_mode(cbs->set);
}

/*****************************************************************************/

static void intensity_light(Widget w, XtPointer data,
                                XmScaleCallbackStruct *cbs)
{
  float f = (float)cbs->value/100.0;
  drawer_float(GEOMID(uistate.targetgeom), DRAWER_LIGHT_INTENSITY, f);
}

/*****************************************************************************/
