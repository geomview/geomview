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

#include <math.h>
#include "geom.h"
#include "geomclass.h"
#include "transform.h"
#include "hpoint3.h"
#include "sphereP.h"
#include "bezier.h"

static float ctrlPnts[] = {
#ifdef old
  1, 0, 0, 1,	1, 0, 1, 1, 	0, 0, 2, 2,
  1, 1, 0, 1,	1, 1, 1, 1,	0, 0, 2, 2,
  0, 2, 0, 2,	0, 2, 2, 2,	0, 0, 4, 4
#else
  0, 0, 1, 1,  0, 1, 1, 1,  0, 2, 0, 2,
  1, 0, 1, 1,  1, 1, 1, 1,  2, 2, 0, 2,
  2, 0, 0, 2,  2, 0, 0, 2,  4, 0, 0, 4,
#endif
};

static Transform reflections[] = {
  {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
  {{1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
  {{-1,0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
  {{-1,0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
  {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, -1, 0}, {0, 0, 0, 1}},
  {{1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, -1, 0}, {0, 0, 0, 1}},
  {{-1,0, 0, 0}, {0,-1, 0, 0}, {0, 0, -1, 0}, {0, 0, 0, 1}},
  {{-1,0, 0, 0}, {0, 1, 0, 0}, {0, 0, -1, 0}, {0, 0, 0, 1}}
};

Sphere *SphereCreate(exist, classp, a_list) 
Geom *exist;
GeomClass *classp;
va_list *a_list;
{
  Geom *quadrant;
  Geom *unitsphere;
  Sphere *sphere;
  int nencompass_points = 0;
  int attr;
  Transform *axis = NULL;
  HPoint3 *encompass_points = NULL;

  if (exist == NULL) {
    sphere = OOGLNewE(Sphere, "SphereCreate:  new Sphere");
    GGeomInit(sphere, classp, SPHEREMAGIC, NULL);
    sphere->geomhandle = NULL;
    sphere->geom = NULL;
    sphere->tlisthandle = NULL;
    sphere->tlist = NULL;
    sphere->axishandle = NULL;
    sphere->radius = 1.0;
    sphere->space = TM_EUCLIDEAN;
    sphere->location = L_NONE;
    sphere->origin = L_NONE;
    HPt3From(&(sphere->center), 0.0, 0.0, 0.0, 1.0);
  } else sphere = (Sphere *)exist;

  while ((attr = va_arg (*a_list, int))) switch (attr) {
  case CR_CENTER:
    sphere->center = *va_arg(*a_list, HPoint3 *);
    break;
  case CR_RADIUS:
    sphere->radius = va_arg(*a_list, double);
    break;
  case CR_SPACE:
    sphere->space = va_arg(*a_list, int);
    break;
  case CR_ENCOMPASS_POINTS:
    encompass_points = va_arg(*a_list, HPoint3 *);
    break;
  case CR_NENCOMPASS_POINTS:
    nencompass_points = va_arg(*a_list, int);
    break;
  case CR_AXIS:
    axis = va_arg(*a_list, Transform *);
    break;
  default:
    OOGLError (0, "SphereCreate: Undefined option: %d",attr);
    return NULL;
  }
  HPt3Normalize(&(sphere->center), &(sphere->center));

  quadrant = GeomCCreate(NULL, BezierMethods(), 
			 CR_DEGU, 2, CR_DEGV, 2, 
			 CR_DIM, 4, CR_POINT, ctrlPnts, CR_END);
  unitsphere = GeomCreate("tlist", CR_NELEM, 8, CR_ELEM, reflections,
			  CR_END);
  sphere->geom = GeomCCreate(NULL, InstMethods(), CR_GEOM, quadrant, CR_TLIST,
			    unitsphere, CR_END);

  SphereSwitchSpace(sphere, sphere->space);
  if (nencompass_points && encompass_points != NULL) 
    SphereEncompassHPt3N(sphere, encompass_points, nencompass_points, 
			 (axis == NULL) ? TM_IDENTITY : *axis);
  return sphere;
}
