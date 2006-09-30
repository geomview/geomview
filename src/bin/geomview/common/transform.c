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

#include <stdio.h>
#include <math.h>
#include "geomclass.h"
#include "sphere.h"
#include "mg.h"
#include "drawer.h"
#include "ui.h"
#include "transform.h"
#include "transformn.h"
#include "pickP.h"
#include "lang.h"
#include "bbox.h"

/* NOTE: This file consists of things which Nathaniel Thurston added
   during his re-doing of the motion stuff in summer 1992.  This stuff
   should eventually be moved into other files --- some as part of
   geomview, others as part of the transform library.
   mbp Wed Aug 19 00:44:02 1992 */

#define ERROR(fmt, arg) OOGLError(1, fmt, arg)

Motion *allMotions = NULL;

int is_descendant(int id, int ancestor_id);
int id_exists(int id);
int get_parent(int id);
void set_space(int new);
void apply_ND_transform(Transform deltaT, int moving, int center, int frame);
void drawer_set_ND_xform(int id, TransformN *T);
TransformN *drawer_get_ND_transform(int from_id, int to_id);

static void _translate(float amount[3], Point *pt, Transform T,
				int space, int frame);
static void _translate_scaled(float amount[3], Point *pt, Transform T,
				int space, int frame);
static void euclidean_translate(float amount[3], Point *pt, Transform T,
				int space, int frame);
static void euclidean_translate_scaled(float amount[3], Point *pt, Transform T,
 				 int space, int frame);
static void hyperbolic_translate(float amount[3], Point *pt, Transform T,
				int space, int frame);
static void hyperbolic_translate_scaled(float amount[3], Point *pt, Transform T,
 				 int space, int frame);
static void spherical_translate(float amount[3], Point *pt, Transform T,
				int space, int frame);
static void spherical_translate_scaled(float amount[3], Point *pt, Transform T,
 				int space, int frame);
static void _rotate(float amount[3], Point *pt, Transform T,
				int space, int frame);
static void _scale(float amount[3], Point *pt, Transform T,
				int space, int frame);

/* 
 * This should DEFINITELY go somewhere else, but I'm not sure where
 * Geom *id_bbox(int objID, int coordsysID)
 * objID = id of the object to make the bounding box for
 * coordsysID = coordinate system in which to create the bounding box
 * -Celeste
 */
Geom *id_bbox(int geomID, int coordsysID) {
  int i;
  DGeom *geomObj;
  Geom *bbox = NULL, *other_bbox;
  Transform geom2coordsys;

  if (!ISGEOM(geomID)) {
    OOGLError(1, "%s\n%s",
	      "Cannot create the bounding box of something which is",
	      "not a geom.");
    return NULL;
  }
  MAYBE_LOOP(geomID, i, T_GEOM, DGeom, geomObj) {
    drawer_get_transform(geomObj->id, geom2coordsys, coordsysID);
    other_bbox = GeomBound(geomObj->Lgeom, geom2coordsys);
    if (bbox == NULL) bbox = other_bbox;
    else {
      BBoxUnion3((BBox *)bbox, (BBox *)other_bbox, (BBox *)bbox);
      GeomDelete(other_bbox);
    }
  }
  return bbox;
}

Geom *id_bsphere(int geomID, int coordsysID) {
  int i;
  DGeom *geomObj;
  Geom *sphere = NULL, *other_sphere, *obj;
  Transform geom2coordsys;

  if (!ISGEOM(geomID)) {
    OOGLError(0, "id_bsphere: %s\n%s",
	      "Cannot create the bounding sphere of something which is",
	      "not a geom.");
    return NULL;
  }
  MAYBE_LOOP(geomID, i, T_GEOM, DGeom, geomObj) {
    drawer_get_transform(geomObj->id, geom2coordsys, coordsysID);
    if (geomObj->Lgeom == NULL) continue;
    GeomGet(geomObj->Lgeom, CR_GEOM, &obj);
    other_sphere = GeomBoundSphere(obj, geom2coordsys, drawerstate.space);
    if (sphere == NULL) sphere = other_sphere;
    else {
      SphereUnion3((Sphere *)sphere, (Sphere *)other_sphere, 
		   (Sphere *)sphere);
      GeomDelete(other_sphere);
    }
  }
  return sphere;
}

void drawer_transform(
  int moving_id, int center_id, int frame_id,
  int transform_type,
  float amount[3],
  float timeunit,
  char *repeat_type,
  int smoothanim
) {
  Motion motion;
  int i;
  float augment;

  motion.moving_id = real_id(moving_id);
  motion.center_id = real_id(center_id);
  if (motion.center_id == SELF)
    motion.center_id = motion.moving_id;
  motion.frame_id = real_id(frame_id);
  if (motion.frame_id == SELF)
    motion.frame_id = motion.moving_id;
  motion.amount[0] = amount[0];
  motion.amount[1] = amount[1];
  motion.amount[2] = amount[2];
  motion.timeunit = timeunit;
  motion.timeleft = 0;
  motion.smooth = 0;
  switch (transform_type) {

  case ROTATE_KEYWORD:
    motion.transform = _rotate;
    break;

  case TRANSLATE_KEYWORD:
    motion.transform = _translate;
    break;
  case E_TRANSLATE_KEYWORD:
    motion.transform = euclidean_translate;
    break;
  case H_TRANSLATE_KEYWORD:
    motion.transform = hyperbolic_translate;
    break;
  case S_TRANSLATE_KEYWORD:
    motion.transform = spherical_translate;
    break;

  case TRANSLATE_SCALED_KEYWORD:
    motion.transform = _translate_scaled;
    break;
  case E_TRANSLATE_SCALED_KEYWORD:
    motion.transform = euclidean_translate_scaled;
    break;
  case H_TRANSLATE_SCALED_KEYWORD:
    motion.transform = hyperbolic_translate_scaled;
    break;
  case S_TRANSLATE_SCALED_KEYWORD:
    motion.transform = spherical_translate_scaled;
    break;

  case SCALE_KEYWORD:
    /* Take logarithms so we can scale incrementally.
     * Refuse to scale down to zero, or to change sign.
     */
    for(i = 0; i < 3; i++)
	motion.amount[i] = amount[i] > 0 ? log(amount[i]) : 0;
    motion.transform = _scale;
    break;

  default:
    ERROR("Undefined transform type %d", transform_type);
    return;
  }
  
  augment = 0;
  if(!strcmp(repeat_type, "transform-incr") && uistate.inertia > 1) {
    augment = 1 - 1./uistate.inertia;
  }

  delete_like_motions(&motion, augment);

  if (!strcmp(repeat_type, "transform-set"))
    set_motion(&motion);
  else {
    /* Avoid applying ineffective motions */
    if(motion.amount[0]==0 && motion.amount[1]==0 && motion.amount[2]==0)
	return;
    if (!strcmp(repeat_type, "transform")) {
	if(motion.timeunit > 0) {
	    motion.timeleft = motion.timeunit;
	    motion.smooth = smoothanim;
	    insert_motion(&motion);
	} else {
	    apply_motion(&motion, motion.timeunit);
	}
    } else if (!strcmp(repeat_type, "transform-incr")) {
	insert_motion(&motion);
    } else
	ERROR("unknown transform applier %s", repeat_type);
  }
}


/* 
 * center = NULL or center = "center" ---> position-[at | toward] the
 *	center of the bounding box of ref_id
 * center = "origin" ---> position-[at | toward] the origin of 
 * 	ref_id's coordinate system
 */
void drawer_position( int moving_id, int ref_id, char *position_type,
		     char *center) {
  int i;
  DObject *moveObj;
  Transform ref2w, w2moving, T;
  HPoint3 min, max;
  int use_origin;

  /* ptWorld = reference point in World coordinates, 
   * ptMoving = reference point in coordinate system of moving object */
  Point ptWorld, ptMoving;
  Geom *bbox = NULL;

  if (ref_id == SELF) 
    ref_id = moving_id;

  if (center == NULL || !strcmp(center, "center")) use_origin = 0;
  else if (!strcmp(center, "origin")) use_origin = 1;
  else {
    OOGLError(1, "undefined center %s", center);
    return;
  }
  if (!ISGEOM(ref_id)) use_origin = 1;

  /* First, get the point to which we are relatively positioning in
   * world coordinates (using world coordinates is somewhat arbitrary -
   * there just needs to be an intermediate coordinate system). */
  if (use_origin) {
    drawer_get_transform(ref_id, ref2w, WORLDGEOM);
    ptWorld.x = ptWorld.y = ptWorld.z = 0; 
    ptWorld.w = 1; 
    HPt3Transform(ref2w, &ptWorld, &ptWorld);
  } else {
    if (spaceof(ref_id) != TM_EUCLIDEAN)
      OOGLError(1, "Computing bounding box while in non-eudlidean space");
    bbox = id_bbox(ref_id, WORLDGEOM);
    BBoxMinMax((BBox *)bbox, &min, &max);
    GeomDelete(bbox);
    HPt3Normalize(&min, &min);
    HPt3Normalize(&max, &max);
    ptWorld.x = (min.x + max.x) / 2.0;
    ptWorld.y = (min.y + max.y) / 2.0;
    ptWorld.z = (min.z + max.z) / 2.0;
    ptWorld.w = 1.0;
  }

  MAYBE_LOOP(moving_id, i, T_NONE, DObject, moveObj) {

    drawer_get_transform(WORLDGEOM, w2moving, moveObj->id);
    HPt3Transform(w2moving, &ptWorld, &ptMoving);

    if (!strcmp(position_type, "position-at")) {
      TmSpaceTranslateOrigin(T, &ptMoving, spaceof(moving_id));
    }
    else if (!strcmp(position_type, "position-toward"))
      TmCarefulRotateTowardZ(T, &ptMoving);
    else if (!strcmp(position_type, "position"))
      drawer_get_transform(ref_id, T, moveObj->id); 
    else {
      ERROR("undefined position type %s", position_type);
      return;
    }
    drawer_post_xform(moveObj->id, T);
  }
}

void set_motion(Motion *motion)
{
  TransformStruct ts;
  ts.h = NULL;
  TmIdentity(ts.tm);
  gv_xform_set(motion->moving_id, &ts);
    /* Note: the following resets the *entire* N-D xform to the identity,
     * not just that in the subspace we're looking through.
     * Might want to have finer control.  XXX - 7/28/93, slevy & holt
     */
  drawer_set_ND_xform(motion->moving_id, NULL);
  apply_motion(motion, motion->timeunit);
}

void apply_motion(Motion *motion, float dt)
{
  Transform T_m, T_c, T_f, T_l;
  Transform T_R, T_Rinv, T;
  Point pt;
  float amount[3];
  float tscale;

  if (motion->moving_id == UNIVERSE
   || !id_exists(motion->moving_id)
   || !id_exists(motion->center_id)
   || !id_exists(motion->frame_id)) {
    motion->timeleft = 0;	/* Prevent recursion */
    delete_like_motions(motion, 0.);
    return;
  }

  drawer_get_transform(motion->moving_id, T_m, UNIVERSE);
  drawer_get_transform(motion->center_id, T_c, UNIVERSE);
  drawer_get_transform(motion->frame_id, T_f, UNIVERSE);


  /* If the frame is in a different space than the center, then
     translate the frame to the center of the universe.  This is
     really a hack meant to deal with Euclidean cameras looking at
     models of hyperbolic space; it arranges for the frame to appear
     at the center of the universe in case it lies outside the model. */
  if (spaceof(motion->frame_id) != spaceof(motion->center_id)) {
    pt.x = pt.y = pt.z = 0; pt.w = 1;
    TmInvert(T_f, T);
    PtTransform(T, &pt, &pt);
    TmSpaceTranslateOrigin(T, &pt, spaceof(motion->frame_id));
    TmConcat(T, T_f, T_f);
  }
  
  /* Find the location of the origin of T_c in T_f */
  /* location = origin . T_c . Inverse[T_f] */
  pt.x = pt.y = pt.z = 0; pt.w = 1; 
  if (motion->frame_id != motion->center_id) {
    TmInvert(T_f, T);
    TmConcat(T_c, T, T);
    PtTransform(T, &pt, &pt);

    /* Move T_f so that its origin coincides with that of T_c */
    TmSpaceTranslateOrigin(T, &pt, spaceof(motion->moving_id));
    TmConcat(T, T_f, T_f);
  }

  /* Find the transformation from T_f to T_m */
  TmInvert(T_f, T_R);
  TmConcat(T_m, T_R, T_R);

  /* Compute the local transformation.  pt may be used for scaling */
  /* If a time unit was specified along with the motion,
   * scale the motion according to elapsed "real" time.
   * If no time unit provided, apply exactly the given motion.
   */
  if (motion->timeunit < .00001) {
    if (motion->timeunit) 
      fprintf(stderr, "motion->timeunit = %x\n", 
	      *(unsigned *)(void *)&motion->timeunit);
    motion->timeunit = 0.0;
  } 
  tscale = (motion->timeunit != 0.) ? dt / motion->timeunit : 1.0;

  if(motion->timeleft != 0) {	/* Animating? */
    float t0 = motion->timeleft / motion->timeunit;
    float t1 = t0 - tscale;
    if(t1 < 0) t1 = 0;
    tscale = motion->smooth ? t0*t0*(3 - 2*t0) - t1*t1*(3 - 2*t1) : t0 - t1;
  }
  amount[0] = motion->amount[0] * tscale;
  amount[1] = motion->amount[1] * tscale;
  amount[2] = motion->amount[2] * tscale;
  pt.x = pt.y = pt.z = 0; pt.w = 1;
  HPt3Transform(T_R, &pt, &pt);
  (motion->transform)(amount, &pt, T_l, spaceof(motion->moving_id),
		      motion->frame_id);

#ifdef notdef
  /* No, this is wrong!  -slevy */
  /*
   * We want to consider the object hierararchy as being rooted at 
   * frame_id.  The practical effect of this consideration is that
   * transformations act backwards when the moving id is an ancestor
   * of the frame id.
   */
  if (is_descendant(motion->frame_id, motion->moving_id)) {
    TmInvert(T_l, T);
    TmCopy(T, T_l);
  }
#endif
  
  /* Conjugate the local transform to put it into the coordinate system of T_m*/
  TmInvert(T_R, T_Rinv);
  TmConcat(T_R, T_l, T);
  TmConcat(T, T_Rinv, T);

  if (!finite(T[0][0])) {
    fprintf(stderr, "Matrix is not finite!\n");
  } else {
	/* Apply the matrix to the moving object */
    drawer_post_xform(motion->moving_id, T);
  }

  if(drawerstate.NDim > 0) {
    /* Apply subspace transform to the N-dimensional matrices too. */
    apply_ND_transform(T_l, motion->moving_id, motion->center_id, motion->frame_id);
  }

  if(motion->timeleft) {
    motion->timeleft -= dt;
    if(motion->timeleft <= 0) {
	motion->timeleft = 0;	/* Prevent recursion */
	delete_motion(motion);
    }
  }
}

static void _translate(float amount[3], Point *pt, Transform T, int space,
		int frame)
{
  TmSpaceTranslate(T, amount[0], amount[1], amount[2], space);
}

static void euclidean_translate(float amount[3], Point *pt, Transform T, int space,
			 int frame)
{
  TmTranslate(T, amount[0], amount[1], amount[2]);
}

static void hyperbolic_translate(float amount[3], Point *pt, Transform T, int space,
			  int frame)
{
  TmHypTranslate(T, amount[0], amount[1], amount[2]);
}

static void spherical_translate(float amount[3], Point *pt, Transform T, int space,
			 int frame)
{
  TmSphTranslate(T, amount[0], amount[1], amount[2]);
}

static void _translate_scaled(float amount[3], Point *pt, Transform T, int space,
		       int frame)
{
  double dist = 1;
  float scaled_amount[3];

  switch (space) {
  default:
  case TM_EUCLIDEAN:
    dist = sqrt(pt->x * pt->x + pt->y * pt->y + pt->z * pt->z)
	    + scaleof(frame) * drawerstate.motionscale;
    break;
  case TM_HYPERBOLIC:
    /* NYI */
    break;
  case TM_SPHERICAL:
    /* NYI */
    break;
  }
  scaled_amount[0] = amount[0] * dist;
  scaled_amount[1] = amount[1] * dist;
  scaled_amount[2] = amount[2] * dist;
  _translate(scaled_amount, pt, T, space, frame);
}

static void euclidean_translate_scaled(float amount[3], Point *pt, Transform T,
				int space, int frame)
{
  _translate_scaled(amount, pt, T, TM_EUCLIDEAN, frame);
}

static void hyperbolic_translate_scaled(float amount[3], Point *pt, Transform T,
				 int space, int frame)
{
  hyperbolic_translate(amount, pt, T, space, frame); /* scaling is NYI */
}

static void spherical_translate_scaled(float amount[3], Point *pt, Transform T,
				int space, int frame)
{
  spherical_translate(amount, pt, T, space, frame); /* scaling is NYI */
}

  
static void _rotate(float amount[3], Point *pt, Transform T, int space, int frame)
{
  float dist = sqrt(amount[0]*amount[0]
		    + amount[1]*amount[1]
		    + amount[2]*amount[2]);
  TmRotate(T, dist, (Point3 *)amount);
}

static void _scale(float amount[3], Point *pt, Transform T, int space, int frame)
{
  TmScale(T, exp(amount[0]),exp(amount[1]),exp(amount[2]));
}

void insert_motion(Motion *motion)
{
  Motion *copy = OOGLNew(Motion);
  *copy = *motion;
  copy->next = allMotions;
  allMotions = copy;
}

void apply_all_motions(float dt)
{
  Motion *m, *next;
  for (m = allMotions; m; m = next) {
    next = m->next;
    apply_motion(m, dt);
  }
}

void delete_motion(Motion *m)
{
  Motion **s;

  for(s = &allMotions; *s != m && *s != NULL; s = &(*s)->next)
    ;
  if(*s == m) {
    *s = m->next;
    if(m->timeleft)		/* If we're in mid-animation, ... */
	apply_motion(m, 1e10);	/* finish anim step before cancelling. */
				/* It will free itself. */
    else
	OOGLFree(m);
  }
}

static int is_translation(Motion *m)
{
  return m->transform != _rotate && m->transform != _scale;
}

void delete_like_motions(Motion *m, float augment)
{
  Motion **s = &allMotions;
  Motion *n;
  int was_translation = is_translation(m);

  while (*s) {
    if ((*s)->moving_id == m->moving_id
/*   && (*s)->center_id == m->center_id --- this makes motion more intuitive
     && (*s)->frame_id == m->frame_id   --- njt*/
     && ((*s)->transform == m->transform ||
		/* Consider scaled and non-scaled translations equivalent */
	 (was_translation && is_translation(*s))) ) {

      if(augment != 0) {
	/* Incorporate some fraction of the existing motion into the
	 * motion that is replacing it.
	 */
	float scl = augment * ((*s)->timeunit!=0 && m->timeunit!=0
				? m->timeunit / (*s)->timeunit : 1);
	m->amount[0] = (1-augment)*m->amount[0] + scl*(*s)->amount[0];
	m->amount[1] = (1-augment)*m->amount[1] + scl*(*s)->amount[1];
	m->amount[2] = (1-augment)*m->amount[2] + scl*(*s)->amount[2];
      }

      n = (*s)->next;
      if((*s)->timeleft)	/* If we're in mid-animation, */
	apply_motion(*s, 1e10); /* finish anim step before cancelling */
      else
	OOGLFree(*s);
      *s = n;
    } else {
      s = &(*s)->next;
    }
  }
}

void stop_motions(int id)
{
  Motion **s = &allMotions;
  Motion *n;
  while (*s) {
    if ((*s)->moving_id == id) {
	/* Should we finish partially-complete animations? */
      n = (*s)->next;
      OOGLFree(*s);
      *s = n;
    } else {
      s = &(*s)->next;
    }
  }
}
  
void do_motion(float dt)
{
  Motion *s,*next;
  for (s = allMotions; s; s = next) {
    next = s->next;
    apply_motion(s, dt);
  }
}

LDEFINE(zoom, LVOID,
       "(zoom           CAM-ID FACTOR)\n\
	Zoom CAM-ID, multiplying its field of view by FACTOR.\n\
	FACTOR should be a positive number.")
{
  float width;
  DView *dv;
  int id;
  float amount;
  LDECLARE(("zoom", LBEGIN,
	    LID, &id,
	    LFLOAT, &amount,
	    LEND));

  id = real_id(id);
  if (!ISCAM(id) || !(dv = (DView *)drawer_get_object(id))) {
    OOGLError(0, "zoom: Can only zoom a camera");
    return Lnil;
  }
  CamGet(dv->cam, CAM_HALFYFIELD, &width);
  CamSet(dv->cam, CAM_HALFYFIELD, width / amount, CAM_END);
  CamGet(dv->cam, CAM_FOV, &width);
  drawer_float(id, DRAWER_FOV, width);
  return Lt;
}

LDEFINE(ezoom, LVOID,
"(ezoom          GEOM-ID FACTOR)\n\
	Same as zoom but multiplies by exp(zoom).  Obsolete.")
{
  int id;
  float f;
  LDECLARE(("ezoom", LBEGIN,
	    LID, &id,
	    LFLOAT, &f,
	    LEND));
  gv_zoom( id, (float)exp((float)f) );
  return Lt;
}



LDEFINE(scale, LVOID,
"(scale          GEOM-ID FACTOR [FACTORY FACTORZ])\n\
	Scale GEOM-ID, multiplying its size by FACTOR.  The factors \n\
	should be positive numbers.  If FACTORY and FACTORZ are \n\
	present and non-zero, the object is scaled by FACTOR in x, by \n\
	FACTORY in y, and by FACTORZ in z.  If only FACTOR is present, \n\
	the object is scaled by FACTOR in x, y, and z.  Scaling only \n\
	really makes sense in Euclidean space.  Mouse-driven scaling in \n\
	other spaces is not allowed;  the scale command may be issued \n\
	in other spaces but should be used with caution because it may \n\
	cause the data to extend beyond the limits of the space.")
{
  TransformStruct ts;
  int id;
  float x, y = 0.0, z = 0.0;
  LDECLARE(("scale", LBEGIN,
	    LID, &id,
	    LFLOAT, &x,
	    LOPTIONAL,
	    LFLOAT, &y,
	    LFLOAT, &z,
	    LEND));

  if (!ISGEOM(id = real_id(id))) {
    OOGLError(0, "scale: Can only scale geometry.");    
    return Lnil; 
  }
  ts.h = NULL;
  if (y != 0.0 && z != 0.0) TmScale(ts.tm, x, y, z);
  else TmScale(ts.tm, x, x, x);
  gv_xform(id, &ts);
  if(drawerstate.NDim > 0) {
    TransformN *Tobj = drawer_get_ND_transform(id, get_parent(id));
    if(Tobj == NULL)
	Tobj = TmNCreate(drawerstate.NDim, drawerstate.NDim, NULL);
    drawer_set_ND_xform(id, CtmNScale(x, Tobj, Tobj));
  }
  return Lt;
}

LDEFINE(escale, LVOID,
"(escale          GEOM-ID FACTOR)\n\
	Same as scale but multiplies by exp(scale).  Obsolete.")
{
  int id;
  float f;
  LDECLARE(("escale", LBEGIN,
	    LID, &id,
	    LFLOAT, &f,
	    LEND));
  f = exp(f);
  gv_scale( id, f, f, f );
  return Lt;
}

void drawer_post_xform(int id, Transform T)
{
  TransformStruct ts;
  Transform N, NInv;
  DObject *dobj;
  if((dobj = drawer_get_object(id)) == NULL) return;

  if (ISGEOM(id)) {
    GeomGet(((DGeom *)dobj)->Item, CR_AXIS, ts.tm);
    if (((DGeom *)dobj)->citizenship == ORDINARY) {
      GeomGet(((DGeom *)dobj)->Inorm, CR_AXIS, N);
      TmInvert(N, NInv);
      TmConcat(NInv, T, T);
      TmConcat(T, N, T);
    }
  } else {
    CamGet(((DView *)dobj)->cam, CAM_C2W, ts.tm);
  }
  TmConcat(T, ts.tm, ts.tm);
  ts.h = NULL;
  gv_xform_set(id, &ts);
}

int motions_exist() {
  return allMotions != NULL;
}

/* The following functions really don't belong here.  Please move them. */

int is_descendant(int id, int ancestor_id)
{
  DGeom *dg;
  if (id == ancestor_id) return 1;
  else if (ancestor_id == WORLDGEOM
   && (dg = (DGeom *)drawer_get_object(id)) != NULL
   && ISGEOM(dg->id) && dg->citizenship == ORDINARY)
    return 1;
  else return 0;
}

LDEFINE(real_id, LSTRING,
	"(real-id ID)\n\
	Returns a string canonically identifying the given ID,\n\
	or \"nil\" if the object does not exist.  Examples:\n\
	 (if (real-id fred) (delete fred))\n\
	deletes \"fred\" if it exists but reports no error if it doesn't, and\n\
	 (if (= (real-id targetgeom) (real-id World)) () (delete targetgeom))\n\
	deletes \"targetgeom\" if it is different from the World.\n")
{
    int id;
    char *str;
    DObject *obj;

    LDECLARE(("real-id", LBEGIN,
	LSTRING, &str,
	LEND));
    id = drawer_idbyname(str);
    obj = drawer_get_object(id);
    if(obj == NULL) return Lnil;
    str = obj->name[1] ? obj->name[1] : obj->name[0];
    return (LTOOBJ(LSTRING))(&str);
}

int real_id(int id)
{
  if (id > 0) return id;
  switch(id) {
    case UNIVERSE: return UNIVERSE; break;
    case SELF: return SELF; break;
    case FOCUSID: return CAMID(uistate.mousefocus); break;
    case TARGETID: return real_id(uistate.targetid); break;
    case CENTERID: return real_id(uistate.centerid); break;
    case TARGETGEOMID: return GEOMID(uistate.targetgeom); break;
    case TARGETCAMID: return CAMID(uistate.targetcam); break;
    case ALLGEOMS: return WORLDGEOM; break;
    case ALLCAMS: return ALLCAMS; break;
    case DEFAULTCAMID: return DEFAULTCAMID; break;
    default: ERROR("bizarre id %d", id); return WORLDGEOM; break;
  }
}

int id_exists(int id) {
  return id == UNIVERSE || drawer_get_object(id) != NULL;
}

LDEFINE(transform, LVOID,
       "(transform      objectID centerID frameID [rotate|translate|translate-scaled|scale] x y z [dt] [\"smooth\"])\n\
	Apply a motion (rotation, translation, scaling) to object \"objectID\";\n\
	that is, construct and concatenate a transformation matrix with\n\
	objectID's transform  The 3 IDs involved are the object\n\
	that moves, the center of motion, and the frame of reference\n\
	in which to apply the motion.  The center is easiest understood\n\
	for rotations: if centerID is the same as objectID then it will\n\
	spin around its own axes; otherwise the moving object will orbit\n\
	the center object.  Normally frameID, in whose coordinate system\n\
	the (mouse) motions are interpreted, is \"focus\", the current camera.\n\
	Translations can be scaled proportional to the\n\
	distance between the target and the center. Support for\n\
	spherical and hyperbolic as well as Euclidean space is\n\
	built-in: use the \"space\" command to change spaces.  With type\n\
	\"rotate\" x, y, and z are floats specifying angles in RADIANS.\n\
	For types \"translate\" and \"translate-scaled\" x, y, and z are\n\
	floats specifying distances in the coordinate system of the\n\
	center object.  The optional \"dt\" field allows a simple form of\n\
	animation; if present, the object moves by just that amount during\n\
	approximately \"dt\" seconds, then stops.  If present and followed by\n\
	the \"smooth\" keyword, the motion is animated with a 3t^2-2t^3\n\
	function, so as to start and stop smoothly.  If absent, the motion is\n\
	applied immediately.")
{
  int moving_id, center_id, frame_id, transform_type, smooth = 0;
  float amount[3], during = 0;

  LDECLARE(("transform", LBEGIN,
	    LID, &moving_id,
	    LID, &center_id,
	    LID, &frame_id,
	    LKEYWORD, &transform_type,
	    LFLOAT, &amount[0],
	    LFLOAT, &amount[1],
	    LFLOAT, &amount[2],
	    LOPTIONAL,
	    LFLOAT, &during,
	    LKEYWORD, &smooth,
	    LEND));
  drawer_transform(moving_id, center_id, frame_id, transform_type,
		   amount, during, "transform", smooth);
  return Lt;
}

LDEFINE(transform_incr, LVOID,
       "(transform-incr  objectID centerID frameID [rotate|translate|translate-scaled|scale] x y z [dt])\n\
	Apply continuing motion: construct a transformation matrix and\n\
	concatenate it with the current transform of objectID every\n\
	refresh (sets objectID's incremental transform). Same syntax\n\
	as transform.  If optional \"dt\" argument is present,\n\
	the object is moved at each time step such that its average motion\n\
	equals one instance of the motion per \"dt\" seconds.  E.g.\n\
	  (transform-incr  World World World  rotate  6.28318 0 0  10.0)\n\
	rotates the World about its X axis at 1 turn (2pi radians) per 10 seconds.\n")
{
  int moving_id, center_id, frame_id, transform_type, smooth = 0;
  float amount[3];
  float timeunit = 0;

  LDECLARE(("transform-incr", LBEGIN,
	    LID, &moving_id,
	    LID, &center_id,
	    LID, &frame_id,
	    LKEYWORD, &transform_type,
	    LFLOAT, &amount[0],
	    LFLOAT, &amount[1],
	    LFLOAT, &amount[2],
	    LOPTIONAL, LFLOAT, &timeunit,
	    LKEYWORD, &smooth,
	    LEND));
  drawer_transform(moving_id, center_id, frame_id, transform_type,
		   amount, timeunit, "transform-incr", smooth);
  return Lt;
}

LDEFINE(transform_set, LVOID,
       "(transform-set objectID centerID frameID [rotate|translate|translate-scaled|scale] x y z)\n\
	Set objectID's transform to the constructed transform.\n\
	Same syntax as transform.")
{
  int moving_id, center_id, frame_id, transform_type;
  float amount[3];

  LDECLARE(("transform-set", LBEGIN,
	    LID, &moving_id,
	    LID, &center_id,
	    LID, &frame_id,
	    LKEYWORD, &transform_type,
	    LFLOAT, &amount[0],
	    LFLOAT, &amount[1],
	    LFLOAT, &amount[2],
	    LEND));
  drawer_transform(moving_id, center_id, frame_id, transform_type,
		   amount, 0., "transform-set", NO_KEYWORD);
  return Lt;
}


LDEFINE(position, LVOID,
       "(position       objectID otherID)\n\
	Set the transform of objectID to that of otherID.")
{
  int moving_id, ref_id;
  LDECLARE(("position", LBEGIN,
	    LID, &moving_id,
	    LID, &ref_id,
	    LEND));
  drawer_position(moving_id, ref_id, "position", NULL);
  return Lt;
}

LDEFINE(position_at, LVOID,
       "(position-at    objectID otherID [center | origin])\n\
	Translate objectID to the center of the bounding box or the \n\
	origin of the coordinate system of otherID (parallel translation).\n\
	Default is center.")
{
  int moving_id, ref_id;
  char *center = NULL;
  LDECLARE(("position-at", LBEGIN,
	    LID, &moving_id,
	    LID, &ref_id,
	    LOPTIONAL,
	    LSTRING, &center,
	    LEND));
  drawer_position(moving_id, ref_id, "position-at", center);
  return Lt;
}

LDEFINE(position_toward, LVOID,
       "(position-toward objectID otherID [center | origin])\n\
	Rotate objectID so that the center of the bounding box\n\
	or the origin of the coordinate system of the otherID\n\
	lies on the positive z-axis of the first object.  Default is\n\
	the center of the bounding box.")
{
  int moving_id, ref_id;
  char *center = NULL;
  LDECLARE(("position-toward", LBEGIN,
	    LID, &moving_id,
	    LID, &ref_id,
	    LOPTIONAL,
	    LSTRING, &center,
	    LEND));
  drawer_position(moving_id, ref_id, "position-toward", center);
  return Lt;
}

/* 
 * These are the functions which are being added to do away with the 
 * default normalization.  They will eventually not be prefixed by 
 * new 
 */

/* What kind of brain-damaged idiot did code the following line???? */
/* #define TmNDelete(x) */

LDEFINE(new_center, LVOID,
	"(new-center [id])\n\
	Stop id, then set id's transform to the identity.  Default id \n\
	is target.  Also, if the id is a camera, calls \n\
	(look-recenter World id).  The main function of the call to \n\
	(look-recenter) is to place the camera so that it is pointing \n\
	parallel to the z axis toward the center of the world.")
{
  int i;
  DObject *obj;
  int id = uistate.targetid;
  LDECLARE(("new-center", LBEGIN,
	    LOPTIONAL, 
	    LID, &id,
	    LEND));
  MAYBE_LOOP(id, i, T_NONE, DObject, obj) {
    drawer_stop(obj->id);
    gv_xform_set(obj->id, &ts_identity);
    if(ISGEOM(obj->id) && ((DGeom *)obj)->NDT != NULL) {
	TmNDelete( ((DGeom *)obj)->NDT );  ((DGeom *)obj)->NDT = NULL;
	TmNDelete( ((DGeom *)obj)->NDTinv );  ((DGeom *)obj)->NDTinv = NULL;
    }
	
  }
  /* This must be separate since the center of the world might change */
  MAYBE_LOOP(id, i, T_NONE, DObject, obj) 
    if (ISCAM(obj->id)) gv_look_recenter(WORLDGEOM, obj->id);

  return Lt;
}

LDEFINE(new_reset, LVOID,
	"(new-reset)\n\
	Equivalent to (progn (new-center ALLGEOMS)(new-center ALLCAMS))")
{
  LDECLARE(("new-reset", LBEGIN,
	    LEND));
  gv_new_center(ALLGEOMS);
  gv_new_center(ALLCAMS);
  return Lt;
}


/*
 * The following macros are used by look-encompass
 */

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef FUDGE
#define FUDGE 1.e-6
#endif 


/* 
 * emcompass_size is the fraction of the total window size that
 * will be filled by the object 
 */
#define ENCOMPASS_DEFAULTS 1.0, 1000., 0.1, 3.0
struct encompass {
    float view_frac;
    float clip_ratio;
    float near_margin;
    float far_margin;
};

static struct encompass enc = { ENCOMPASS_DEFAULTS };
static struct encompass edflt = { ENCOMPASS_DEFAULTS };

LDEFINE(look_encompass_size, LLIST,
	"(look-encompass-size [view-fraction  clip-ratio  near-margin far-margin])\n\
	Sets/returns parameters used by (look-encompass).\n\
	view-fraction is the portion of the camera window filled by the object,\n\
	clip-ratio is the max allowed ratio of near-to-far clipping planes.\n\
	The near clipping plane is 1/near-margin times closer than the near\n\
	edge of the object, and the far clipping plane is far-margin times\n\
	further away.  Returns the list of current values.\n\
	Defaults: .75  100  0.1  4.0\n")
{
  LDECLARE(("look-encompass-size", LBEGIN,
	   LOPTIONAL,
	   LFLOAT, &enc.view_frac,
	   LFLOAT, &enc.clip_ratio,
	   LFLOAT, &enc.near_margin,
	   LFLOAT, &enc.far_margin,
	   LEND));
  if(enc.view_frac <= 0) enc.view_frac = edflt.view_frac;
  if(enc.clip_ratio <= 0) enc.clip_ratio = edflt.clip_ratio;
  if(enc.near_margin <= 0) enc.near_margin = edflt.near_margin;
  if(enc.far_margin <= 0) enc.far_margin = edflt.far_margin;
  return LMakeArray(LFLOAT, (char *)&enc, 4);
}

/* 
 * look_encompass
 * Moves the camera back far enough to see all of an object
 * ?space == I'm not sure if this will work in non-Euclidean space
 * author: fowler
 * date: 12/92
 */
LDEFINE(look_encompass, LVOID,
	"(look-encompass [objectID] [cameraID])\n\
	Moves cameraID backwards or forwards until its field of view\n\
	surrounds objectID. This routine works only in Euclidean space.  \n\
	If objectID is not specified, it is assumed to be the world.  \n\
	If cameraID is not specified, it is assumed to be the targetcam.  \n\
	See also (look-encompass-size).")
{
  int i;
  Sphere *sphere;
  int objID = WORLDGEOM, camID = CAMID(uistate.targetcam);
  DView *dv;
  Transform w2c;
  float aspect, near, far, focallen, yfield, minfield, newyfield;
  int perspective;
  HPoint3 center, fromcam;
  float radius, effradius, zrange, newzrange, newfar, newnear;

  LDECLARE(("look-encompass", LBEGIN,
	    LOPTIONAL,
	    LID, &objID,
	    LID, &camID,
	    LEND));
  objID = real_id(objID);
  if (!ISGEOM(objID)) {
    OOGLError(0, "look-encompass: first argument must be a geom");
    return Lnil;
  }
  if (!ISCAM(camID)) {
    OOGLError(0, "look-encompass: second argument must be a camera");
    return Lnil;
  }
  if (spaceof(objID) != TM_EUCLIDEAN) {
    OOGLError(0, "look-encompass does not work in non-euclidean spaces.");
    return Lnil;
  }

  /* Figure out the bounding sphere of the object in world coordinates */
  sphere = (Sphere *)id_bsphere(objID, UNIVERSE);
  if (sphere == NULL) return Lnil;

  SphereCenter(sphere, &center);
  radius = SphereRadius(sphere);
  GeomDelete((Geom *)sphere);

  MAYBE_LOOP(camID, i, T_CAM, DView, dv) {

    drawer_get_transform(UNIVERSE, w2c, dv->id);

    CamGet(dv->cam, CAM_HALFYFIELD, &yfield);
    CamGet(dv->cam, CAM_ASPECT, &aspect);
    CamGet(dv->cam, CAM_NEAR, &near);
    CamGet(dv->cam, CAM_FAR, &far);
    CamGet(dv->cam, CAM_FOCUS, &focallen);
    CamGet(dv->cam, CAM_PERSPECTIVE, &perspective);

    HPt3Transform(w2c, &center, &fromcam);
    if(fromcam.x < 0) fromcam.x = -fromcam.x;
    if(fromcam.y < 0) fromcam.y = -fromcam.y;
    zrange = -fromcam.z;

    /* Camera's field in its minimum direction. */
    minfield = aspect<1 ? yfield*aspect : yfield;
    if (minfield <= 0.0) {
	OOGLError(0, "look-encompass: Erroneous field of view / aspect ratio combination.");
	minfield = aspect = yfield = 1.0;
    }

    /* Handle the case of look-encompass applied to a point.
     * Then, pretend the sphere's radius matches the camera's current
     * field of view in its focal plane.
     */
    effradius = radius;
    if(effradius <= 0)
	effradius = (perspective ? minfield * zrange : minfield)
			* enc.view_frac;
	

    if (perspective) {
	/* Perspective case
	 * Move the camera backward or forward until the sphere is tangent
	 * to the frustrum.
	 * minfield is tan(1/2 * field-of-view-in-shorter-direction).
	 * First newzrange term is the Z-distance we'd need to enclose the
	 *   sphere if it lay on the camera's Z axis.
	 * Rest is the extra we need to account for its displacement from Z
	 */

	newzrange = effradius/enc.view_frac * sqrt(1 + 1/(minfield*minfield))
		      + MAX(fromcam.y / yfield, fromcam.x / (yfield*aspect));
    } else {
	/* Orthogonal case
	 * Change the field of view of the camera so that the frustrum is 
	 * tangent to the sphere
	 */
	newyfield = MAX( effradius+fromcam.y, (effradius+fromcam.x) / aspect )
			/ enc.view_frac;
	CamSet(dv->cam, CAM_HALFYFIELD, newyfield, CAM_END);
	newzrange = zrange;
    }
    newnear = enc.near_margin*(newzrange - effradius);
    newfar = enc.far_margin * (newzrange + effradius);
    if(newnear < near || newfar > far || near*enc.clip_ratio < far) {
	float newnewz = effradius
			* (enc.clip_ratio * enc.near_margin - enc.far_margin)
			/ (enc.clip_ratio * enc.near_margin + enc.far_margin);
	if(newzrange < newnewz)
	    newzrange = newnewz;
	newnear = enc.near_margin * (newzrange - effradius);
	newfar  =  enc.far_margin * (newzrange + effradius);
    }
    drawer_float(dv->id, DRAWER_FAR, newfar);
    drawer_float(dv->id, DRAWER_NEAR, newnear);
    drawer_float(dv->id, DRAWER_FOCALLENGTH, newzrange);
    gv_transform(dv->id, dv->id, dv->id, TRANSLATE_KEYWORD, 0.0, 0.0,
		 newzrange - zrange, 0, NO_KEYWORD);
  }
  return Lnil;
}


/* 
 * look-toward
 * Point camera(s) toward object(s)
 * author: fowler
 * date: 12/92
 */
LDEFINE(look_toward, LVOID,
	"(look-toward [objectID] [cameraID] [origin | center])\n\
	Rotates the named camera to point toward the origin of the\n\
	object's coordinate system, or the center of the object's\n\
	bounding box (in non-Euclidean space, the origin will be used \n\
	automatically).  Default objectID is the world, default camera\n\
	is targetcam, default location to point towards is the center\n\
	of the bounding box.")
{
  int objID = WORLDGEOM, camID = CAMID(uistate.targetcam);
  char *center = NULL;

  LDECLARE(("look-toward", LBEGIN,
	    LOPTIONAL,
	    LID, &objID,
	    LID, &camID,
	    LSTRING, &center,
	    LEND));
  objID = real_id(objID);

  if (!ISCAM(camID)) {
    OOGLError(1, "Second argument must be a camera");
    return Lnil;
  }

  gv_position_toward(camID, objID, 
		     spaceof(objID) == TM_EUCLIDEAN ? center : "origin");
  
  return Lt;
}

/*
 * look
 * Point camera(s) toward object(s), then move camera(s) back far enough
 * that the entire object(s) is visible
 * Also, set the camera's focal length to indicate that this is the
 * object of interest.
 * author: fowler
 * date: 12/92
 */
LDEFINE(look, LVOID,
	"(look [objectID] [cameraID])\n\
	Rotates the named camera to point toward the center of the \n\
	bounding box of the named object (or the origin in hyperbolic or \n\
	spherical space).  In Euclidean space, moves the camera \n\
	forward or backward until the object appears as large \n\
	as possible while still being entirely visible.  Equivalent to \n\
	progn ( \n\
		(look-toward [objectID] [cameraID] {center | origin})\n\
		[(look-encompass [objectID] [cameraID])] \n\
	) \n\
	If objectID is not specified, it is assumed to be World.  If \n\
	cameraID is not specified, it is assumed to be targetcam.")
{
  int objID = WORLDGEOM, camID = CAMID(uistate.targetcam);

  LDECLARE(("look", LBEGIN,
	    LOPTIONAL,
	    LID, &objID,
	    LID, &camID,
	    LEND));
  objID = real_id(objID);

  if (!ISCAM(camID)) {
    OOGLError(0, "look: Second argument must be a camera");
    return Lnil;
  }
  
  gv_look_toward(objID, camID, "center");
  if ((spaceof(objID) == TM_EUCLIDEAN) && (ISGEOM(objID)))
    gv_look_encompass(objID, camID);

  return Lt;
}


/*
 * look-recenter
 * (See LDEFINE statement for explanation)
 * author: fowler
 * date: 12/92
 */
LDEFINE(look_recenter, LVOID,
	"(look-recenter [objectID] [cameraID])\n\
	Translates and rotates the camera so that it is looking in the \n\
	-z direction (in objectID's coordinate system) at the center of \n\
	objectID's bounding box (or the origin of the coordinate system \n\
	in non-Eudlidean space).  In Euclidean space, the camera is also \n\
	moved as close as possible to the object while allowing the \n\
	entire object to be visible.  Also makes sure that the y-axes of \n\
	objectID and cameraID are parallel.")
{
  int i;
  DView *camObj;
  TransformStruct obj2univ;
  Transform obj2univtm;
  int objID = WORLDGEOM, camID = CAMID(uistate.targetcam);

  LDECLARE(("look-recenter", LBEGIN,
	    LOPTIONAL,
	    LID, &objID,
	    LID, &camID,
	    LEND));
  objID = real_id(objID);  
  if (!ISGEOM(objID)) {
    OOGLError(0, "look-recenter: First argument must be a geom");
    return Lnil;
  }
  if (!ISCAM(camID)) {
    OOGLError(0, "look-recenter: Second argument must be a camera");
    return Lnil;
  }

  obj2univ.h = NULL;
  drawer_get_transform(objID, obj2univtm,  UNIVERSE);
  /* It is probably not a good idea to deform the camera geometry (in
   * general the spectator remains undeformed or suffers serious
   * injuries ...). So we keep only the orthogonal part and through
   * away the deformations.
   */
  TmPolarDecomp(obj2univtm, obj2univ.tm);

  MAYBE_LOOP(camID, i, T_CAM, DView, camObj) { 
    gv_xform_set(camObj->id, &obj2univ);
    gv_position_at(camObj->id, objID, 
		   spaceof(objID) == TM_EUCLIDEAN ? "center" : "origin");
  }

  if (spaceof(objID) == TM_EUCLIDEAN) 
    gv_look_encompass(objID, camID);
  
  return Lt;
}

static void get_geom_transform(DGeom *dg, Transform T_to_parent)
{
    Transform Tm, Tn;
    if(GeomGet(dg->Item, CR_AXIS, Tm) > 0 &&
		GeomGet(dg->Inorm, CR_AXIS, Tn) > 0)
	TmConcat(Tn, Tm, T_to_parent);
    else
	TmIdentity(T_to_parent);
}


/*
 * Compute transformation FROM id's coordinates TO a given coord system.
 * to_id may be any real id, or SELF (always yields identity transform),
 * or UNIVERSE (yields transform to universal coordinate system).
 */

void drawer_get_transform(int from_id, Transform T, int to_id)
{
  DObject *obj;
  Transform Tfrom, Tto, Ttoinv;

#define	DGobj  ((DGeom *)obj)
#define	DVobj  ((DView *)obj)

  if(from_id == to_id || to_id == SELF) {
    TmIdentity(T);
    return;
  }

  if((obj = drawer_get_object(from_id)) == NULL) {
    if(to_id == UNIVERSE) {
	OOGLError(1, "drawer_get_transform: can't handle from_id %d", from_id);
	TmIdentity(T);
	return;
    }
  } else {
    if(from_id < 0)
	from_id = obj->id;
    if (ISGEOM(from_id)) {
	switch(to_id) {
	case UNIVERSE:
	    get_geom_transform(DGobj, T);
	    if(DGobj->citizenship == ORDINARY) {
		get_geom_transform(dgeom[0], Tfrom);
		TmConcat(T, Tfrom, T);
	    }
	    return;
	case WORLDGEOM:
	    if(DGobj->citizenship == ORDINARY) {
		get_geom_transform(DGobj, T);
		return;
	    }
	}
    } else if (ISCAM(from_id)) {
	if(to_id == UNIVERSE) {
	    CamGet(DVobj->cam, CAM_C2W, T);
	    return;
	}
    }
  }

  /* If the above easy cases couldn't hack it, do it the hard way:
   * Handle A -> B as (A -> UNIVERSE) * (B -> UNIVERSE)^-1
   */
  drawer_get_transform(to_id, Tto, UNIVERSE);
  TmInvert(Tto, Ttoinv);
  drawer_get_transform(from_id, Tfrom, UNIVERSE);
  TmConcat(Tfrom, Ttoinv, T);
}


/* Get a geom's transform to its parent */
static TransformN *
get_geom_ND_transform(DGeom *dg)
{
  if(dg->NDT == NULL && drawerstate.NDim > 0)
    dg->NDT = TmNIdentity(TmNCreate(drawerstate.NDim, drawerstate.NDim, NULL));
  return REFINCR(TransformN, dg->NDT);
}

TransformN *
drawer_get_ND_transform(int from_id, int to_id)
{
  DObject *obj;
  TransformN *Tfrom, *Tto, *T;

  if(from_id == to_id || to_id == SELF) {
    return NULL;
  }

  if((obj = drawer_get_object(from_id)) == NULL) {
    if(to_id == UNIVERSE) {
	OOGLError(1, "drawer_get_transform: can't handle from_id %d", from_id);
	return NULL;
    }
  } else {
    if(from_id < 0)
	from_id = obj->id;
    if (ISGEOM(from_id)) {
	switch(to_id) {
	case UNIVERSE:
	    T = get_geom_ND_transform(DGobj);
	    if(DGobj->citizenship == ORDINARY && dgeom[0]->NDT) {
		if(T == NULL)
		    return REFINCR(TransformN, dgeom[0]->NDT);
		else if(dgeom[0]->NDT == NULL)
		    return T;
		else {
		    TransformN *To = TmNConcat(T, dgeom[0]->NDT, NULL);
		    TmNDelete(T);
		    return To;
		}
	    }
	    return T;
	case WORLDGEOM:
	    if(DGobj->citizenship == ORDINARY) {
		return REFINCR(TransformN, DGobj->NDT);
	    }
	}
    } else if (ISCAM(from_id)) {
	if(to_id == UNIVERSE) {
	    T = (obj && ((DView *)obj)->cluster) ?
		REFINCR(TransformN, ((DView *)obj)->cluster->C2W) : NULL;
	    return T ? T :
		TmNIdentity(TmNCreate(drawerstate.NDim,drawerstate.NDim,NULL));
	}
    }
  }

  /* If the above easy cases couldn't hack it, do it the hard way:
   * Handle A -> B as (A -> UNIVERSE) * (B -> UNIVERSE)^-1
   */
  Tto = drawer_get_ND_transform(to_id, UNIVERSE);
  Tfrom = drawer_get_ND_transform(from_id, UNIVERSE);
  if(Tto) {
    TransformN *Tot = TmNInvert(Tto, NULL);
    TmNDelete(Tto);
    if(Tfrom) {
	T = TmNConcat(Tfrom, Tot, NULL);
	TmNDelete(Tfrom);
	TmNDelete(Tot);
    } else {
	T = Tot;
    }
    return T;
  } else {
    return Tfrom;
  }
}

void
drawer_set_ND_xform(int id, TransformN *T)
{
   DObject *obj;
   TransformN **tp = NULL;

   if((obj = drawer_get_object(id)) != NULL) {
	if(ISCAM(obj->id)) {
	    if(((DView *)obj)->cluster == NULL)	/* N-D camera? */
		return;
	    tp = &((DView *)obj)->cluster->C2W;
	    drawerstate.changed |= 1;
	} else {
	    tp = &DGobj->NDT;
	    obj->changed |= 1;
	}
   }
   if(tp) {
	if(*tp) TmNDelete(*tp);
	*tp = T ? T :
	      TmNIdentity(TmNCreate(drawerstate.NDim, drawerstate.NDim, NULL));
   }
}

int
get_parent(int id)
{
  DGeom *dg;

  if(ISGEOM(id) && (dg = (DGeom *)drawer_get_object(id)) != NULL) {
     if(dg->citizenship == ORDINARY) return WORLDGEOM;
  }
  /* Cameras, etc. are all referred to UNIVERSE */
  return UNIVERSE;
}

void
apply_ND_transform(Transform delta, int moving, int center, int frame)
{
  HPointN *origin;
  TransformN *Tgf = NULL, *Tfg = NULL, *Tcf = NULL;
  TransformN *Tmf = NULL, *Tmg = NULL, *Tfp = NULL, *Tmp = NULL;
  DView *dv;
  int perm[4];
  int cam;

  /* Construct new coord system 'g' with the orientation of 'frame' but
   * with its origin at 'center'.
   * It's in this system that 'delta' is defined.
   * Tgf is the transform from this system to 'frame'.
   */
  origin = HPtNCreate(drawerstate.NDim, NULL);
  Tcf = drawer_get_ND_transform(center, frame);
  if(Tcf) HPtNTransform( Tcf, origin, origin );
  TmNDelete(Tcf);
  Tgf = TmNSpaceTranslateOrigin(Tgf, origin);
  Tfg = TmNInvert(Tgf, Tfg);	/* Should be sped up: negate "origin" rather than inverting */
  HPtNDelete(origin);

  cam = frame;
  if(!ISCAM(cam)) cam = center;
  if(!ISCAM(cam)) cam = moving;
  if(ISCAM(cam) && (dv = (DView *)drawer_get_object(cam)) != NULL) {
    memcpy(perm, dv->NDPerm, sizeof(perm));
  } else {
    perm[0] = 0; perm[1] = 1; perm[2] = 2;
    perm[3] = -1;
  }

  Tmf = drawer_get_ND_transform(moving, frame);
  Tmg = Tmf ? TmNConcat( Tmf, Tfg, Tmg ) : REFINCR(TransformN, Tfg);
  TmNApplyDN( Tmg, perm, delta );

  Tmf = TmNConcat( Tmg, Tgf, Tmf );
  Tfp = drawer_get_ND_transform(frame, get_parent(moving));
  Tmp = Tfp ? TmNConcat(Tmf, Tfp, Tmp) : REFINCR(TransformN, Tmf);
  drawer_set_ND_xform( moving, Tmp );
  TmNDelete( Tgf );
  TmNDelete( Tfg );
  /*TmNDelete( Tcf );*/ /* already done above */
  TmNDelete( Tmf );
  TmNDelete( Tmg );
  TmNDelete( Tfp );
}


void make_center(char *objname, Point3 *pt)
{
    TransformStruct ts;
    int cid;

    if((cid = drawer_idbyname(objname)) == NOID) {
	GeomStruct gs;
	gs.h = NULL;
	gs.geom = NULL;
	cid = gv_new_alien(objname, &gs);
    }
    ts.h = NULL;
    TmTranslate(ts.tm, pt->x, pt->y, pt->z);
    gv_xform_set( cid, &ts );
    gv_ui_center(cid);
}

void make_center_from_pick(char *objname, Pick *pick, int camid)
{
    Point3 center, fromcam;
    Transform Tuni, Tcam;
    int index;
    DView *dv;

	/* Apply primitive -> World transform.
	 * XXX if we ever implement object hierarchy, a Center like this
	 * should be referred to the object we picked, not the Universe.
	 */
    drawer_get_transform(WORLDGEOM, Tuni, UNIVERSE);
    Pt3Transform(pick->Tw, &pick->got, &center);
    Pt3Transform(Tuni, &center, &center);	/* Convert to universe coords */
    make_center(objname, &center);
    MAYBE_LOOP(camid, index, T_CAM, DView, dv) {
	drawer_get_transform(UNIVERSE, Tcam, dv->id);
	Pt3Transform(Tcam, &center, &fromcam);
	drawer_float(dv->id, DRAWER_FOCALLENGTH, Pt3Length(&fromcam));
    }
}
