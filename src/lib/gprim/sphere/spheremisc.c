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
  bbox = GeomBound((Geom *)sphere, TM_IDENTITY, NULL, NULL);
  BBoxMinMax((BBox *)bbox, min, max);
  GeomDelete(bbox);
  HPt3Normalize(min, min);
  HPt3Normalize(max, max);
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
    Pt3Unit((Point3*)(void *)&diff);
    center.x = b->center.x + diff.x*b->radius;
    center.y = b->center.y + diff.y*b->radius;
    center.z = b->center.z + diff.z*b->radius;
    center.w = 1.0;

    GeomSet((Geom*)dest, CR_RADIUS, a->radius, CR_CENTER, &a->center, CR_END);
    SphereAddHPt3(dest, &center, TM_IDENTITY);
  }

  return dest;
}

void SphereEncompassBounds(Sphere *sphere, HPoint3 *points) {
  int i, j;
  float span, maxspan;
  HPoint3 *d1, *d2, center;


  d1 = d2 = &points[0];
  maxspan = 0.0;
  for (i = 0; i < 6; i++)
    for (j = i+1; j < 6; j++) {
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
			
int SphereAddHPt3(Sphere *sphere, HPoint3 *point, Transform T)
{
  float radius, old_to_p, old_to_new;
  HPoint3 center, newpoint;

  HPt3Transform(T, point, &newpoint);
  if(newpoint.w != 1)
    HPt3Normalize(&newpoint, &newpoint);
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
  for (i = 0; i < 6; i++) spanPts[i] = point[0];
  MaxDimensionalSpanN(spanPts, point, n);
  HPt3TransformN(T, spanPts, spanPts, 6);
  SphereEncompassBounds(sphere, spanPts);
  SphereAddHPt3N(sphere, point, n, T);
}

/* 0 has min x, 1 has max x, 2 has min y... */
void MaxDimensionalSpan(HPoint3 *spanPts, HPoint3 *point)
{
  HPoint3 pt;
  if(point->w != 1 && point->w != 0) {
    HPt3Normalize(point, &pt);
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
  HPoint3 pt, *point;
  
  for (i = 0; i < n; i++) {
    point = &points[i];
    if(point->w != 1 && point->w != 0) {
	HPt3Normalize(point, &pt);
	point = &pt;
    }
    if (point->x < spanPts[0].x) spanPts[0] = *point;
    else if (point->x > spanPts[1].x) spanPts[1] = *point;

    if (point->y < spanPts[2].y) spanPts[2] = *point;
    else if (point->y > spanPts[3].y) spanPts[3] = *point;

    if (point->z < spanPts[4].z) spanPts[4] = *point;
    else if (point->z > spanPts[5].z) spanPts[5] = *point;
  }
}
