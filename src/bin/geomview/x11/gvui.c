/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Geometry Technologies, Inc.
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

#if defined(HAVE_CONFIG_H) && !defined(CONFIG_H_INCLUDED)
#include "config.h"
#endif

static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Geometry Technologies, Inc.";

#include "mg.h"
#include "../common/drawer.h"
#include "../common/ui.h"
#include "../common/comm.h"
#include "../common/space.h"
#include "../common/lights.h"
#include "../common/event.h"
#include "../common/motion.h"
#include "../common/worldio.h"
#include "../common/lang.h"

/* Geomview Icon */
#include "bitmaps/icon"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/signal.h>
#include "X11/IntrinsicP.h" /* for XtMoveWidget */

#include "mibload.h"
#include "gvui.h"

static char *x_application_name = "geomview"; /* used to be "gv" */

/******************************* Index defines *******************************/

#define BLACKINDEX      0
#define WHITEINDEX      7

#define FACEINDEX       250
#define EDGEINDEX       251
#define NORMALINDEX     252
#define BBOXINDEX       253
#define BACKINDEX       254
#define LIGHTINDEX      255

/****************************** Table of Panels ******************************/

static struct panel {
  char *name;
  char key[2];
  int  shown;		/* Currently shown?  Initial value is default. */
  void (*show)();
  void (*load)();
  int  position;
  int  x, y;
  
  Widget shell;
  Widget formwantskeys;
} panels[] = {
  { "\0", "", False, NULL, NULL, 0, 0, 0, NULL },
  { "geomview", "Pm", True, NULL, ui_load_mainpanel, 0, 0, 0, NULL },
  { "Tools", "Pt", True, NULL, ui_load_toolpanel, 0, 0, 0, NULL },
  { "Appearance", "Pa", False, NULL, ui_load_appearancepanel, 0, 0, 0, NULL },
  { "Cameras", "Pc", False, NULL, ui_load_cameraspanel, 0, 0, 0, NULL },
  { "Lighting", "Pl", False, ui_show_lightspanel, ui_load_lightspanel, 0, 0, 0, NULL },
  { "Commands", "PC", False, NULL, ui_load_commandspanel, 0, 0, 0, NULL },
  { "Materials", "PM", False, NULL, ui_load_materialpanel, 0, 0, 0, NULL },
  { "Credits", "PA", False, NULL, ui_load_creditspanel, 0, 0, 0, NULL },
  { "Save", ">", False, ui_show_savepanel, ui_load_savepanel, 0, 0, 0, NULL },
  { "Files", "Pf", False, ui_show_filespanel, ui_load_filespanel, 0, 0, 0, NULL },
  { "Load", "<", False, ui_show_loadpanel, ui_load_loadpanel, 0, 0, 0, NULL },
  { "Color", "", False, ui_show_colorpanel, ui_load_colorpanel, 0, 0, 0, NULL },
};

/************************** Global Widget Variables **************************/

Widget	TopLevel,
	MainWindow,
        MainMenu,
        MainForm;

XtAccelerators KeyAccels;
Atom	AProtocol;
Arg	args[20];
int     n;
XtAppContext App;
extern	Display	*dpy;
extern  GC	mib_gc;
extern int real_id(int);

Pixmap geomicon;

/******************************* Global Variables ****************************/

int      YScrnSize, XScrnSize;
UIState  uistate;
int      gvbitdepth;
Visual  *gvvisual;
Colormap gvcolormap;
int	 gvpriv=0;

/*****************************************************************************/

void ui_select(int id);
void ui_find_visual();

/*****************************************************************************/

static void set_x_application_name()
{
  char *name = getenv("GEOMVIEW_X_NAME");
  if ((name != NULL) && (strlen(name)>0)) {
    x_application_name = name;
  }
  /* else use default x_application_name set above */
}
      

/*****************************************************************************/

static int panelsready = 0;
	
void load_interface(argc, argv)
unsigned int argc;
char **argv;
{
  String fallbacks[] = {
    "gv*fontList:        -adobe-helvetica-medium-r-normal--14-*-*-*-p-*-iso8859-1",
    "*BorderWidth:   0",

#ifdef dark_ui
    "*Foreground:    gray20",
    "*Background:    gray70",
/* see if we can set shadow colors
    "*TopShadowColor: #fbfbfbfbfbfb",
    "*BottomShadowColor: #6b6b6b6b6b6b",
*/
/*
    "*Foreground:     #000000",
    "*Background:     #848484",
*/
#endif

#ifdef light_ui
    "*Foreground:     #000000000000",
    "*Background:     #afafafafafaf",
#endif

    "*XmToggleButton.selectColor:    yellow",
    "*XmToggleButton.indicatorSize:    16",
    "*XmToggleButtonGadget.selectColor:    yellow",
    "*XmToggleButtonGadget.indicatorSize:    16",
    "*fontList:        -adobe-helvetica-medium-r-normal--14-*-*-*-p-*-iso8859-1",
    "gv*Load*fontList:    -adobe-helvetica-medium-r-normal--12-120-75-75-p-67-iso8859-1",
    "gv*Save*fontList:    -adobe-helvetica-medium-r-normal--12-120-75-75-p-67-iso8859-1",
    "Commands*fontList:    -adobe-helvetica-medium-r-normal--12-120-75-75-p-67-iso8859-1",
    "Tools*fontList:    -adobe-helvetica-medium-r-normal--12-120-75-75-p-67-iso8859-1",
    NULL
  };

  static XtActionsRec actions[1] = { { "GVKey", key_action } };
  static String keyaccels = { "<KeyPress>: GVKey()" };
  

  static char *argvblah[] = { "Geomview",
	/* We turn off blinking text cursors for two reasons:
	 * - Motif grabs the main loop if the cursor sits in a text box
	 *   for more than one blink cycle, and our graphics can't move.
	 * - Much worse, SGI Motif under Irix 4 (at least) seems to fail to
	 *   remove its blink timer when we XtUnrealizeWidget the panel
	 *   containing it.  So, moments later, we die of an X error
	 *   because of a reference to a bad window.
	 * Disabling blink prevents all this.
	 */
    "-xrm", "*XmText*blinkRate:	0",

	/* Encourage old versions of Motif to interpret our keyboard shortcuts.
	 * It works in some situations, depending on just what the cursor
	 * overlies.
	 */
    "-xrm", "*XmForm*accelerators:	<KeyPress>: GVKey()",
    "-xrm", "*XmFrame*translations: #override\\n<KeyPress>: GVKey()",
    "-xrm", "Geomview*KeyText*translations: <KeyPress>: GVKey()",
    "-xrm", "Geomview*XmList*translations: ~s ~c ~m ~a <Btn1Down>:ListBeginSelect()\\n~s ~c ~m ~a <Btn1Up>:ListEndSelect()\\n<KeyPress>: GVKey()",
    "-xrm", "Appearance*XmList*translations: ~s ~c ~m ~a <Btn1Down>:ListBeginSelect()\\n~s ~c ~m ~a <Btn1Up>:ListEndSelect()\\n<KeyPress>: GVKey()",
    "-xrm", "Cameras*XmList*translations: ~s ~c ~m ~a <Btn1Down>:ListBeginSelect()\\n~s ~c ~m ~a <Btn1Up>:ListEndSelect()\\n<KeyPress>: GVKey()",
    "-xrm", "Lights*XmList*translations: ~s ~c ~m ~a <Btn1Down>:ListBeginSelect()\\n~s ~c ~m ~a <Btn1Up>:ListEndSelect()\\n<KeyPress>: GVKey()",
    "-xrm", "Credits*XmList*translations: ~s ~c ~m ~a <Btn1Down>:ListBeginSelect()\\n~s ~c ~m ~a <Btn1Up>:ListEndSelect()\\n<KeyPress>: GVKey()",
    "-xrm", "",
  };
  int argcblah = COUNT(argvblah);


/* Code for TopLevel */
/*****************************************************************************/
	
  n = 0;

  set_x_application_name();

  if(getenv("GV_NO_X_MAGIC")) {
    TopLevel = XtAppInitialize(&App,
			       x_application_name,
			       NULL,
			       0,
			       &argc,
			       argv,
			       fallbacks,
			       args,
			       n);
  } else {
    if(getenv("GVXRM")) argvblah[COUNT(argvblah)-1] = getenv("GVXRM");
    TopLevel = XtAppInitialize(&App,
			       x_application_name,
			       NULL,
			       0,
			       &argcblah /*&argc*/,
			       argvblah /*argv*/,
			       fallbacks,
			       args,
			       n);
  }

#if 0
  if(getenv("GVXRM")) argvblah[COUNT(argvblah)-1] = getenv("GVXRM");
  TopLevel = XtAppInitialize(&App, "gv", NULL, 0, &argcblah /*&argc*/, argvblah
			/*argv*/, fallbacks, args, n);
#endif

  XtAppAddActions(App, actions, 1);
  KeyAccels = XtParseAcceleratorTable(keyaccels);

  dpy = XtDisplay(TopLevel);
  YScrnSize = XtScreen(TopLevel)->height;
  XScrnSize = XtScreen(TopLevel)->width;

  ui_find_visual();

  geomicon = XCreatePixmapFromBitmapData(dpy, DefaultRootWindow(dpy),
		oogl_bits, oogl_width, oogl_height,
		WhitePixel(dpy, DefaultScreen(dpy)),
		BlackPixel(dpy, DefaultScreen(dpy)),
		DefaultDepth(dpy, DefaultScreen(dpy)));

}

/*****************************************************************************/

void ui_highlight(int newid)
{
  int newtype = TYPEOF(newid);
  DGeom *dg;
  DView *v;
  static Color black = {0.0, 0.0, 0.0};
  Color c;
  extern int mg_ctxselect(mgcontext *);


  if ( newtype == T_GEOM &&
      (dg = (DGeom *) drawer_get_object(newid)) && dg->bboxap ) {
    c = uistate.pickedbboxcolor;
    gv_bbox_color( GEOMID(uistate.targetgeom), black.r, black.g, black.b );
    gv_bbox_color( newid, c.r, c.g, c.b );
  } else if (newtype == T_CAM) { /* pop cam window in case it's obscured */
    v = (DView *) drawer_get_object(newid);
    if (v) {
      mg_ctxselect(v->mgctx); /* winset happens in here */
      /* winpop */
    }
  }

}

/*****************************************************************************/

void ui_action(int val)
{
}

/*****************************************************************************/

void ui_mousefocus(int index)
{
  uistate.mousefocus = index;
  if (uistate.targetcam == INDEXOF(FOCUSID)) {
    ui_select(CAMID(uistate.mousefocus));
  }

}

/*****************************************************************************/

void ui_objectchange()
{
  ui_adjustpicklist(-1);
  ui_adjustpicklist(uistate.targetid);
}

/*****************************************************************************/

void ui_maybe_refresh(int id)
{
   if ( id==WORLDGEOM || id==GEOMID(uistate.targetgeom) || id==ALLGEOMS
       || id==TARGETGEOMID) {
    ui_select(GEOMID(uistate.targetgeom));
  } else if( id==ALLCAMS || id==FOCUSID || id==TARGETCAMID
        || id==CAMID(uistate.targetcam) || id==CAMID(uistate.mousefocus) ) {
    ui_select(CAMID(uistate.targetcam));
  } else {
    ui_select(NOID);
  }
}

/*****************************************************************************/

void ui_select(int id)
{
  DObject *obj;

  if ((obj = drawer_get_object(uistate.targetid)))
  {
    ui_tool_targetset(drawer_id2name(obj->id));
    ui_tool_centerset(drawer_id2name(uistate.centerid));

    if (ISCAM(id))
      ui_target_cameraspanel(id);
    if (ISGEOM(id))
    {
      ui_target_appearancepanel(id);
      ui_target_materialpanel(id);
    }

    ui_refresh_mainpanel(id);
  }
}

/*****************************************************************************/

void ui_windowWillOpen(DView *dv)
{
  ui_create_camera(MainWindow, dv);
}

/*****************************************************************************/

void ui_windowDidOpen(DView *dv)
{
}

/*****************************************************************************/

void ui_showpanel(int index, int show)
{
  struct panel *pan;
  
  if(index >= COUNT(panels) || index < 0)	/* protect us from showing */
    return;					/* non-existent panels */

  pan = &panels[index];

  if(show < 0)
    show = !pan->shown;
  pan->shown = show;

  if(!panelsready)
    return;

  if (pan->shell == NULL) {		/* or panels which have not */
    if(pan->load == NULL || !show)	/* yet been loaded */
	return;
    silence_errors();
    (*pan->load)();
    resume_errors();
  }

  if(show) {
    if(pan->position) {
      XtMoveWidget(pan->shell, pan->x, pan->y);
      pan->position = 0;
    }
    if(pan->show != NULL)	/* Call any panel-specific pre-show func */
	(*pan->show)();
    XtRealizeWidget(pan->shell);
    XRaiseWindow(dpy, XtWindow(pan->shell));
  } else {
    XtUnrealizeWidget(pan->shell);
  }
}

/*****************************************************************************/

LDEFINE(ui_target, LVOID,
       "(ui-target      ID [yes|no])\n\
        Set the target of user actions (the selected line of the\n\
        target object browser) to ID.  The second argument specifies\n\
        whether to make ID the current object regardless of its type.\n\
        If \"no\", then ID becomes the current object of its type\n\
        (geom or camera).  The default is \"yes\".  This command may\n\
        result in a change of motion modes based on target choice.")
{
  DObject *obj;
  int id, newtype;
  int immediate=YES_KEYWORD;

  LDECLARE(("ui-target", LBEGIN,
            LID, &id,
            LOPTIONAL,
            LKEYWORD, &immediate,
            LEND));
  immediate = boolval("ui-target", immediate);
  newtype = TYPEOF(id);

  if (id == uistate.targetid ) return Lt;
  if ( !(obj = drawer_get_object(id))
      || (id == TARGETID) || (id == CENTERID)
      || (id == SELF) || (id == UNIVERSE) || (id == PRIMITIVE)) {
    return Lt;
  }
  if (immediate) {
    /* ui_highlight must be called before set_ui_target_id */
    /* freeze the browser so old target name doesn't flash
       during highlight. */

    ui_highlight(id);
    set_ui_target_id( id );
    ui_adjustpicklist(id);
    if (ISGEOM(id))
    {
	uistate.targetcam = INDEXOF(FOCUSID);
	ui_select(real_id(FOCUSID));
    } else if(ISCAM(id)) {
	ui_raise_window( real_id(id) );
    }
    ui_select( id );
  } else {
    /* immediate == NOIMMEDIATE: for cases like deleting geometry
     * with the command language when current target is a camera.
     * update targettype but don't change browser.
     */
    switch (newtype) {
    case T_GEOM: uistate.targetgeom = INDEXOF(id); break;
    case T_CAM: uistate.targetcam = INDEXOF(id); break;
    }
  }
  return Lt;
}

LDEFINE(ui_center, LVOID,
       "(ui-center      ID)\n\
        Set the center for user interface (i.e. mouse) controlled\n\
        motions to object ID.")
{
  int id;
  LDECLARE(("ui-center", LBEGIN,
            LID, &id,
            LEND));

  if ( (id == NOID) || (id == CENTERID) ) {
    id = uistate.centerid;
  } else {
    set_ui_center(id);
  }
  ui_tool_centerset(drawer_id2name(id));
  return Lt;
}

/*****************************************************************************/

void ui_pickcolor(int val)
{
  DView      *dv;
  Color      *old;
  char       *name;
  Appearance *ap;
  DGeom      *dg;
  int         index;


  ap = drawer_get_ap(GEOMID(uistate.targetgeom));

  switch (val)
  {
    case DRAWER_BACKCOLOR:
	if ((dv = (DView *)drawer_get_object(real_id(CAMID(uistate.targetcam)))))
	{
	  ui_config_colorpanel("Background", &(dv->backcolor), BACKINDEX, val,
		real_id(CAMID(uistate.targetcam)));
	  ui_showpanel(P_COLOR, 1);
	}
	return;

    case DRAWER_DIFFUSE:
	name = "Faces"; index = FACEINDEX;
	old = (Color *)&ap->mat->diffuse;
	break;

    case DRAWER_EDGECOLOR:
	name = "Edges"; index = EDGEINDEX;
	old = &ap->mat->edgecolor;
	break;

    case DRAWER_NORMALCOLOR:
	name = "Normals"; index = NORMALINDEX;
	old = &ap->mat->normalcolor;
	break;

    case DRAWER_BBOXCOLOR:
	name = "BBoxes"; index = BBOXINDEX;
	dg = (DGeom*)drawer_get_object(GEOMID(uistate.targetgeom));
	old = &dg->bboxap->mat->edgecolor;
	break;

    default:
      break;
  }

  ui_config_colorpanel(name, old, index, val, GEOMID(uistate.targetgeom));
  ui_showpanel(P_COLOR, 1);
}

/*****************************************************************************/

int ui_name2panel(char *name)
{
  extern int      strcasecmp(const char *, const char *);
  extern int      strncasecmp(const char *, const char *, size_t);
  int i;

  if (strcasecmp(name, "main") == 0) return P_MAIN;
  for (i=P_MAX; --i > 0 && strcasecmp(name, panels[i].name) != 0;);
  if (i == 0)
    for(i=P_MAX; --i > 0 && strncasecmp(name, panels[i].name, 4) != 0; ) ;

  return i;
}

/*****************************************************************************/

void ui_add_mode(char *name, PFI proc, int type)
{

  ui_install_mode(name, proc, type);
}

/*****************************************************************************/

void ui_remove_mode(char *name)
{
  int current;

  current = ui_mode_index(name);
  ui_uninstall_mode(name);

  if (uistate.mode_current == current)
    gv_event_mode(uistate.modenames[0]);
}

/*****************************************************************************/

extern int gv_debug;
static int oldstderr = -1;

void silence_errors()
{
  if(!gv_debug) {
    if(oldstderr < 0)
	oldstderr = dup(fileno(stderr));
    freopen("/dev/null", "w", stderr);
  }
}

void resume_errors()
{
  if(oldstderr >= 0) {
    freopen("/dev/null", "w", stderr);
    dup2(oldstderr, fileno(stderr));
    close(oldstderr);
    oldstderr = -1;
  }
}

/*****************************************************************************/

void ui_init()
{
  silence_errors();
  ui_load_mainpanel();
  ui_load_appearancepanel();
  ui_load_cameraspanel();
  ui_load_lightspanel();
  ui_load_materialpanel();
  ui_load_loadpanel();
  resume_errors();
  ui_init_snaphelp();

  cui_init();
  clight_init();

  gv_event_mode(OBJROTATE);
  uistate.cursor_on = 1;
}


/*****************************************************************************/

Widget ui_make_panel_and_form(char *title, char *rootstr, Boolean wantkeys,
				Widget *formp)
{
  Arg args[20];
  int n;
  Widget shell;
  int panel = ui_name2panel(title);

  n = 0;
  XtSetArg(args[n], XmNtitle, title); n++;
  XtSetArg(args[n], XmNdeleteResponse, XmDO_NOTHING);n++;
  XtSetArg(args[n], XmNkeyboardFocusPolicy, XmPOINTER); n++;
  XtSetArg(args[n], XmNallowShellResize, True); n++;

  if(gvvisual->class & 1) {	/* If PseudoColor or other dynamic visual */
    XtSetArg(args[n], XmNvisual, gvvisual); n++;
    XtSetArg(args[n], XmNdepth, gvbitdepth); n++;
    XtSetArg(args[n], XmNcolormap, gvcolormap); n++;
  }

  if (panels[panel].position) {
    XtSetArg(args[n], XmNx, panels[panel].x); n++;
    XtSetArg(args[n], XmNy, panels[panel].y); n++;
  }
  XtSetArg(args[n], XmNiconPixmap, geomicon); n++;
  XtSetArg(args[n], XmNmwmDecorations, MWM_DECOR_ALL); n++;
  XtSetArg(args[n], XmNmwmFunctions, MWM_FUNC_ALL); n++;

  set_x_application_name();
  shell = XtAppCreateShell(title, x_application_name, topLevelShellWidgetClass, dpy,
                                args, n);

  /* Close when requested by WM */
  AProtocol = XmInternAtom(dpy, "WM_DELETE_WINDOW", False);
  XmAddWMProtocolCallback(shell, AProtocol, (XtCallbackProc) ui_hide,
		title);

  if(formp != NULL) {
    n = 0;
    XtSetArg(args[n], XmNrubberPositioning, True); n++;
    XtSetArg(args[n], XmNresizable, True); n++;
    if(wantkeys) {
	XtSetArg(args[n], XmNaccelerators, KeyAccels); n++;
    }
    *formp = XmCreateForm(shell, title, args, n);
    if(wantkeys) panels[panel].formwantskeys = *formp;
    XtManageChild(*formp);
  }

  panels[panel].shell = shell;
  
  return shell;
}

/*****************************************************************************/

void
ui_hide(Widget w, XtPointer data, XmAnyCallbackStruct *cbs)
{
  int panel = ui_name2panel((char *)data);
  ui_showpanel(panel, 0);
}

void
ui_show(Widget w, XtPointer data, XmAnyCallbackStruct *cbs)
{
  int panel = ui_name2panel((char *)data);
  ui_showpanel(panel, 1);
}


/*****************************************************************************/

LDEFINE(ui_panel, LVOID,
            "(ui-panel       PANELNAME  {on|off} [ WINDOW ] )\n\
        Do or don't display the given user-interface panel.\n\
        Case is ignored in panel names.  Current PANELNAMEs are:\n\
                geomview        main panel\n\
                tools           motion controls\n\
                appearance      appearance controls\n\
                cameras         camera controls\n\
                lighting        lighting controls\n\
                obscure         obscure controls\n\
                materials       material properties controls\n\
                command         command entry box\n\
                credits         geomview credits\n\
        By default, the \"geomview\" and \"tools\" panels appear when\n\
        geomview starts.  If the optional Window is supplied, a\n\
        \"position\" clause (e.g. (ui-panel obscure on { position xmin\n\
        xmax ymin ymax }) sets the panel's default position.  (Only\n\
        xmin and ymin values are actually used.)  A present but empty\n\
        Window, e.g.  \"(ui-panel obscure on {})\" causes interactive\n\
        positioning.")
{
  char *panelname;
  int index, on;
  struct panel *p;
  WindowStruct *ws=NULL;
  WnPosition wp;

  LDECLARE(("ui-panel", LBEGIN,
            LSTRING, &panelname,
            LKEYWORD, &on,
            LOPTIONAL,
            LWINDOW, &ws,
            LEND));

  if((index = ui_name2panel(panelname)) == 0) {
    if (!strcmp(panelname, "Obscure"))
      return Lnil;
    fprintf(stderr, "ui-panel: expected name of a panel, got \"%s\"\n",
            panelname);
    return Lnil;
  }

  if (ws)
  {
    p = &(panels[index]);
    if (WnGet(ws->wn, WN_PREFPOS, &wp) > 0)
    {
      p->x = wp.xmin;
      p->y = wp.ymin;
    }
    if (p->shown && p->shell != NULL) {
	XtMoveWidget(p->shell, p->x, p->y);
	p->position = 0;
    } else {
	p->position = 1; /* position window when it shows up later */
    }
  } 

  ui_showpanel( index, boolval("ui-panel", on));

  return Lt;
}

/*****************************************************************************/

/* Dummy -- we don't bother to freeze the UI.  How do we do that in Motif? */

LDEFINE(ui_freeze, LVOID,
        "(ui-freeze [on|off])\n\
        Toggle updating user interface panels. Off by default."
)
{
  int kw = -1;
  LDECLARE(("ui-freeze", LBEGIN,
        LOPTIONAL,
        LKEYWORD, &kw,
        LEND));
  return Lnil;
}

/*****************************************************************************/

int ui_panelshown(int index)
{

  return panels[index].shown;
}

/*****************************************************************************/

void ui_final_init()
{
  int i;

		/* It's only safe to load tool & color panels now,
		 * since they rely on mgx11 for colors, which is ready only
		 * after ui_init().
		 */
  silence_errors();
  ui_load_colorpanel();
  ui_load_toolpanel();
  resume_errors();
  panelsready = 1;

  ui_select(CAMID(uistate.targetcam));
  ui_select(GEOMID(uistate.targetgeom));
  lights_changed_check();
  for(i = 1; i < P_MAX; i++)
    if (panels[i].shown)
      ui_showpanel(i, 1);
  ui_objectchange();

}

/*****************************************************************************/

void ui_update()
{
  int i;
  emodule *em;

  if(uistate.emod_check) {
    for(i=0, em=VVEC(uistate.emod,emodule); i<VVCOUNT(uistate.emod); ) {
        if(VVEC(uistate.emod,emodule)[i].pid < 0 &&
            (em->link == NULL || PoolInputFile(em->link) == NULL)) {
                emodule_reap(em);
        } else {
            i++, em++;
        }
    }
    uistate.emod_check = 0;
  }
  if(uistate.emod_changed) {
    ui_adjustemodulelist(uistate.emod_changed);
    uistate.emod_changed = 0;
  }
}

/*****************************************************************************/

LDEFINE(ui_emodule_define, LVOID,
"(emodule-define  NAME  SHELL-COMMAND ...)\n\
        Define an external module called NAME, which then appears in the\n\
        external-module browser.  The SHELL-COMMAND string\n\
        is a UNIX shell command which invokes the module.\n\
        See emodule-run for discussion of external modules.")
{
  emodule *em;

  char *title, *cmd;
  LDECLARE(("emodule-define", LBEGIN,
            LSTRING, &title,
            LSTRING, &cmd,
            LEND));

  em = ui_emodule_install(0, strdup(title), (PFI)emodule_run);
  em->text = strdup(cmd);
  ui_adjustemodulelist(1);
  return Lt;
}

/*****************************************************************************/

static void ename_fix(char *c, const char *e, int n)
{
  char num[80];
  int ni=0, ei=0;
  if (e[0] != '[') goto copy;
  ei = 1;
  num[0] = '\0';
  while (ei<n && e[ei++] != ']') num[ni++] = e[ei-1];
  if (ei == n) goto copy;
  num[ni] = '\0';
  strncpy(c,e+ei,n);
  c[strlen(c)-1] = '0';
  strcat(c,"[");
  strcat(c,num);
  strcat(c,"]");
  return;
 copy:
  strncpy(c,e,n);
  return;
}

static int emod_compare(const void *e1, const void *e2)
{
  char c1[80], c2[80];

  ename_fix(c1,((emodule *)e1)->name,80);
  ename_fix(c2,((emodule *)e2)->name,80);

  return strcmp(c1,c2);
}

LDEFINE(emodule_sort, LVOID,
"(emodule-sort)\n\
        Sorts the modules in the application browser alphabetically.")
{
  emodule *em;
  size_t nel, size;
  LDECLARE(("emodule-sort", LBEGIN,
            LEND));

  em = VVEC(uistate.emod, emodule);
  nel = VVCOUNT(uistate.emod);
  size = sizeof(emodule);

  qsort(em, nel, size, emod_compare);
  ui_adjustemodulelist(1);
  return Lt;
}

/*****************************************************************************/

LDEFINE(ui_emodule_start, LVOID,
"(emodule-start  NAME)\n\
        Starts the external module NAME, defined by emodule-define.\n\
        Equivalent to clicking on the corresponding module-browser entry.")
{
  emodule *em;
  int i;
  char *name;
  LDECLARE(("emodule-start", LBEGIN,
            LSTRING, &name,
            LEND));
  i = ui_emodule_index(name, &em);
  if(i < 0) return Lnil;
  (*em->func)(em);
  return Lt;
}

/*****************************************************************************/

void ui_build_browser(Widget a_list, int count, char *names[])
{
  XmString str;
  int      i;

  XmListDeleteAllItems(a_list);
  for (i = 0; i < count; i++)
  {
    str = XmStringCreateSimple(names[i]);
    XmListAddItemUnselected(a_list, str, 0);
    XmStringFree(str);
  }
}

/*****************************************************************************/

void ui_set_ftext(Widget w, float val)
{
  char buf[80], *str;

  sprintf(buf,"%g",val);
  str = XmTextFieldGetString(w);
  if (strcmp(str, buf))
    XmTextFieldSetString(w, buf);
  XtFree(str);
}

/*****************************************************************************/

void ui_set_itext(Widget w, int val)
{
  char buf[80], *str;

  sprintf(buf,"%d",val);
  str = XmTextFieldGetString(w);
  if (strcmp(str, buf))
    XmTextFieldSetString(w, buf);
  XtFree(str);
}

/*****************************************************************************/
