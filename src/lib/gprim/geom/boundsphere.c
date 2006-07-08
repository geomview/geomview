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

#if defined(HAVE_CONFIG_H) && !defined(CONFIG_H_INCLUDED)
#include "config.h"
#endif

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

#include "hpoint3.h"
#include "geomclass.h"
#include "geom.h"
#include "bbox.h"
#include "create.h"

/* Take this out once the create flags are all in the right place */
#include "sphereP.h"

Geom *
GeomBoundSphereFromBBox(Geom *geom, Transform T, int space)
{
  HPoint3 minmax[2];
  Geom *sphere, *bbox = GeomBound(geom, T);

  if (bbox == NULL) return NULL;
  BBoxMinMax((BBox *)bbox, &minmax[0], &minmax[1]);
  GeomDelete(bbox);
  HPt3Normalize(&minmax[0], &minmax[0]);
  HPt3Normalize(&minmax[1], &minmax[1]);

  sphere = GeomCreate("sphere", CR_ENCOMPASS_POINTS, minmax,
		      CR_NENCOMPASS_POINTS, 2, CR_SPACE, space,
		      CR_END);
  if (sphere != NULL && T != NULL) GeomTransform(sphere, T);
  return sphere;
}

Geom *
GeomBoundSphere(geom, T, space)
     Geom *geom;
     Transform T;
     int space;
{

  if (geom && geom->Class->boundsphere) {
    return (*geom->Class->boundsphere) (geom, T==NULL ? TM_IDENTITY : T,
					space);
  }

  return GeomBoundSphereFromBBox(geom, T, space);
}
