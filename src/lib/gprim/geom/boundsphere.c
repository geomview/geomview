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

#include "hpoint3.h"
#include "geomclass.h"
#include "geom.h"
#include "bbox.h"
#include "create.h"

/* Take this out once the create flags are all in the right place */
#include "sphereP.h"

Geom *GeomBoundSphereFromBBox(Geom *geom,
			      Transform T, TransformN *TN, int *axes,
			      int space)
{
  Geom *sphere, *bbox;
  HPoint3 minmax[2];

  /* The BoundSphere really is something 3-dimensional */
  if (axes == NULL) {
    static int dflt_axes[] = { 1, 2, 3, 0 };
    axes = dflt_axes;
  }

  bbox = GeomBound(geom, T, TN);

  if (bbox == NULL)
    return NULL;

  if (TN) {
    HPointN *minmaxN[2] = { NULL, NULL };
    
    BBoxMinMaxND((BBox *)bbox, &minmaxN[0], &minmaxN[1]);
    GeomDelete(bbox);
    HPtNToHPt3(minmaxN[0], axes, &minmax[0]);
    HPtNToHPt3(minmaxN[1], axes, &minmax[1]);
    HPtNDelete(minmaxN[0]);
    HPtNDelete(minmaxN[1]);
  } else {
    BBoxMinMax((BBox *)bbox, &minmax[0], &minmax[1]);
    GeomDelete(bbox);
  }

  HPt3Dehomogenize(&minmax[0], &minmax[0]);
  HPt3Dehomogenize(&minmax[1], &minmax[1]);

  sphere = GeomCreate("sphere", CR_ENCOMPASS_POINTS, minmax,
		      CR_NENCOMPASS_POINTS, 2, CR_SPACE, space,
		      CR_END);

#if 0
  /* ???? is this correct? The bounding box has already been
     transformed by T ... */
  if (sphere != NULL && T != NULL) GeomTransform(sphere, T, TN);
#endif

  return sphere;
}

Geom *GeomBoundSphere(Geom *geom,
		      Transform T, TransformN *TN, int *axes,
		      int space)
{

  if (geom && geom->Class->boundsphere) {
	  return (*geom->Class->boundsphere) (geom, T, TN, axes, space);
  }

  return GeomBoundSphereFromBBox(geom, T, TN, axes, space);
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
