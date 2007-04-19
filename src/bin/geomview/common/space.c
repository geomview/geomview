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

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "drawer.h"
#include "ui.h"
#include "lang.h"
#include "lisp.h"
#include "mg.h"
#include "geom.h"
#include "vect.h"
#include "color.h"
#include "camera.h"
#include "space.h"

#define	DGobj(obj)  ((DGeom *)obj)
#define	DVobj(obj)  ((DView *)obj)

char *
spacename(int space)
{
  switch(space) {
  case TM_SPHERICAL: return "spherical";
  case TM_EUCLIDEAN: return "euclidean";
  case TM_HYPERBOLIC: return "hyperbolic";
  default: return NULL;
  }
}

void
camera_space_relabel(int id)
{
  DView *dv = (DView *)drawer_get_object(id);
  char *extra=NULL;
  char *fmt;
  char label[256];

  if(!ISCAM(id) || dv == NULL || dv->mgctx == NULL)
    return;
  mgctxselect(dv->mgctx);
  extra = keywordname(hmodelkeyword("", dv->hmodel));
  switch(spaceof(WORLDGEOM)) {
  case TM_SPHERICAL: fmt = "%s (spherical %s view)"; break;
  case TM_HYPERBOLIC: fmt = "%s (hyperbolic %s view)"; break;
  case TM_EUCLIDEAN: fmt = "%s (Euclidean%s view)";
    switch (dv->hmodel) {
    case VIRTUAL:
    case PROJECTIVE:
      extra = "";
      break;
    case CONFORMALBALL:
      extra = " inverted";
      break;
    }
    break;
  default: return;
  }
  sprintf(label, fmt, dv->name[1], extra);
  mgctxset(MG_WnSet, WN_NAME, label, WN_END, MG_END);
}

/*
 * Is this a good directory to select in the file browser by default?
 * Say it is if it contains the 3-char prefix of our space name.
 */
int
our_space_dir(char *dirname)
{
  char *sname, sub[4];
  if((sname = spacename(spaceof(WORLDGEOM))) == NULL)
    return 0;
  sprintf(sub, "%.3s", sname);
  return (int)(long)strstr(dirname, sub);
}
    

Geom *unitsphere(int n, ColorA *color);

#define N  50

Geom *
unitsphere(int n, ColorA *color)
{
  int i, j;
  float si[N+1], ci[N+1], sj[N+1], cj[N+1];
  Point3 verts[(N-2 + N)*N];
  short vcounts[N-2 + N/2];
  short ccounts[N-2 + N/2];
  Point3 *p;
  short *vc;
  Geom *g;
  float excess = 1. / cos(M_PI / n);
 
  for(i = 0; i <= n; i++) {
    float t = i * 2*M_PI / n;
    sj[i] = sin(t) * excess;
    cj[i] = cos(t) * excess;
    t = i * M_PI / n;
    si[i] = sin(t);
    ci[i] = cos(t);
  }

  memset(ccounts, 0, sizeof(ccounts));
  ccounts[0] = color ? 1 : 0;

  /* Construct n-2 parallels */
  p = verts;
  vc = vcounts;
  for(i = 1; i < n-1; i++) {
    float y = ci[i];
    float r = si[i];

    *vc++ = -n;		/* Closed line with n vertices */
    for(j = 0; j < n; j++) {
      p->x = r*cj[j];
      p->y = y;
      p->z = r*sj[j];
      p++;
    }
  }
  /* Construct n/2 meridians, each a full circle */
  for(j = 0; j < n/2; j++) {
    float s = sj[j];
    float c = cj[j];

    *vc++ = -2*n;
    for(i = 0; i < n; i++) {
      p->x = c*si[i];
      p->y = ci[i];
      p->z = s*si[i];
      p++;
    }
    for(i = n; i > 0; i--) {
      p->x = -c*si[i];
      p->y = ci[i];
      p->z = -s*si[i];
      p++;
    }
  }
  g = GeomCreate("vect",
		 CR_NVECT,	n-2 + n/2,
		 CR_VECTC,	vcounts,
		 CR_COLRC,	ccounts,
		 CR_NVERT,	(n-2 + n) * n,
		 CR_POINT,	verts,
		 CR_NCOLR,	color ? 1 : 0,
		 CR_COLOR,	color,
		 CR_END);
  return g;
}

void
set_hsphere_draw(int id, int draw)
{
  static ColorA color = { .3, .2, .15, .7 };
  static Geom *hsphere = NULL;
  DView *dv;

  if (!ISCAM(id)) return;
  dv = (DView*)drawer_get_object(id);

  if (draw < 0) {
    /* toggle */
    draw = (dv->hsphere == NULL);
  }
  
  if (draw) {
    if (!hsphere) {
      hsphere = unitsphere(16, &color);
    }
    dv->hsphere = hsphere;
  } else {
    dv->hsphere = NULL;
  }
  ui_maybe_refresh(id);
}

NDcam *
NDcluster(char *name)
{
  NDcam *c;

  for(c = drawerstate.NDcams; c != NULL; c = c->next)
    if(strcmp(c->name, name) == 0) return c;
  return NULL;
}

NDcam *
NDnewcluster(char *name)
{
  NDcam *c;

  c = NDcluster(name);
  if(c == NULL) {
    c = OOGLNewE(NDcam, "NDcam cluster");
    c->name = strdup(name);
    c->C2W = TmNIdentity(TmNCreate(drawerstate.NDim,drawerstate.NDim,NULL));
    c->W2C = NULL;
    c->next = drawerstate.NDcams;
    drawerstate.NDcams = c;
  }
  return c;
}

void
NDdeletecluster(NDcam *c)
{	/* Add this someday.  Note that there may be other references to this cluster. XXX */
}

LDEFINE(ND_axes, LLIST,
	"(ND-axes CAMID [CLUSTERNAME [Xindex Yindex Zindex [Windex]]])\n\
	In our model for N-D viewing (enabled by (dimension)), objects in\n\
	N-space are viewed by N-dimensional \"camera clusters\".\n\
	Each real camera window belongs to some cluster, and shows &\n\
	manipulates a 3-D axis-aligned projected subspace of the N-space seen\n\
	by its cluster.  Moving one camera in a cluster affects its siblings.\n\
\n	The ND-axes command configures all this.  It specifies a camera's\n\
	cluster membership, and the set of N-space axes which become the\n\
	3-D camera's X, Y, and Z axes.  Axes are specified by their indices,\n\
	from 1 to N for an N-dimensional space.  Cluster CLUSTERNAME is\n\
	implicitly created if not previously known.\n\
	In principle it is possible to map the homogeneous component\n\
	of a conformal 4 point to some other index; this would be done\n\
	by specifying 0 for one of Xindex, Yindex or Zindex and giving\n\
	Windex some positive value. This is probably not useful because\n\
	Geomview does not support non-Euclidean geometries for in higher\n\
	dimensions.\n\
	\n\
	To read a camera's configuration, use \"(echo (ND-axes CAMID))\".\n\
        The return value is an array of 4 integers, the last one should\n\
	be 0.")
{
  int axes[4];
  char *camname, *clustername = NULL;
  int i, cam;
  DView *dv;

  axes[0] = axes[1] = axes[2] = -1;
  axes[3] = 0;
  LDECLARE(("ND-axes", LBEGIN,
	    LSTRING, &camname,
	    LOPTIONAL,
	    LSTRING, &clustername,
	    LINT, &axes[0],
	    LINT, &axes[1],
	    LINT, &axes[2],
	    LINT, &axes[3],
	    LEND));
  if((cam = drawer_idbyname(camname)) == NOID ||
     (dv = (DView *)drawer_get_object(cam)) == NULL || !ISCAM(dv->id)) {
    OOGLError(0, "ND-axes: unknown camera %s", camname);
    return Lnil;
  } else if(axes[0] < 0) {
    /* return the current ND configuratino of cam */
    LList *l = NULL;
    if(dv->cluster) {
      l = LListAppend(NULL, LTOOBJ(LSTRING)(&dv->cluster->name));
      for(i = 0; i < 4; i++)
	l = LListAppend(l, LNew( LINT, &dv->NDPerm[i] ));
    }
    return LNew( LLIST, &l );
  } else {
    /* possibly generate a new cluster, or add to an existing
     * cluster. NOTE: we first reset the camera to its default state.
     */
    NDcam *c = NDnewcluster(clustername);

    for(i = 0; i < 4; i++) {
      if (axes[i] < 0) {
	const char names[4] = { 'X', 'Y', 'Z', 'W' };
	OOGLError(1,
		  "ERROR: bogus ND-axes specification (%d %d %d %d), %c-entry "
		  "must not be negative.\n",
		  axes[0], axes[1], axes[2], axes[3], names[i]);
	return Lnil;
      }
    }
    NDdeletecluster(dv->cluster); /* a no-op yet */
    dv->cluster = c;
    for(i = 0; i < 4; i++) {
      dv->NDPerm[i] = axes[i];
    }

    CamReset(dv->cam);

    dv->changed |= CH_GEOMETRY;
    return Lt;
  }
}

static void set_dimension(int d, TransformN **Tp, TransformN **Tinvp)
{
  if(Tinvp) {
    TmNDelete(*Tinvp);
    *Tinvp = NULL;
  }
  if(Tp == NULL) {
    return;
  }
  if(d == 0) {
    TmNDelete(*Tp);
    *Tp = NULL;
  } else {
    if(TmNGetSize(*Tp, NULL,NULL) > d) {
      TmNDelete(*Tp);
      *Tp = NULL;
    }
    *Tp = TmNPad(*Tp, d, d, *Tp);
  }
}

LDEFINE(dimension, LLIST,
	"(dimension [N])\n\
	Sets or reads the space dimension for N-dimensional viewing.\n\
	(Since calculations are done using homogeneous coordinates,\n\
	this means matrices are (N+1)x(N+1).)\n\
	With no arguments, returns the current dimension, or 0 if\n\
	N-dimensional viewing has not been enabled. Note that N has to be"
	"at least 4 to enable ND-viewing, otherwise ND-viewing will be"
	"disabled.")
{
  int i, d = -1;

  LDECLARE(("dimension", LBEGIN,
	    LOPTIONAL,
	    LINT, &d,
	    LEND));

  if (d < 0) {
    d = drawerstate.NDim-1;
    if(d < 0) {
      d = 0;
    }
    return LNew(LINT, &d);
  } else {
    NDcam *c;
    if (d < 4) { /* we do not allow low-dimension ND-view */
      d = 0;
      for(i = 0; i < dview_max; i++) {
	if(dview[i] && dview[i]->cluster) {
	  NDdeletecluster(dview[i]->cluster);
	  dview[i]->cluster = NULL;
	}
      }
    } else {
      d++;	/* Include homogeneous coordinate drawerstate.NDim */
    }
    if(drawerstate.NDim != d) {
      for(c = drawerstate.NDcams; c != NULL; c = c->next) {
	set_dimension(d, &c->C2W, &c->W2C);
      }
      for(i = 0; i < dview_max; i++) {
	if(dview[i]) {
	  dview[i]->changed = CH_GEOMETRY;
	}
      }
      for(i = 0; i < dgeom_max; i++) {
	if(dgeom[i]) {
	  set_dimension(d, &dgeom[i]->NDT, &dgeom[i]->NDTinv);
	  dgeom[i]->bboxvalid = false;
	  dgeom[i]->changed = CH_GEOMETRY;
	}
      }
      drawerstate.changed = true;
      drawerstate.NDim = d;
    }
  }
  return Lt;
}

LDEFINE(ND_xform, LTRANSFORMN,
        "(ND-xform OBJID [ntransform { idim odim ... }]\n"
	"Concatenate the given ND-transform with the current "
	"ND-transform of the object (apply the ND-transform to "
	"object ID, as opposed to simply setting its ND-transform)."
	"Note that ND-transforms have their homogeneous coordinate at "
	"index 0, while 3D transform have it at index 3.")
{
  int id;
  TransformN *T = NULL;
  TmNStruct *ts = NULL;
  DObject *obj;
  NDcam *cl = NULL;

  LDECLARE(("ND-xform", LBEGIN,
	    LID, &id,
	    LTRANSFORMN, &ts,
	    LEND));

  if((obj = drawer_get_object(id)) == NULL || drawerstate.NDim == 0)
    return Lnil;

  if(ISGEOM(obj->id)) {
    T = ((DGeom *)obj)->NDT;
  } else if(ISCAM(obj->id) && (cl = ((DView *)obj)->cluster)) {
    T = cl->C2W;
  }

  if (!T) {
    T = REFGET(TransformN, ts->tm);
  } else {
    TmNConcat(ts->tm, T, T);
  }

  if(ISGEOM(obj->id)) {
    DGobj(obj)->NDT = T;
    GeomSet(DGobj(obj)->Item, CR_NDAXIS, DGobj(obj)->NDT, CR_END);
    obj->changed |= CH_GEOMETRY;
  } else if(cl != NULL) {
    cl->C2W = T;
    drawerstate.changed = true;
  }
  TmIdentity(obj->Incr);
  obj->redraw = true;
  obj->moving = (obj->updateproc != NULL);
  return Lt;
}

LDEFINE(ND_xform_set, LTRANSFORMN,
	"(ND-xform-set OBJID [ntransform { idim odim  ... }])\n\
	Sets the N-D transform of the given object.\n\
	In dimension N, this is an (N+1)x(N+1) matrix, so in that case\n\
	idim and odim are expected to be both equal to (N+1). Note that\n\
	all cameras in a camera-cluster have the same N-D transform.\n"
	"Note that ND-transforms have their homogeneous coordinate at "
	"index 0, while 3D transform have it at index 3.")
{
  int id;
  DObject *obj;
  TmNStruct *ts = NULL;
  TransformN *T = NULL;
  NDcam *cl = NULL;

  LDECLARE(("ND-xform-set", LBEGIN,
	    LID, &id,
	    LOPTIONAL,
	    LTRANSFORMN, &ts,
	    LEND));

  if((obj = drawer_get_object(id)) == NULL || drawerstate.NDim == 0)
    return Lnil;

  if(ISGEOM(obj->id)) {
    T = ((DGeom *)obj)->NDT;
  } else if(ISCAM(obj->id) && (cl = ((DView *)obj)->cluster)) {
    T = cl->C2W;
  }

  /* (ND-xform-set id transformn { ... }) -> set transform */
  TmNDelete(T);
  if(ISGEOM(obj->id)) {
    ((DGeom *)obj)->NDT = REFGET(TransformN, ts->tm);
    GeomSet(DGobj(obj)->Item, CR_NDAXIS, DGobj(obj)->NDT, CR_END);
    obj->changed |= CH_GEOMETRY;
  } else if(cl != NULL) {
    cl->C2W = REFGET(TransformN, ts->tm);
    drawerstate.changed = true;
  }
  return Lt;
}

LDEFINE(ND_xform_get, LTRANSFORMN,
	"(ND-xform-get ID [from-ID])\n\
	Returns the N-D transform of the given object in the coordinate\n\
	system of from-ID (default \"universe\"), in the sense\n\
	<point-in-ID-coords> * Transform = <point-in-from-ID-coords>"
	"Note that ND-transforms have their homogeneous coordinate at "
	"index 0, while 3D transform have it at index 3.")
{
  int from_id;
  int to_id = UNIVERSE;
  TmNStruct *ts;
  TransformN *tm;
  
  extern TransformN *drawer_get_ND_transform(int from_id, int to_id);

  LDECLARE(("ND-xform-get", LBEGIN,
	    LID, &from_id,
	    LOPTIONAL,
	    LID, &to_id,
	    LEND));
  if((tm = drawer_get_ND_transform(from_id, to_id)) == NULL)
    return Lnil;
  ts = OOGLNewE(TmNStruct, "TmN");
  ts->h = NULL;
  ts->tm = tm;
  return LNew(LTRANSFORMN, &ts); 
}


LDEFINE(ND_color, LLIST,
	"(ND-color CAMID [ (( [ID] (x1 x2 x3 ... xN) v r g b a   v r g b a  ... )\n\
	((x1 ... xN)  v r g b a  v r g b a ...) ...)] )\n\
	Specifies a function, applied to each N-D vertex, which determines the\n\
	colors of N-dimensional objects as shown in camera CAMID.\n\
	Each coloring function is defined by a vector (in ID's coordinate system)\n\
	[x1 x1 ... xN] and by a sequence of value (v)/color(r g b a) tuples,\n\
	ordered by increasing v.  The inner product v = P.[x] is linearly\n\
	interpolated in this table to give a color.\n\
	If ID is omitted, the (xi) vector is assumed in universe coordinates.\n\
	The ND-color command specifies a list of such functions; each vertex\n\
	is colored by their sum (so e.g. green intensity could indicate\n\
	projection along one axis while red indicated another.\n\
	An empty list, as in (ND-color CAMID ()), suppresses coloring.\n\
	With no second argument, (ND-color CAMID) returns that camera's\n\
	color-function list.\n\
	Even when coloring is enabled, objects tagged with the \"keepcolor\"\n\
	appearance attribute are shown in their natural colors.\n")
{
  int i, j, k, id;
  DView *dv;
  LList noarg;
  LList *l = &noarg, *ents;
  cmap *cm;
  cent *ce;
  char *err;
  int nents, nfields, ncolors;

  LDECLARE(("ND-color", LBEGIN,
	    LID, &id,
	    LOPTIONAL,
	    LLITERAL,
	    LLIST, &l,
	    LEND));

  if((dv = (DView *)drawer_get_object(id)) == NULL || !ISCAM(dv->id)) {
    OOGLError(0, "ND-color: expected camera name");
    return Lnil;
  }

  if(l == &noarg) {
    ents = NULL;
    cm = dv->NDcmap;
    for(i = 0; i < dv->nNDcmap; i++, cm++) {
      DObject *dobj = drawer_get_object(cm->coords);
      char *dname = (dobj && dobj->name[1]) ? dobj->name[1] : "universe";

      dname = strdup(dname);
      l = LListAppend(NULL, LNew(LSTRING, (char *)&dname));
      l = LListAppend(l, LMakeArray(LFLOAT,
				    (char *)(cm->axis->v+1), cm->axis->dim-1));
      for(k = VVCOUNT(cm->cents), ce = VVEC(cm->cents, cent); --k > 0; ce++) {
	l = LListAppend(l, LNew(LFLOAT, (char *)&ce->v));
	l = LListAppend(l, LNew(LFLOAT, (char *)&ce->c.r));
	l = LListAppend(l, LNew(LFLOAT, (char *)&ce->c.g));
	l = LListAppend(l, LNew(LFLOAT, (char *)&ce->c.b));
	l = LListAppend(l, LNew(LFLOAT, (char *)&ce->c.a));
      }
      ents = LListAppend(ents, LNew(LLIST, &l));
    }
    return LNew(LLIST, &ents);
  }

  ents = l;

  nents = LListLength(ents);
  if(nents > MAXCMAP) {
    OOGLError(0, "Only %d colormaps allowed per camera; using first %d of %d",
	      MAXCMAP, MAXCMAP, nents);
    nents = MAXCMAP;
  }

  cm = dv->NDcmap;
  for(i = 1; i <= nents; i++, cm++) {

    LObject *ent = LListEntry(ents, i);
    LList *entlist, *axis;
    int dim;
    /*
     * Each component of the 'ents' list looks like:
     *  LLIST         ---  v0,r0,g0,b0,a0, v1,r1,g1,b1,a1, ...
     *    x1,x2,x3,...
     */
    if(! LFROMOBJ(LLIST)(ent, &entlist)) {
      err = "ND-color: expected list of lists";
      goto no;
    }

    cm->coords = UNIVERSE;
    if( entlist->car->type == LSTRING ) {
      cm->coords = drawer_idbyname(LSTRINGVAL(entlist->car));
      if(cm->coords == NOID) {
	OOGLError(0, "ND-color: unknown coordinate system %s, using 'universe'",
		  LSTRINGVAL(entlist->car));
	cm->coords = UNIVERSE;
      }
      entlist = entlist->cdr;
    }

    if(! LFROMOBJ(LLIST)(entlist->car, &axis)) {
      err = "ND-color: expected N-D projection axis";
      goto no;
    }

    dim = LListLength(axis);
    cm->axis = cm->axis ? HPtNPad(cm->axis, dim + 1, cm->axis)
      : HPtNCreate(dim+1, NULL);
    /* The projection axis is a vector, so its homogeneous component is zero */
    cm->axis->v[0] = 0;
    /* Extract the real components */
    for(j = 1; j < dim+1; j++) {
      if(!LFROMOBJ(LFLOAT)(LListEntry(axis, j), &cm->axis->v[j])) {
	err = "Non-numeric entry in projection axis?";
	goto no;
      }
    }

    entlist = entlist->cdr;	/* Look at the remainder of the list */
    nfields = LListLength(entlist);
    ncolors = nfields / 5;
    if(nfields % 5 != 0 || nfields == 0) {
      err = "Each colormap should contain a multiple of 5 numbers: v0 r0 g0 b0 a0  v1 r1 g1 b1 a1 ...";
      goto no;
    }

    vvneeds(&cm->cents, ncolors+1);
    ce = VVEC(cm->cents, cent);


    for(j = 1; j <= nfields; j += 5, ce++) {
      ce->interp = 1;
      if(!LFROMOBJ(LFLOAT)(LListEntry(entlist, j), &ce->v) ||
	 !LFROMOBJ(LFLOAT)(LListEntry(entlist, j+1), &ce->c.r) ||
	 !LFROMOBJ(LFLOAT)(LListEntry(entlist, j+2), &ce->c.g) ||
	 !LFROMOBJ(LFLOAT)(LListEntry(entlist, j+3), &ce->c.b) ||
	 !LFROMOBJ(LFLOAT)(LListEntry(entlist, j+4), &ce->c.a) ) {
	err = "Non-numeric entry in colormap?";
	goto no;
      }
    }
    VVCOUNT(cm->cents) = ncolors+1;
    ce = &VVEC(cm->cents, cent)[ncolors];
    *ce = ce[-1];
    ce->v = 1e20;		/* Huge value terminates list */
  }
  dv->nNDcmap = nents;
  dv->changed |= CH_GEOMETRY;
  return Lt;

 no:
  OOGLError(0, err);
  return Lnil;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
