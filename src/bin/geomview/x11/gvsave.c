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
#include "../common/drawer.h"
#include "../common/ui.h"
#include "../common/comm.h"
#include "../common/worldio.h"
#include "../common/lang.h"
#include "gvui.h"
static /* prefix to ``char Root[]...'' */
#include "interface/Save.mib"

extern Display *dpy;
extern Widget TopLevel;

/* private methods and variables */
/*****************************************************************************/

static void select_type(Widget, XtPointer, XmAnyCallbackStruct *);
static void save_selection(Widget, XtPointer,
			XmFileSelectionBoxCallbackStruct *);
static int  suitable_id(int, char **);
static Widget shell, saveobj, geomobj, saveform, savewrapper, filetext;
static int    savechoice = 0;
static int blinkrate;

#define SAVE_WIO        0
#define SAVE_RMan       1
#define SAVE_SNAP       2
#define SAVE_PS		3
#define SAVE_PANELS     4

static struct saveops {
    int special;
    HandleOps *ops;
    int flag;
    char *name;
} save[] = {
	/* Note:
	 * This table *must* match the list of items given in the
	 * interface/Save.mib file for the ObjectMenu widget!
	 */
    { SAVE_WIO, &CommandOps,    0,              "Commands"              },
    { SAVE_WIO, &GeomOps,       SELF,           "Geometry alone"        },
    { SAVE_WIO, &GeomOps,       WORLDGEOM,      "Geometry [in world]"   },
    { SAVE_WIO, &GeomOps,       UNIVERSE,       "Geometry [in universe]"},
    { SAVE_RMan, NULL,          TIFF_KEYWORD,   "RMan [->tiff]"         },
    { SAVE_RMan, NULL,          FRAME_KEYWORD,  "RMan [->frame]"        },
    { SAVE_SNAP, NULL,          4,              "PPM GLX off-screen snapshot"},
    { SAVE_SNAP, NULL,          3,              "PPM Mesa off-screen snapshot"},
    { SAVE_SNAP, NULL,		2,		"SGI screen snapshot"   },
    { SAVE_SNAP, NULL,		1,		"PPM screen snapshot"   },
    { SAVE_SNAP, NULL,          0,		"PPM software snapshot" },
    { SAVE_PS,  NULL,		0,		"PostScript snapshot"	},
    { SAVE_WIO, &CamOps,        UNIVERSE,       "Camera"                },
    { SAVE_WIO, &TransOps,      WORLDGEOM,      "Transform [to world]"  },
    { SAVE_WIO, &TransOps,      UNIVERSE,       "Transform [to universe]"},
    { SAVE_WIO, &WindowOps,     UNIVERSE,       "Window"                },
};

/* Indexed by save[].flag value. "ppm" must come first (hard-coded below) */
static char *snaptypenames[] = {
    "ppm", "ppmscreen", "sgi", "ppmosmesa", "ppmosglx"
};

extern snap_entry snapshot_table[];

/*****************************************************************************/

void ui_load_savepanel()
{
  int	     n;
  Widget     savetypeform,
	     temp;

  mib_Widget *savetypeload,
	     *m_saveobj;
  mib_Menu   *objectmenu;
  XmString   label_text;
  XmString   xname;
  static char Save[] = "Save";

/*****************************************************************************/

  shell = ui_make_panel_and_form(Save, Root, False, True, &savewrapper);

/*****************************************************************************/

  saveform = XmCreateFileSelectionBox(savewrapper, "Save", NULL, 0);
  XtManageChild(saveform);

  temp = XmFileSelectionBoxGetChild(saveform, XmDIALOG_HELP_BUTTON);
  XtUnmanageChild(temp);

  XtAddCallback(saveform, XmNokCallback, (XtCallbackProc) save_selection,
		(XtPointer) NULL);

  temp = XmFileSelectionBoxGetChild(saveform, XmDIALOG_CANCEL_BUTTON);

  xname = XmStringCreateSimple("Hide");
  XtVaSetValues(temp, XmNlabelString, xname, NULL);
  XmStringFree(xname);

  XtAddCallback(temp, XmNactivateCallback, (XtCallbackProc) ui_hide,
		(XtPointer) Save);

/*****************************************************************************/

/*savetypeload = mib_load_interface(saveform, "interface/Save.mib",
			MI_FROMFILE);*/
  savetypeload = mib_load_interface(saveform, Root,
			MI_FROMSTRING);
  savetypeform = savetypeload->me;

  m_saveobj  = mib_find_name(savetypeload, "ObjectMenu");
  saveobj    = m_saveobj->me;
  objectmenu = (mib_Menu *)m_saveobj->myres;

  filetext = XmFileSelectionBoxGetChild(saveform, XmDIALOG_TEXT);
  XtVaGetValues(filetext, XmNblinkRate, &blinkrate, NULL);

  label_text = XmStringCreateLtoR("", XmSTRING_DEFAULT_CHARSET);
  XtVaSetValues(saveobj, XmNlabelString, label_text, NULL);
  XmStringFree(label_text);

  for (n = 0; n < objectmenu->numitems; n++)
  {
    XtAddCallback(objectmenu->items[n], XmNactivateCallback,
			(XtCallbackProc) select_type, (XtPointer)(long) n);
    /* If this is a possibly-absent snapshot item, check whether it's
     * really supported.
     */
    if(save[n].special == SAVE_SNAP && save[n].flag != 0) {
	int ok = 0;
	snap_entry *s;
	for(s = snapshot_table; s->name != NULL; s++) {
	    if(strcmp(s->name, snaptypenames[save[n].flag]) == 0) {
		ok = 1;
		break;
	    }
	}
	if(!ok) XtUnmanageChild(objectmenu->items[n]);
    }
  }

  geomobj    = mib_find_name(savetypeload, "Object")->me;
  XmTextFieldSetString(geomobj, "World");

  XtManageChild(savetypeform);

}

void ui_show_savepanel()
{
  XtVaSetValues(filetext, XmNblinkRate, blinkrate, NULL);
  XtManageChild(saveform);
}

/*****************************************************************************/

static int suitable_id(int savetype, char **namep)
{
   struct saveops *sp = &save[savetype];
   int id = drawer_idbyname(*namep);

   if(savetype >= 0 && savetype < COUNT(save)) {
        if(sp->ops == &CamOps || sp->ops == &WindowOps
                            || sp->special == SAVE_RMan
                            || sp->special == SAVE_SNAP
			    || sp->special == SAVE_PS) {
            if(!ISCAM(id)) id = FOCUSID;
        } else if(sp->ops == &GeomOps && !ISGEOM(id))
            id = GEOMID(uistate.targetgeom);
   }
   *namep = drawer_id2name(id);
   return (id != UNIVERSE && drawer_get_object(id) == NULL) ? NOID : id;
}

/*****************************************************************************/

static void select_type(Widget w, XtPointer data, XmAnyCallbackStruct *cbs)
{
  savechoice = (int)(long)data;
}

/*****************************************************************************/

static void save_selection(Widget w, XtPointer data,
		XmFileSelectionBoxCallbackStruct *cbs)
{
  char *fname, *object, *wobject;
  int   id;
  Pool *p;
  struct saveops *sp;
  Dimension width, height;

  if (!XmStringGetLtoR(cbs->value, XmSTRING_DEFAULT_CHARSET, &fname))
    return;

  if (!fname || *fname == '\0') {
    return;
  }

  wobject = object = XmTextFieldGetString(geomobj);

  if(fname == NULL || object == NULL || *fname == '\0' || *object == '\0'
                   || savechoice < 0)
  {
    goto out;
  }
  if((id = suitable_id(savechoice, &object)) == NOID) {
    XmTextFieldSetString(geomobj, "?");
    goto out;
  }
  sp = &save[savechoice];
  switch(sp->special) {
  case SAVE_WIO:
    p = PoolStreamTemp(fname,
		       NULL, (!strcmp(fname,"-"))?stdout:NULL, 1, sp->ops);
    if(p == NULL) {
        OOGLError(1, "Can't open output: %s: %s", fname, sperror());
        return;
    }
    worldio(sp->ops, p, sp->flag, id);
    PoolClose(p);
    PoolDelete(p);
    break;
  case SAVE_RMan:
    gv_rib_display(sp->flag, fname);
    gv_rib_snapshot(id, fname);
    break;
  case SAVE_SNAP:
    gv_snapshot(id, fname, snaptypenames[sp->flag], 0, 0);
    break;
  case SAVE_PS:
    gv_snapshot(id, fname, "ps", 0, 0);
    break;
  case SAVE_PANELS:
    /* Do nothing ... yet :-) */
    break;
  }
  /* Ugh.  Older Motif versions (?) don't remember to cancel their
   * blink timers if we simply unrealize the shell widget from inside this
   * callback routine.  Even stuffing the XtUnrealizeWidget() in a workproc
   * doesn't fix it.   So remove its contents before unmapping it.
   * But, then we need to save and restore its size, too, lest it be
   * zero size when it reappears.  Something's broken here, but I can't
   * guess what.
   */
  XtSetKeyboardFocus(shell, None);
  XtVaSetValues(filetext, XmNblinkRate, 0, NULL);
  XtVaGetValues(shell, XmNwidth, &width, XmNheight, &height, NULL);
  XtUnmanageChild(saveform);
  ui_showpanel(P_SAVE, 0);
  XtVaSetValues(shell, XmNwidth, width, XmNheight, height, NULL);
  uistate.savewhat = NOID;

 out:
  if (fname) {
    XtFree(fname);
  }
  if (wobject) {
    XtFree(wobject);
  }
}

/*****************************************************************************/

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 4 ***
 * End: ***
 */
