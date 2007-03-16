/* Copyright (C) 2007 Claus-Justus Heine
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

#include "mg.h"
#include "sphereP.h"
#include "bsptreeP.h"

static inline bool check_remesh(Sphere *sphere, const Appearance *ap)
{
  if (sphere->geomflags & SPHERE_REMESH) {
    return true;
  }
  if (ap->valid & APF_DICE) {
    if (sphere->ntheta == ap->dice[0] &&
	sphere->nphi == ap->dice[1]) {
      return false;
    }
    sphere->ntheta = ap->dice[0];
    sphere->nphi = ap->dice[1];
    sphere->geomflags |= SPHERE_REMESH;
    return true;
  }
  return false;
}

Sphere *
SphereDraw(Sphere *sphere)
{
  /* the mesh is generated lazily on demand, so do it now if necessary */
  if (check_remesh(sphere, mggetappearance())) {
    SphereReDice(sphere);
  }

  /* then just call the method from the base-class */
  return (Sphere *)sphere->Class->super->draw((Geom *)sphere);
}

Sphere *SphereBSPTree(Sphere *sphere, BSPTree *bsptree, int action)
{
  /* no need to do stuff if we cannot be translucent */
  if (never_translucent((Geom *)sphere)) {
    return sphere;
  }

  /* the mesh is generated lazily on demand, so do it now if necessary */
  if (action == BSPTREE_ADDGEOM && (sphere->geomflags & SPHERE_REMESH)) {
    SphereReDice(sphere);
  }
    
  /* and now call the method from the base class */
  return
    (Sphere *)sphere->Class->super->bsptree((Geom *)sphere, bsptree, action);
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
