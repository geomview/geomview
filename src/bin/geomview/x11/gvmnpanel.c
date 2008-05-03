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
#include "../common/lang.h"
#include <X11/StringDefs.h>
#include "gvui.h"
static /* prefix to ``char Root[]...'' */
#include "interface/MainPanel.mib"


extern Display *dpy;
extern UIState uistate;

/* private methods and variables */
/*****************************************************************************/

static void select_object(Widget, XtPointer, XmListCallbackStruct *);
static void select_module(Widget, XtPointer, XmListCallbackStruct *);
static void file_menu_callbacks(Widget, char *);
static void edit_menu_callbacks(Widget, char *);
static void inspect_menu_callbacks(Widget, XtPointer);
static void motion_menu_callbacks(Widget, char *);
static void help_menu_callbacks(Widget, XtPointer);
static void choose_space(Widget, int);
static void BuildBrowserMenu();
static int id2menuindex(int);

static int *menuindex = NULL;
static int menucount = 0;
static int currentspace = 0, loaded = 0;
static Widget shell,
		ObjectList,
		EmodList,
		MotionToggle[3],
		SpaceRadio[3],
		KeyText;

static MenuItem file_menu[] = {
  { "New Camera", &xmPushButtonGadgetClass, 'N', "Ctrl<Key>N","[v+]",
        file_menu_callbacks, "N", (MenuItem *)NULL},
  { "Open            ", &xmPushButtonGadgetClass, 'O', "Ctrl<Key>O","[ < ]",
        file_menu_callbacks, "O", (MenuItem *)NULL},
  { "Save               ", &xmPushButtonGadgetClass, 'S', "Ctrl<Key>S","[ > ]",
        file_menu_callbacks, "S", (MenuItem *)NULL},
  { "_sepr", &xmSeparatorGadgetClass, (char)0, (char*)NULL, (char*)NULL,
        (void (*)())NULL, (XtPointer)NULL, (MenuItem *)NULL },
  { "Exit                   ", &xmPushButtonGadgetClass, 'E', "Ctrl<Key>X","[ Q ]",
        file_menu_callbacks, "X", (MenuItem *)NULL},
  { NULL, NULL }
};

static MenuItem edit_menu[] = {
  { "Copy", &xmPushButtonGadgetClass, 'C', NULL, NULL,
        edit_menu_callbacks, "C", (MenuItem *)NULL},
  { "Delete", &xmPushButtonGadgetClass, 'D', "Ctrl<Key>D", "[dd]",
        edit_menu_callbacks, "D", (MenuItem *)NULL},
  { NULL, NULL }
};

static MenuItem inspect_menu[] = {
  { "Tools",      &xmPushButtonGadgetClass, 'T', "Ctrl<Key>T", "[Pt]",
        inspect_menu_callbacks, (XtPointer)P_TOOL, (MenuItem *)NULL},
  { "Appearance", &xmPushButtonGadgetClass, 'A', "Ctrl<Key>A", "[Pa]",
        inspect_menu_callbacks, (XtPointer)P_APPEARANCE, (MenuItem *)NULL},
  { "Material  ", &xmPushButtonGadgetClass, 'M', "Ctrl<Key>M", "[PM]",
        inspect_menu_callbacks, (XtPointer)P_MATERIAL, (MenuItem *)NULL},
  { "Lights         ", &xmPushButtonGadgetClass, 'L', "Ctrl<Key>L", "[Pl]",
        inspect_menu_callbacks, (XtPointer)P_LIGHTS, (MenuItem *)NULL},
  { "Cameras        ", &xmPushButtonGadgetClass, 'C', "Ctrl<Key>C", "[Pc]",
        inspect_menu_callbacks, (XtPointer)P_CAMERA, (MenuItem *)NULL},
  { "Commands       ", &xmPushButtonGadgetClass, 'O', "Ctrl<Key>P", "[PC]",
        inspect_menu_callbacks, (XtPointer)P_COMMANDS, (MenuItem *)NULL},
  { NULL, NULL }
};

#if 0
static MenuItem space_menu[] = {
  { "Euclidean",   &xmPushButtonGadgetClass, 'E', NULL, "[me]",
	choose_space, (XtPointer)EUCLIDEAN, (MenuItem *)NULL},
  { "Hyperbolic",   &xmPushButtonGadgetClass, 'H', NULL, "[mh]",
	choose_space, (XtPointer)HYPERBOLIC, (MenuItem *)NULL},
  { "Spherical",   &xmPushButtonGadgetClass, 'S', NULL, "[ms]",
	choose_space, (XtPointer)SPHERICAL, (MenuItem *)NULL},
  { NULL, NULL }
};
#endif

static MenuItem motion_menu[] = {
  { "Tools", &xmPushButtonGadgetClass, 'T', "Ctrl<Key>T", "[Pt]",
        motion_menu_callbacks, "T", (MenuItem *)NULL},
  { "_sepr", &xmSeparatorGadgetClass, (char)0, (char*)NULL, (char*)NULL,
        (void (*)())NULL, (XtPointer)NULL, (MenuItem *)NULL },
  { "Inertia", &xmToggleButtonGadgetClass, 'I', "Alt<Key>I", "[ui]",
        motion_menu_callbacks, "1", (MenuItem *)NULL},
  { "Constrain motion", &xmToggleButtonGadgetClass, 'C', "Alt<Key>M", "[uc]",
        motion_menu_callbacks, "2", (MenuItem *)NULL},
  { "Own coordinates", &xmToggleButtonGadgetClass, 'O', "Alt<Key>O", "[uo]",
        motion_menu_callbacks, "3", (MenuItem *)NULL},
  { NULL, NULL }
};

static MenuItem help_menu[] = {
  { "About", &xmPushButtonGadgetClass, 'a', "Alt<Key>A", "[PA]",
    help_menu_callbacks, (XtPointer)P_CREDITS, (MenuItem *)NULL },
  { "Manual (PDF)", &xmPushButtonGadgetClass, 'h', "Alt<Key>P", "[PP]",
    help_menu_callbacks, "p", (MenuItem *)NULL },
  { "Manual (HTML)", &xmPushButtonGadgetClass, 'h', "Alt<Key>H", "[PH]",
    help_menu_callbacks, "h", (MenuItem *)NULL },
  { "Credits", &xmPushButtonGadgetClass, 'A', "Alt<Key>A", "[PA]",
    help_menu_callbacks, (XtPointer)P_CREDITS, (MenuItem *)NULL },
  { NULL, NULL }
};

void ui_refresh_mainpanel(int id)
{
  int newspace, i;

  if (!loaded)
    return;

  newspace = uispace(spaceof(WORLDGEOM));

  if (currentspace != newspace)
  {
    currentspace = newspace;

    for (i = 0; i < 3; i++)
    {
      if (i != currentspace)
	XtVaSetValues(SpaceRadio[i], XmNset, False, NULL);
    }

    XtVaSetValues(SpaceRadio[currentspace], XmNset, True, NULL);
  }

  XtVaSetValues(MotionToggle[0], XmNset, uistate.inertia ? True : False, NULL);
  XtVaSetValues(MotionToggle[1], XmNset, uistate.constrained ? True : False,
			NULL);
  XtVaSetValues(MotionToggle[2], XmNset, uistate.ownmotion ? True : False,
			NULL);
}

static void motion_menu_callbacks(Widget widget, char *text)
{
  DrawerKeyword val = DRAWER_NOKEYWORD;

  switch (text[0])
  {
    case 'T':
      ui_showpanel(P_TOOL, 1);
      break;

    case '1':
      val = DRAWER_INERTIA;
      break;

    case '2':
      val = DRAWER_CONSTRAIN;
      break;

    case '3':
    default:
      val = DRAWER_OWNMOTION;
      break;
  }

  drawer_int(GEOMID(uistate.targetgeom), val,
	     XmToggleButtonGadgetGetState(widget));
}

/* make the motion menu state accessible through GCL commands */
LDEFINE(ui_motion, LVOID,
	"(ui-motion {inertia|constrain|own-coordinates} {on|off|yes|no})\n"
	"Enable or disable certain properties of mouse-controlled motion. "
	"The purpose of this command is to give access to the toggles of the "
	"`motion'-menu of the main panels through GCL commands. "
	"\n\n"
	"`inertia': "
	"Normally, moving objects have inertia: if the mouse is still moving "
	"when the button is released, the selected object continues to move. "
	"When `inertia' is off, objects cease to move as soon as you release "
	"the mouse."
	"\n\n"
	"`constrain': "
	"It's sometimes handy to move an object in a direction aligned with a "
	"coordinate axis: exactly horizontally or vertically. Calling "
	"`(ui-motion constrain on)' changes the interpretation of mouse "
	"motions to allow this; approximately-horizontal or "
	"approximately-vertical mouse dragging becomes exactly horizontal "
	"or vertical motion. Note that the motion is still along the "
	"X or Y axes of the camera in which you move the mouse, not "
	"necessarily the object's own coordinate system."
	"\n\n"
	"`own-coordinates': "
	"It's sometimes handy to move objects with respect to the coordinate "
	"system where they were defined, rather than with respect to some "
	"camera's view.  When `(ui-motion own-coordinates on) has been called, "
	"all motions are interpreted that way: dragging the mouse rightward "
	"in translate mode moves the object in its own +X direction, and so "
	"on.  May be especially useful in conjunction with the "
	"(ui-motion constrain on) command.")
{
  Keyword enabled = NO_KEYWORD, flagkw = NO_KEYWORD;
  DrawerKeyword motionflag = DRAWER_NOKEYWORD;
  bool onoff;
  int btnidx = -1;
  
  LDECLARE(("ui-motion", LBEGIN,
	    LKEYWORD, &flagkw,
	    LKEYWORD, &enabled,
	    LEND));

  switch (flagkw) {
  case MOTION_INERTIA_KEYWORD:
    motionflag = DRAWER_INERTIA;
    btnidx = 0;
    break;
  case MOTION_CONSTRAIN_KEYWORD:
    motionflag = DRAWER_CONSTRAIN;
    btnidx = 1;
    break;
  case MOTION_OWN_COORDS_KEYWORD:
    motionflag = DRAWER_OWNMOTION;
    btnidx = 2;
    break;
  default:
    OOGLError(1,
	      "Wrong keyword, expected one out of "
	      "{\"%s\", \"%s\", \"%s\"}\n",
	      keywordname(MOTION_INERTIA_KEYWORD),
	      keywordname(MOTION_CONSTRAIN_KEYWORD),
	      keywordname(MOTION_OWN_COORDS_KEYWORD));
    break;
    return Lnil;
  }

  switch (enabled) {
  case YES_KEYWORD:
  case ON_KEYWORD:
    onoff = true;
    break;
  case NO_KEYWORD:
  case OFF_KEYWORD:
    onoff = false;
    break;
  default:
    OOGLError(1,
	      "Wrong keyword, expected one out of "
	      "{\"%s\", \"%s\", \"%s\", \"%s\"}\n",
	      keywordname(YES_KEYWORD),
	      keywordname(ON_KEYWORD),
	      keywordname(NO_KEYWORD),
	      keywordname(OFF_KEYWORD));
    return Lnil;
  }

  drawer_int(GEOMID(uistate.targetgeom), motionflag, onoff);
  
  XtVaSetValues(MotionToggle[btnidx], XmNset, onoff ? True : False, NULL);

  return Lt;
}

char htmlbrowser[PATH_MAX] = { '\0', }, pdfviewer[PATH_MAX] = { '\0', };

static void help_menu_callbacks(Widget widget, XtPointer ptr)
{
  if ((int)(long)ptr == P_CREDITS) {
    ui_showpanel(P_CREDITS, 1);
  } else if (*(char *)ptr == 'h') {
    ui_manual_browser("html");
  } else { /* if (*(char *)ptr == 'p') { */
    ui_manual_browser("pdf");
  }
}

static void choose_space(Widget w, int item)
{
  int  space;

  if (item == currentspace)
    return;
 
  currentspace = item;
  switch (currentspace)
  {
    default:
    case EUCLIDEAN:
      space = EUCLIDEAN_KEYWORD;
      break;
    case HYPERBOLIC:
      space = HYPERBOLIC_KEYWORD;
      break;
    case SPHERICAL:
      space = SPHERICAL_KEYWORD;
      break;
  }
  gv_space(space);
}

/*****************************************************************************/

void ui_load_mainpanel()
{
  Arg	     args[20];
  int	     n;

  Widget     mainwindow,
	     mainmenu,
	     mainform,
	     cascade,
	     space,
	  /*inspect,*/
             help,
	     title,
	     HideButton;

  char       buf[40];
  static char gvtitle[100];
  XmString    GVtitle;
  mib_Widget *mainload;
  XmString   str[3];
  XFontStruct *font;
  XmFontList   fontlist;
  static char Geomview[] = "Geomview";

/*****************************************************************************/

  shell = ui_make_panel_and_form(Geomview, Root, True, True, NULL);

  XtVaSetValues(shell, XmNtitle, "Geomview - The Geometry Center", NULL);

/*****************************************************************************/

  mainwindow = XtCreateManagedWidget("Geomview", xmMainWindowWidgetClass,
				     shell, args, 0);

/*****************************************************************************/

  n = 0;
  XtSetArg (args[n], XmNrubberPositioning, True); n++;

  mainmenu = XmCreateSimpleMenuBar(mainwindow, "MainMenu", args, n);

  BuildMenu(mainmenu, XmMENU_PULLDOWN, "File", 'F', file_menu);
  BuildMenu(mainmenu, XmMENU_PULLDOWN, "Edit", 'E', edit_menu);
  BuildMenu(mainmenu, XmMENU_PULLDOWN, "Inspect", 'I', inspect_menu);

/*****************************************************************************/

  str[0] = XmStringCreateSimple("Space");
  cascade = XtVaCreateManagedWidget("button_3",	/* lesstif demands "button_<N>" */
				    xmCascadeButtonGadgetClass, mainmenu,
				    XmNlabelString, str[0],
				    XmNmnemonic, (XID)'S',
				    NULL);
  XmStringFree(str[0]);

  str[0] = XmStringCreateSimple("Euclidean    [me]");
  str[1] = XmStringCreateSimple("Hyperbolic   [mh]");
  str[2] = XmStringCreateSimple("Spherical     [ms]");
  space = XmVaCreateSimplePulldownMenu(mainmenu, "_pulldown", 3,
	(XtCallbackProc) choose_space,
        XmVaRADIOBUTTON, str[0], '\0', NULL, NULL,
        XmVaRADIOBUTTON, str[1], '\0', NULL, NULL,
        XmVaRADIOBUTTON, str[2], '\0', NULL, NULL,
        XmNradioBehavior, True,
        XmNradioAlwaysOne, True,
        (XtCallbackProc) NULL);
  XmStringFree(str[0]);
  XmStringFree(str[1]);
  XmStringFree(str[2]);

  XtVaSetValues(cascade,
	XmNsubMenuId,   space,
	NULL);

  for (n = 0; n < 3; n++)
  {
    sprintf(buf, "button_%d", n);
    SpaceRadio[n] = XtNameToWidget(space, buf);
  }
#if 0

  for (n = 0; n < 3; n++) {
    SpaceRadio[n] = XtNameToWidget(space, space_menu[n].label);
  }
#endif
  XtVaSetValues(SpaceRadio[0], XmNset, True, NULL);
  currentspace = 0;

/*****************************************************************************/

  BuildMenu(mainmenu, XmMENU_PULLDOWN, "Motion", 'M', motion_menu);

  for (n = 0; n < 3; n++)
  {
    sprintf(buf, "button_%d", n);
    MotionToggle[n] = (Widget) motion_menu[n+2].subitems;
  }

  help = BuildMenu(mainmenu, XmMENU_PULLDOWN, "Help", 'H', help_menu);

  XtVaSetValues(mainmenu, XmNmenuHelpWidget, help, NULL);

  XtManageChild(mainmenu);

/*****************************************************************************/

  n = 0;
  XtSetArg (args[n], XmNrubberPositioning, True); n++;
  XtSetArg (args[n], XmNtraversalOn, False); n++;
  XtSetArg(args[n], XmNresizable, False); n++;
  XtSetArg(args[n], XmNaccelerators, KeyAccels); n++;

  mainform = XmCreateForm(mainwindow, "Geomview", args, n);
/*mainload = mib_load_interface(mainform, "interface/MainPanel.mib",
		MI_FROMFILE);*/
  mainload = mib_load_interface(mainform, Root, MI_FROMSTRING);

  /* Prevent automatic resizing; otherwise the main widget changes
     everytime we start or stop a new module. */
  XtVaSetValues(mainload->me, XmNresizePolicy, XmRESIZE_NONE, NULL);

  XtManageChild(mainform);
  XtInstallAccelerators(mainmenu, mainform);

/*****************************************************************************/

  HideButton = mib_find_name(mainload, "HideButton")->me;
  ObjectList = mib_find_name(mainload, "ObjectList")->me;
  EmodList   = mib_find_name(mainload, "EmoduleList")->me;
  KeyText    = mib_find_name(mainload, "KeyText")->me;

  XtVaSetValues(EmodList, XmNselectionPolicy, XmSINGLE_SELECT, NULL);
  XtVaSetValues(ObjectList, XmNselectionPolicy, XmSINGLE_SELECT, NULL);

  XtAddCallback(HideButton, XmNactivateCallback, (XtCallbackProc)ui_hide,
		(XtPointer)Geomview);

  XtAddCallback(ObjectList, XmNsingleSelectionCallback,
		(XtCallbackProc)select_object, (XtPointer)NULL);
  XtAddCallback(EmodList, XmNsingleSelectionCallback,
		(XtCallbackProc)select_module, (XtPointer)NULL);

  XtVaSetValues(KeyText, XmNeditable, False,
			 XmNcursorPositionVisible, False, NULL);

/*****************************************************************************/

  /* set geomview title font */
  title = mib_find_name(mainload, "GVTitle")->me;
  font = XLoadQueryFont(dpy,
        "-adobe-helvetica-bold-r-normal--24-240-75-75-p-138-iso8859-1");
  fontlist = XmFontListCreate(font, "bigger");
  sprintf(gvtitle, "Geomview %s", geomview_version);
  GVtitle = XmStringCreateSimple(gvtitle);
  XtVaSetValues(title, XmNfontList, fontlist, XmNlabelString, GVtitle, NULL);

/*****************************************************************************/

  loaded = 1;
}


/*****************************************************************************/

static void file_menu_callbacks(Widget widget, char *text)
{
  CameraStruct cs;
  DView *dv;

  switch (text[0])
  {
    case 'N':
      dv = (DView *)drawer_get_object(FOCUSID);
      cs.h = NULL;
      cs.cam = dv && dv->cam ? CamCopy(dv->cam, NULL) : NULL;
      gv_new_camera(NULL, &cs);
      break;

    case 'X':
      gv_exit();
      break;

    case 'O':
      ui_showpanel(P_FILEBROWSER, 1);
      break;

    case 'S':
      ui_showpanel(P_SAVE, 1);

    default:
      break;
  }
}

/*****************************************************************************/

static void edit_menu_callbacks(Widget widget, char *text)
{
  switch (text[0])
  {
    case 'U':
      break;
    case 'C':
      gv_copy(uistate.targetid, (char *)NULL);
      break;
    case 'P':
      break;
    case 'D':
      gv_delete(uistate.targetid);
      break;
    default:
      break;
  }
}

/*****************************************************************************/

static void inspect_menu_callbacks(Widget widget, XtPointer code)
{
  ui_showpanel((int)(long)code, 1);
}

/*****************************************************************************/
#if XmVersion == 1001	/* Motif 1.1 lacks XmListPosToBounds(); fake it. */
#define cent __cent	/* evade typedef name conflict with oogl routines */
#define XtMoveWidget __XtMoveWidget
#include <Xm/ListP.h>
#undef cent
#undef __XtMoveWidget

static int xmListVisible(Widget w, int position)
{
    XmListWidget lw = (XmListWidget) w;
    if (position == 0) position = lw->list.itemCount;

    if ((position >=  lw->list.itemCount)    ||
        (position <   lw->list.top_position) ||
        (position >= (lw->list.top_position + lw->list.visibleItemCount)))
          return (False);
    return True;
}
#define XmListPosToBounds(w, pos, junk1, junk2, junk3, junk4)  xmListVisible(w, pos)
#endif /* Motif 1.1 */


static void showselected(int slot)
{
  static int oldslot = -1;
  if(!XmListPosToBounds(ObjectList, slot, NULL, NULL, NULL, NULL)) {
    /* It's not already visible; try to scroll as little as possible to
     * make it so.  If prev selection was above this one, place it
     * near the bottom of the visible area, otherwise near the top.
     */
    if(oldslot > 0 && oldslot < slot && slot > 4)
	    XmListSetBottomPos(ObjectList, slot < menucount ? slot-1 : slot);
	else
	    XmListSetPos(ObjectList, slot > 4 ? slot-1 : 1);
  }
}

void ui_adjustpicklist(int id)
{
  if (id == -1) {
    BuildBrowserMenu();
    showselected( id2menuindex(uistate.targetid) + 1 );
  } else {
    int mind = id2menuindex(id)+1;
    XmListSelectPos(ObjectList, mind, False);
    showselected(mind);
  }
}

/*****************************************************************************/

/*
 * Show what's happening on the keyboard.
 * Commands are enclosed in [brackets].  In case of error, a '?' appears
 * following the close-bracket.
 */
void
ui_keyboard(int ch)
{
  static char kybd[12];
  static int nextc = 0;
  if(ch <= 0) {
    ui_keyboard(']');
    if(ch < 0) ui_keyboard('?');
    nextc = 0;
  } else if(ch < ' ') {
    if(ch == '\033') {
	ui_keyboard('E'); ui_keyboard('S'); ui_keyboard('C');
    } else {
	ui_keyboard('^');
	ui_keyboard(ch + 'A' - 1);
    }
  } else if(ch == 0177) {
    ui_keyboard('^'); ui_keyboard('?');
  } else {
    if(nextc >= (int)sizeof(kybd)-1) {
	memcpy(kybd, kybd+1, sizeof(kybd)-2);
	nextc = sizeof(kybd)-2;
    } else if(nextc == 0) {
	kybd[nextc++] = '[';
    }
    kybd[nextc++] = ch;
    kybd[nextc] = '\0';
    XmTextFieldSetString(KeyText, kybd);
  }
}

  

/*****************************************************************************/

static void select_object(Widget w, XtPointer data, XmListCallbackStruct *cbs)
{
  gv_ui_target(menuindex[cbs->item_position - 1], IMMEDIATE);
}

/*****************************************************************************/

static int id2menuindex(int id)
{
  extern int real_id(int);
  int i;

  if (drawer_cam_count() == 1 &&
      (id == TARGETCAMID || id == ALLCAMS || id == FOCUSID))
    id = real_id(id);
  for (i=0; i<menucount; ++i)
    if (menuindex[i] == id) return i;
  return -1;
}

static XmString *BuildObjectMenuList( int **menuindex, int *count )
{
  int i;
  int n, camcount;
  DObject *obj, **objs;
  char buf[150];
  XmString *menulist;

  camcount = drawer_cam_count();
    /* add 1 for "Current Camera" entry if multiple cameras */
  *count = camcount + drawer_geom_count() + (camcount > 1 ? 1 : 0);

  menulist = OOGLNewNE(XmString, *count, "menulist array");
  *menuindex = OOGLNewNE(int, *count, "no space for menuindex array");

  n = 0;
  objs = (DObject**)dgeom;
  for (i=0; i<dgeom_max; ++i) {
    if ( (obj=objs[i]) != NULL && ((DGeom*)obj)->citizenship != ALIEN) {
      if (obj->name[1] != NULL)
        sprintf(buf, "[%.64s] %.64s", obj->name[0], obj->name[1]);
      else
        strcpy(buf, obj->name[0]);
      (*menuindex)[n] = obj->id;
      menulist[n] = XmStringCreateSimple(buf);
      ++n;
    }
  }
  if (camcount > 1) {
    (*menuindex)[n] = FOCUSID;
    menulist[n] = XmStringCreateSimple("[c] Current Camera");
    ++n;
  }
  objs = (DObject **)dview;
  for (i=0; i<dview_max; ++i) {
    if ( (obj=objs[i]) != NULL ) {
      if (obj->name[1] == NULL) obj->name[1] = "Default Camera";
      sprintf(buf, "[%.64s] %.64s", obj->name[0], obj->name[1]);
      (*menuindex)[n] = obj->id;
      menulist[n] = XmStringCreateSimple(buf);
      ++n;
    }
  }
  /* Report how many we really found -- there may have been gaps */
  *count = n;
  return menulist;
}

static void DestroyStringList( XmString *list, int count )
{
  while ( --count >= 0 )
    XmStringFree(list[count]);
  OOGLFree(list);
}

static void
BuildBrowserMenu()
{
  XmString *menulist;

  XmListDeleteAllItems(ObjectList);

  if (menuindex)
    OOGLFree(menuindex);

  menulist = BuildObjectMenuList(&menuindex, &menucount);

  XmListAddItems(ObjectList, menulist, menucount, 0);
  DestroyStringList(menulist, menucount);
}

/*****************************************************************************/

void ui_adjustemodulelist(int interest)
{
  XmString *items;
  int i;
  emodule *em;

  XmListDeleteAllItems(EmodList);
  
  if(VVCOUNT(uistate.emod) > 0) {
    items = OOGLNewNE(XmString, VVCOUNT(uistate.emod), "emodules");

    for(i=0, em=VVEC(uistate.emod,emodule); i<VVCOUNT(uistate.emod); i++, em++)
	items[i] = XmStringCreateSimple(em->name);

    XmListAddItems(EmodList, items, VVCOUNT(uistate.emod), 0);
    DestroyStringList(items, VVCOUNT(uistate.emod));

    if (interest > 1)
	XmListSetBottomPos(EmodList, interest + 2);
  }
}

/*****************************************************************************/

static void select_module(Widget w, XtPointer data, XmListCallbackStruct *cbs)
{
  int i = cbs->item_position - 1;
  gv_ui_emodule_start(VVINDEX(uistate.emod, emodule, i)->name);
}

/*****************************************************************************/

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
