/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Geometry Technologies, Inc.
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

#if defined(HAVE_CONFIG_H) && !defined(CONFIG_H_INCLUDED)
#include "config.h"
#endif

static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Geometry Technologies, Inc.";

#include "geom.h"
#include "create.h"
#include "polylistP.h"
#include "sphere.h"

Geom *PolyListSphere(p, T, space)
     PolyList *p;
     Transform T;
     int space;
{
  int i;
  Sphere *sphere;
  HPoint3 spanPts[6];

  if (p == NULL || p->n_verts == 0 || p->n_polys == 0) return NULL;

  if(p->geomflags & VERT_4D)
	return GeomBoundSphereFromBBox((Geom *)p, T, space);

  sphere = (Sphere *)GeomCreate("sphere", CR_CENTER, &p->vl[0].pt,
				CR_RADIUS, 0.0, CR_AXIS, T, CR_SPACE, space,
				CR_END);
  /* For convenience (if not efficiency) assume all the vertices are 
   * used */
  for (i = 0; i < 6; i++) spanPts[i] = p->vl[0].pt;
  for (i = 0; i < p->n_verts; i++) MaxDimensionalSpan(spanPts, &p->vl[i].pt);
  HPt3TransformN(T, spanPts, spanPts, 6);
  SphereEncompassBounds(sphere, spanPts);
  for (i = 0; i < p->n_verts; i++) 
    SphereAddHPt3(sphere, &p->vl[i].pt, T);
  
  return (Geom *)sphere;
}
						 
