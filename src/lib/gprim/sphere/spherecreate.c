/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 * Copyright (C) 2007 Claus-Justus Heine
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

#include <math.h>
#include "geom.h"
#include "geomclass.h"
#include "transform.h"
#include "hpoint3.h"
#include "sphereP.h"
#include "tlist.h"

DEF_FREELIST(Sphere);

void SphereFreeListPrune(void)
{
  FreeListNode *old;
  size_t size = 0;
  
  while (SphereFreeList) {
    old = SphereFreeList;
    SphereFreeList = old->next;
    OOGLFree(old);
    size += sizeof(Sphere);
  }
  OOGLWarn("Freed %ld bytes.\n", size);
}

#if BEZIER_SPHERES
#include "bezier.h"

static float ctrlPnts[] = {
#if old
  1, 0, 0, 1,	1, 0, 1, 1, 	0, 0, 2, 2,
  1, 1, 0, 1,	1, 1, 1, 1,	0, 0, 2, 2,
  0, 2, 0, 2,	0, 2, 2, 2,	0, 0, 4, 4
#else
  0, 0, 1, 1,  0, 1, 1, 1,  0, 2, 0, 2,
  1, 0, 1, 1,  1, 1, 1, 1,  2, 2, 0, 2,
  2, 0, 0, 2,  2, 0, 0, 2,  4, 0, 0, 4,
#endif
};

#define V4 geomtfm[0]

#endif

/* To support all texture mappings supported by mktxmesh we have to
 * compose the sphere of different parts.
 */
static const int ngeomtfm[] = { 8, 2, 8, 8, 4, 8 };

static const Transform geomtfm[][8] = {
  { /* SPHERE_TXNONE, octants */
    {{ 1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{ 1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{-1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{-1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{ 1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0,-1, 0}, {0, 0, 0, 1}},
    {{ 1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0,-1, 0}, {0, 0, 0, 1}},
    {{-1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0,-1, 0}, {0, 0, 0, 1}},
    {{-1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0,-1, 0}, {0, 0, 0, 1}}
  },
  { /* SPHERE_TXSINUSOIDAL, reflection on (x,y)-plane */
    {{ 1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{ 1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0,-1, 0}, {0, 0, 0, 1}},
  },
  { /* SPHERE_TXCYLINDRIAL, octants */
    {{ 1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{ 1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{-1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{-1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{ 1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0,-1, 0}, {0, 0, 0, 1}},
    {{ 1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0,-1, 0}, {0, 0, 0, 1}},
    {{-1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0,-1, 0}, {0, 0, 0, 1}},
    {{-1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0,-1, 0}, {0, 0, 0, 1}}
  },
  { /* SPHERE_TXRECTANGULAR, octants */
    {{ 1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{ 1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{-1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{-1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{ 1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0,-1, 0}, {0, 0, 0, 1}},
    {{ 1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0,-1, 0}, {0, 0, 0, 1}},
    {{-1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0,-1, 0}, {0, 0, 0, 1}},
    {{-1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0,-1, 0}, {0, 0, 0, 1}}
  },
  { /* SPHERE_TXSTEREOGRAPHIC, quarters */
    {{ 1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{ 1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{-1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{-1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
  },
  { /* SPHERE_TXONESIDE, octants */
    {{ 1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{ 1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{-1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{-1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{ 1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0,-1, 0}, {0, 0, 0, 1}},
    {{ 1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0,-1, 0}, {0, 0, 0, 1}},
    {{-1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0,-1, 0}, {0, 0, 0, 1}},
    {{-1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0,-1, 0}, {0, 0, 0, 1}}
  },
};

/* for the transformation of the texture co-ordinates (if any) */
static const Transform textfm[][8] = {
  { /* SPHERE_TXNONE, nothing */
    {{ 0, }},
  },
  { /* SPHERE_TXSINUSOIDAL */
    {{ 1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{ 1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {0, 1, 0, 1}},
  },
  {/* SPHERE_TXCYLINDRIAL */
    {{ 1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0.0, 0, 0, 1}},
    {{-1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {1.0, 0, 0, 1}},
    {{ 1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0.5, 0, 0, 1}},
    {{-1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0.5, 0, 0, 1}},
    {{ 1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {0.0, 1, 0, 1}},
    {{-1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {1.0, 1, 0, 1}},
    {{ 1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {0.5, 1, 0, 1}},
    {{-1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {0.5, 1, 0, 1}},
  },
  { /* SPHERE_TXRECTANGULAR */
    {{ 1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{-1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {1.0, 0, 0, 1}},
    {{ 1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0.5, 0, 0, 1}},
    {{-1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0.5, 0, 0, 1}},
    {{ 1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {0.0, 1, 0, 1}},
    {{-1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {1.0, 1, 0, 1}},
    {{ 1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {0.5, 1, 0, 1}},
    {{-1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {0.5, 1, 0, 1}},
  },
  { /* SPHERE_TXSTEREOGRAPHIC */
    {{ 1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{ 1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{-1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{-1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
  },
  { /* SPHERE_TXONESIDE */
    {{ 1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{ 1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    {{-1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {1, 0, 0, 1}},
    {{-1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {1, 0, 0, 1}},
    {{ 1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {0, 1, 0, 1}},
    {{ 1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {0, 1, 0, 1}},
    {{-1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {1, 1, 0, 1}},
    {{-1, 0, 0, 0}, {0,-1, 0, 0}, {0, 0, 1, 0}, {1, 1, 0, 1}},
  }
};

/* Cache the tlists, there is no need to copy them over and over again. */
static Geom *geomTlist[COUNT(geomtfm)];
static Geom *texTlist[COUNT(textfm)];

static inline void sphere_make_meshhandle(Sphere *sphere)
{
    char meshhname[sizeof("\aSphere::")+sizeof(void *)*2];
    extern HandleOps GeomOps;
    Handle *handle;

    sprintf(meshhname, "\aSphere::%lx", (unsigned long)sphere);
    handle = HandleDoCreate(meshhname, &GeomOps);
    GeomCCreate((Geom *)sphere, InstMethods(),
		CR_NOCOPY, CR_GEOMHANDLE, handle, CR_END);
}

/* Do we need a SphereCopy()? In principle yes. */
Sphere *SphereCopy(Sphere *sphere)
{
  Sphere *nsphere;

  /* simply create a new one and copy over the three defining fields */
  nsphere =
    (Sphere *)GeomCCreate(NULL, SphereMethods(),
			  CR_SPACE, sphere->space,
			  CR_CENTER, &sphere->center,
			  CR_RADIUS, &sphere->radius,
			  CR_SPHERETX, sphere->geomflags & SPHERE_TXMASK,
			  CR_END);
  TmCopy(sphere->axis, nsphere->axis);

  return nsphere;
}

Sphere *SphereCreate(Geom *exist, GeomClass *classp, va_list *a_list)
{
  Sphere *sphere;
  int nencompass_points = 0;
  int attr, copy = 1;
  unsigned texmeth;
  Transform *axis = NULL;
  HPoint3 *encompass_points = NULL;

  if (exist == NULL) {
    FREELIST_NEW(Sphere, sphere);
    GGeomInit(sphere, classp, SPHEREMAGIC, NULL);
    sphere->freelisthead = &SphereFreeList;
    TmIdentity(sphere->axis);
    sphere->NDaxis = NULL;
    sphere->geomhandle = NULL;
    sphere->geom = NULL;
    sphere->tlisthandle = NULL;
    sphere->tlist = NULL;
    sphere->txtlisthandle = NULL;
    sphere->txtlist = NULL;
    sphere->axishandle = NULL;
    sphere->NDaxishandle = NULL;
    sphere->location = L_NONE;
    sphere->origin = L_NONE;
    sphere->radius = 1.0;
    sphere->space = TM_EUCLIDEAN;
    HPt3From(&(sphere->center), 0.0, 0.0, 0.0, 1.0);
    sphere->ntheta = sphere->nphi = SPHERE_DEFAULT_MESH_SIZE;
  } else {
    sphere = (Sphere *)exist;
  }

  texmeth = sphere->geomflags & SPHERE_TXMASK;

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
  case CR_SPHERETX:
    switch (texmeth = va_arg(*a_list, int)) {
    case SPHERE_TXNONE:
    case SPHERE_TXSINUSOIDAL:
    case SPHERE_TXCYLINDRICAL:
    case SPHERE_TXRECTANGULAR:
    case SPHERE_TXSTEREOGRAPHIC:
    case SPHERE_ONEFACE:
      break;
    default:
      OOGLError(1, "Undefined texture mapping method: %d", texmeth);
      if (exist == NULL) {
	GeomDelete ((Geom *)sphere);
      }
      return NULL;
    }
    break;
  case CR_AXIS:
    axis = va_arg(*a_list, Transform *);
    break;
  default:
    if (GeomDecorate(sphere, &copy, attr, a_list)) {
      OOGLError (0, "ListCreate: Undefined attribute: %d", attr);

      if(exist == NULL)
	GeomDelete ((Geom *)sphere);
      return NULL;
    }
    break;
  }
  HPt3Dehomogenize(&(sphere->center), &(sphere->center));

#if BEZIER_SPHERES
  if (sphere->geom == NULL) {
    /* No need to generate those objects anew if we already have a
     * sphere, they never change. Also, we need to make a copy of the
     * control-points with malloc(); should probably change the Bezier
     * object instead.
     */
    float *CPctrlPnts;
    Geom *quadrant;
    Geom *unitsphere;

    /* Bezier does not make a copy of the control points */
    CPctrlPnts = OOGLNewNE(float, (2+1)*(2+1)*4, "copy of sphere ctrl points");
    memcpy(CPctrlPnts, ctrlPnts, sizeof(ctrlPnts));
    quadrant = GeomCCreate(NULL, BezierMethods(), 
			   CR_DEGU, 2, CR_DEGV, 2, 
			   CR_DIM, 4,
			   CR_POINT, CPctrlPnts,
			   CR_END);

    /* TList does make a copy of the transformations */
    unitsphere = GeomCreate("tlist", CR_NELEM, 8, CR_ELEM, V4, CR_END);
    GeomCCreate((Geom *)sphere, InstMethods(),
		CR_GEOM, quadrant,
		CR_TLIST, unitsphere,
		CR_END);
    GeomDelete(quadrant);
    GeomDelete(unitsphere);
  }
#else
  if (sphere->geomhandle == NULL) {
    /* force remeshing on redraw, but not earlier. */
    sphere->geomflags |= SPHERE_REMESH;
    sphere_make_meshhandle(sphere);
  }
  if (texmeth != (sphere->geomflags & SPHERE_TXMASK)) {
    int texidx = SPHERE_TXMETH(texmeth);
    
    /* force remeshing on redraw, but not earlier. */
    sphere->geomflags |= SPHERE_REMESH;
    sphere->geomflags &= ~SPHERE_TXMASK;
    sphere->geomflags |= texmeth;

    if (texidx > 0 && texTlist[texidx] == NULL) {
      texTlist[texidx] = GeomCCreate(NULL , TlistMethods(),
				     CR_NELEM, ngeomtfm[texidx],
				     CR_ELEM, textfm[texidx],
				     CR_END);
    }
    GeomCCreate((Geom *)sphere, InstMethods(),
		CR_TLIST, NULL, CR_TXTLIST, texTlist[texidx], CR_END);
  }
  if (sphere->tlist == NULL) {
    int texidx = SPHERE_TXMETH(sphere->geomflags);

    /* force remeshing on redraw, but not earlier. */
    sphere->geomflags |= SPHERE_REMESH;

    if (geomTlist[texidx] == NULL) {
      geomTlist[texidx] = GeomCCreate(NULL , TlistMethods(),
				      CR_NELEM, ngeomtfm[texidx],
				      CR_ELEM, geomtfm[texidx], CR_END);
    }
    GeomCCreate((Geom *)sphere, InstMethods(),
		CR_TLIST, geomTlist[texidx], CR_END);
  }
#endif

  SphereSwitchSpace(sphere, sphere->space);
  if (nencompass_points && encompass_points != NULL) 
    SphereEncompassHPt3N(sphere, encompass_points, nencompass_points, 
			 (axis == NULL) ? TM_IDENTITY : *axis);

  if (sphere->ap && sphere->ap->mat &&
      (sphere->ap->mat->valid & MTF_ALPHA) &&
      sphere->ap->mat->diffuse.a != 1.0) {
    sphere->geomflags |= COLOR_ALPHA;
  } else {
    sphere->geomflags &= ~COLOR_ALPHA;
  }

  return sphere;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
