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

/* Authors: Stuart Levy, Tamara Munzner, Mark Phillips */

#include "mg.h"
#include "fsa.h"
#include "drawer.h"
#include "event.h"
#include "ui.h"
#include "comm.h"
#include "transform.h"
#include "transformn.h"
#include "lang.h"
#include "ooglutil.h"
#include "geom.h"
#include "camera.h"
#include "space.h"
#include "list.h"		/* for ListRemove(), ListAppend() */
#include "comment.h"
#include "transobj.h"
#include "ntransobj.h"
#include "lang.h"
#include "lispext.h"
#include "lights.h"
#include "cmodel.h"
#include "mouse.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>

/* Use sys/file.h since unistd.h doesn't exist on the NeXT, and
   hopefully sys/file.h is sufficient on both NeXT and IRIS [ needed
   for use of access() ]. */
/*#include <unistd.h>*/
#include <sys/file.h>	

static Fsa name_fsa = NULL;

DrawerState drawerstate;

static int drawer_dgeom(int id, GeomStruct *gs);

static int spaceval(char *s, int val);
static HModelValue hmodelval(char *s, int val);
static int normalval(char *s, int val);
extern Appearance *GeomAppearance( Geom * );
extern mgshadefunc softshader(int id);

TransformStruct ts_identity;

#define	DGEOM_MAX	16
#define	DVIEW_MAX	8

int dgeom_max = DGEOM_MAX;
int dview_max = DVIEW_MAX;
DGeom **dgeom = NULL;
DView **dview = NULL;

static int nwins = 1;

static Appearance *base_defaultap = NULL;
static Appearance *worldap = NULL;
extern Color initial_defaultbackcolor;

static GeomStruct nullgs = {NULL, NULL};    /* Substitute for optional arg */

extern HandleOps GeomOps, WindowOps;

#define DGobj(obj) ((DGeom *)obj)
#define DVobj(obj) ((DView *)obj)

#define name_object(obj, ni, name) _name_object((DObject*)(obj),ni,name)

static char *	_name_object(DObject *obj, int ni, char *name);
static void	track_changes();
static void	winmoved(int camid);
static bool     nonidentity(Transform T);
static DGeom *	new_dgeom(char *from, enum citizenship citizenship);
static DView *	new_dview();
static void	updateit(float dt);
static void	draw_view(DView *dv);
static void	really_draw_view(DView *dv);
static void	reconfigure_view(DView *dv);
static char *	new_object_name(int type);
static void	object_changed(Handle **hp, DObject *obj, void *seqno);
static int	object_register(Handle **hp, Ref *ignored, DObject *obj);
static void	update_dgeom(DGeom *);
static void	update_view(DView *);
static void	normalize(DGeom *dg, int normalization);
static char *	unique_name(char *name, int id);
static void	delete_geometry(DGeom *dg);
static void	delete_camera(DView *dv);
static void	dview_set_hmodel_camspace(DView *dv, int space);

#ifdef MANIFOLD
static int verbose_manifold = 0;
static int trivial_visibility = 0;
#endif

/*
 * Pure inquiry functions -- these do not change any state.
 */

int
drawer_geom_count(void)
{
  int i, n;
  DGeom *dg;

  n = 1;
  LOOPSOMEGEOMS(i,dg,ORDINARY) ++n;
  return n;
}

int
drawer_cam_count(void)
{
  int i, n;
  DView *dv;

  n = 0;
  LOOPVIEWS(i,dv) ++n;
  return n;
}

int
drawer_idbyctx( mgcontext *ctx )
{
  int i;
  DView *dv;

  LOOPVIEWS(i,dv) {
    if(dv->mgctx == ctx)
      return CAMID(i);
  }
  return NOID;
}

DObject *
drawer_get_object( int id )
{
 recheck:
  if(ISGEOM(id)) {
    switch(id) {
    case ALLGEOMS: id = WORLDGEOM; goto recheck;
    case TARGETID: id = uistate.targetid; goto recheck;
    case SELF:
    case TARGETGEOMID: id = GEOMID(uistate.targetgeom); goto recheck;
    case CENTERID: id = uistate.centerid; goto recheck;
    default:
      if(INDEXOF(id) < 0 || INDEXOF(id) >= dgeom_max) return NULL;
      return (DObject *)dgeom[INDEXOF(id)];
    }
  } else if(ISCAM(id)) {
    switch(id) {
    case DEFAULTCAMID: return (DObject *)&drawerstate.defview;
    case FOCUSID: case ALLCAMS: id = CAMID(uistate.mousefocus); goto recheck;
    case TARGETCAMID: id = CAMID(uistate.targetcam); goto recheck;
    default:
      if(INDEXOF(id) < 0 || INDEXOF(id) >= dview_max) return NULL;
      return (DObject *)dview[INDEXOF(id)];
    }
  }
  return NULL;
}

int
drawer_idmatch(int id1, int id2)
{
  switch (id1) {
  case ALLGEOMS:
    switch (id2) {
    case ALLGEOMS:		return 1;
    case ALLCAMS:		return 0;
    case FOCUSID:		return 0;
    case TARGETGEOMID:		return 1;
    case TARGETCAMID:		return 0;
    case TARGETID:		return ISGEOM(uistate.targetid);
    case CENTERID:		return ISGEOM(uistate.centerid);
    case NOID:			return 0;
    case DEFAULTCAMID:		return 0;
    case SELF:			return 0;
    case UNIVERSE:		return 0;
    case PRIMITIVE:		return 0;
    default:			return ISGEOM(id2) && drawer_get_object(id2);
    }
    break;
  case ALLCAMS:
    switch (id2) {
    case ALLGEOMS:		return 0;
    case ALLCAMS:		return 1;
    case FOCUSID:		return 1;
    case TARGETGEOMID:		return 0;
    case TARGETCAMID:		return 1;
    case TARGETID:		return ISCAM(uistate.targetid);
    case CENTERID:		return ISCAM(uistate.targetid);
    case NOID:			return 0;
    case DEFAULTCAMID:		return 1;
    case SELF:			return 0;
    case UNIVERSE:		return 0;
    case PRIMITIVE:		return 0;
    default:			return ISCAM(id2) && drawer_get_object(id2);
    }
    break;
  case FOCUSID:
    switch (id2) {
    case ALLGEOMS:		return 0;
    case ALLCAMS:		return 1;
    case FOCUSID:		return 1;
    case TARGETGEOMID:		return 0;
    case TARGETCAMID:		return uistate.mousefocus==CAMID(uistate.targetcam);
    case TARGETID:		return uistate.mousefocus==uistate.targetid;
    case CENTERID:		return uistate.mousefocus==uistate.centerid;
    case NOID:			return 0;
    case DEFAULTCAMID:		return 0;
    case SELF:			return 0;
    case UNIVERSE:		return 0;
    case PRIMITIVE:		return 0;
    default:			return uistate.mousefocus==id2;
    }
    break;
  case TARGETGEOMID:
    switch (id2) {
    case ALLGEOMS:		return 1;
    case ALLCAMS:		return 0;
    case FOCUSID:		return 0;
    case TARGETGEOMID:		return 1;
    case TARGETCAMID:		return 0;
    case TARGETID:		return ISGEOM(uistate.targetid);
    case CENTERID:		return uistate.centerid==uistate.targetid;
    case NOID:			return 0;
    case DEFAULTCAMID:		return 0;
    case SELF:			return 0;
    case UNIVERSE:		return 0;
    case PRIMITIVE:		return 0;
    default:			return GEOMID(uistate.targetgeom)==id2;
    }
    break;
  case TARGETCAMID:
    switch (id2) {
    case ALLGEOMS:		return 0;
    case ALLCAMS:		return 1;
    case FOCUSID:		return uistate.mousefocus==uistate.targetid;
    case TARGETGEOMID:		return 0;
    case TARGETCAMID:		return 1;
    case TARGETID:		return ISCAM(uistate.targetid);
    case CENTERID:		return ISCAM(uistate.centerid);
    case NOID:			return 0;
    case DEFAULTCAMID:		return 0;
    case SELF:			return 0;
    case UNIVERSE:		return 0;
    case PRIMITIVE:		return 0;
    default:			return CAMID(uistate.targetcam)==id2;
    }
    break;
  case TARGETID:
    switch (id2) {
    case ALLGEOMS:		return ISGEOM(uistate.targetid);
    case ALLCAMS:		return ISCAM(uistate.targetid);
    case FOCUSID:		return ISCAM(uistate.targetid);
    case TARGETGEOMID:		return ISGEOM(uistate.targetid);
    case TARGETCAMID:		return ISCAM(uistate.targetid);
    case TARGETID:		return 1;
    case CENTERID:		return uistate.targetid==uistate.centerid;
    case NOID:			return 0;
    case DEFAULTCAMID:		return 0;
    case SELF:			return 0;
    case UNIVERSE:		return 0;
    case PRIMITIVE:		return 0;
    default:			return uistate.targetid==id2;
    }
    break;
  case CENTERID:
    switch (id2) {
    case ALLGEOMS:		return ISGEOM(uistate.centerid);
    case ALLCAMS:		return ISCAM(uistate.centerid);
    case FOCUSID:		return uistate.centerid==uistate.mousefocus;
    case TARGETGEOMID:		return uistate.centerid==GEOMID(uistate.targetgeom);
    case TARGETCAMID:		return uistate.centerid==CAMID(uistate.targetcam);
    case TARGETID:		return uistate.centerid==uistate.targetid;
    case CENTERID:		return 1;
    case NOID:			return 0;
    case DEFAULTCAMID:		return 0;
    case SELF:			return 0;
    case UNIVERSE:		return 0;
    case PRIMITIVE:		return 0;
    default:			return uistate.centerid==id2;
    }
    break;
  case NOID:
    switch (id2) {
    case ALLGEOMS:		return 0;
    case ALLCAMS:		return 0;
    case FOCUSID:		return 0;
    case TARGETGEOMID:		return 0;
    case TARGETCAMID:		return 0;
    case TARGETID:		return 0;
    case CENTERID:		return 0;
    case NOID:			return 1;
    case DEFAULTCAMID:		return 0;
    case SELF:			return 0;
    case UNIVERSE:		return 0;
    case PRIMITIVE:		return 0;
    default:			return 0;
    }
    break;
  case DEFAULTCAMID:
    switch (id2) {
    case ALLGEOMS:		return 0;
    case ALLCAMS:		return 1;
    case FOCUSID:		return 0;
    case TARGETGEOMID:		return 0;
    case TARGETCAMID:		return 0;
    case TARGETID:		return 0;
    case CENTERID:		return 0;
    case NOID:			return 0;
    case DEFAULTCAMID:		return 1;
    case SELF:			return 0;
    case UNIVERSE:		return 0;
    case PRIMITIVE:		return 0;
    default:			return 0;
    }
    break;
  case SELF:
    switch (id2) {
    case ALLGEOMS:		return 0;
    case ALLCAMS:		return 0;
    case FOCUSID:		return 0;
    case TARGETGEOMID:		return 0;
    case TARGETCAMID:		return 0;
    case TARGETID:		return 0;
    case CENTERID:		return 0;
    case NOID:			return 0;
    case DEFAULTCAMID:		return 0;
    case SELF:			return 1;
    case UNIVERSE:		return 0;
    case PRIMITIVE:		return 0;
    default:			return 0;
    }
    break;
  case UNIVERSE:
    switch (id2) {
    case ALLGEOMS:		return 0;
    case ALLCAMS:		return 0;
    case FOCUSID:		return 0;
    case TARGETGEOMID:		return 0;
    case TARGETCAMID:		return 0;
    case TARGETID:		return 0;
    case CENTERID:		return 0;
    case NOID:			return 0;
    case DEFAULTCAMID:		return 0;
    case SELF:			return 0;
    case UNIVERSE:		return 1;
    case PRIMITIVE:		return 0;
    default:			return 0;
    }
    break;
  case PRIMITIVE:
    switch (id2) {
    case ALLGEOMS:		return 0;
    case ALLCAMS:		return 0;
    case FOCUSID:		return 0;
    case TARGETGEOMID:		return 0;
    case TARGETCAMID:		return 0;
    case TARGETID:		return 0;
    case CENTERID:		return 0;
    case NOID:			return 0;
    case DEFAULTCAMID:		return 0;
    case SELF:			return 0;
    case UNIVERSE:		return 0;
    case PRIMITIVE:		return 1;
    default:			return 0;
    }
    break;
  default:
    switch (id2) {
    case ALLGEOMS:		return ISGEOM(id1) && drawer_get_object(id1);
    case ALLCAMS:		return ISCAM(id1) && drawer_get_object(id1);
    case FOCUSID:		return id1==uistate.mousefocus;
    case TARGETGEOMID:		return id1==GEOMID(uistate.targetgeom);
    case TARGETCAMID:		return id1==CAMID(uistate.targetcam);
    case TARGETID:		return id1==uistate.targetid;
    case CENTERID:		return id1==uistate.centerid;
    case NOID:			return 0;
    case DEFAULTCAMID:		return 0;
    case SELF:			return 0;
    case UNIVERSE:		return 0;
    case PRIMITIVE:		return 0;
    default:			return id1==id2;
    }
    break;
  }
}



/*
 * Map an id to a short name 
 */
char *
drawer_id2name( int id )
{
  switch(id) {
  case ALLGEOMS:  return "g*";
  case ALLCAMS:   return "c*";
  case FOCUSID:	  return "focus";
  case TARGETGEOMID:return "g.";
  case TARGETCAMID: return "c.";
  case SELF:	  return "self";
  case UNIVERSE:  return "universe";
  case DEFAULTCAMID:return "defaultcam";
  case TARGETID:   return "target";
  case CENTERID:   return "center";
  case BBOXCENTERID: return "bbox-center";
  case PRIMITIVE:  return "primitive";
  case NOID:  return "none";
  default:
    if(id > 0) {
      DObject *obj = drawer_get_object(id);
      if(obj) return obj->name[1];
    }
  }
  OOGLError(1, "drawer_id2name unknown id %d", id);
  return "?";
}

/*
 * Utility routine for MAYBE_LOOP_ALL macro.
 * Finds object(s) of the given type (any type if T_NONE),
 * and returns a DObject * each time called.
 * Returns NULL after all appropriate objects have been returned.
 * Assumes that the iterator variable *indexp is incr'd by one between calls.
 * *indexp does double duty: for id's which refer to a single object, it's
 * used as a flag so that the object is returned exactly once (*indexp==0).
 * For broadcast id's (ALLGEOMS, ALLCAMS), *indexp is the current table index.
 * Note that MAYBE_LOOP_ALL does *not* treat WORLDGEOM specially.
 */
DObject *
drawer_next_object( int id, int *indexp, int type )
{
  int index = *indexp;
  int limit;
  DObject **objs;

  if(id == DEFAULTCAMID && type != T_GEOM)
    return index > 0 ? NULL : (DObject *)&drawerstate.defview;

  /* Calling real_id() twice handles TARGETCAMID which might 
   * yield FOCUSID */
  if(INDEXOF(id) < 0 && INDEXOF(id) != ALLINDEX)
    id = real_id(real_id(id));

  if(type != T_NONE && type != TYPEOF(id)) return NULL;

  switch(TYPEOF(id)) {
  case T_CAM:  limit = dview_max; objs = (DObject **)dview; break;
  case T_GEOM: limit = dgeom_max; objs = (DObject **)dgeom; break;
  default: return NULL;
  }

  if(INDEXOF(id) == ALLINDEX) {
    while(index < limit && objs[index] == NULL)
      index++;
    *indexp = index;
  } else if(index == 0)
    index = INDEXOF(id);
  else
    return NULL;

  return (index < 0 || index >= limit) ? NULL : objs[index];
}

/*
 * Utility routine for MAYBE_LOOP macro.
 * Like drawer_next_object() above, but arranges that id==WORLDGEOM
 * is mapped to a broadcast to all ordinary (non-World, non-alien) DGeom's.
 */
DObject *
drawer_next_bcast( int id, int *indexp, int type )
{
  DObject *obj;

  if(INDEXOF(id) < 0 && INDEXOF(id) != ALLINDEX)
    id = real_id(real_id(id));
  if(id == WORLDGEOM)
    while((obj = drawer_next_object(ALLGEOMS, indexp, type)) != NULL
	  && DGobj(obj)->citizenship == ALIEN)
      (*indexp)++;
  else
    obj = drawer_next_object(id, indexp, type);
  return obj;
}


/*
 * Get the indicated object's net Appearance, merging everything from the
 * backstop appearance down.
 * For use by the UI, to tell what current settings are.
 */
Appearance *
drawer_get_ap( int id )
{
  DGeom *dg;
  Appearance *ap, *nap;

  ap = drawerstate.ap;
  nap = worldap;
  if(nap)
    ap = ApMerge(nap, ap, 0);

  dg = (!ISGEOM(id) || id == GEOMID(ALLINDEX))
    ? NULL : (DGeom *)drawer_get_object(id);
  if(dg && dg != dgeom[0] && (nap = GeomAppearance(dg->Item)) != NULL) {
    nap = ApMerge(nap, ap, 0);
    if(ap != drawerstate.ap) ApDelete(ap);
    ap = nap;
  }
  if(ap == drawerstate.ap)
    RefIncr((Ref *)ap);
  return ap;
}


/*
 * Checks whether any object is moving (so that the main loop shouldn't block).
 * Returns 1 if so, 0 otherwise.
 */
bool
drawer_moving(void)
{
  int i;
  DObject *o;

  if (motions_exist())
    return true;
  for(i = 0; i < dgeom_max; i++)
    if((o = (DObject *)dgeom[i]) != NULL && (o->moving || o->changed))
      return true;
  for(i = 0; i < dview_max; i++)
    if((o = (DObject *)dview[i]) != NULL && (o->moving || o->changed))
      return true;
  return false;
}


/* 
 * Public functions
 */

/*-----------------------------------------------------------------------
 * Function:	drawer_idbyname
 * Description:	parse a name and return the corresponding id
 * Args:	*name: name to parse
 * Returns:	id of corresponding dgeom or view
 * Author:	mbp
 * Date:	Fri Nov 29 13:42:43 1991
 * Notes:	returns NOID if there is no object with the given name
 */
int
drawer_idbyname(char *name)
{
  int id;

  switch (id=(int)(long)fsa_parse(name_fsa, name)) {
  case TARGETID:
    id = uistate.targetid;
    break;
  case TARGETGEOMID:
    id = GEOMID(uistate.targetgeom);
    break;
  case TARGETCAMID:
    id = CAMID(uistate.targetcam);
    break;
  default:	/* including FOCUSID -- keep literal */
    break;
  }
  return id;
}

int
drawer_name2metaid(char *name)
{
  return (int)(long)fsa_parse(name_fsa, name);
}



LDEFINE(name_object, LVOID,
	"(name-object ID NAME)\n"
	"Assign a new NAME (a string) to ID.  A number is appended if "
	"that name is in use (for example, \"foo\" -> \"foo<2>\").  The new "
	"name, possibly with number appended, may be used as object's "
	"id thereafter.")
{
  int id;
  char *name;
  LDECLARE(("name-object", LBEGIN,
	    LID, &id,
	    LSTRING, &name,
	    LEND));
  drawer_name_object( id, 1, name );
  return Lt;
}

void
drawer_name_object(int id, int ni, char *name)
{
  DObject *obj = drawer_get_object(id);

  if (obj)
    name_object( obj, ni, name );
  ui_objectchange();
}

LDEFINE(update_draw, LVOID,
	"(update-draw CAM-ID  [timestep_in_seconds])\n"
	"Apply each incremental motion once and then draw CAM-ID. "
	"Applies \"timestep\" seconds' worth of motion, or uses elapsed real "
	"time if \"timestep\" is absent or zero.")
{
  int id;
  float dt = 0;
  LDECLARE(("update-draw", LBEGIN,
	    LID, &id,
	    LOPTIONAL, LFLOAT, &dt,
	    LEND));
  gv_update(dt);
  gv_draw(id);
  return Lt;
}

LDEFINE(draw, LVOID,
	"(draw CAM-ID)\n"
	"Draw the view in CAM-ID, if it needs redrawing. See also \"redraw\".")
{
  DView *dv;
  int index;
  int id;

  LDECLARE(("draw", LBEGIN,
	    LID, &id,
	    LEND));

  MAYBE_LOOP_ALL(id, index, T_CAM, DView, dv) {
    draw_view(dv);
  }
  return Lt;
}

LDEFINE(update, LVOID,
	"(update [timestep_in_seconds])\n"
	"Apply each incremental motion once. Uses timestep if it's present and "
	"nonzero; otherwise motions are proportional to elapsed real time.")
{
  float realdt, dt = 0;
  LDECLARE(("update", LBEGIN,
	    LOPTIONAL, LFLOAT, &dt,
	    LEND));
  realdt = elapsed(&drawerstate.lastupdate, &drawerstate.lastupdate);
  if(realdt > uistate.longwhile)
    realdt = uistate.longwhile;
  if(dt != 0 || realdt != 0)
    updateit(dt == 0 ? realdt : dt);
  track_changes();
  return Lt;
}

void
drawer_updateproc(int id, PFI func)
{
  DObject *obj;
  int index;
  
  MAYBE_LOOP_ALL(id, index, T_NONE, DObject, obj) {
    obj->updateproc = func;
    obj->moving = ((obj->updateproc!=NULL) || nonidentity(obj->Incr));
  }
}

LDEFINE(redraw, LVOID,
	"(redraw CAM-ID)\n"
	"States that the view in CAM-ID should be redrawn on the "
	"next pass through the main loop or the next invocation of \"draw\".")
{
  DView *dv;
  int index;
  int id;
  LDECLARE(("redraw", LBEGIN,
	    LID, &id,
	    LEND));

  MAYBE_LOOP_ALL(id, index, T_CAM, DView, dv) {
    dv->newcam = true;
    dv->frozen &= ~SOFT_FROZEN;
  }
  track_changes();
  return Lt;
}

LDEFINE(freeze, LVOID,
	"(freeze         CAM-ID  [hard-freeze])\n"
	"Freeze CAM-ID; drawing in this camera's window is turned off "
	"until it is explicitly redrawn with \"(redraw CAM-ID)\", after "
	"which time drawing resumes as normal.  hard-freeze (default false) "
	"is intended to be \"true\" only when windows are iconified, "
 	"making them immune even to \"redraw\" until another "
	"\"(freeze CAM-ID false)\".")
{
  DView *dv;
  int index;
  int id;
  Keyword hard = NO_KEYWORD;
  int freeze;

  LDECLARE(("freeze", LBEGIN,
	    LID, &id,
	    LOPTIONAL,
	    LKEYWORD, &hard,
	    LEND));

  freeze = boolval("freeze", hard) ? HARD_FROZEN : SOFT_FROZEN;

  MAYBE_LOOP_ALL(id, index, T_CAM, DView, dv) {
    dv->frozen = freeze;
  }
  return Lt;
}

void 
drawer_pause(int pause)
{
  drawerstate.pause = pause;
  ui_action(ACTION_PAUSE);
}


void 
drawer_stop(int id)
{
  if (id == NOID) {
    gv_xform_incr(ALLCAMS,  &ts_identity);
    gv_xform_incr(ALLGEOMS, &ts_identity);
    gv_xform_incr(WORLDGEOM,&ts_identity);
  } else {
    gv_xform_incr(id, &ts_identity);
  }
}

static void center_geom(int id, int center)
{
  int index;
  DGeom *dg;

  MAYBE_LOOP_ALL(id, index, T_GEOM, DGeom, dg) {
    gv_transform_set(dg->id, center, center,
		     TRANSLATE_KEYWORD, 0, 0, 0);
    drawer_stop(dg->id);
  }
}

void drawer_center(int id)
{
  if (TYPEOF(id) == T_NONE) {
    drawer_stop(NOID);
    center_geom(ALLGEOMS, WORLDGEOM);
    center_geom(WORLDGEOM, UNIVERSE);
    gv_camera_reset(ALLCAMS);
    gv_ui_center_origin(ORIGIN_KEYWORD);
  } else if (TYPEOF(id) == T_CAM) {
    gv_camera_reset(id);
  } else if (TYPEOF(id) == T_GEOM) {
    center_geom(id, id==WORLDGEOM ? UNIVERSE : WORLDGEOM);
  }
}


LDEFINE(xform_set, LVOID,
	"(xform-set ID TRANSFORM)\n"
	"Overwrite the current object transform with TRANSFORM (set "
	"object ID's transform to TRANSFORM).")
{
  DObject *obj;
  int index;
  TransformStruct *ts;
  int id;

  LDECLARE(("xform-set", LBEGIN,
	    LID, &id,
	    LTRANSFORM, &ts,
	    LEND));

  MAYBE_LOOP_ALL(id, index, T_NONE, DObject, obj) {
    if (id == ALLGEOMS && DGobj(obj)->citizenship == ALIEN)
      continue;
    if (ts->tm == NULL) {
      /*
       * Special case for switching externally-controlled motion on & off
       */
      if(ISGEOM(id))
	GeomSet(obj->Item, CR_AXISHANDLE, ts->h, CR_END);
      else
	CamSet(DVobj(obj)->cam, CAM_C2WHANDLE, ts->h, CAM_END);
    } else {
      if (ISGEOM(id))
	GeomSet(DGobj(obj)->Item,
		CR_AXIS, ts->tm, CR_AXISHANDLE, ts->h, CR_END);
      else 
	CamSet(DVobj(obj)->cam,
	       CAM_C2W, ts->tm, CAM_C2WHANDLE, ts->h, CAM_END);
    }
    TmIdentity(obj->Incr);
    obj->redraw = true;
    if (ts->h) {
      obj->changed |= CH_TRANSFORM;
    }
    obj->moving = (obj->updateproc != NULL);
  }
  return Lt;
}

LDEFINE(xform, LVOID,
	"(xform ID TRANSFORM)\n"
	"Apply TRANSFORM to object ID, as opposed to simply setting its "
	"transform, so the effective position "
	"of the object will be the concatenation of TRANSFORM with the "
	"current object transform.")
{
  int id;
  TransformStruct *ts;
  DObject *obj;
  int index;

  LDECLARE(("xform", LBEGIN,
	    LID, &id,
	    LTRANSFORM, &ts,
	    LEND));

  MAYBE_LOOP_ALL(id, index, T_NONE, DObject, obj) {
    if (id == ALLGEOMS && DGobj(obj)->citizenship==ALIEN) {
      continue;
    }
    if (TYPEOF(id) == T_GEOM) {
      GeomTransform(DGobj(obj)->Item, ts->tm, NULL);
    } else {
      CamTransform(DVobj(obj)->cam, ts->tm);
    }
    TmIdentity(obj->Incr);
    obj->redraw = true;
    obj->moving = (obj->updateproc!=NULL);
  }
  return Lt;
}

LDEFINE(xform_incr, LVOID,
	"(xform-incr     ID TRANSFORM)\n"
	"Apply continual motion: concatenate TRANSFORM with the current "
	"transform of the object every refresh (set object ID's "
	"incremental transform to TRANSFORM).")
{
  DObject *obj;
  int index;
  TransformStruct *ts;
  int id;
  LDECLARE(("xform-incr", LBEGIN,
	    LID, &id,
	    LTRANSFORM, &ts,
	    LEND));

  MAYBE_LOOP_ALL(id, index, T_NONE, DObject, obj) {
    stop_motions(obj->id);
    if (id == ALLGEOMS && DGobj(obj)->citizenship == ALIEN) {
      continue;
    }
    if(obj->incrhandle != NULL) {
      HandlePDelete(&obj->incrhandle);
    }
    if((obj->incrhandle = ts->h) != NULL) {
      HandleRegister(&obj->incrhandle, NULL, obj->Incr, TransUpdate);
      object_register(&obj->incrhandle, NULL, obj);
    } else {
      TmCopy(ts->tm, obj->Incr);
    }
    obj->moving = ( nonidentity(obj->Incr) || (obj->updateproc!=NULL) );
  }
  return Lt;
}

LDEFINE(new_camera, LID,
	"(new-camera     name [CAMERA])\n"
	"Create a new camera with the given name (a string).  If a "
	"camera with that name already exists, the new object is given "
	"a unique name.  If CAMERA is omitted a default camera is used.")
{
  DView *dv;
  int id;
  CameraStruct *cs=NULL;
  char *name;
  LDECLARE(("new-camera", LBEGIN,
	    LSTRING, &name,
	    LOPTIONAL,
	    LCAMERA, &cs,
	    LEND));

  dv = new_dview();
  if (!name) name = "Camera";
  name_object(dv, 1, name );
  mgctxset(MG_WnSet, WN_NAME, dv->name[1], WN_END, MG_END);

  ui_windowWillOpen(dv);
  mgctxset(MG_SHOW, 1, MG_END);
  ui_windowDidOpen(dv);

  ui_objectchange();
  id=gv_camera(dv->name[0], cs);
  camera_space_relabel(id);
  return LNew( LID, &id );
}

LDEFINE(camera, LINT,
	"(camera CAM-ID [CAMERA])\n"
	"Specify data for CAM-ID; CAMERA is a string giving an OOGL "
	"camera specification.  If no camera CAM-ID exists, "
	"it is created; in this case, the second argument is optional, "
	"and if omitted, a default camera is used.  See also: new-camera.")
{
  DView *dv;
  int index;
  CameraStruct *cs=NULL;
  char *name;
  int id;
  LDECLARE(("camera", LBEGIN,
	    LSTRING, &name,
	    LOPTIONAL,
	    LCAMERA, &cs,
	    LEND));
  if((id = drawer_idbyname(name)) == NOID) {
    id=gv_new_camera( name, cs );
  } else {
    MAYBE_LOOP(id, index, T_CAM, DView, dv) {
      /* if cam is NULL stick with whatever camera id already has */
      if (cs->cam) {
	RefIncr((Ref*)cs->cam);
	RefIncr((Ref*)cs->h);
	CamDelete(dv->cam);
	HandlePDelete(&dv->camhandle);
	dv->cam = cs->cam;
	dv->camhandle = cs->h;
	if(dv->camhandle) {
	  HandleRegister(&dv->camhandle, (Ref *)dv, &dv->cam, HandleUpdRef);
	  object_register(&dv->camhandle, NULL, (DObject *)dv);
	}
      }
      if(dv->cam == NULL) {
	dv->cam = CamCopy(drawerstate.defview.cam, NULL);
      }
      TmIdentity(dv->Incr);
      dv->moving = (dv->updateproc!=NULL);
      dv->newcam = true;
    }
    /*  gv_camera_reset(id); */
  }
  return LNew( LINT, &id );
}

LDEFINE(camera_reset, LVOID,
	"(camera-reset CAM-ID)\n"
	"Reset CAM-ID to its default value.")
{
  DView *dv;
  int index;
  int id;
  LDECLARE(("camera-reset", LBEGIN,
	    LID, &id,
	    LEND));

  MAYBE_LOOP(id, index, T_CAM, DView, dv) {
    CamReset(dv->cam);
    drawer_stop(dv->id);
    if ((spaceof(WORLDGEOM) == TM_SPHERICAL) && (dv->hmodel == CONFORMALBALL)) {
      CamTranslate( dv->cam, 0.0, 0.0, 10.0 );
    }

    if (drawerstate.NDim > 0) {
      /* Note: the following resets the *entire* N-D xform to the
       * identity, not just that in the subspace we're looking
       * through.  Might want to have finer control.  XXX - 7/28/93,
       * slevy & holt
       */
      drawer_set_ND_xform(dv->id, NULL);
    }
    TmIdentity(dv->Incr);
    dv->moving = (dv->updateproc!=NULL);
    dv->redraw = true;
  }
  ui_maybe_refresh(id);
  return Lt;
}

static int
drawer_new_dgeom(char *name, GeomStruct *gs, enum citizenship citizenship)
{
  int id;
  DGeom *dg;

  dg = new_dgeom( NULL, citizenship );  
  if (name)
    name_object(dg, 1, name);
  ui_objectchange();
  id = drawer_dgeom(dg->id, gs);
  if (citizenship==ALIEN) {
    drawer_int( id, DRAWER_NORMALIZATION, NONE );
    drawer_int( id, DRAWER_BBOXDRAW, 0 );
  }
  GeomDice(dg->Item, dg->bezdice, dg->bezdice);
  return id;
}

LDEFINE(new_alien, LID,
	"(new-alien      name [GEOMETRY])\n"
	"Create a new alien (geom not in the world) with the given name "
	"(a string).  GEOMETRY is a string giving an OOGL geometry "
	"specification.  If GEOMETRY is omitted, the new alien "
	"is given an empty geometry.  If an object with that name "
	"already exists, the new alien is given a unique name.  The "
	"light beams that are used to move around the lights are an "
	"example of aliens. They're drawn but are not controllable the "
	"way ordinary objects are: they don't appear in the object "
	"browser and the user can't move them with the normal motion "
	"modes.")
{
  GeomStruct *gs = &nullgs;
  char *name;
  int id;

  LDECLARE(("new-alien", LBEGIN,
	    LSTRING, &name,
	    LOPTIONAL,
	    LGEOM, &gs,
	    LEND));
  if (gs->geom) RefIncr((Ref*)(gs->geom));
  if (gs->h) RefIncr((Ref*)(gs->h));
  id=drawer_new_dgeom(name, gs, ALIEN);
  return LNew( LID, &id );
}

LDEFINE(new_geometry, LID,
	"(new-geometry   name [GEOMETRY])\n\
	Create a new geom with the given name (a string).  GEOMETRY is\n\
	a string giving an OOGL geometry specification.  If\n\
	GEOMETRY is omitted, the new object is given an empty geometry.\n\
	If an object with that name already exists, the new object is\n\
	given a unique name.")
{
  int id;
  GeomStruct *gs = &nullgs;
  char *name;
  LDECLARE(("new-geometry", LBEGIN,
	    LSTRING, &name,
	    LOPTIONAL,
	    LGEOM, &gs,
	    LEND));
  if (gs->geom) RefIncr((Ref*)(gs->geom));
  if (gs->h) RefIncr((Ref*)(gs->h));
  id = drawer_new_dgeom(name, gs, ORDINARY);
  return LNew( LID, &id );
}

static int
drawer_dgeom(int id, GeomStruct *gs)
{
  DGeom *dg;
  int index;

  if (id == WORLDGEOM) {
    OOGLError(1,"drawer_dgeom: Mere mortals cannot change the World Geom!");
  } else {
    MAYBE_LOOP(id, index, T_GEOM, DGeom, dg) {  
      GeomSet(dg->Lgeom, CR_HANDLE_GEOM, gs->h, gs->geom, CR_END);
      dg->changed = CH_GEOMETRY;

      /* (re-)generate the BSP-tree for this object
       *
       * We could also hang the BSP-tree to dg->Item, but that would
       * require special handling of BSP-trees in InstTransform[To]().
       */
      GeomBSPTree(dg->Lgeom, NULL, BSPTREE_CREATE);
    }
  }

  /* Caller expects us to consume the objects */
  GeomDelete(gs->geom);
  HandleDelete(gs->h);

  return id;
}

LDEFINE(geometry, LVOID,
	"(geometry       GEOM-ID [GEOMETRY])\n\
	Specify the geometry for GEOM-ID.  GEOMETRY is a string\n\
	giving an OOGL geometry specification.  If no object\n\
	called GEOM-ID exists, it is created; in this case the\n\
	GEOMETRY argument is optional, and if omitted, the new\n\
	object GEOM-ID is given an empty geometry.")
{
  char *name;
  GeomStruct *gs = &nullgs;
  int id;

  LDECLARE(("geometry", LBEGIN,
	    LSTRING, &name,
	    LOPTIONAL,
	    LGEOM, &gs,
	    LEND));
  /* we RefIncr() because gv_geometry() (& gv_new_geometry())
     expect to own the geom and handle passed in, but the lisp
     interpreter actually owns these

     cH: this is wrong, only drawer[_new]_dgeom() expects to own the
     object. As gv_new_geometry() increases the ref-count itself we
     must not do it here.
  */
  if((id = drawer_idbyname(name)) == NOID) {
    gv_new_geometry( name, gs );
  } else {
    if (gs->geom) RefIncr((Ref*)(gs->geom));
    if (gs->h) RefIncr((Ref*)(gs->h));
    drawer_dgeom(id, gs);
  }
  return Lt;
}


static int
drawer_replace_geometry(int id, int *p, int pn, GeomStruct *gs)
{
  DObject *obj;
  int count;
  int ind;
  Geom *where;

  MAYBE_LOOP(id, ind, T_GEOM, DObject, obj) {
    for (GeomGet(DGobj(obj)->Lgeom,
		 CR_GEOM, &where); where && pn>0; pn--, p++) {
      for (count = 0; count < *p; count++) {
	GeomGet(where, CR_CDR, &where);
      }
      if (pn > 1) {
	GeomGet(where, CR_GEOM, &where);
      }
    }
    if (where) {
      GeomCCreate(where,
		  ListMethods(), CR_HANDLE_GEOM, gs->h, gs->geom, CR_END);
      obj->changed = CH_GEOMETRY;
    }
  }
  return id;
}

LDEFINE(replace_geometry, LVOID,
	"(replace-geometry GEOM-ID PART-SPECIFICATION GEOMETRY)\n\
	Replace a part of the geometry for GEOM-ID.")
{
  char *name;
  int p[40];
  int pn = 40;
  GeomStruct *gs=NULL;
  int id;

  LDECLARE(("replace-geometry", LBEGIN,
	    LSTRING, &name,
	    LHOLD, LARRAY, LINT, p, &pn,
	    LGEOM, &gs,
	    LEND));
  if ((id = drawer_idbyname(name)) != NOID)
    drawer_replace_geometry(id, p, pn, gs);
  return Lt;
}
  
    
LDEFINE(copy, LVOID,
	"(copy [ID] [name])\n\
	Copies an object or camera.  If ID is not specified, it \n\
	is assumed to be targetgeom.  If name is not specified, it \n\
	is assumed to be the same as the name of ID.")
{
  DObject *obj;
  int id = TARGETGEOMID, newid=0;
  int i;
  char *name = NULL;
  GeomStruct geomstruct;
  CameraStruct camstruct;
  TransformStruct transformstruct;
  ApStruct apstruct;
  /* If the user copies the world, this will crash unless we remember not
   * to copy things that are copies of something else.  That is what this 
   * array is for. */
  VARARRAY(iscopy, char, 2*dgeom_max);

  LDECLARE(("copy", LBEGIN,
	    LOPTIONAL,
	    LID, &id,
	    LSTRING, &name,
	    LEND));

  id = real_id(id);
  geomstruct.h = camstruct.h = transformstruct.h = apstruct.h = NULL;

  memset(iscopy, 0, 2*dgeom_max * sizeof(char));

  MAYBE_LOOP(id, i, T_NONE, DObject, obj) {
    switch(TYPEOF(obj->id)) {
    case T_GEOM:
      /* Copying the world = copying each thing in the world */
      if (((DGeom *)obj)->citizenship == THEWORLD || iscopy[i]) break;
      GeomGet(((DGeom *)obj)->Lgeom, CR_GEOM, &geomstruct.geom);
      if (((DGeom *)obj)->citizenship == ALIEN)
	newid = gv_new_alien(name == NULL ? obj->name[1] : name, &geomstruct); 
      else 
	newid = gv_new_geometry(name == NULL ? obj->name[1] : name, 
				&geomstruct); 
      apstruct.ap = GeomAppearance(obj->Item);
      gv_merge_ap(newid, &apstruct);
      
      GeomGet(((DGeom *)obj)->Item, CR_AXIS, transformstruct.tm);
      iscopy[INDEXOF(newid)] = 1;
      break;
    case T_CAM:
      camstruct.cam = ((DView *)obj)->cam;
      newid = gv_new_camera(name == NULL ? obj->name[1] : name,
			    &camstruct);
      drawer_get_transform(obj->id, transformstruct.tm, WORLDGEOM);
      break;
    }
    gv_xform_set(newid, &transformstruct);
  }

  return Lt;
}

LDEFINE(delete, LVOID,
	"(delete ID)\n\
	Delete object or camera ID.")
{
  DObject *obj;
  int index;
  int id;
  unsigned int wasfrozen = UNFROZEN;

  LDECLARE(("delete", LBEGIN,
	    LID, &id,
	    LEND));

  /* Real-ize the given id.  In case id == TARGETGEOMID, remember we
   * may change the target geom during deletion.  Don't let this change
   * what's being deleted!
   */
  id = real_id(id);
  wasfrozen = uistate.freeze;
  gv_ui_freeze(1);
  MAYBE_LOOP(id, index, T_NONE, DObject, obj) {
    int obj_id = obj->id; /* remember, must not access obj->id after
			   * obj has gone.
			   */
    if(obj_id == WORLDGEOM || obj_id == DEFAULTCAMID)
      continue;
    if (obj->name[0])
      fsa_install( name_fsa, obj->name[0], NOID );
    if (obj->name[1])
      fsa_install( name_fsa, obj->name[1], NOID );

    HandleUnregisterAll((Ref *)obj,
			(void *)(long)(int)obj->seqno, object_changed);
    stop_motions(obj_id);
    switch(TYPEOF(obj_id)) {
    case T_GEOM:
      delete_geometry((DGeom *)obj); break;
    case T_CAM:
      delete_camera((DView *)obj); break;
    }
    if (uistate.centerid==obj_id)
      gv_ui_center(TARGETID);
  }
  gv_ui_freeze(wasfrozen);
  return Lt;
}

LDEFINE(scene, LVOID,
	"(scene          CAM-ID [GEOMETRY])\n\
	Make CAM-ID look at GEOMETRY instead of at the universe.")
{
  DView *dv;
  NDcam *cluster;
  int index;
  GeomStruct *gs = &nullgs;
  int id;

  LDECLARE(("scene", LBEGIN,
	    LID, &id,
	    LOPTIONAL,
	    LGEOM, &gs,
	    LEND));

  if (drawerstate.NDim > 0 && ISCAM(id) &&
      id != ALLCAMS && (cluster = ((DView *)drawer_get_object(id))->cluster)) {
    MAYBE_LOOP(ALLCAMS, index, T_CAM, DView, dv) {
      if (dv->cluster == cluster) {
	RefIncr((Ref *)gs->geom);
	GeomDelete(dv->Item);
	dv->Item = gs->geom;
	dv->changed = CH_GEOMETRY;
      }
    }
    return Lt;
  }

  MAYBE_LOOP(id, index, T_CAM, DView, dv) {
    RefIncr((Ref *)gs->geom);
    GeomDelete(dv->Item);
    dv->Item = gs->geom;
    dv->changed = CH_GEOMETRY;
  }

  return Lt;
}

LDEFINE(winenter, LVOID,
	"(winenter       CAM-ID)\n\
	Tell geomview that the mouse cursor is in the window\n\
	of CAM-ID.  This function is for development purposes\n\
	and is not intended for general use.")
{
  int id;
  LDECLARE(("winenter", LBEGIN,
	    LID, &id,
	    LEND));
  winmoved(id);
  ui_mousefocus(INDEXOF(id));
  return Lt;
}

LDEFINE(merge_ap, LVOID,
	"(merge-ap       GEOM-ID APPEARANCE)\n\
	Merge in some appearance characteristics to GEOM-ID.\n\
	Appearance parameters include surface and line color, shading\n\
	style, line width, and lighting.")
{
  DObject *obj;
  int index;
  ApStruct *as;
  int id;
  LDECLARE(("merge-ap", LBEGIN,
	    LID, &id,
	    LAP, &as,
	    LEND));

  if (id == WORLDGEOM)
    worldap = ApMerge(as->ap, worldap, APF_INPLACE|APF_OVEROVERRIDE);
  MAYBE_LOOP(id, index, T_NONE, DObject, obj) {  
    Appearance *thisap = GeomAppearance(obj->Item);
    GeomSet(obj->Item, CR_NOCOPY, CR_APPEAR,
	    ApMerge(as->ap, thisap, APF_INPLACE|APF_OVEROVERRIDE), CR_END);
    obj->redraw = true;
  }
  ui_maybe_refresh(id);
  return Lt;
}

void
drawer_set_ap(int id, Handle *h, Appearance *ap)
{
  DObject *obj;
  int index;

  if(id == WORLDGEOM || id == ALLGEOMS) {
    ApDelete(worldap);
    worldap = ap;
  }
  MAYBE_LOOP(id, index, T_NONE, DObject, obj) {
    GeomSet(obj->Item, CR_NOCOPY, /* CR_APHANDLE, h */
	    CR_APPEAR, ap ? ApCopy(ap,NULL) : NULL, CR_END );
    obj->redraw = true;
  }
  ui_maybe_refresh(id);
}

LDEFINE(dice, LVOID,
	"(dice           GEOM-ID N)\n\
	Dice any Bezier patches within GEOM-ID into NxN meshes; default 10.")
{
  int id, dice;
  LDECLARE(("dice", LBEGIN,
	    LID, &id,
	    LINT, &dice,
	    LEND));
  drawer_int( id, DRAWER_BEZDICE, dice );
  return Lt;
}

LDEFINE(bbox_draw, LVOID,
	"(bbox-draw      GEOM-ID [yes|no])\n\
	Say whether GEOM-ID's bounding-box should be drawn; \"yes\" if omitted.")
{
  int id;
  Keyword yes = YES_KEYWORD;

  LDECLARE(("bbox-draw", LBEGIN,
	    LID, &id,
	    LOPTIONAL,
	    LKEYWORD, &yes,
	    LEND));
  drawer_int( id, DRAWER_BBOXDRAW, boolval("bbox-draw", yes) );
  return Lt;
}

LDEFINE(camera_draw, LVOID,
	"(camera-draw    CAM-ID [yes|no])\n\
	Say whether or not cameras should be drawn in CAM-ID; \"yes\" if omitted.")
{
  int id;
  Keyword yes = YES_KEYWORD;

  LDECLARE(("camera-draw", LBEGIN,
	    LID, &id,
	    LOPTIONAL,
	    LKEYWORD, &yes,
	    LEND));
  drawer_int( id, DRAWER_CAMERADRAW, boolval("camera-draw", yes) );
  return Lt;
}

LDEFINE(evert, LVOID,
	"(evert          GEOM-ID [yes|no])\n\
	Set the normal eversion state of GEOM-ID.  If the second argument\n\
	is omitted, toggle the eversion state.")
{
  int id, val = -1;
  LDECLARE(("evert", LBEGIN,
	    LID, &id,
	    LOPTIONAL,
	    LKEYWORD, &val,
	    LEND));
  drawer_int( id, DRAWER_EVERT, val );
  return Lt;
}

LDEFINE(soft_shader, LVOID,
	"(soft-shader  CAM-ID  {on|off|toggle})\n\
	Select whether to use software or hardware shading in that camera.")
{
  DView *dv;
  int id, i, on = -1;

  LDECLARE(("soft-shader", LBEGIN,
	    LID, &id,
	    LOPTIONAL,
	    LKEYWORD, &on,
	    LEND));

  MAYBE_LOOP(id, i, T_CAM, DView, dv) {
    if(on == TOGGLE_KEYWORD)
      on = dv->shader ? OFF_KEYWORD : ON_KEYWORD;
    dv->shader = (on == ON_KEYWORD) ? softshader(dv->id) : NULL;
    if(dv->mgctx) {
      mgctxselect(dv->mgctx);
      mgctxset(MG_SHADER, dv->shader, MG_END);
      gv_redraw(dv->id);
    }
    ui_maybe_refresh(dv->id);
  }
  return Lt;
}



LDEFINE(stereowin, LLIST,
	"(stereowin	CAM-ID  [no|horizontal|vertical|colored] [gapsize])\n\
	Configure CAM-ID as a stereo window.\n\
	no: entire window is a single pane, stereo disabled\n\
	horizontal: split left/right: left is stereo eye#0, right is #1.\n\
	vertical: split top/bottom: bottom is eye#0, top is #1.\n\
	colored: panes overlap, red is stereo eye#0, cyan is #1.\n\
	A gap of \"gapsize\" pixels is left between subwindows;\n\
	if omitted, subwindows are adjacent.\n\
	If both layout and gapsize are omitted, e.g. (stereowin CAM-ID),\n\
	returns current settings as a ``(stereowin ...)'' command list.\n\
	This command doesn't set stereo projection; use ``merge camera'' or\n\
	``camera'' to set the stereyes transforms, and ``merge window'' or\n\
	``window'' to set the pixel aspect ratio & window position if needed.")
{
  int id;
  Keyword kw = NOT_A_KEYWORD;
  DView *dv;
  int gapsize = 0;

  LDECLARE(("stereowin", LBEGIN,
	    LID, &id,
	    LOPTIONAL,
	    LKEYWORD, &kw,
	    LINT, &gapsize,
	    LEND));

  if(!ISCAM(id) || (dv = (DView *)drawer_get_object(id)) == NULL) {
    OOGLError(0, "stereowin: expected camera, got %s", drawer_id2name(id));
    return Lnil;
  }
  if(kw == NOT_A_KEYWORD) {
    return LLISTTOOBJ
      ( LListAppend
	( LListAppend
	  ( LListAppend
	    ( LListAppend(NULL, LSYMBOLTOOBJ("stereowin")),
	      LTOOBJ(LID)(&id)),
	    LTOOBJ(LKEYWORD)(&dv->stereo)),
	  LTOOBJ(LINT)(&dv->stereogap)));
  }
  if(kw != NO_KEYWORD && kw != HORIZONTAL_KEYWORD && kw != VERTICAL_KEYWORD
     && kw != COLORED_KEYWORD) {
    OOGLError(0, "stereowin: expected "
	      "\"no\" or "
	      "\"horizontal\" or "
	      "\"vertical\" or "
	      "\"colored\", not \"%s\"", keywordname(kw));
    return Lnil;
  }
  dv->stereo = kw;
  dv->stereogap = gapsize;
  gv_redraw(id);		/* Schedule redraw/window reconfiguration */
  return Lt;
}
	

LDEFINE(space, LVOID,
	"(space {euclidean|hyperbolic|spherical})\n\
	Set the space associated with the world.")
{
  int space_keyword, space, id, index;
  DView *dv;

  LDECLARE(("space", LBEGIN,
	    LKEYWORD, &space_keyword,
	    LEND));

  space = spaceval("space", space_keyword);
  drawerstate.space = space;

  switch (space) {
  default:
  case TM_EUCLIDEAN:
    break;
  case TM_HYPERBOLIC:
  case TM_SPHERICAL:
    drawer_int( ALLGEOMS, DRAWER_BBOXDRAW, 0 );
    drawer_int( ALLGEOMS, DRAWER_NORMALIZATION, NONE );
    break;
  }

  id = ALLCAMS;
  MAYBE_LOOP(id, index, T_CAM, DView, dv) {
    int mgspace = space;

#if 0
    if (space == TM_HYPERBOLIC || space == TM_SPHERICAL) {
      switch (dv->hmodel) {
      default:
      case VIRTUAL: mgspace |= TM_VIRTUAL; break;
      case PROJECTIVE: mgspace |= TM_PROJECTIVE; break;
      case CONFORMALBALL: mgspace |= TM_CONFORMAL_BALL; break;
      }
    }
#endif

    mgspace |= TM_VIRTUAL;
    mgctxselect(dv->mgctx);
    mgctxset(MG_SPACE, mgspace, MG_END);

    gv_hmodel(dv->id, VIRTUAL_KEYWORD);

    if(dv->shader)
      mgctxset(MG_SHADER, dv->shader = softshader(dv->id), MG_END);

  }

  gv_xform_set(ALLGEOMS, &ts_identity);
  gv_xform_set(WORLDGEOM, &ts_identity);
  drawer_stop(NOID);

#if 0
  /* why are these here ????  mbp Wed Jun  9 14:21:34 1993 */
  filepanel = ui_name2panel("Files");
  if(ui_panelshown(filepanel)) ui_showpanel(filepanel, 1);	/* Reselect */
#endif

  return Lt;
}

LDEFINE(hmodel, LVOID,
	"(hmodel CAMID {virtual|projective|conformal})\n\
	Set the model used to display geometry in\n\
	this camera; see also \"space\".")
{
  int id, index, model_keyword;
  DView *dv;
  
  LDECLARE(("hmodel", LBEGIN,
	    LID, &id,
	    LKEYWORD, &model_keyword,
	    LEND));
  
  MAYBE_LOOP(id, index, T_CAM, DView, dv) {
    int mgspace;
    mgctxselect(dv->mgctx);
    mgctxget(MG_SPACE, &mgspace);
    mgspace = TM_SPACE(mgspace);
    dv->hmodel = hmodelval("hmodel", model_keyword);
    dview_set_hmodel_camspace(dv,mgspace);
    /*    if (mgspace == TM_HYPERBOLIC) */
    switch (dv->hmodel) {
    default:
    case VIRTUAL:
      gv_hsphere_draw(id, 0);
      break;
    case PROJECTIVE:
    case CONFORMALBALL:
      gv_hsphere_draw(id, 1);
      break;
    }
    switch (dv->hmodel) {
    default:
    case VIRTUAL: mgspace |= TM_VIRTUAL; break;
    case PROJECTIVE: mgspace |= TM_PROJECTIVE; break;
    case CONFORMALBALL: mgspace |= TM_CONFORMAL_BALL;


      break;
      
      break;
    }
    mgctxset(MG_SPACE, mgspace, MG_END);
    gv_camera_reset(id);
    camera_space_relabel(id);
  }
  return Lt;
}

LDEFINE(inhibit_warning, LVOID,
	"(inhibit-warning STRING)\n\
	Inhibit warning inhbits geomview from displaying a\n\
	particular warning message determined by STRING.\n\
	At present there are no warning messages that this\n\
	applies to, so this command is rather useless.")
{
  char *s=NULL;
  
  LDECLARE(("inhibit-warning", LBEGIN,
	    LSTRING, &s,
	    LEND));
  
  return Lt;
}

static void
dview_set_hmodel_camspace(DView *dv, int space)
{
  int camspace;
  
  switch (dv->hmodel) {
  default:
  case VIRTUAL:
    camspace = space;
    break;
  case PROJECTIVE:
  case CONFORMALBALL:
    camspace = TM_EUCLIDEAN;
    break;
  }
  CamSet(dv->cam, CAM_SPACE, camspace, CAM_END);
}


LDEFINE(hsphere_draw, LVOID,
	"(hsphere-draw   CAMID [yes|no])\n\
	Say whether to draw a unit sphere: the sphere at infinity in\n\
	hyperbolic space, and a reference sphere in Euclidean and spherical\n\
	spaces.  If the second argument is omitted, \"yes\" is assumed.")
{
  int id;
  Keyword yes = YES_KEYWORD;

  LDECLARE(("hsphere-draw", LBEGIN,
	    LID, &id,
	    LOPTIONAL,
	    LKEYWORD, &yes,
	    LEND));
  drawer_int( id, DRAWER_HSPHERE, boolval("hsphere-draw", yes) );
  return Lt;
}

LDEFINE(pickable, LVOID,
	"(pickable       GEOM-ID {yes|no})\n\
	Say whether or not GEOM-ID is included in the pool of objects\n\
	that could be returned from the pick command.")
{
  int id;
  Keyword yes = YES_KEYWORD;

  LDECLARE(("pickable", LBEGIN,
	    LID, &id,
	    LOPTIONAL,
	    LKEYWORD, &yes,
	    LEND));
  drawer_int( id, DRAWER_PICKABLE, boolval("pickable", yes) );
  return Lt;
}

LDEFINE(normalization, LVOID,
	"(normalization  GEOM-ID {each|none|all|keep})\n\
	Set the normalization status of GEOM-ID.\n\
	\"none\" suppresses all normalization.\n\
	\"each\" normalizes the object's bounding box to fit into the unit\n\
	   sphere, with the center of its bounding box translated\n\
	   to the origin.  The box is scaled such that its long diagonal,\n\
	   sqrt((xmax-xmin)^2 + (ymax-ymin)^2 + (zmax-zmin)^2), is 2.\n\
	\"all\" resembles \"each\", except when an object is changing\n\
	   (e.g. when its geometry is being changed by an external program).\n\
\n\
	   Then, \"each\" tightly fits the bounding box around the\n\
	   object whenever it changes and normalizes accordingly,\n\
	   while \"all\" normalizes the union of all variants of the object\n\
	   and normalizes accordingly.\n\
\n\
	\"keep\" leaves the current normalization transform unchanged\n\
	when the object changes.  It may be useful to apply \"each\" or\n\
	\"all\" normalization apply to the first version of a changing\n\
	object to bring it in view, then switch to \"keep\"")
{
  int id, val;
  LDECLARE(("normalization", LBEGIN,
	    LID, &id,
	    LKEYWORD, &val,
	    LEND));
  val = normalval("normalization", val);

  drawer_int( id, DRAWER_NORMALIZATION, val );
  return Lt;
}

void
drawer_int(int id, DrawerKeyword key, int ival)
{
  DObject *obj;
  int index;
  int flag, override;
  ApStruct as;

  as.h = NULL;

  switch (key) {
  case DRAWER_FACEDRAW: flag = override = APF_FACEDRAW; goto setflag;
  case DRAWER_EDGEDRAW: flag = override = APF_EDGEDRAW; goto setflag;
  case DRAWER_VECTDRAW: flag = override = APF_VECTDRAW; goto setflag;
  case DRAWER_NORMALDRAW: flag = override = APF_NORMALDRAW; goto setflag;
  case DRAWER_TEXTUREDRAW: flag = override = APF_TEXTURE; goto setflag;
  case DRAWER_TEXTUREQUAL: flag = override = APF_TXMIPMAP|APF_TXMIPINTERP|APF_TXLINEAR; goto setflag;
  case DRAWER_BACKCULL: flag = override = APF_BACKCULL; goto setflag;
  case DRAWER_EVERT: flag = override = APF_EVERT; goto setflag;
  case DRAWER_SHADELINES: flag = override = APF_SHADELINES; goto setflag;
  case DRAWER_CONCAVE: flag = override = APF_CONCAVE; goto setflag;
  setflag:
    as.ap = ApCreate(ival ? AP_DO : AP_DONT, flag,
		     AP_OVERRIDE, override & uistate.apoverride, AP_END);
    goto mergeap;

  case DRAWER_TRANSLUCENCY:
    flag = override = APF_TRANSP;
    as.ap = ApCreate(ival < 0 ? AP_DONT : AP_DO, flag,
		     AP_TRANSLUCENCY, abs(ival) - 1,
		     AP_OVERRIDE, APF_TRANSP & uistate.apoverride, AP_END);
    goto mergeap;

  case DRAWER_SHADING:
    as.ap = ApCreate(AP_SHADING, ival,
		     AP_OVERRIDE, APF_SHADING & uistate.apoverride, AP_END);
    goto mergeap;

  case DRAWER_LINEWIDTH:
    as.ap = ApCreate(AP_LINEWIDTH, ival,
		     AP_OVERRIDE, APF_LINEWIDTH & uistate.apoverride, AP_END);
    goto mergeap;

  case DRAWER_BEZDICE:
    as.ap = ApCreate(AP_DICE, ival, ival,
		     AP_OVERRIDE, APF_DICE & uistate.apoverride, AP_END);
    gv_merge_ap(id, &as);
    ApDelete(as.ap);
#if 0
    MAYBE_LOOP(id, index, T_NONE, DObject, obj) {
      DGobj(obj)->bezdice = ival;
      GeomDice(DGobj(obj)->Item, ival, ival);
      obj->changed = CH_GEOMETRY;
    }
#endif
    break;

  mergeap:
    gv_merge_ap(id, &as);
    ApDelete(as.ap);
    return;		/* No need to ui_maybe_refresh(), merge_ap does it. */

  case DRAWER_PROJECTION:
    MAYBE_LOOP(id, index, T_CAM, DObject, obj) {
      CamSet(DVobj(obj)->cam, CAM_PERSPECTIVE, ival, CAM_END);
      obj->changed = CH_GEOMETRY;
    }
    break;

  case DRAWER_BBOXDRAW:
    MAYBE_LOOP(id, index, T_GEOM, DObject, obj) {
      DGobj(obj)->bboxdraw = ival;
      obj->redraw = true;
    }
    break;
    

  case DRAWER_NORMALIZATION:
    MAYBE_LOOP(id, index, T_GEOM, DObject, obj) {
      normalize(DGobj(obj), ival);
    }
    break;

  case DRAWER_PICKABLE:
    MAYBE_LOOP(id, index, T_GEOM, DObject, obj) {
      DGobj(obj)->pickable = ival != 0;
    }
    break;

  case DRAWER_HSPHERE:
    MAYBE_LOOP(id, index, T_CAM, DObject, obj) {
      set_hsphere_draw( id, ival );
      obj->redraw = true;
    }
    break;

  case DRAWER_CAMERADRAW:
    MAYBE_LOOP(id, index, T_CAM, DObject, obj) {
      DVobj(obj)->cameradraw = ival != 0;
      obj->redraw = true;
    }
    break;

  case DRAWER_DOUBLEBUFFER:
    MAYBE_LOOP(id, index, T_CAM, DObject, obj) {
      int opts;
      if(DVobj(obj)->mgctx == NULL) continue;
      mgctxselect(DVobj(obj)->mgctx);
      mgctxget(MG_SETOPTIONS, &opts);
      mgctxset(ival==0 || (ival<0 && opts&MGO_DOUBLEBUFFER) ?
	       MG_UNSETOPTIONS : MG_SETOPTIONS,
	       MGO_DOUBLEBUFFER, MG_END);
      obj->redraw = true;
    }
    break;

  case DRAWER_APOVERRIDE:
    /* Select whether appearance controls use the 'override' feature
     * to supplant user settings.  Might be nice if this were settable
     * per feature rather than across the board.
     */
    uistate.apoverride = ival ? ~0 : 0;
    MAYBE_LOOP(id, index, T_GEOM, DObject, obj) {
      ApUseOverrides( GeomAppearance(obj->Item), uistate.apoverride );
      if(obj->id == WORLDGEOM) {
	ApUseOverrides( worldap, uistate.apoverride );
      }
      if(obj->id != WORLDGEOM && !uistate.apoverride) {
	/* Allow pre-existing World settings to win over those in children */
	ApLetPropagate( worldap, GeomAppearance(obj->Item) );
      }
      obj->redraw = true;
    }
    break;

  case DRAWER_INERTIA:
    uistate.inertia = ival<0 ? !uistate.inertia : ival;	break;
  case DRAWER_CONSTRAIN:
    uistate.constrained = ival<0 ? !uistate.constrained : ival; break;
  case DRAWER_OWNMOTION:
    uistate.ownmotion = ival<0 ? !uistate.ownmotion : ival;	break;

  default: ; /* This should never happen */

  }    
  ui_maybe_refresh(id);
}

LDEFINE(lines_closer, LVOID,
	"(lines-closer   CAM-ID DIST)\n\
	Draw lines (including edges) closer to the camera than polygons\n\
	by DIST / 10^5  of the Z-buffer range.  DIST = 3.0 by default.\n\
	If DIST is too small, a line lying on a surface may be\n\
	dotted or invisible, depending on the viewpoint.\n\
	If DIST is too large, lines may appear in front of surfaces\n\
	that they actually lie behind.  Good values for DIST vary with\n\
	the scene, viewpoint, and distance between near and far clipping\n\
	planes.  This feature is a kludge, but can be helpful.")
{
  float dist;
  int id;
  LDECLARE(("lines-closer", LBEGIN,
	    LID, &id,
	    LFLOAT, &dist,
	    LEND));
  drawer_float(id, DRAWER_LINE_ZNUDGE, dist);
  return Lt;
}

void
drawer_float(int id, DrawerKeyword key, float fval)
{
  DObject *obj;
  ApStruct as;
  int index;
  int attr, over;
  LInterest *interested;

  as.h = NULL;

  switch (key) {

  case DRAWER_NEAR: attr = CAM_NEAR; goto setcam;
  case DRAWER_FAR: attr = CAM_FAR; goto setcam;
  case DRAWER_FOV: attr = CAM_FOV; goto setcam;
  case DRAWER_FOCALLENGTH: attr = CAM_FOCUS; goto setcam;
  setcam:
    interested = LInterestList("merge");
    MAYBE_LOOP(id, index, T_CAM, DObject, obj) {
      DVobj(obj)->cam = CamSet(DVobj(obj)->cam, attr, fval, CAM_END);
      DVobj(obj)->redraw = true;
      if(interested)
	gv_merge(&CamOps, DVobj(obj)->id, (Ref *)DVobj(obj)->cam);
    }
    break;

  case DRAWER_KA: attr = MT_Ka; over = MTF_Ka; goto domt;
  case DRAWER_KD: attr = MT_Kd; over = MTF_Kd; goto domt;
  case DRAWER_KS: attr = MT_Ks; over = MTF_Ks; goto domt;
  case DRAWER_SHININESS: attr = MT_SHININESS; over = MTF_SHININESS; goto domt;
  case DRAWER_ALPHA: attr = MT_ALPHA; over = MTF_ALPHA; goto domt;
  domt:
    as.ap = ApCreate(AP_MtSet, attr, fval,
		     MT_OVERRIDE, over & uistate.apoverride,
		     MT_END, AP_END);
    goto mergeap;

  case DRAWER_NORMSCALE:
    as.ap = ApCreate(AP_NORMSCALE, fval,
		     AP_OVERRIDE, APF_NORMSCALE & uistate.apoverride, AP_END);
    goto mergeap;
  mergeap:
    gv_merge_ap(id, &as);
    ApDelete(as.ap);
    return;

  case DRAWER_LIGHT_INTENSITY:
    set_light_intensity( fval );
    drawerstate.changed = true;
    break;

  case DRAWER_LINE_ZNUDGE:
    MAYBE_LOOP(id, index, T_CAM, DObject, obj) {
      DVobj(obj)->lineznudge = fval;
      DVobj(obj)->redraw = true;
    }
    drawerstate.defview.lineznudge = fval;
    break;
  default: {
    /* report some kind of error? */
  }

  }
  ui_maybe_refresh(id);
}

LDEFINE(backcolor, LVOID,
	"(backcolor      CAM-ID R G B)\n\
	Set the background color of CAM-ID; R G B are numbers\n\
	between 0 and 1.")
{
  int id;
  Color color;
  LDECLARE(("backcolor", LBEGIN,
	    LID, &id,
	    LFLOAT, &(color.r),
	    LFLOAT, &(color.g),
	    LFLOAT, &(color.b),
	    LEND));
  drawer_color( id, DRAWER_BACKCOLOR, &color );
  return Lt;
}

LDEFINE(bbox_color, LVOID,
	"(bbox-color     GEOM-ID R G B)\n\
	Set the bounding-box color of GEOM-ID; R G B are numbers\n\
	between 0 and 1.")
{
  int id;
  Color color;
  LDECLARE(("bbox-color", LBEGIN,
	    LID, &id,
	    LFLOAT, &(color.r),
	    LFLOAT, &(color.g),
	    LFLOAT, &(color.b),
	    LEND));
  drawer_color( id, DRAWER_BBOXCOLOR, &color );
  return Lt;
}

void
drawer_color(int id, DrawerKeyword key, Color *col)
{
  int index;
  ApStruct as;
  DObject *obj;

  as.h = NULL;
  
  switch (key) {

  case DRAWER_DIFFUSE: 
    as.ap = ApCreate(AP_MtSet, MT_DIFFUSE, col, MT_AMBIENT, col, 
		     MT_OVERRIDE, MTF_DIFFUSE & uistate.apoverride, MT_END, AP_END);
    goto doap;	
  case DRAWER_EDGECOLOR: 
    as.ap = ApCreate(AP_MtSet, MT_EDGECOLOR, col, 
		     MT_OVERRIDE, MTF_EDGECOLOR & uistate.apoverride, MT_END, AP_END); 
    goto doap;
  case DRAWER_NORMALCOLOR: 
    as.ap = ApCreate(AP_MtSet, MT_NORMALCOLOR, col, 
		     MT_OVERRIDE, MTF_NORMALCOLOR & uistate.apoverride, MT_END, AP_END); 
    goto doap;
  doap:
    gv_merge_ap(id, &as);
    ApDelete(as.ap);
    break;

  case DRAWER_BBOXCOLOR:
    MAYBE_LOOP(id, index, T_GEOM, DObject, obj) {
      if (DGobj(obj)->bboxap && DGobj(obj)->bboxap->mat) {
	MtSet(DGobj(obj)->bboxap->mat, MT_EDGECOLOR, col, 
	      MT_OVERRIDE, MTF_EDGECOLOR & uistate.apoverride, MT_END);
	obj->redraw = true;
      }
      else {
	OOGLError(0,"object with %1d has no bboxap!");
      }
    }
    uistate.pickedbboxcolor = *col;
    break;

  case DRAWER_BACKCOLOR:
    drawerstate.defview.backcolor = *col;
    MAYBE_LOOP(id, index, T_CAM, DObject, obj) {
      DVobj(obj)->backcolor = *col;
      DVobj(obj)->redraw = true;
    }
    break;

  case DRAWER_LIGHTCOLOR:
    set_light_color(col);
    break;

  default: {
    /* report some kind of error? */
  }

  }
  ui_maybe_refresh(id);
}

LDEFINE(window, LVOID,
	"(window         CAM-ID  WINDOW)\n\
	Specify attributes for the window of CAM-ID, e.g. its size\n\
	or initial position, in the OOGL Window syntax.\n\
	The special CAM-ID \"default\" specifies\n\
	properties of future windows (created by \"camera\" or\n\
	\"new-camera\").")
{
  int index;
  DView *dv;
  int id;
  WindowStruct *ws;

  LDECLARE(("window", LBEGIN,
	    LID, &id,
	    LWINDOW, &ws,
	    LEND));

  MAYBE_LOOP(id, index, T_CAM, DView, dv) {
    WnDelete(dv->win);
    dv->win = WnCopy(ws->wn);
    if(dv->mgctx) {
      mgctxselect(dv->mgctx);
      mgctxset(MG_WINDOW, dv->win, MG_END);
    }
  }
  return Lt;
}

/*
 * Everything below here is internal to the drawer module (and should
 * be declared static to enforce this).
 */

/*
 * Track all changes made to dgeoms and views, by checking their
 * "changed", "moving" and "redraw" flags.
 * Effect:
 *	For each changed ordinary DGeom, calls update_dgeom()
 *		to possibly recompute the bounding box and/or renormalize.
 *	Set the "redraw" flags on all DViews which:
 *		are marked as "changed" or "moving", or
 *	Also set "redraw" on all DViews which view the world,
 *		if any DGeoms are "changed" or "moving".
 *
 * Environment:
 *	"changed" flags are set by gv_geometry(), etc.
 *		and by callback routines from external changes.
 *		They're cleared by this routine.
 *	"redraw" flags are cleared by draw_view().
 *	"moving" is set by drawer_xform_incr() and drawer_updateproc().
 */
static void
track_changes()
{
  bool worldchange = drawerstate.changed;
  bool viewchange = 0;
  int i;
  DObject *o;

  for(i = 0; i < dgeom_max; i++) {
    if((o = (DObject *)dgeom[i]) != NULL) {
      if(o->moving)
	worldchange = true;
      if(o->changed || o->redraw) {
	worldchange = true;
	update_dgeom((DGeom*)o);
	o->changed = CH_NOTHING;
	o->redraw = false;
      }
    }
  }
  for(i = 0; i < dview_max; i++) {
    if((o = (DObject *)dview[i]) != NULL) {
      if(o->moving || ((DView *)o)->newcam
	 || (worldchange && o->Item == drawerstate.universe))
	o->changed = true;
      if(o->changed) {
	update_view((DView *)o);
	viewchange = true;
      }
      if(!((DView*)o)->frozen) {
	viewchange = viewchange || o->redraw;
      }
    }
  }
  if(viewchange) {
    for(i = 0; i < dview_max; i++)
      if((o = (DObject *)dview[i]) != NULL)
	if(((DView *)o)->cameradraw)
	  o->redraw = true;
  }
  drawerstate.changed = false;
}

LDEFINE(merge_baseap, LVOID,
	"(merge-baseap   APPEARANCE)\n\
	Merge in some appearance characteristics to the base default\n\
	appearance (applied to every geom before its own apperance).\n\
	Lighting is typically included in the base appearance.")
{
  ApStruct *as;
  LDECLARE(("merge-baseap", LBEGIN,
	    LAP, &as,
	    LEND));
  drawerstate.ap = ApMerge(as->ap,
			   drawerstate.ap ? drawerstate.ap : base_defaultap,
			   APF_OVEROVERRIDE);
  drawerstate.apseq++;
  lights_changed_check();
  return Lt;
}

void
drawer_merge_camera( int id, Camera *cam )
{
  DView *dv;
  int index;

  MAYBE_LOOP(id, index, T_CAM, DView, dv) {
    CamMerge(cam, dv->cam);
    dv->newcam = true;
  }
}

void
drawer_merge_window( int id, WnWindow *win )
{
  DView *dv;
  int index;

  MAYBE_LOOP(id, index, T_CAM, DView, dv) {
    WnMerge(win, dv->win);
    if(dv->mgctx) {
      mgctxselect(dv->mgctx);
      mgctxset(MG_WINDOW, dv->win, MG_END);
      dv->newcam = true;
    }
  }
}

static void delete_geometry(DGeom *dg)
{
  int i, index = INDEXOF(dg->id);

  if(dgeom[index] == NULL)	/* Already (being) deleted -- forget it */
    return;

  if (dg->bbox_center != NOID) {
    /* Delete the bbox-center object */
    gv_delete(dg->bbox_center);
    dg->bbox_center = NOID;
  }

  dgeom[index] = NULL;
  switch (dg->citizenship) {
  case ORDINARY:
    dgeom[0]->Lgeom = ListRemove( dgeom[0]->Lgeom, dg->Item );
    GeomReplace( dgeom[0]->Inorm, dgeom[0]->Lgeom );
    ui_objectchange();
    if (uistate.targetgeom==index) {
      /* We deleted the target -- find next lowest one */
      for (i=index; --i>=0 && dgeom[i]==NULL; );
      gv_ui_target( GEOMID(i), 
		    TYPEOF(uistate.targetid) == T_GEOM ? IMMEDIATE : NOIMMEDIATE );
    }
    break;
  case ALIEN:
    if (ListRemove(drawerstate.universe, dg->Item) != drawerstate.universe) {
      OOGLError(0, "\
head of the universe changed --- this shouldn't happen; please report this bug!");
    }
    break;
  default: ; /* This should never happen */
  }
  HandlePDelete(&dg->incrhandle);
  OOGLFree(dg);
  drawerstate.changed = true;
}

static void delete_camera(DView *dv)
{
  int index = INDEXOF(dv->id);
  int i;
		    
  if(dview[index] == NULL)	/* Already (being) deleted -- forget it */
    return;

  dview[index] = NULL;
  mgctxdelete(dv->mgctx);
  CamDelete(dv->cam);
  HandlePDelete(&dv->camhandle);
  GeomDelete(dv->Item);
  OOGLFree(dv);
  ui_objectchange();
  if (index == uistate.mousefocus) {
    for(i = dview_max; --i > 0 && (dview[i] == NULL ||
				   dview[i]->frozen != UNFROZEN); )
      ;
    ui_mousefocus(i);
  }
  if (drawer_get_object(TARGETCAMID) == NULL) {
    if (drawer_get_object(FOCUSID) == NULL) {
      gv_ui_target( WORLDGEOM, IMMEDIATE);
    } else {
      gv_ui_target( FOCUSID, 
		    TYPEOF(uistate.targetid) == T_CAM ? IMMEDIATE : NOIMMEDIATE);
    }
  }
}

void
drawer_init(char *apdefault, char *defaultcam, char *windefault)
{
  int i;
  DGeom *dg;
  IOBFILE *f;

  ts_identity.h = NULL;
  TmIdentity(ts_identity.tm);

  f = iobfileopen(fmemopen(apdefault, strlen(apdefault), "rb"));
  if(f)
    drawerstate.ap =
      base_defaultap =
      ApFLoad(f, "built-in appearance");
  iobfclose(f);
  RefIncr((Ref *)drawerstate.ap);
  drawerstate.apseq++;

  /* Install default window position if not already set */
  if(drawerstate.defview.win == NULL)
    comm_object(windefault,
		&WindowOps, NULL, (Ref **)(void *)&drawerstate.defview.win,
		COMM_NOW);
  drawerstate.defview.backcolor = initial_defaultbackcolor;
  if(drawerstate.defview.cam == NULL)
    drawerstate.defview.cam =
      CamCreate(CAM_FOV, 40.0, CAM_NEAR, 0.1, CAM_FAR, 100.0, CAM_END);

  /* Create a not-quite-instantiated window */
  drawerstate.defview.mgctx =
    mgctxcreate(MG_CAMERA, drawerstate.defview.cam,
		MG_BACKGROUND, &drawerstate.defview.backcolor,
		MG_APPEAR, drawerstate.ap,
		MG_WINDOW, drawerstate.defview.win,
		MG_SHOW, 0,
		MG_END );

  drawerstate.defview.id = DEFAULTCAMID;
  drawerstate.defview.lineznudge = 3.;
  drawerstate.space = TM_EUCLIDEAN;
  drawerstate.pause = 0;
  /* drawerstate.motionscale controls the minimum rate of scaled translations;
     it should be small, for accurate mouse tracking, but not quite zero,
     lest scaled motions do nothing when a moving object coincides with the
     center of motion.  (At present, the only scaled translation is Z-motion
     in [o]rbit mode.) slevy 93.10.15.
  */
  drawerstate.motionscale = 0.03;
  name_fsa = fsa_initialize(name_fsa, NOID);
  fsa_install(name_fsa, "focus",	(void *)FOCUSID);
  fsa_install(name_fsa, "default",	(void *)DEFAULTCAMID);
  fsa_install(name_fsa, "defaultcam",	(void *)DEFAULTCAMID);
  fsa_install(name_fsa, "target",	(void *)TARGETID);
  fsa_install(name_fsa, "targetgeom",	(void *)TARGETGEOMID);
  fsa_install(name_fsa, "g.",		(void *)TARGETGEOMID);
  fsa_install(name_fsa, "targetcam",	(void *)TARGETCAMID);
  fsa_install(name_fsa, "targetcamera",	(void *)TARGETCAMID);
  fsa_install(name_fsa, "c.",		(void *)TARGETCAMID);
  fsa_install(name_fsa, "center",	(void *)CENTERID);
  fsa_install(name_fsa, "bbox-center",  (void *)BBOXCENTERID);
  fsa_install(name_fsa, "allgeoms",	(void *)ALLGEOMS);
  fsa_install(name_fsa, "g*",		(void *)ALLGEOMS);
  fsa_install(name_fsa, "allcams",	(void *)ALLCAMS);
  fsa_install(name_fsa, "c*",		(void *)ALLCAMS);
  fsa_install(name_fsa, "worldgeom",	(void *)WORLDGEOM);
  fsa_install(name_fsa, "world",	(void *)WORLDGEOM);
  fsa_install(name_fsa, "self",		(void *)SELF);
  fsa_install(name_fsa, ".",		(void *)SELF);
  fsa_install(name_fsa, "universe",	(void *)UNIVERSE);
  fsa_install(name_fsa, "Universe",	(void *)UNIVERSE);
  fsa_install(name_fsa, "primitive",	(void *)PRIMITIVE);
  /* The following four names are here for backward compatibility
   * of the 'write' command with the midsummer '92 version.
   */
  fsa_install(name_fsa, "bare",		(void *)SELF);	
  fsa_install(name_fsa, "wrap",		(void *)WORLDGEOM);
  fsa_install(name_fsa, "cumulative",	(void *)UNIVERSE);
  fsa_install(name_fsa, "cum",		(void *)UNIVERSE);

  dgeom = OOGLNewNE(DGeom *, dgeom_max, "dgeom array");
  dview = OOGLNewNE(DView *, dview_max, "view array");
  for (i=0; i<dgeom_max; ++i) dgeom[i] = NULL;
  for (i=0; i<dview_max; ++i) dview[i] = NULL;

  /*
   * Build the structure:
   *	drawerstate.world -->	INST  [world modelling & appearance]
   *	  == dgeom[0]->Item	  |
   *				  |
   *	dgeom[0]->Inorm		INST  [world normalizing]
   *				  |
   *	dgeom[0]->Lgeom --> LIST  --  LIST -- LIST ... [all objects]
   */

  dgeom[0] = dg = OOGLNewE(DGeom, "world DGeom");
  dg->normalization = EACH;
  dg->bboxdraw = 1;
  dg->bezdice = 10;
  drawer_init_dgeom(dg, GEOMID(0), THEWORLD);
  dg->Inorm = GeomCreate("inst", CR_END);
  dg->bboxap = ApCreate(AP_DONT, APF_FACEDRAW, AP_DO, APF_EDGEDRAW, 
			AP_OVERRIDE, APF_EDGEDRAW,
			AP_MtSet,
			MT_EDGECOLOR,&uistate.pickedbboxcolor,
			MT_OVERRIDE, MTF_EDGECOLOR,
			MT_END,
			AP_END);
  drawerstate.world = GeomCreate("inst", CR_GEOM, dg->Inorm, CR_END);
  dg->Item = drawerstate.world;
  dg->Lgeom = NULL;
  TmIdentity(dg->Incr);
  name_object(dg, 1, "World");
  dg->bboxvalid = false;

  drawerstate.universe = GeomCreate("list", CR_GEOM, drawerstate.world, CR_END);
  drawerstate.defview.Item = drawerstate.universe;
  GeomDice(dg->Item, dg->bezdice, dg->bezdice);
  f = iobfileopen(fmemopen(defaultcam, strlen(defaultcam), "rb"));
  drawerstate.camgeom = GeomFLoad(f, "built-in camera geometry");
  iobfclose(f);
  drawerstate.camproj = 0;

#ifdef MANIFOLD
  drawerstate.nDV = 0;
  drawerstate.nMT = 0;
  drawerstate.d1 = 1.0;
  drawerstate.i1 = 0.5;
  verbose_manifold = ( getenv("VERBOSE_MANIFOLD") != NULL );
  trivial_visibility = ( getenv("TRIVIAL_VISIBILITY") != NULL );
#endif /* MANIFOLD */

}

static void
winmoved(int camid)
{
  WnWindow *wn;
  DView *dv = (DView *)drawer_get_object(camid);

  if(dv != NULL && dv->mgctx != NULL) {
    mgctxselect(dv->mgctx);
    mgctxget(MG_WINDOW, &wn);
    WnGet(wn, WN_CURPOS, &drawerstate.winpos);
  }
}


static bool nonidentity(Transform T)
{
  return memcmp(T, TM_IDENTITY, sizeof(Transform)) != 0;
}

static DGeom *
new_dgeom(char *from, enum citizenship citizenship)
{
  DGeom *dg;
  Geom *g = NULL;
  Handle *h = NULL;
  int i;

  if(from != NULL)
    (void) comm_object(from, &GeomOps, &h, (Ref **)(void *)&g, COMM_LATER);

  for (i=0; i<dgeom_max && dgeom[i]!=NULL; ++i);
  if (i==dgeom_max) {
    int j;
    dgeom = OOGLRenewNE(DGeom *, dgeom, (dgeom_max += dgeom_max-1), "enlarge DGeom array");
    for (j=i; j<dgeom_max; ++j) dgeom[j] = NULL;
  }
  dg = dgeom[i] = OOGLNewE(DGeom, "new_dgeom dg");

  drawer_init_dgeom(dg, GEOMID(i), citizenship);

  dg->bboxdraw = dgeom[0]->bboxdraw;
  dg->Lgeom = GeomCreate("list", CR_HANDLE_GEOM, h, g, CR_END);
  dg->Lbbox = GeomCreate("list", CR_NOCOPY, CR_APPEAR,
			 dg->bboxap=ApCopy(dgeom[0]->bboxap, NULL), CR_END);
  ListAppend(dg->Lgeom, dg->Lbbox);
  dg->Inorm = GeomCreate("inst", CR_NOCOPY, CR_GEOM, dg->Lgeom, CR_END);
  dg->Item = GeomCreate("inst", CR_NOCOPY, CR_GEOM, dg->Inorm, CR_END);
  dg->changed = CH_GEOMETRY;
  dg->pickable = true;
  switch (citizenship) {
  case ORDINARY:
    if(dgeom[0]->Lgeom == NULL)
      dgeom[0]->Lgeom = GeomCreate("list", CR_NOCOPY,
				   CR_GEOM, dg->Item, CR_END);
    else
      ListAppend( dgeom[0]->Lgeom, dg->Item );
    GeomReplace( dgeom[0]->Inorm, dgeom[0]->Lgeom );
    break;
  case ALIEN:
    ListAppend(drawerstate.universe, dg->Item);
    break;
  default:
    break; /* cannot happen, shut-off a compiler warning */
  }

  return dg;
}

void
drawer_init_dgeom(DGeom *dg, int id, enum citizenship citizenship)
{
  dg->id = id;
  dg->citizenship = citizenship;
  TmIdentity(dg->Incr);
  dg->incrhandle = NULL;
  dg->updateproc = NULL;
  dg->moving = 0;
  dg->bezdice = dgeom[0]->bezdice;
  dg->name[0] = NULL;
  dg->name[1] = NULL;
  name_object(dg, 0, new_object_name(T_GEOM));
  dg->bboxdraw = dgeom[0]->bboxdraw;
  dg->normalization = dgeom[0]->normalization;
  dg->bboxvalid = false;
  dg->changed = CH_NOTHING;
  dg->redraw = false;
  dg->seqno = 0;
  dg->NDT = NULL;
  dg->NDTinv = NULL;
  dg->bbox_center = NOID;
#if 0
  GeomSet(dg->Item, CR_NDAXIS, dg->NDT, CR_END);
#endif
}

/*-----------------------------------------------------------------------
 * Function:	drawer_make_bbox
 * Description:	Make a new bounding box.
 * Args:	*dg:   parent DGeom
 *		combine: flag; if non-zero, takes union of old & new bboxes;
 *			     if zero, fits bbox tightly about object.
 * Returns:	
 * Author:	munzner
 * Date:	Tue Dec 10 20:42:20 1991
 * Notes:	Geom must be non-NULL.
 *              If old bbox exists is deleted, since new one always created.
 *              Default appearance: draw black edges, no faces.
 *		Assumes (if 'combine') that Lbbox list includes Lgeom list.
 */
void
drawer_make_bbox(DGeom *dg, bool combine)
{
  Geom *bbox;

  /* Don't create bbox for world dgeom */
  if (dg->id == WORLDGEOM) {
    return;
  }

  if (!dg->bboxvalid) {
    if(!combine) {
      GeomReplace(dg->Lbbox, NULL);
    }
    bbox =
      GeomBound(dg->Lgeom, drawerstate.NDim > 0 ? NULL : TM_IDENTITY, NULL);
    if (bbox) {
      GeomReplace(dg->Lbbox, bbox);
      GeomDelete(bbox);		/* Only Lbbox needs it now */
      dg->bboxvalid = true;
      if (dg->bbox_center != NOID) {
	/* Delete the old center object */
	gv_delete(dg->bbox_center);
	dg->bbox_center = NOID;
      }
    }
  }
}

/*-----------------------------------------------------------------------
 * Function:	_name_object
 * Description:	name an object
 * Args:	*obj: the object to be named
 *		ni: name index --- 0 or 1
 *		*name: object's name name
 * Returns:	nothing
 * Author:	mbp
 * Date:	Thu Dec  5 18:12:22 1991
 * Notes:	Don't call this procedure directly.  Use the
 *		'name_object'
 *		macro defined above; it casts
 *		its first argument (a DGeom* or a DView*) to Object*.
 */
static char *
_name_object(DObject *obj, int ni, char *name)
{
  char *tail;
  
  /* uninstall the old name, if any */
  if (obj->name[ni] != NULL) {
    fsa_install( name_fsa, obj->name[ni], NOID );
    OOGLFree(obj->name[ni]);
    obj->name[ni] = NULL;
  }

  /* get tail if full pathname */
  tail = strrchr(name, '/');
  if (tail) tail++;
  else tail = name;
  
  
  /* install the new name */
  obj->name[ni] = unique_name(tail, obj->id);
  if ( ni==0 && strcmp(obj->name[ni], tail)!=0 ) {
    OOGLError(0,"warning: attempt to use existing first name: \"%s\"\n\
  using \"%s\" instead", tail, obj->name[ni]);
  }
  fsa_install( name_fsa, obj->name[ni], (void *)(long)obj->id );
  return obj->name[ni];
}

static DView *
new_dview()
{
  DView *dv;
  char window_name[64], *view_name;
  int i;
  WnPosition wp;

  for (i=0; i<dview_max && dview[i]!=NULL; ++i);
  if (i==dview_max) {
    int j;
    dview = OOGLRenewNE(DView *, dview, (dview_max += dview_max-1), 
			"enlarge DView array");
    for (j=i; j<dview_max; ++j) dview[j] = NULL;
  }
  dv = dview[i] = OOGLNewE(DView, "new view");
  dv->id = CAMID(i);
  view_name = new_object_name(T_CAM);
  dv->cam = CamCopy(drawerstate.defview.cam, NULL);
  CamSet(dv->cam, CAM_SPACE, drawerstate.space, CAM_END);
  dv->win = WnCopy(drawerstate.defview.win);
  dv->backcolor = drawerstate.defview.backcolor;
  dv->camhandle = NULL;
  RefIncr((Ref*)(drawerstate.universe));
  dv->Item = drawerstate.universe;
  dv->extradraw = NULL;
  dv->apseq = drawerstate.apseq;
  TmIdentity(dv->Incr);
  dv->incrhandle = NULL;
  dv->updateproc = NULL;
  dv->moving = false;
  dv->changed = CH_NOTHING;
  dv->frozen = UNFROZEN;
  dv->seqno = 0;
  dv->lineznudge = drawerstate.defview.lineznudge;
  dv->newcam = true;
  dv->name[0] = NULL;
  dv->name[1] = NULL;
  name_object(dv, 0, view_name);
  sprintf(window_name, "geomview %s", view_name);
  dv->cameradraw = false;
  dv->hsphere = NULL;
  dv->hmodel = VIRTUAL;
  dv->stereo = NO_KEYWORD;
  dv->stereogap = 0;
  dv->shader = NULL;
  dv->mgctx =
    mgctxcreate(MG_CAMERA, dv->cam,
		MG_BACKGROUND, &dv->backcolor,
		MG_APPEAR, drawerstate.ap,
		MG_WINDOW, dv->win,
		MG_SHOW, 0,	/* Don't show window until we set its name */
		MG_END );
  if(WnGet(dv->win, WN_PREFPOS, &wp) > 0) {
    wp.xmin += 40; wp.ymin -= 40; wp.xmax += 40; wp.ymax -= 40;
    WnSet(drawerstate.defview.win, WN_PREFPOS, &wp, WN_END);
  }
  if(i >= nwins)
    nwins = i+1;
  for(i = 0; i < 3; i++) dv->NDPerm[i] = i+1;
  dv->NDPerm[3] = 0;
  dv->nNDcmap = 0;
  dv->cluster = NULL;
  for(i = 0; i < MAXCMAP; i++) {
    dv->NDcmap[i].axis = NULL;
    VVINIT(dv->NDcmap[i].cents, cent, 5);
  }
  return dv;
}

static void
updateit(float dt)
{
  int i;
  DView *dv;
  DGeom *dg;
  Geom *g;

  do_motion(dt);
  for(i = 0; i < dgeom_max; i++) {
    if((dg = dgeom[i]) == NULL)
      continue;
    if (dg->updateproc) {
      GeomGet(dg->Lgeom, CR_GEOM, &g);
      (*(dg->updateproc))(g);
    } else {
      GeomTransform(dg->Item, dg->Incr, NULL);
    }
  }
  LOOPVIEWS(i,dv) {
    if (dv->updateproc)
      (*(dv->updateproc))(dv->cam);
    else
      CamTransform(dv->cam, dv->Incr);
  }
}

static void
draw_view(DView *dv)
{
  /* Colors for red/cyan stereo */
  static int lefteyemask = MGO_NOGREEN|MGO_NOBLUE;	/* Red */
  static int righteyemask = MGO_NORED;			/* Green+Blue */
  Appearance *ap;

  if((dv->frozen == UNFROZEN) &&
     (dv->redraw || (dv->changed != CH_NOTHING) || dv->newcam)) {
    mgctxselect(dv->mgctx);
    mgctxset(MG_CAMERA, dv->cam, MG_BACKGROUND, &dv->backcolor,
	     MG_ZNUDGE, dv->lineznudge * 1.0e-5,
	     MG_UNSETOPTIONS,
	     MGO_NORED|MGO_NOGREEN|MGO_NOBLUE|MGO_INHIBITCLEAR,
	     MG_END);
    if(drawerstate.apseq > dv->apseq) {
      mgsetappearance(drawerstate.ap, MG_SET);
      dv->apseq = drawerstate.apseq;
    }
    ap = GeomAppearance(dv->Item);
    if(ap)
      mgsetappearance(ap, MG_MERGE);

    if(dv->newcam)
      reconfigure_view(dv);
    if(dv->stereo != NO_KEYWORD) {
      mgctxset(MG_WnSet, WN_VIEWPORT, &dv->vp[1], WN_END,
	       MG_CamSet, CAM_STEREOEYE, 1, CAM_END,
	       MG_SETOPTIONS, MGO_INHIBITSWAP, MG_END);
      if(dv->stereo == COLORED_KEYWORD)
	mgctxset(MG_SETOPTIONS, lefteyemask, MG_END);
      really_draw_view(dv);
      mgctxset(MG_WnSet, WN_VIEWPORT, &dv->vp[0], WN_END,
	       MG_CamSet, CAM_STEREOEYE, 0, CAM_END,
	       MG_UNSETOPTIONS, MGO_INHIBITSWAP|lefteyemask,
	       MG_END);
      if(dv->stereo == COLORED_KEYWORD) {
	/* In Iris GL, clearing clears everything, including the
	 * left-eye image we just drew.  In OpenGL, it doesn't.
	 * So *only* if we're doing GL drawing, select MGO_INHIBITCLEAR
	 * along with the right-eye image planes.
	 */
	mgctxset(MG_SETOPTIONS,
		 _mgf.mg_devno == MGD_GL ? righteyemask|MGO_INHIBITCLEAR
		 : righteyemask,
		 MG_END);
      }
    }
    really_draw_view(dv);
    dv->changed = CH_NOTHING;
    dv->redraw = false;
  }
}

/*
 * Reconfigure view.  Assumes the proper window is already selected.
 */
static void
reconfigure_view(DView *dv)
{
  WnWindow *win = NULL;
  WnPosition wp, gap;
  int xsize, ysize;
  int isstereo = 1;

  mgctxget(MG_WINDOW, &win);
  WnGet(win, WN_CURPOS, &wp);

  xsize = wp.xmax - wp.xmin + 1;
  ysize = wp.ymax - wp.ymin + 1;
  dv->vp[0].xmin = dv->vp[0].ymin = 0;
  dv->vp[0].xmax = xsize-1;
  dv->vp[0].ymax = ysize-1;
  dv->vp[1] = dv->vp[0];
  gap = dv->vp[0];

  switch(dv->stereo) {
  case NO_KEYWORD:
    isstereo = 0;
    break;
  case HORIZONTAL_KEYWORD:
    dv->vp[0].xmax = (xsize - dv->stereogap) / 2;
    dv->vp[1].xmin = dv->vp[0].xmax + dv->stereogap;
    gap.xmin = dv->vp[0].xmax + 1;
    gap.xmax = dv->vp[1].xmin - 1;
    break;
  case VERTICAL_KEYWORD:
    dv->vp[0].ymax = (ysize - dv->stereogap) / 2;
    dv->vp[1].ymin = dv->vp[0].ymax + dv->stereogap;
    gap.ymin = dv->vp[0].ymax + 1;
    gap.ymax = dv->vp[1].ymin - 1;
    break;
  case COLORED_KEYWORD:
    /* Viewports identical, stereo enabled. */
    break;
  default:
    break;
  }
  if(dv->stereogap > 0) {
    mgctxset(MG_WnSet, WN_VIEWPORT, &gap, WN_END, MG_END);
    mgworldbegin(); mgworldend();	/* Force clear */
    mgworldbegin(); mgworldend();	/* ... of both buffers */
  }
  mgctxset(MG_WnSet, WN_VIEWPORT, &dv->vp[0], WN_END,
	   MG_CamSet, CAM_STEREO, isstereo, CAM_END, MG_END);
  mgreshapeviewport();
  dv->newcam = false;
  if(dv->id == CAMID(uistate.mousefocus))
    winmoved(dv->id);
}

static void
really_draw_view(DView *dv)
{
#if 0
#include <sys/time.h>
  static int cnt;
  static struct timeval tv_old[1];

  if (cnt % 100 == 0) {
    struct timeval tv_new[1];
    double elapsed;

    gettimeofday(tv_new, NULL);
    
    if (cnt > 0) {
      elapsed  = (double)(tv_new->tv_sec - tv_old->tv_sec);
      elapsed += (double)(tv_new->tv_usec - tv_old->tv_usec) / 1e6;      
      fprintf(stderr, "%d frames in %e seconds, fps = %e\n",
	      cnt, elapsed, (double)cnt/elapsed);
      cnt = 0;
    }
    *tv_old = *tv_new;
  }
  
  cnt++;
#endif

  mgpushappearance();
  mgworldbegin();

  if(dv->cluster == NULL && drawerstate.NDim > 0) {
    /* If we're in N-D mode but this camera isn't equipped,
     * give it a default N-D => 3-D projection now.
     */
    gv_ND_axes(dv->name[0],
	       drawerstate.NDcams ? drawerstate.NDcams->name : "default",
	       1, 2, 3, 0);
  }

  if(dv->cluster != NULL) {
    /* ND-drawing, works also with scenes; the (scene ...) command
     * takes care of attaching all cameras of a cluster to the same
     * scene.
     */
    NDcam *cluster = dv->cluster;
    int dim = TmNGetSize(cluster->C2W, NULL,NULL);
    NDstuff *nds;
    TransformN *W2C = NULL, *W2U = NULL;

    cluster->W2C = TmNInvert(cluster->C2W, cluster->W2C);
    W2C = TmNCopy(cluster->W2C, NULL);
    W2U = TmNIdentity(TmNCreate(dim, dim, NULL));

    nds = drawer_init_ndstuff(dv, W2C, W2U);
    drawer_transform_ndstuff(nds, NULL);
    mgctxset(MG_NDCTX, nds, MG_END);

    GeomDraw(dv->Item);

    /* draw other camera's if requested; take their respective
     * ND-transform into account.
     */
    if (drawerstate.camgeom != NULL && dv->cameradraw) {
      DView *otherv;
      int otheri;
      Transform T3;
      TransformN *ovC2W = NULL;

      LOOPVIEWS(otheri, otherv) {
	if (otherv != dv && otherv->Item == dv->Item) {

	  if(drawerstate.camproj) {
	    /* Mmmh. ND?? */
	    Transform Tt;
	    CamView(otherv->cam, Tt);
	    TmInvert(Tt, T3);
	  } else {
	    CamGet(otherv->cam, CAM_C2W, T3);
	  }

	  if (otherv->cluster && otherv->cluster->C2W) {
	    ovC2W = TmNCopy(otherv->cluster->C2W, ovC2W);
	  } else {
	    ovC2W = ovC2W
	      ? TmNIdentity(ovC2W)
	      : TmNIdentity(TmNCreate(drawerstate.NDim,
				      drawerstate.NDim, NULL));
	  }
	  TmNApplyT3TN(T3, otherv->NDPerm, ovC2W);
	  TmNMap(ovC2W, otherv->NDPerm, ovC2W);

	  drawer_transform_ndstuff(nds, ovC2W);
	  GeomDraw(drawerstate.camgeom);

	  /* we assume that cameras have no translucent component */
	}
      }

      TmNDelete(ovC2W);
    }

    TmNDelete(W2U);
    TmNDelete(W2C);
    drawer_destroy_ndstuff(nds);
    
    mgctxset(MG_NDCTX, NULL, MG_END);
  } else {
    /* Normal case.  Just draw whatever's attached to this camera */
    GeomDraw(dv->Item);

    /* new */
    if (dv->hsphere != NULL) {
      /* if we're in conformal ball model, switch to projective
	 model for drawing the sphere at infinity, then switch back */

      int mgspace;
      
      mgctxget(MG_SPACE, &mgspace);
      if (dv->hmodel == CONFORMALBALL) {
	mgctxset(MG_SPACE, TM_SPACE(mgspace) | TM_PROJECTIVE, MG_END);
	GeomDraw(dv->hsphere);
	mgctxset(MG_SPACE, mgspace, MG_END);
      } else {
	GeomDraw(dv->hsphere);
      }
    }

    /* drawing of other cameras in ND has been handled above */
    if (drawerstate.camgeom &&
	dv->cameradraw && dv->Item == drawerstate.universe) {
      DView *otherv;
      int otheri;
      Transform T, Tt;
      LOOPVIEWS(otheri,otherv) {
	if (otherv != dv && otherv->Item == drawerstate.universe) {
	  mgpushtransform();
	  if(drawerstate.camproj) {
	    CamView(otherv->cam, Tt);
	    TmInvert(Tt, T);
	  } else
	    CamGet(otherv->cam, CAM_C2W, T);
	  mgtransform(T);
	  GeomDraw(drawerstate.camgeom);
	  mgpoptransform();
	}
      }
    }
  }

  if (dv->extradraw) (dv->extradraw)(dv);
    
  mgworldend();
  mgpopappearance();
}

static char *
new_object_name(int type)
{
  static char name[16];
  int i;

  for(i = 0; ; i++) {
    sprintf(name, "%c%d", type == T_GEOM ? 'g' : 'c', i);
    if(drawer_idbyname(name) == NOID)
      return name;
  }
}

/*-----------------------------------------------------------------------
 * Function:	ordinal
 * Description:	extract the ordinal number of a DView or DGeom from its name
 * Args:	*name: DView or DGeom's name --- assumed to be of the form
 *		  "x<number>", where x is any char
 * Returns:	<number>
 * Author:	mbp
 * Date:	Sat Nov 30 22:42:54 1991
 */
/*
  static int
  ordinal(char *name)
  {
  return( atoi( &name[1] ) );
  }
*/

static void
object_changed(Handle **hp, DObject *obj, void *seqno)
{
  if(obj->seqno != (long)seqno) {
    /* Obsolete reference -- remove this Handle callback */
    HandleUnregisterJust(hp, (Ref*)obj, seqno, object_changed);
  } else {
    obj->changed = CH_GEOMETRY;
  }
}

static int
object_register(Handle **hp, Ref *thing, DObject *o)
{
  HandleRegister(hp, (Ref *)o, (void *)(long)(int)o->seqno, object_changed);
  return 0;
}

/*
 * Something has changed in this dgeom's geometry.
 * Discard and (if necessary) recompute bounding box.
 * In any case, scan for Handles.
 */
static void
update_dgeom(DGeom *dg)
{
  if(dg->id == WORLDGEOM) {
    return;
  }
  if(dg->changed/* & CH_GEOMETRY*/) {
    dg->bboxvalid = false;
    HandleUnregisterAll((Ref *)dg, (void *)(long)(int)dg->seqno, object_changed);
    dg->seqno++;
    /*
     * Find all Handles in this DGeom, and a callback for each.
     */
    GeomHandleScan(dg->Item, object_register, dg);
  }
  if(dg->bboxdraw || dg->normalization != NONE) {
    drawer_make_bbox(dg, dg->normalization == ALL);
  }
  ApSet(dg->bboxap, dg->bboxdraw ? AP_DO : AP_DONT, APF_EDGEDRAW, 
	AP_OVERRIDE, APF_EDGEDRAW & uistate.apoverride, AP_END);
  GeomDice(dg->Item, dg->bezdice, dg->bezdice);
  normalize(dg, dg->normalization);
}

static void
update_view(DView *v)
{
  v->seqno++;
  if(v->Item != drawerstate.universe)
    GeomHandleScan(v->Item, object_register, v);
  CamHandleScan(v->cam, object_register, v);
}

static void
normalize(DGeom *dg, int normalization)
{
  float cx,cy,cz,cs,r,dx,dy,dz;
  HPoint3 min,max;
  Geom *bbox = NULL;
  Transform tmp;

  dg->normalization = normalization;
  if(dg->id == WORLDGEOM) return;
  switch(normalization) {
  case NONE:
    GeomTransformTo(dg->Inorm, NULL, NULL);
    break;

  case EACH:
  case ALL:
    if (!dg->bboxvalid) {
      drawer_make_bbox(dg, normalization == ALL);
    }
    GeomGet(dg->Lbbox, CR_GEOM, &bbox);
    if (bbox != NULL) {
      BBoxMinMax((BBox*)bbox, &min, &max);
      cx = (max.x + min.x) * .5;
      cy = (max.y + min.y) * .5;
      cz = (max.z + min.z) * .5;
      dx = max.x - min.x;
      dy = max.y - min.y;
      dz = max.z - min.z;
      r = sqrt(dx*dx + dy*dy + dz*dz);
      cs = (r == 0) ? 1 : 2.0/r;
      TmScale(tmp,cs,cs,cs);
      CtmTranslate(tmp,-cx,-cy,-cz);
      GeomTransformTo(dg->Inorm, tmp, NULL);
    }
    break;
    /* case KEEP: Leave normalization transform alone */
  }
  dg->redraw = true;
}    

void
drawer_nuke_world()
{
  gv_delete( ALLGEOMS );
}

void
drawer_nuke_cameras(int keepzero)
{
  int i,i0;

  i0 = keepzero ? 1 : 0;
  for (i=i0; i<dview_max;  ++i)
    gv_delete( CAMID(i) );
}

LDEFINE(exit, LVOID,
	"(exit)\n\
	Terminates geomview.")
{
  LDECLARE(("exit", LBEGIN,
	    LEND));
  ui_cleanup();
  exit(0);
  return Lt;
}

/*-----------------------------------------------------------------------
 * Function:	unique_name
 * Description:	return a unique object name with a given prefix
 * Args:	*name: the prefix
 * Returns:	
 * Author:	mbp
 * Date:	Wed Jan 22 18:05:26 1992
 * Notes:	
 */
static char *
unique_name(char *name, int id)
{
  char newname[256];
  int i;

  if ( fsa_parse( name_fsa, name ) == NOID )
    return strdup(name);
  
  i = INDEXOF(id);
  do {
    sprintf(newname, "%.240s<%d>", name, i++);
  } while(fsa_parse(name_fsa, newname) != NOID);
  return strdup(newname);
}

#ifdef MANIFOLD

int
drawer_read_manifold(char *file)
{
  FILE *fp = fopen(file, "rb");
  int i,n;

  setlinebuf(stderr);

  if (fp == NULL) return 0;

  n = fgetni(fp, 1, &(drawerstate.nMT), 0);
  if (n != 1) {
    OOGLError(0,"%1d", n);
    return 0;
  }
  if (verbose_manifold)
    fprintf(stderr, "mfd: nMT = %1d\n", drawerstate.nMT);

  n = fgettransform(fp, drawerstate.nMT, (float*)(drawerstate.MT), 0);
  if (n != drawerstate.nMT) {
    OOGLError(0,"%1d", n);
    return 0;
  }
  if (verbose_manifold) {
    fprintf(stderr, "mfd: MT[] = ");
    fputtransform(stderr, drawerstate.nMT, (float*)(drawerstate.MT), 0);
  }

  n = fgetni(fp, 1, &(drawerstate.nDV), 0);
  if (n != 1) {
    OOGLError(0,"%1d", n);
    return 0;
  }
  if (verbose_manifold) {
    fprintf(stderr, "mfd: nDV = %1d\n", drawerstate.nDV);
  }

  n = fgetnf(fp, drawerstate.nDV*4, (float*)(drawerstate.DV), 0);
  if (n != drawerstate.nDV*4) {
    OOGLError(0,"%1d", n);
    return 0;
  }
  if (verbose_manifold) {
    fprintf(stderr, "mfd: DV[] = ");
    for (i=0; i<drawerstate.nDV; ++i) {
      fprintf(stderr, "  %f  %f  %f  %f\n",
	      drawerstate.DV[i].x,
	      drawerstate.DV[i].y,
	      drawerstate.DV[i].z,
	      drawerstate.DV[i].w);
    }
  }

  fclose(fp);
  return 1;
}

/*-----------------------------------------------------------------------
 * Function:	visible
 * Description:	test whether a convex polyhedron is visible
 * Args:	T: complete world->NDC transform
 *		v: list of vertices of polyhedron
 *		n: number of vertices in list v
 * Returns:	0 or 1
 * Author:	mbp
 * Date:	Thu Aug  6 13:43:02 1992
 * Notes:	Uses an axial bounding box test: is any part of the
 *		the axial bounding box of the projections of the
 *		vertices v visible?  This is guaranteed to accept all
 *		visible polyhedra, and may accept some invisible ones
 *		too.
 */
int
visible(Transform T, HPoint3 *v, int n)
{
#define MIN -2
#define MAX  2
  float x,y,z;
  float
    xmin = MAX,
    xmax = MIN,
    ymin = MAX,
    ymax = MIN,
    zmin = MAX,
    zmax = MIN;
  int i;
  Point3 pv;

  if (trivial_visibility) return 1;

  for (i=0; i<n; ++i) {
    HPt3TransPt3( T, &v[i], &pv );
    if (pv.x < xmin) xmin = pv.x;
    else if (pv.x > xmax) xmax = pv.x;
    if (pv.y < ymin) ymin = pv.y;
    else if (pv.y > ymax) ymax = pv.y;
    if (pv.z < zmin) zmin = pv.z;
    else if (pv.z > zmax) zmax = pv.z;
  }

#if 0
  printf("x: [%f\t, %f]\n", xmin, xmax);
  printf("y: [%f\t, %f]\n", ymin, ymax);
  printf("z: [%f\t, %f]\n", zmin, zmax);
#endif

  /* if entire polyhedron is outside clipping region, reject */
  if (zmax <= -1 || zmin >= 1) return 0;
  if (xmax <= -1 || xmin >= 1) return 0;
  if (ymax <= -1 || ymin >= 1) return 0;

  /* otherwise accept */
  return 1;
#undef MAX
#undef MIN
}

float intensity(float d, float d1, float i1)
{
  float val, d12;

  d12 = d1*d1;
  val = i1 * d12 / ( (1 - i1)*d*d + i1*d12 );
  return val;
}

#endif /* MANIFOLD */

static HModelValue hmodelval(char *s, int val)
{
  switch (val) {
  case VIRTUAL_KEYWORD: return VIRTUAL;
  case PROJECTIVE_KEYWORD: return PROJECTIVE;
  case CONFORMALBALL_KEYWORD: return CONFORMALBALL;
  default:
    fprintf(stderr, "%s: invalid model keyword (assuming \"virtual\")\n",s);
    return VIRTUAL;
  }
}


Keyword hmodelkeyword(char *s, HModelValue val)
{
  switch (val) {
  case VIRTUAL: return VIRTUAL_KEYWORD;
  case PROJECTIVE: return PROJECTIVE_KEYWORD;
  case CONFORMALBALL: return CONFORMALBALL_KEYWORD;
  default:
    fprintf(stderr, "%s: invalid model (assuming \"virtual\")\n",s);
    return VIRTUAL_KEYWORD;
  }
}



static int normalval(char *s, int val)
{
  switch (val) {
  case NONE_KEYWORD: return NONE;
  case EACH_KEYWORD: return EACH;
  case KEEP_KEYWORD: return KEEP;
  case ALL_KEYWORD: return ALL;
  default:
    fprintf(stderr, "%s: invalid normalization type (assuming \"none\")\n",s);
    return NONE;
  }
}

/*-----------------------------------------------------------------------
 * Function:	spaceof
 * Description:	return the space of an object
 * Args:	id: the object
 * Returns:	TM_EUCLIDEAN, TM_HYPERBOLIC, or TM_SPHERICAL
 * Author:	mbp
 * Date:	Sun Dec 13 22:28:58 1992
 * Notes:	We need to be able to tell what space an object is in,
 *		in order to know what kinds of transformations to
 *		apply to it (any maybe for other reasons, e.g.
 *		shading).  OOGL cameras know about what space they're
 *		in.  OOGL geoms do not (at this time, anyway).  The
 *		main rationale for this is that the concept of "space"
 *		applies to a whole tree of geoms; every geom in the
 *		tree is in the same space.  The way geomview currently
 *		deals with this is to use "drawerstate.space" to hold
 *		the space designation for the world.  All
 *		space-sensitive procedures should check the space of
 *		an id by calling spaceof(), however, rather than
 *		referencing drawerstate.space directly.  This is
 *		because at some point we may generalize to having more
 *		than one world or some other mechanism for keeping
 *		track of what space things are in.
 */
int
spaceof(int id)
{
  DView *dv;
  int space;
  if (ISCAM(id)) {
    dv = (DView*)drawer_get_object(id);
    if (dv == NULL) {
      OOGLError(0,"spaceof: unknown camera id %1d (assuming TM_EUCLIDEAN)\n",
		id);
      return TM_EUCLIDEAN;
    }
    CamGet(dv->cam, CAM_SPACE, &space);
    return space;
  }
  return drawerstate.space;
}



static int spaceval(char *s, int val)
{
  switch (val) {
  case EUCLIDEAN_KEYWORD: return TM_EUCLIDEAN;
  case HYPERBOLIC_KEYWORD: return TM_HYPERBOLIC;
  case SPHERICAL_KEYWORD: return TM_SPHERICAL;
  default:
    fprintf(stderr, "%s: invalid space keyword (assuming \"euclidean\")\n",s);
    return EUCLIDEAN;
  }
}

  
/*-----------------------------------------------------------------------
 * Function:	scaleof
 * Description:	return a number indicating the scale of an object
 * Args:	id: the object
 * Returns:	the scale number
 * Author:	mbp
 * Date:	Thu Feb 11 11:10:19 1993
 */
float scaleof(int id)
{
  float scale = 1.0;

  if (ISCAM(id)) {
    DView *dv;
    dv = (DView*)drawer_get_object(id);
    if (dv == NULL) {
      OOGLError(0,"scaleof: unknown camera id %1d (returning scale = 1)\n",
		id);
    } else {
      /* for cameras return the focal length */
      CamGet(dv->cam, CAM_FOCUS, &scale);
    }
  } else {
    DGeom *dg;
    HPoint3 min, max;
    BBox *bbox;

    dg = (DGeom*)drawer_get_object(id);
    if (dg == NULL) {
      OOGLError(0,"scaleof: unknown geom id %1d (returning scale = 1)\n",
		id);
    } else {
      /* for geoms return (geometric mean of 1 + bbox side lengths) - 1*/
      GeomGet(dg->Lbbox, CR_GEOM, &bbox);
      if (bbox != NULL) {
	BBoxMinMax((BBox*)bbox, &min, &max);
	scale = pow( fabs((double)((max.x-min.x+1)
				   *(max.y-min.y+1)
				   *(max.z-min.z+1))), .333 ) - 1;
      }
    }
  }
  return scale;
}

LDEFINE(set_motionscale, LVOID,
	"(set-motionscale X)\n\
	Set the motion scale factor to X (default value 0.5).  These\n\
	commands scale their motion by an amount which depends on the\n\
	distance from the frame to the center and on the size of the\n\
	frame.  Specifically, they scale by\n\
	        dist + scaleof(frame) * motionscale\n\
	where dist is the distance from the center to the frame and\n\
	motionscale is the motion scale factor set by this function.\n\
	Scaleof(frame) measures the size of the frame object.")
{
  float scale;
  LDECLARE(("set-motionscale", LBEGIN,
	    LFLOAT, &scale,
	    LEND));
  drawerstate.motionscale = scale;
  return Lt;
}


LDEFINE(set_conformal_refine, LVOID,
	"(set-conformal-refine CMX [N [SHOWEDGES]])\n\
	Sets the parameters for the refinement algorithm used in drawing\n\
	in the conformal model.  CMX is the cosine of the maximum angle\n\
	an edge can bend before it is refined.  Its value should be between\n\
	-1 and 1; the default is 0.95; decreasing its value will cause less\n\
	refinement.  N is the maximum number of iterations of refining;\n\
	the default is 6.  SHOWEDGES, which should be \"no\" or \"yes\",\n\
	determines whether interior edges in the refinement are drawn.")
{
  float cmb;
  int maxsteps = -1;
  int showedges = -1;
  extern void set_cm_refine(double cm_cmb, int cm_mr, int cm_ss);

  LDECLARE(("set-conformal-refine", LBEGIN,
	    LFLOAT, &cmb,
            LOPTIONAL, 
	    LINT, &maxsteps, 
	    LKEYWORD, &showedges,
            LEND));
  set_cm_refine((double)cmb,maxsteps,showedges);
  return Lt;
}

static void
traverse(Pool *p, Geom *where, int *pickpath, int *curpath, 
	 int *curbase, int pn)
{
  Geom *new;
  char *name;
  int depth;

  if (!where) return;
  name = GeomName(where);
  if (!strcmp(name, "comment")) {
    /* check if our current path matches the pickpath
       decrement the depth since we don't match the last level */
    depth = curpath - curbase -1;
    while (depth >= 0){ 
      if (curbase[depth] != pickpath[depth]) break;
      if (depth == 0) {
	/* we're set */
	fprintf(PoolOutputFile(p), "{");
	CommentExport((Comment*)where, p);
	fprintf(PoolOutputFile(p), "}");
      }
      depth--;
    }
  } else if (!strcmp(name, "list")) {
    /* push down a level */
    curpath++;
    while (where) {
      /* traverse the list. 
	 the cdr of a list is a list, but don't descend another level */
      GeomGet(where, CR_CAR, &new);
      if (new) {
	(*curpath)++;
	traverse(p, new, pickpath, curpath, curbase, pn);
      }
      GeomGet(where, CR_CDR, &where);
    }
    /* pop up a level */
    *curpath = -1;
    curpath--;
  } else if (!strcmp(name, "inst")) {
    GeomGet(where, CR_GEOM, &new);
    if (new) {
      /* push down a level */
      curpath++;
      (*curpath)++;
      traverse(p, new, pickpath, curpath, curbase, pn);
      /* pop up a level */
      *curpath = -1;
      curpath--;
    }
  }
}

#if 0
static void 
drawer_write_comments(char *fname, int id, int *pickpath, int pn)
{
}
#endif

LDEFINE(write_comments, LVOID,
	"(write-comments FILENAME GEOMID PICKPATH)\n\
	write OOGL COMMENT objects in the GEOMID hierarchy at the\n\
 	level of the pick path to FILENAME. Specifically, COMMENTS\n\
 	at level (a b c ... f g) will match pick paths of the form\n\
 	(a b c ... f *) where * includes any value of g, and also\n\
 	any values of possible further indices h,i,j, etc. The pick\n\
 	path (returned in the \"pick\" command) is a list of\n\
 	integer counters specifying a subpart of a hierarchical\n\
 	OOGL object. Descent into a complex object (LIST or INST)\n\
 	adds a new integer to the path. Traversal of simple objects\n\
 	increments the counter at the current level.\n\
 	Individual COMMENTS are enclosed by curly braces, and the\n\
 	entire string of zero, one, or more COMMENTS (written in\n\
 	the order in which they are encountered during hierarchy\n\
 	traversal) is enclosed by parentheses.\n\
        \n\
        Note that arbitrary data can only be passed through the OOGL\n\
 	libraries as full-fledged OOGL COMMENT objects, which can be\n\
 	attached to other OOGL objects via the LIST type as described\n\
 	above. Ordinary comments in OOGL files (i.e. everything after\n\
 	'#' on a line) are ignored at when the file is loaded and\n\
 	cannot be returned.")

{
  int id;
  int pickpath[40];
  int pn = 40;
  char *fname;
  Pool *p, *op;
  Geom *where;
  int count;
  bool temppool = false;
  int curpath[40];
  Lake *brownie;

  LDECLARE(("write-comments", LBEGIN,
	    LLAKE, &brownie,
	    LSTRING, &fname,
	    LID, &id,
	    LHOLD, LARRAY, LINT, pickpath, &pn,
	    LEND));

  /* The Lake is a dummy argument that gives us access to the
     right output file pointer */
  p = POOL(brownie);

  if (fname[0] == '-') {
    if (PoolOutputFile(p)) {
      op = p;
    } else {
      op = PoolStreamTemp(fname, NULL, stdout, 1, &GeomOps);
      temppool = true;
    }
  } else {
    op = PoolStreamTemp(fname, NULL, NULL, 1, &GeomOps);
    temppool = true;
  }
  if(op == NULL || PoolOutputFile(op) == NULL) {
    fprintf(stderr, "write: cannot open \"%s\": %s\n", fname, sperror());
    return Lnil;
  }

  for (count=0; count < 40; count++) {
    curpath[count] = -1;
  }

  /* get to the right starting place */
  GeomGet(((DGeom *)drawer_get_object(id))->Lgeom, CR_GEOM, &where);
  fprintf(PoolOutputFile(op), "( ");
  traverse(op, where, pickpath, curpath-1, curpath, pn);
  fprintf(PoolOutputFile(op), ")\n");
  fflush(PoolOutputFile(op));
  if (temppool) {
    PoolClose(op);
    PoolDelete(op);
  }

  return Lt;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
