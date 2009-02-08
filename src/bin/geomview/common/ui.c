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


/* Authors: Stuart Levy, Tamara Munzner, Mark Phillips,
   Celeste Fowler */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdlib.h>
#include "lisp.h"
#include "lang.h"
#include "drawer.h"
#include "transform.h"
#include "ui.h"
#include "motion.h"
#include "event.h"
#include "comm.h"


char *shades[5] = {
	"[0as] Constant",	/* APF_CONSTANT */
	"[1as] Flat",		/* APF_FLAT */
	"[2as] Smooth",		/* APF_SMOOTH */
	"[3as] CSmooth",	/* APF_CSMOOTH */
	"[4as] VCflat",	        /* APF_VCFLAT */
};

char *translucencies[3] = {
  "[0mt] BSP-Tree Alpha Blending",
  "[1mt] Screen Door",
  "[2mt] Simple Alpha Blending"
};

char *norm[4] = {
	"[0N] None",		/* NONE */
	"[1N] Individual",	/* EACH */
	"[2N] Sequence",	/* ALL  */
	"[3N] Keep",		/* KEEP */
};

char *proj[2] = {
	"[0vp]  Orthographic",	/* ORTHOGRAPHIC */
	"[1vp]  Perspective",	/* PERSPECTIVE */
};

char *spc[3] = {
	"[me] Euclidean",	/* EUCLIDEAN */
	"[mh] Hyperbolic",	/* HYPERBOLIC */
	"[ms] Spherical",	/* SPHERICAL */
};

char *mdl[3] = {
	"[mv] Virtual",		/* VIRTUAL */
	"[mp] Projective",	/* PROJECTIVE */
	"[mc] Conformal", 	/* CONFORMALBALL */
};


char OBJROTATE[]	=	"[r] Rotate";
char OBJTRANSLATE[]	=	"[t] Translate";
char OBJZOOM[]		=	"[z] Cam Zoom";
char OBJSCALE[]		=	"[s] Geom Scale";
char OBJFLY[]		=	"[f] Cam Fly";
char OBJORBIT[]		=	"[o] Cam Orbit";
char LIGHTEDIT[] 	=	"[le] Edit Lights";

/* in ui.c */

void cui_init()
{

  uistate.targetid = WORLDGEOM;
  uistate.centerid = TARGETID;
  uistate.bbox_center = 0;
  uistate.targetgeom = 0; /* World geom is index 0 */
  uistate.targetcam = INDEXOF(FOCUSID);
  uistate.cam_wm_focus = 0; /* bad idea */
  uistate.mode_count = 0;
  uistate.emod_dir = NULL;
  uistate.apoverride = ~0;	/* Enable override by default */
  uistate.inertia = 1;		/* Enable inertia by default */
  uistate.pick_invisible = 1;	/* Pick invisible as well as visible objects? */

  /*
   * The order of these calls must be the same as the declared integers
   * in ui.h!! 
   */
  ui_install_mode(OBJROTATE,	minterp_rotate,		T_NONE);
  ui_install_mode(OBJZOOM,	minterp_zoom, 		T_CAM);
  ui_install_mode(OBJTRANSLATE,	minterp_translate,	T_NONE);
  ui_install_mode(OBJFLY,	minterp_fly,		T_CAM);
  ui_install_mode(OBJORBIT,	minterp_orbit,		T_CAM);
  ui_install_mode(OBJSCALE,	minterp_scale, 		T_GEOM);
  VVINIT(uistate.emod, emodule, 10);
  vvzero(&uistate.emod);
  uistate.savewhat = NOID;
  uistate.cursor_still = UI_CURSOR_STILL; 
  uistate.cursor_twitch = UI_CURSOR_TWITCH;
  uistate.longwhile = 2.5;		/* Max credible inter-redraw interval */
  uistate.backface = 0;
  return;
}

void ui_reinstall_mode(char *name, PFI proc, int type, int index)
{
  uistate.modenames[index] = name;
  uistate.modes[index] = proc;
  uistate.modetype[index] = type;
}

void ui_install_mode(char *name, PFI proc, int type)
{
  if(uistate.mode_count >= MAXMODES) {
    OOGLError(1, "Motion-mode table full (max %d entries)", MAXMODES);
    uistate.mode_count = MAXMODES-1;
  }
  uistate.modenames[uistate.mode_count] = name;
  uistate.modes[uistate.mode_count] = proc;
  uistate.modetype[uistate.mode_count] = type;
  ++uistate.mode_count;

D1PRINT(("ui_install_mode: name=%s, proc=%x\n", name, proc));

}

void ui_uninstall_mode(char *name)
{
  int i = ui_mode_index(name);

  if (i<0) return;
  --uistate.mode_count;
  while (i<uistate.mode_count) {
    uistate.modenames[i] = uistate.modenames[i+1];
    uistate.modes[i] = uistate.modes[i+1];
    uistate.modetype[i] = uistate.modetype[i+1];
    i++;
  }
}

/* 
 * Match names
 */
int ui_mode_index(char *name)
{
  int i;

  for (i=0; i<uistate.mode_count; ++i) {
    if(!strcasecmp(name, uistate.modenames[i]))
	return i;
  }
  OOGLError(0, "ui_mode_index: unknown mode \"%s\"", name);
  return 0;
}

void ui_emodule_uninstall(int k)
{
  emodule *em;
  int i;
  if(k < 0 || k >= VVCOUNT(uistate.emod))
    return;
  em = &VVEC(uistate.emod, emodule)[k];
  for(i = k+1; i < VVCOUNT(uistate.emod); i++, em++)
    *em = *(em+1);
  VVCOUNT(uistate.emod)--;
  uistate.emod_changed = k+1;
}

emodule *
ui_emodule_install(int before, char *ename, PFI func)
{
  int i,k;
  emodule *em, *emp;

  if((k = ui_emodule_index(ename,NULL)) >= 0)
    ui_emodule_uninstall(k);
  k = VVCOUNT(uistate.emod)++;
  vvneeds(&uistate.emod, VVCOUNT(uistate.emod));
  if(before > k) before = k; else if(before < 0) before = 0;

  /* Shift the part of emodule table below the insertion point down
     one unit.  Don't use bcopy() for this because it does not
     correctly handle overlapping src and dst.
     mbp Wed Sep  9 19:07:39 1992 */
  em = VVEC(uistate.emod, emodule);
  for (i=VVCOUNT(uistate.emod)-1, emp = &em[i]; i>before; i--, emp--)
    *emp = *(emp-1);

  em = &VVEC(uistate.emod, emodule)[before];
  em->name = strdup(ename);
  em->func = (PFV)func;
  em->dir =  uistate.emod_dir;
  /* Other emodule fields zeroed */
  uistate.emod_changed = before+1;
  return em;
}

LDEFINE(emodule_clear, LVOID,
	"(emodule-clear)\n\
	Clears the geomview application (external module) browser.")
{
  LDECLARE(("emodule-clear", LBEGIN,
	    LEND));

  while (VVCOUNT(uistate.emod))
    ui_emodule_uninstall(0);
  return Lt;
}

int
ui_emodule_index(char *ename, emodule **emp)
{
  int i;
  emodule *em;
  if(!emp) emp = &em;
  for (i=0, em=VVEC(uistate.emod, emodule); i<VVCOUNT(uistate.emod); i++, em++)
	if (strcasecmp(em->name, ename)==0) {
	    *emp = em;
	    return i;
	}
  *emp = NULL;
  return -1;
}

/*-----------------------------------------------------------------------
 * Function:	set_ui_target
 * Description:	set the target of user actions
 * Args:	type: T_GEOM or T_CAM
 *		index: index of current geom or current cam
 * Returns:	
 * Author:	mbp
 * Date:	Thu Nov 21 14:04:28 1991
 * Notes:	equivalent to set_ui_target_id( ID(type, index) )
 */
void
set_ui_target(int type, int index)
{
  uistate.targetid = ID(type, index);
  switch (uistate.targettype = type) {
  case T_GEOM:
    uistate.targetgeom = index;
    if (uistate.bbox_center && uistate.targetid != WORLDGEOM) {
      make_center_from_bbox("CENTER", uistate.targetid);
    }
    break;
  case T_CAM:
    uistate.targetcam = index;
    break;
  }
}

void
set_ui_center(int id)
{
  uistate.centerid = id;
}

void
set_ui_center_origin(int use_bbox_center)
{
  uistate.bbox_center = use_bbox_center != 0; /* boolean value */
  if (uistate.bbox_center && uistate.targetid != WORLDGEOM) {
    make_center_from_bbox("CENTER", uistate.targetid);
  } else {
    gv_ui_center(TARGETID);
  }
}

void
set_ui_wm_focus(int cam_wm_focus)
{
  uistate.cam_wm_focus = cam_wm_focus != 0;
}

/*-----------------------------------------------------------------------
 * Function:	set_ui_target_id
 * Description:	set the target id of user actions
 * Args:	id: id to target
 * Returns:	
 * Author:	mbp
 * Date:	Thu Nov 21 14:05:33 1991
 * Notes:	equivalent to set_ui_target( TYPEOF(id), INDEXOF(id) )
 */
void
set_ui_target_id(int id)
{
  set_ui_target( TYPEOF(id), INDEXOF(id) );
}


void ui_cleanup()
{
  int i;
  for(i = VVCOUNT(uistate.emod); --i >= 0; ) {
    if(VVEC(uistate.emod, emodule)[i].pid > 0) {
	emodule_kill(&VVEC(uistate.emod, emodule)[i]);
    }
  }
  gv_delete(ALLCAMS);
}

/* maybe replace this later with something more useful: */
void ui_targeting()
{}


/**********************************************************************/

/* for debugging */
void print_emodtable()
{
  emodule *em;
  int i;

  for(i=0, em=VVEC(uistate.emod,emodule); i<VVCOUNT(uistate.emod); i++, em++) {
    fprintf(stderr, "em[%2d] = %s\n", i, em->name);
  }
  return;
}

LDEFINE(cursor_still, LVOID,
	"(cursor-still [INT])\n\
	Sets the number of microseconds for which the cursor must not\n\
	move to as holding still.  If INT is not specified,\n\
	the value will be reset to the default.")
{
  uistate.cursor_still = UI_CURSOR_STILL;
  LDECLARE(("cursor-still", LBEGIN,
	    LOPTIONAL,
	    LINT, &uistate.cursor_still,
	    LEND));
  return Lt;
}

LDEFINE(cursor_twitch, LVOID,
	"(cursor-twitch	[INT])\n\
	Sets the distance which the cursor must not move (in x or\n\
	y) to as holding still.  If INT is not specified,\n\
	the value will be reset to the default.")
{
  uistate.cursor_twitch = UI_CURSOR_TWITCH;
  LDECLARE(("cursor-twitch", LBEGIN,
	    LOPTIONAL,
	    LINT, &uistate.cursor_twitch,
	    LEND));
  return Lt;
}

LDEFINE(ap_override, LVOID,
	"(ap-override [on|off])\n\
	Selects whether appearance controls should override objects' own\n\
	settings.  On by default.  With no arguments, returns current setting.")
{
  Keyword kw = NOT_A_KEYWORD;

  LDECLARE(("ap-override", LBEGIN,
	LOPTIONAL,
	LKEYWORD, &kw,
	LEND));

  if (kw == NOT_A_KEYWORD) {
    Keyword on = uistate.apoverride ? ON_KEYWORD : OFF_KEYWORD;
    return LNew(LKEYWORD, &on);
  }
  drawer_int(WORLDGEOM, DRAWER_APOVERRIDE, boolval("ap-override", kw));
  return Lt;
}


int
uispace(int space)
{
  switch (space) {
  case TM_EUCLIDEAN:  return EUCLIDEAN;
  case TM_HYPERBOLIC: return HYPERBOLIC;
  case TM_SPHERICAL: return SPHERICAL;
  }
  return -1;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
