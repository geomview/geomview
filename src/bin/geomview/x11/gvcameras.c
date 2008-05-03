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

#include <X11/X.h>
#include "mibload.h"
#include "mibwidgets.h"
#include "../common/drawer.h"
#include "../common/ui.h"
#include "../common/lang.h"
#include "gvui.h"
static /* prefix to ``char Root[]...'' */
#include "interface/Cameras.mib"

extern Display *dpy;
extern Pixmap   geomicon;

/* public methods */
/*****************************************************************************/

void   ui_target_cameraspanel(int);

/* private methods and variables */
/*****************************************************************************/

static void background_color(Widget, XtPointer, XmAnyCallbackStruct *);
static void soft_shading(Widget, XtPointer, XmToggleButtonCallbackStruct *);
static void draw_cams(Widget, XtPointer, XmToggleButtonCallbackStruct *);
static void draw_sphere(Widget, XtPointer, XmToggleButtonCallbackStruct *);
static void toggle_dither(Widget, XtPointer, XmToggleButtonCallbackStruct *);
static void toggle_singlebuffer(Widget w, XtPointer data, XmToggleButtonCallbackStruct *cbs);
static void list_callbacks(Widget, XtPointer, XmListCallbackStruct *);
static void text_callbacks(Widget, XtPointer, XmAnyCallbackStruct *);
static void depth_callbacks(Widget, XtPointer, XmAnyCallbackStruct *);
static void ui_float(char *, DrawerKeyword, int, float, float);

static Widget ProjectList,
	      CurrentCam,
	      ModelList,
	      SphereToggle,
	      DitherToggle,
	      SingleBuffer,
	      SoftShade,
	      DrawCams,
	      NearClipText,
	      FarClipText,
	      FOVText,
	      FocalText,
	      LinesText;
mib_Widget    *DepthMenu;


static int    currentcam_id, modelpos, projpos;
static int    dither_state = 0;
static int    depth_state = 0;
static float  keepfov;
static char   lastcamname[255];
static char   store[255];
/*static char   geometry[40];*/
/*static int    pos_later = 0, posx = 0, posy = 0;*/


/*****************************************************************************/

void ui_load_cameraspanel()
{
  int	     i;
  Widget     camerasform,
	     HideButton,
	     TempWidget;

  mib_Widget *camerasload;
  static char Cameras[] = "Cameras";

/*****************************************************************************/

  ui_make_panel_and_form(Cameras, Root, False, True, &camerasform);

/*camerasload = mib_load_interface(camerasform, "interface/Cameras.mib",
		MI_FROMFILE);*/
  camerasload = mib_load_interface(camerasform, Root,
		MI_FROMSTRING);
 
  XtManageChild(camerasform);

/*****************************************************************************/

  HideButton   = mib_find_name(camerasload, "HideButton")->me;
  ProjectList  = mib_find_name(camerasload, "ProjectList")->me;
  ModelList    = mib_find_name(camerasload, "SpaceModelList")->me;
  CurrentCam   = mib_find_name(camerasload, "CurrentLabel")->me;
  DrawCams     = mib_find_name(camerasload, "DrawCamsButton")->me;
  SphereToggle = mib_find_name(camerasload, "DrawSphereToggle")->me;
  DitherToggle = mib_find_name(camerasload, "DitherToggle")->me;
  SingleBuffer = mib_find_name(camerasload, "Singlebuffer")->me;
  SoftShade    = mib_find_name(camerasload, "SoftShadeButton")->me;
  NearClipText = mib_find_name(camerasload, "NCText")->me;
  FarClipText  = mib_find_name(camerasload, "FCText")->me;
  FOVText      = mib_find_name(camerasload, "FOVText")->me;
  FocalText    = mib_find_name(camerasload, "FLText")->me;
  LinesText    = mib_find_name(camerasload, "LCText")->me;
  DepthMenu    = mib_find_name(camerasload, "DepthMenu");

  XtAddCallback(HideButton, XmNactivateCallback, (XtCallbackProc) ui_hide,
			(XtPointer)Cameras);
  XtAddCallback(ProjectList, XmNbrowseSelectionCallback,
			(XtCallbackProc) list_callbacks,
			(XtPointer) DRAWER_PROJECTION);
  XtAddCallback(ModelList, XmNbrowseSelectionCallback,
			(XtCallbackProc) list_callbacks,
			(XtPointer) NULL);
  XtAddCallback(DrawCams, XmNvalueChangedCallback,
			(XtCallbackProc) draw_cams,
			(XtPointer) NULL);
  XtAddCallback(SphereToggle, XmNvalueChangedCallback,
			(XtCallbackProc) draw_sphere,
			(XtPointer) NULL);
  XtAddCallback(DitherToggle, XmNvalueChangedCallback,
			(XtCallbackProc) toggle_dither,
			(XtPointer) NULL);
  XtAddCallback(SingleBuffer, XmNvalueChangedCallback,
			(XtCallbackProc) toggle_singlebuffer,
			(XtPointer) NULL);

  XtAddCallback(NearClipText, XmNactivateCallback,
		(XtCallbackProc) text_callbacks, (XtPointer) DRAWER_NEAR);
  XtAddCallback(NearClipText, XmNlosingFocusCallback,
		(XtCallbackProc) text_callbacks, (XtPointer) DRAWER_NEAR);

  XtAddCallback(FarClipText, XmNactivateCallback,
		(XtCallbackProc) text_callbacks, (XtPointer) DRAWER_FAR);
  XtAddCallback(FarClipText, XmNlosingFocusCallback,
		(XtCallbackProc) text_callbacks, (XtPointer) DRAWER_FAR);

  XtAddCallback(FOVText, XmNactivateCallback,
		(XtCallbackProc) text_callbacks, (XtPointer) DRAWER_FOV);
  XtAddCallback(FOVText, XmNlosingFocusCallback,
		(XtCallbackProc) text_callbacks, (XtPointer) DRAWER_FOV);

  XtAddCallback(FocalText, XmNactivateCallback,
		(XtCallbackProc) text_callbacks, (XtPointer) DRAWER_FOCALLENGTH);
  XtAddCallback(FocalText, XmNlosingFocusCallback,
		(XtCallbackProc) text_callbacks, (XtPointer) DRAWER_FOCALLENGTH);

  XtAddCallback(LinesText, XmNactivateCallback,
		(XtCallbackProc) text_callbacks, (XtPointer) DRAWER_LINE_ZNUDGE);
  XtAddCallback(LinesText, XmNlosingFocusCallback,
		(XtCallbackProc) text_callbacks, (XtPointer) DRAWER_LINE_ZNUDGE);

  for (i=0; i<3; i++)
      XtAddCallback(((mib_Menu *)DepthMenu->myres)->items[i], 
		    XmNactivateCallback, (XtCallbackProc) depth_callbacks,
		    (XtPointer)(long) i);
		    
/*****************************************************************************/

  TempWidget = mib_find_name(camerasload, "BackColorButton")->me;
  XtAddCallback(TempWidget, XmNactivateCallback,
		(XtCallbackProc) background_color, (XtPointer) NULL);

  /* Determine MG type at run time... */
  if(_mgf.mg_devno == MGD_X11) {	/* if we're using X11 driver */
    XtUnmanageChild(SingleBuffer);
  } else {
    XtUnmanageChild(DepthMenu->me);
    XtAddCallback(SoftShade, XmNvalueChangedCallback,
		(XtCallbackProc) soft_shading, (XtPointer) NULL);
  }

  ui_build_browser(ProjectList, COUNT(proj), proj);
  ui_build_browser(ModelList, COUNT(mdl), mdl);

/*****************************************************************************/

  lastcamname[0] = '\0';

  modelpos = -1;
  projpos = -1;
  keepfov = 0.0;
}

/*****************************************************************************/

static void background_color(Widget w, XtPointer data, XmAnyCallbackStruct *cbs)
{
  ui_pickcolor(DRAWER_BACKCOLOR);
}

/*****************************************************************************/

static void soft_shading(Widget w, XtPointer data,
				XmToggleButtonCallbackStruct *cbs)
{

   gv_soft_shader(currentcam_id, cbs->set ? ON_KEYWORD : OFF_KEYWORD);
}

/*****************************************************************************/

static void draw_cams(Widget w, XtPointer data,
				XmToggleButtonCallbackStruct *cbs)
{
  int    number;
  DView *dv;

  dv = (DView *)drawer_get_object(currentcam_id);
  number = dv ? !dv->cameradraw : 1;
  drawer_int(currentcam_id, DRAWER_CAMERADRAW, number);
}

/*****************************************************************************/

static void draw_sphere(Widget w, XtPointer data,
				XmToggleButtonCallbackStruct *cbs)
{
  drawer_int(currentcam_id, DRAWER_HSPHERE, cbs->set);
}

/*****************************************************************************/

static void toggle_singlebuffer(Widget w, XtPointer data,
				XmToggleButtonCallbackStruct *cbs)
{
  DView *dv;

  if((dv = (DView *)drawer_get_object(currentcam_id)) == NULL)
    return;
  mgctxselect(dv->mgctx);
  mgctxset(cbs->set ? MG_UNSETOPTIONS : MG_SETOPTIONS, MGO_DOUBLEBUFFER,
	MG_END);
  gv_redraw(dv->id);
}

/*****************************************************************************/

static void toggle_dither(Widget w, XtPointer data,
				XmToggleButtonCallbackStruct *cbs)
{
  DView *dv;

  dither_state = cbs->set;

  if((dv = (DView *)drawer_get_object(currentcam_id)) == NULL)
    return;
  mgctxselect(dv->mgctx);
  mgctxset(MG_DITHER, cbs->set, MG_END);
  gv_redraw(dv->id);
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

    if(getfloat(str, &f) && strcmp(str,store)) {
        if(f < min) f = min;
        else if(f > max) f = max;
        drawer_float(id, key, f);
    } else {
        ui_select(id);
    }
}

/*****************************************************************************/

static void text_callbacks( Widget		  w,
			    XtPointer		  data,
			    XmAnyCallbackStruct *cbs )
{
  DrawerKeyword val = (DrawerKeyword)(long)data;
  char *str;
  DView *dv;
  int persp = 0;
  str = XmTextFieldGetString(w);
/*
  XmTextFieldSetInsertionPosition(w, (XmTextPosition) 0);
*/

  switch(val)
  {
	case DRAWER_NEAR:
	case DRAWER_FAR:
	ui_float(str, val, currentcam_id, -1e20, 1e20);
	break;

	case DRAWER_FOV:
	dv = (DView *)drawer_get_object(currentcam_id);
	if(dv)
	    CamGet(dv->cam, CAM_PERSPECTIVE, &persp);
	ui_float(str, val, currentcam_id, 1e-7, persp ? 179.999 : 1e20);
	break;

	case DRAWER_FOCALLENGTH:
	ui_float(str, val, currentcam_id, 1e-20, 1e20);
	break;

	case DRAWER_LINE_ZNUDGE:
	ui_float(str, val, currentcam_id, -10000., 10000.);
	break;

	default:
	break;
  
  }

  XtFree(str);

}

/*****************************************************************************/

static void list_callbacks( Widget		  w,
			    XtPointer		  data,
			    XmListCallbackStruct *cbs )
{
  int id;
  DrawerKeyword val = (DrawerKeyword)(long)data;

  if (!val)
  {
    modelpos = cbs->item_position;
    gv_hmodel(currentcam_id,
	      hmodelkeyword("ModelBrowserProc",
			    (HModelValue)(cbs->item_position - 1)));
    return;
  }
  else
  if (val == DRAWER_PROJECTION)
  {
    projpos = cbs->item_position;
    id = currentcam_id;
    drawer_int( id, val, cbs->item_position - 1);
    return;
  }
  else
    OOGLError(1,"invalid val (%d) in ObscureBrowserProc", val);

  return;
}

/*****************************************************************************/

static void depth_callbacks( Widget		  w,
			    XtPointer		  data,
			    XmAnyCallbackStruct *cbs )
{
    DView *dv;
    int type = (int)(long)data;

    depth_state = type;

    dv = (DView *)drawer_get_object(currentcam_id);
    mgctxselect(dv->mgctx);
    mgctxset(MG_DEPTHSORT, type, MG_END);
    gv_redraw(dv->id);

    return;
}

/*****************************************************************************/

void   ui_target_cameraspanel(int id)
{
  char     *name;
  XmString xname;
  DView   *dv;
  float    f;
  int      persp, opts, exists = 0;
  extern   int id_exists(int);
  extern   int real_id(int);


  if ((exists = id_exists(real_id(id))))
    name = drawer_id2name(real_id(id));
  else
    name = "No Camera";

  currentcam_id = id;

  if (strcmp(name, lastcamname))
  {
    sprintf(lastcamname, "%s", name);

    xname = XmStringCreateSimple(name);
    XtVaSetValues(CurrentCam, XmNlabelString, xname, NULL);
    XmStringFree(xname);
  }

  if (exists)
  {
    dv = (DView *) drawer_get_object(currentcam_id);

    if ((dv->hsphere != NULL) != XmToggleButtonGetState(SphereToggle))
      XmToggleButtonSetState(SphereToggle, dv->hsphere != NULL, False);

    if (dv->cameradraw != XmToggleButtonGetState(DrawCams))
      XmToggleButtonSetState(DrawCams, dv->cameradraw, False);

    mgctxselect(dv->mgctx);
    mgctxget(MG_SETOPTIONS, &opts);
    if(((opts & MGO_DOUBLEBUFFER) == 0) != XmToggleButtonGetState(SingleBuffer))
	XmToggleButtonSetState(SingleBuffer, (opts & MGO_DOUBLEBUFFER) == 0, False);

    if ((dv->shader != NULL) != XmToggleButtonGetState(SoftShade))
        XmToggleButtonSetState(SoftShade, dv->shader != NULL, False);

    update_x11_depth(dv, DepthMenu, DitherToggle, &depth_state, &dither_state);

    if (modelpos != (int)(dv->hmodel + 1))
    {
      XmListSelectPos(ModelList, dv->hmodel + 1, False);
      modelpos = dv->hmodel + 1;
    }

    CamGet(dv->cam, CAM_PERSPECTIVE, &persp);
    if (projpos != (persp + 1))
    {
      XmListSelectPos(ProjectList, persp + 1, False);
      projpos = persp + 1;
    }

    CamGet(dv->cam, CAM_NEAR, &f);
    ui_set_ftext(NearClipText, f);
    CamGet(dv->cam, CAM_FAR, &f);
    ui_set_ftext(FarClipText, f);
    CamGet(dv->cam, CAM_FOV, &f);
    if (f != keepfov)
    {
      keepfov = f;
      ui_set_ftext(FOVText, f);
    }
    CamGet(dv->cam, CAM_FOCUS, &f);
    ui_set_ftext(FocalText, f);
    ui_set_itext(LinesText, (int)dv->lineznudge);
  }

}

/*****************************************************************************/
void update_x11_depth(DView *dv, mib_Widget *DepthMenu, Widget DitherToggle,
			int *depth_state, int *dither_state)
{
    int depth, bitdepth, dither;

    if(XtIsManaged(DepthMenu->me)) {
	mgctxselect(dv->mgctx);
	mgctxget(MG_DEPTHSORT, &depth);

	if ((depth >= 0) && (depth < 3))
	{
	  if (*depth_state != depth)
	  {
	    XtVaSetValues(DepthMenu->me,
		  XmNmenuHistory, ((mib_Menu *)DepthMenu->myres)->items[depth],
		  NULL);
	    *depth_state = depth;
	  }
	}
    }

    mgctxget(MG_BITDEPTH, &bitdepth);
    if (bitdepth == 1)
      XtUnmanageChild(DitherToggle);
    else
    {
      mgctxget(MG_DITHER, &dither);
      if (*dither_state != dither)
      {
        XmToggleButtonSetState(DitherToggle, dither, False);
        *dither_state = dither;
      }
    }
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
