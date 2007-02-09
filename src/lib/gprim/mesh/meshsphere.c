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
#include "meshP.h"
#include "sphere.h"

Geom *MeshBoundSphere(Mesh *mesh, Transform T, TransformN *TN, int *axes,
		      int space)
{
  Geom *sphere;

  if (TN) {

    /* Create a dummy sphere, the center will be corrected later */
    sphere = GeomCreate("sphere", CR_SPACE, space, CR_END);
    
    SphereEncompassPoints((Sphere *)sphere,
			  (float *)mesh->p,
			  (mesh->geomflags & MESH_4D) != 0, 4,
			  mesh->nu * mesh->nv,
			  NULL, TN, axes);

  } else {

    if(mesh->geomflags & MESH_4D)
      return GeomBoundSphereFromBBox((Geom *)mesh, T, TN, axes, space);

    sphere = GeomCreate("sphere", CR_ENCOMPASS_POINTS, mesh->p, 
			CR_NENCOMPASS_POINTS, (mesh->nu * mesh->nv),
			CR_AXIS, T, CR_SPACE, space, CR_END);
  }

  return sphere;
}
