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

#include "geom.h"
#include "create.h"
#include "polylistP.h"
#include "sphere.h"

Geom *PolyListSphere(PolyList *p,
		     Transform T, TransformN *TN, int *axes,
		     int space)
{
  int i;
  Sphere *sphere;

  if (p == NULL || p->n_verts == 0 || p->n_polys == 0) return NULL;

  if (TN) {
    HPointN *tmp = HPtNCreate(5, NULL);
    HPointN *spanPts[2*4];
    HPoint3 spanPts3[2*4];

    /* Create a dummy sphere, the center will be corrected later */
    sphere = (Sphere *)GeomCreate("sphere", CR_CENTER, &p->vl[0].pt,
				  CR_RADIUS, 0.0, CR_AXIS, T, CR_SPACE, space,
				  CR_END);

    if(p->geomflags & VERT_4D) {
      for (i = 0; i < 2*4; i++) {
	spanPts[i] = HPtNCreate(5, NULL);
	Pt4ToHPtN(&p->vl[0].pt, spanPts[i]);
      }
      for (i = 1; i < p->n_verts; i++) {
	Pt4ToHPtN(&p->vl[i].pt, tmp);
	MaxDimensionalSpanHPtN(spanPts, tmp);
      }
      for (i = 0; i < 2*4; i++) {
	HPtNTransformComponents(TN, spanPts[i], axes, &spanPts3[i]);
	HPtNDelete(spanPts[i]);
      }
      SphereEncompassBoundsN(sphere, spanPts3, 4);
      for (i = 0; i < p->n_verts; i++) {
	Pt4ToHPtN(&p->vl[i].pt, tmp);
	SphereAddHPtN(sphere, tmp, NULL, TN, axes);
      }
    } else {
      for (i = 0; i < 2*4; i++) {
	spanPts[i] = HPtNCreate(5, NULL);
	HPt3ToHPtN(&p->vl[0].pt, NULL, spanPts[i]);
      }
      for (i = 0; i < p->n_verts; i++) {
	HPt3ToHPtN(&p->vl[i].pt, NULL, tmp);
	MaxDimensionalSpanHPtN(spanPts, tmp);
      }
      for (i = 0; i < 2*4; i++) {
	HPtNTransformComponents(TN, spanPts[i], axes, &spanPts3[i]);
	/*HPt3Dehomogenize(&spanPts3[i], &spanPts3[i]);*/
	HPtNDelete(spanPts[i]);
      }
      SphereEncompassBoundsN(sphere, spanPts3, 4);
      for (i = 0; i < p->n_verts; i++) {
	HPt3ToHPtN(&p->vl[i].pt, NULL, tmp);
	SphereAddHPtN(sphere, tmp, NULL, TN, axes);
      }
    }
    HPtNDelete(tmp);
  } else {
    HPoint3 spanPts[6];
    
    if(false && (p->geomflags & VERT_4D)) {
      /* 4d, but no 4d transform, act on x,y,z sub-space */
      HPoint3 tmp;

      Pt4ToHPt3(&p->vl[0].pt, NULL, &tmp);
      sphere = (Sphere *)GeomCreate("sphere", CR_CENTER, &tmp,
				    CR_RADIUS, 0.0, CR_AXIS, T, CR_SPACE, space,
				    CR_END);
      /* For convenience (if not efficiency) assume all the vertices
       * are used */
      for (i = 0; i < 6; i++) {
	Pt4ToHPt3(&p->vl[0].pt, NULL, &spanPts[i]);
      }
      for (i = 0; i < p->n_verts; i++) {
	Pt4ToHPt3(&p->vl[i].pt, NULL, &tmp);
	MaxDimensionalSpan(spanPts, &tmp);
      }
      HPt3TransformN(T, spanPts, spanPts, 6);
      SphereEncompassBounds(sphere, spanPts);
      for (i = 0; i < p->n_verts; i++) {
	Pt4ToHPt3(&p->vl[i].pt, NULL, &tmp);
	SphereAddHPt3(sphere, &tmp, T);
      }
    } else {
      
      sphere = (Sphere *)GeomCreate("sphere", CR_CENTER, &p->vl[0].pt,
				    CR_RADIUS, 0.0, CR_AXIS, T, CR_SPACE, space,
				    CR_END);
      /* For convenience (if not efficiency) assume all the vertices
       * are used */
      for (i = 0; i < 6; i++) spanPts[i] = p->vl[0].pt;
      for (i = 0; i < p->n_verts; i++)
	MaxDimensionalSpan(spanPts, &p->vl[i].pt);
      HPt3TransformN(T, spanPts, spanPts, 6);
      SphereEncompassBounds(sphere, spanPts);
      for (i = 0; i < p->n_verts; i++) 
	SphereAddHPt3(sphere, &p->vl[i].pt, T);
    }
  }
  return (Geom *)sphere;
}
						 
/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
