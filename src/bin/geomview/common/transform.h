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

#ifndef _GV_COMMON_TRANSFORM_H_
#define _GV_COMMON_TRANSFORM_H_

#include "lang.h"
#include "drawer.h"

typedef struct Motion {
  int moving_id;
  int center_id;
  int frame_id;
  void (*transform)();
  float amount[3];
  float timeunit;
  float timeleft;	/* Nonzero if bounded animation */
  int smooth;		/* Smooth animation, else linear */
  struct Motion *next;
} Motion;

int drawer_transform(int moving_id, int center_id, int frame_id,
		     Keyword transform_type,
		     float fx, float fy, float fz,
		     float timeunit,
		     char *repeat_type,
		     Keyword smoothanim);

void drawer_position(int moving_id, int ref_id, char *position_type,
		     char *center);
void drawer_zoom(int id, float amount);
void drawer_scale(int id, float amount);
void set_motion(Motion *motion);
void apply_motion(Motion *motion, float dt);

void insert_motion(Motion *motion);
void apply_all_motions(float dt);
void do_motion(float dt);
void delete_motion(Motion *m);
void delete_like_motions(Motion *m, float augmentby);
void drawer_post_xform(int id, Transform T);

int e3space(void *data);
int h3space(void *data);

void stop_motions(int id);
int motions_exist();
int real_id(int id);

void make_center(char *objname, Point3 *pt);
void make_center_from_bbox(char *objname, int obj_id);
void make_center_from_pick(char *objname, Pick *pick, int focalcam);

void drawer_set_ND_xform(int id, TransformN *T);
int real_id(int id);

/* Get a geom's transform to its parent */
static inline TransformN *
get_geom_ND_transform(DGeom *dg)
{
  if (dg->NDT == NULL && drawerstate.NDim > 0) {
    dg->NDT = TmNIdentity(TmNCreate(drawerstate.NDim, drawerstate.NDim, NULL));
    GeomSet(dg->Item, CR_NDAXIS, dg->NDT, CR_END);
    dg->bboxvalid = false; /* invalidate bounding box */
  } else if (drawerstate.NDim == 0) {
    TmNDelete(dg->NDT);
    dg->NDT = NULL;
  }
  return REFGET(TransformN, dg->NDT);
}

#endif /* _GV_COMMON_TRANSFORM_H_ */
