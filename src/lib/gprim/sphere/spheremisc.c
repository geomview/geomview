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
#include "geom.h"
#include "geomclass.h"
#include "transform.h"
#include "sphereP.h"
#include "hpoint3.h"

#ifndef FUDGE
#define FUDGE	1e-6
#endif

void SphereMinMax(Sphere *sphere, HPoint3 *min, HPoint3 *max) 
{
  Geom *bbox;
  bbox = GeomBound((Geom *)sphere, TM_IDENTITY, NULL);
  BBoxMinMax((BBox *)bbox, min, max);
  GeomDelete(bbox);
  HPt3Dehomogenize(min, min);
  HPt3Dehomogenize(max, max);
}

void SphereCenter(Sphere *sphere, HPoint3 *center) 
{
  *center = sphere->center;
}

float SphereRadius(Sphere *sphere) 
{
  return sphere->radius;
}

void SphereSwitchSpace(Sphere *sphere, int space)
{
  Transform T;
  
  sphere->space = space;
  TmScale(sphere->axis, sphere->radius, sphere->radius, sphere->radius);
  TmSpaceTranslate(T, sphere->center.x, sphere->center.y, sphere->center.z,
		   sphere->space);
  GeomTransform((Geom*)sphere, T, NULL);
}

Sphere *SphereUnion3(Sphere *a, Sphere *b, Sphere *dest) {
  int space;
  HPoint3 center, diff;
  float radius = 0.0;

  if (a == NULL && b == NULL) return NULL;
  space = (a != NULL) ? a->space : b->space;
  if (dest == NULL) 
    dest = (Sphere *)GeomCreate("sphere", CR_SPACE, space, CR_END);

  if (a == NULL || b == NULL) {
    if (a == NULL) {
      radius = b->radius;
      center = b->center;
      space = b->space;
    }
    else if (b == NULL) {
      radius = a->radius;
      center = a->center;
      space = a->space;
    } 
    GeomSet((Geom*)dest, CR_RADIUS, radius, CR_CENTER, &center, CR_SPACE, space,
	    CR_END);
  } else {
  
    if (a->space != b->space) 
      OOGLError(1, "Uniting two spheres existing in different spaces.");
    space = a->space;
    if (space != TM_EUCLIDEAN)
      OOGLError(1, "SphereUnion3 currently only works reliably in\n%s",
		"Euclidean space.");
    
    HPt3Sub(&b->center, &a->center, &diff);
    Pt3Unit(HPoint3Point3(&diff));
    center.x = b->center.x + diff.x*b->radius;
    center.y = b->center.y + diff.y*b->radius;
    center.z = b->center.z + diff.z*b->radius;
    center.w = 1.0;

    GeomSet((Geom*)dest, CR_RADIUS, a->radius, CR_CENTER, &a->center, CR_END);
    SphereAddHPt3(dest, &center, TM_IDENTITY);
  }

  return dest;
}

void SphereEncompassBoundsN(Sphere *sphere, HPoint3 *points, int dim)
{
  int i, j;
  float span, maxspan;
  HPoint3 *d1, *d2, center;


  d1 = d2 = &points[0];
  maxspan = 0.0;
  for (i = 0; i < 2*dim; i++)
    for (j = i+1; j < 2*dim; j++) {
      span = HPt3SpaceDistance(&points[i], &points[j], sphere->space);
      if (span > maxspan) {
	maxspan = span;
	d1 = &points[i];
	d2 = &points[j];
      }
    }
  
  /* Find the midpoint here - this will not work in non-Euclidean space */
  center.x = (d1->x/d1->w + d2->x/d2->w) / 2.0;
  center.y = (d1->y/d1->w + d2->y/d2->w) / 2.0;
  center.z = (d1->z/d1->w + d2->z/d2->w) / 2.0;
  center.w = 1.0;

  GeomSet((Geom *)sphere, CR_RADIUS, maxspan / 2.0, CR_CENTER, &center,
	  CR_END);

}

void SphereEncompassBounds(Sphere *sphere, HPoint3 *points)
{
  SphereEncompassBoundsN(sphere, points, 3);
}

			
int SphereAddHPt3(Sphere *sphere, HPoint3 *point, Transform T)
{
  float radius, old_to_p, old_to_new;
  HPoint3 center, newpoint;

  HPt3Transform(T, point, &newpoint);
  if(newpoint.w != 1)
    HPt3Dehomogenize(&newpoint, &newpoint);
  old_to_p = HPt3SpaceDistance(&newpoint, &sphere->center, sphere->space);
  if (old_to_p > sphere->radius) {
    radius = (sphere->radius + old_to_p) / 2.0;
    old_to_new = old_to_p - radius;
    center.x = sphere->center.x + 
      (newpoint.x - sphere->center.x)*old_to_new / old_to_p;
    center.y = sphere->center.y + 
      (newpoint.y - sphere->center.y)*old_to_new / old_to_p;
    center.z = sphere->center.z + 
      (newpoint.z - sphere->center.z)*old_to_new / old_to_p;
    center.w = 1.0;
    GeomSet((Geom *)sphere, CR_RADIUS, radius, CR_CENTER, &center, CR_END);
    return 1;
  } else return 0;
}

int SphereAddHPt3N(Sphere *sphere, HPoint3 *point, int n, Transform T) 
{
  int i, ans = 0;

  for (i = 0; i < n; i++) ans |= SphereAddHPt3(sphere, &point[i], T);
  return ans;
}

void SphereEncompassHPt3N(Sphere *sphere, HPoint3 *point, int n,
			  Transform T) {
  int i;
  HPoint3 spanPts[6];

  if (!n) return;

  spanPts[0] = point[0];
  HPt3Dehomogenize(&spanPts[0], &spanPts[0]);
  for (i = 1; i < 6; i++) {
    spanPts[i] = spanPts[0];
  }
  MaxDimensionalSpanN(spanPts, point+1, n-1);
  HPt3TransformN(T, spanPts, spanPts, 6);
  SphereEncompassBounds(sphere, spanPts);
  SphereAddHPt3N(sphere, point, n, T);
}

/* 0 has min x, 1 has max x, 2 has min y... */
void MaxDimensionalSpan(HPoint3 *spanPts, HPoint3 *point)
{
  HPoint3 pt;
  if(point->w != 1 && point->w != 0) {
    HPt3Dehomogenize(point, &pt);
    point = &pt;
  }
  if (point->x < spanPts[0].x) spanPts[0] = *point;
  else if (point->x > spanPts[1].x) spanPts[1] = *point;
  if (point->y < spanPts[2].y) spanPts[2] = *point;
  else if (point->y > spanPts[3].y) spanPts[3] = *point;
  if (point->z < spanPts[4].z) spanPts[4] = *point;
  else if (point->z > spanPts[5].z) spanPts[5] = *point;
}
	   
void MaxDimensionalSpanN(HPoint3 *spanPts, HPoint3 *points, int n) 
{
  int i;
  
  for (i = 0; i < n; i++) {
    MaxDimensionalSpan(spanPts, &points[i]);
  }
}

/* Same stuff as above, but for HPointN case */
int SphereAddHPtN(Sphere *sphere, HPointN *point,
		  Transform T, TransformN *TN, int *axes)
{
  float radius, old_to_p, old_to_new;
  HPoint3 center, newpoint, tmp;

  if (TN) {
    HPtNTransformComponents(TN, point, axes, &newpoint);
  } else {
    HPtNToHPt3(point, axes, &tmp);
    HPt3Transform(T, &tmp, &newpoint);
  }
  HPt3Dehomogenize(&newpoint, &newpoint);
  old_to_p = HPt3SpaceDistance(&newpoint, &sphere->center, sphere->space);
  if (old_to_p > sphere->radius) {
    radius = (sphere->radius + old_to_p) / 2.0;
    old_to_new = old_to_p - radius;
    center.x = sphere->center.x + 
      (newpoint.x - sphere->center.x)*old_to_new / old_to_p;
    center.y = sphere->center.y + 
      (newpoint.y - sphere->center.y)*old_to_new / old_to_p;
    center.z = sphere->center.z + 
      (newpoint.z - sphere->center.z)*old_to_new / old_to_p;
    center.w = 1.0;
    GeomSet((Geom *)sphere, CR_RADIUS, radius, CR_CENTER, &center, CR_END);
    return 1;
  } else {
    return 0;
  }
}

int SphereAddHPtNN(Sphere *sphere, HPointN **point, int n,
		   Transform T, TransformN *TN, int *axes) 
{
  int i, ans = 0;

  for (i = 0; i < n; i++) ans |= SphereAddHPtN(sphere, point[i], T, TN, axes);
  return ans;
}

void SphereEncompassHPtNN(Sphere *sphere, HPointN **point, int n,
			  Transform T, TransformN *TN, int *axes)
{
  int i, dim = point[0]->dim-1;
  VARARRAY(spanPts, HPointN *, 2*dim);
  VARARRAY(spanPts3, HPoint3, 2*dim);

  if (!n) return;

  spanPts[0] = HPtNCopy(point[0], NULL);
  HPtNDehomogenize(spanPts[0], spanPts[0]);
  for (i = 1; i < 2*dim; i++) {
    spanPts[i] = HPtNCopy(spanPts[0], NULL);
  }
  MaxDimensionalSpanHPtNN(spanPts, point+1, n-1);
  if (TN) {
    for (i = 0; i < 2*dim; i++) {
      HPtNTransformComponents(TN, spanPts[i], axes, &spanPts3[i]);
      HPtNDelete(spanPts[i]);
    }
  } else {
    HPoint3 tmp;
    for (i = 0; i < 2*dim; i++) {
      HPtNToHPt3(spanPts[i], axes, &tmp);
      HPt3Transform(T, &tmp, &spanPts3[i]);
      HPtNDelete(spanPts[i]);
    }    
  }
  SphereEncompassBoundsN(sphere, spanPts3, dim);
  SphereAddHPtNN(sphere, point, n, T, TN, axes);
}

/* 0 has min x, 1 has max x, 2 has min y... */
void MaxDimensionalSpanHPtN(HPointN **spanPts, HPointN *point)
{
  int dim = point->dim, i;
  HPointN tmp;
  VARARRAY(tmpdata, HPtNCoord, dim);

  if (point->v[dim-1] != 1 && point->v[dim-1] != 0) {
    tmp.dim = dim;
    tmp.flags = 0;
    tmp.v = tmpdata;
    point = HPtNCopy(point, &tmp);
    HPtNDehomogenize(point, point);
  }
  for (i = 0; i < dim-1; i++) {
    if (point->v[i] < spanPts[2*i]->v[i])
      HPtNCopy(point, spanPts[2*i]); 
    else if (point->v[i] > spanPts[2*i+1]->v[i])
      HPtNCopy(point, spanPts[2*i+1]);
  }
}
	   
void MaxDimensionalSpanHPtNN(HPointN **spanPts, HPointN **points, int n) 
{
  int i;

  for (i = 0; i < n; i++) {
    MaxDimensionalSpanHPtN(spanPts, points[i]);
  }
}

/* Same as above, but for contiguous data, as used in mesh, ndmesh,
 * npolylist, skel, can be used for VERT_4D case.
 */
int SphereAddPoint(Sphere *sphere, float *point, int vert_4d, int pdim,
		   Transform T, TransformN *TN, int *axes)
{
  float radius, old_to_p, old_to_new;
  HPoint3 center, newpoint, tmp3;
  HPointN tmp;
  VARARRAY(v, HPtNCoord, 5);

  tmp.flags = 0;
  if (pdim == 4) {
    tmp.v = v;
    if (vert_4d) {
      tmp.dim = 5;
      Pt4ToHPtN((HPoint3 *)point, &tmp);
    } else {
      tmp.dim = 4;
      HPt3ToHPtN((HPoint3 *)point, NULL, &tmp);
    }
  } else {
    tmp.v = point;
    tmp.dim = pdim;
  }
  if (TN) {
    HPtNTransformComponents(TN, &tmp, axes, &newpoint);
  } else {
    HPtNToHPt3(&tmp, axes, &tmp3);
    HPt3Transform(T, &tmp3, &newpoint);
  }
  HPt3Dehomogenize(&newpoint, &newpoint);
  
  old_to_p = HPt3SpaceDistance(&newpoint, &sphere->center, sphere->space);
  if (old_to_p > sphere->radius) {
    radius = (sphere->radius + old_to_p) / 2.0;
    old_to_new = old_to_p - radius;
    center.x = sphere->center.x + 
      (newpoint.x - sphere->center.x)*old_to_new / old_to_p;
    center.y = sphere->center.y + 
      (newpoint.y - sphere->center.y)*old_to_new / old_to_p;
    center.z = sphere->center.z + 
      (newpoint.z - sphere->center.z)*old_to_new / old_to_p;
    center.w = 1.0;
    GeomSet((Geom *)sphere, CR_RADIUS, radius, CR_CENTER, &center, CR_END);
    return 1;
  } else {
    return 0;
  }
}

int SphereAddPoints(Sphere *sphere,
		    float *point, int vert_4d, int pdim, int n,
		    Transform T, TransformN *TN, int *axes) 
{
  int i, ans = 0;

  for (i = 0; i < n; i++, point += pdim) {
    ans |= SphereAddPoint(sphere, point, vert_4d, pdim, T, TN, axes);
  }
  
  return ans;
}

void SphereEncompassPoints(Sphere *sphere,
			   float *points, int vert_4d, int pdim, int n,
			   Transform T, TransformN *TN, int *axes)
{
  int i, dim;

  if (!n) {
    return;
  }

  if (pdim != 4) {
    vert_4d = 0;
  }
  
  dim = vert_4d ? pdim : pdim - 1;
  
  {
    VARARRAY(spanPts, HPointN *, 2*dim);
    VARARRAY(spanPts3, HPoint3, 2*dim);

    if (pdim == 4) {
      if (vert_4d) {
	spanPts[0] = Pt4ToHPtN((HPoint3 *)points, NULL);
      } else {
	spanPts[0] = HPt3ToHPtN((HPoint3 *)points, NULL, NULL);
      }
    } else {
      spanPts[0] = HPtNCreate(pdim, points);
    }
    HPtNDehomogenize(spanPts[0], spanPts[0]);
    for (i = 1; i < 2*dim; i++) {
      spanPts[i] = HPtNCopy(spanPts[0], NULL);
    }
  
    MaxNDimensionalSpanN(spanPts, points+pdim, vert_4d, pdim, n-1);
    if (TN) {
      for (i = 0; i < 2*dim; i++) {
	HPtNTransformComponents(TN, spanPts[i], axes, &spanPts3[i]);
      }
    } else {
      HPoint3 pt3;

      for (i = 0; i < 2*dim; i++) {
	HPtNToHPt3(spanPts[i], axes, &pt3);
	HPt3Transform(T, &pt3, &spanPts3[i]);
      }
    }
    SphereEncompassBoundsN(sphere, spanPts3, dim);
    SphereAddPoints(sphere, points, vert_4d, pdim, n, T, TN, axes);
    for (i = 0; i < 2*dim; i++) {
      HPtNDelete(spanPts[i]);
    }
  }
}

void MaxNDimensionalSpanN(HPointN **spanPts,
			  float *points, int vert_4d, int pdim, int n) 
{
  int i;
  HPointN tmp;

  tmp.flags = 0;
  if (pdim == 4) {
    HPtNCoord v[5];
    tmp.v = v;
  
    if (vert_4d) {
      tmp.dim = 5;
      for (i = 0; i < n; i++) {
	Pt4ToHPtN((HPoint3 *)points, &tmp);
	MaxDimensionalSpanHPtN(spanPts, &tmp);
	points += 4;
      }
    } else {
      tmp.dim = 4;
      for (i = 0; i < n; i++) {
	HPt3ToHPtN((HPoint3 *)points, NULL, &tmp);
	MaxDimensionalSpanHPtN(spanPts, &tmp);
	points += 4;
      }
    }
  } else {
    tmp.v = points;
    tmp.dim = pdim;
    for (i = 0; i < n; i++) {
      MaxDimensionalSpanHPtN(spanPts, &tmp);
      tmp.v += pdim;
    }
  }
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
