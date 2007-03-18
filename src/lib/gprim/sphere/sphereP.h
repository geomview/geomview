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
#ifndef SPHEREP_H
#define SPHEREP_H

#include "sphere.h"
#include "instP.h"
#include "mesh.h"

#ifndef BEZIER_SPHERES
# define BEZIER_SPHERES 0 /* define to 1 to use bezier splines, to 0 to
			   * use a mesh based on a polar co-ordinate
			   * system.
			   */
#endif

#define SPHEREFIELDS \
  INSTFIELDS; \
  float radius; \
  HPoint3 center; \
  int space;	  \
  int ntheta;	  \
  int nphi

struct Sphere {
  SPHEREFIELDS;
};

#define SPHEREMAGIC	GeomMagic('s', 1)

#define SPHERE_REMESH GEOMFLAG(0x01) /* need to recompute the mesh */
#define SPHERE_DEFAULT_MESH_SIZE 10  /* otherwise from ap->dice */

/* texture co-ordinates a la Orrery's mkmeshtx*/

enum {
  SPHERE_TXNONE =          GEOMFLAG(0 << 1), /* 0x0 */
  SPHERE_TXSINUSOIDAL =    GEOMFLAG(1 << 1), /* 0x2 */
  SPHERE_TXCYLINDRICAL =   GEOMFLAG(2 << 1), /* 0x4 */
  SPHERE_TXRECTANGULAR =   GEOMFLAG(3 << 1), /* 0x6 */
  SPHERE_TXSTEREOGRAPHIC = GEOMFLAG(4 << 1), /* 0x8 */
  SPHERE_ONEFACE =         GEOMFLAG(5 << 1)  /* 0xa */ 
};

#define SPHERE_TXMASK GEOMFLAG(0xe)
#define SPHERE_TXMETH(flags) (((flags) & SPHERE_TXMASK) >> (GEOMFL_SHIFT+1))

extern GeomClass *SphereMethods();
extern Sphere *SphereCreate(Geom *, GeomClass *, va_list *a_list );
extern Sphere *SphereFLoad(IOBFILE *, char *);
extern Sphere *SphereSave(Sphere *, char *);
extern Sphere *SphereFSave(Sphere *, FILE *, char *);
extern Sphere *SphereDice(Sphere *sphere, int nu, int nv);
#if !BEZIER_SPHERES
extern void SphereReDice(Sphere *sphere);
extern Sphere *SphereBSPTree(Sphere *sphere, BSPTree *bsptree, int action);
extern Sphere *SphereDraw(Sphere *sphere);
#endif
extern Sphere *SphereCopy(Sphere *os);

/* These are non-mallocing if sphere exists; will create a new sphere and
 * return it if sphere does not exist */

#endif

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
