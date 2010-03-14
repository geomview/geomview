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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef MESHPDEF
#define MESHPDEF

#include "geomclass.h"
#include "mesh.h"
#include "bsptree.h"

struct Mesh {
  GEOMFIELDS;
  int     seq;
  int     nu, nv;
  int     umin, umax, vmin, vmax;
  HPoint3 *p;
  Point3  *n;
  Point3  *nq; /* per quad normals */
  TxST    *u;
  ColorA  *c;
};

static inline Ref *MeshRef(Mesh *m)
{
  union castit {
    Ref  ref;
    Mesh mesh;
  };

  return &((union castit *)m)->ref;
}

static inline Geom *MeshGeom(Mesh *m)
{
  union castit {
    Geom geom;
    Mesh mesh;
  };

  return &((union castit *)m)->geom;
}

#define nuverts(m) (m)->nu
#define nvverts(m) (m)->nv
#define nuquads(m) (((m)->flag & MESH_UWRAP) ? (m)->nu : (m)->nu - 1)
#define nvquads(m) (((m)->flag & MESH_VWRAP) ? (m)->nv : (m)->nv - 1)

/*
 * MESHINDEX(u, v, mesh)
 * Returns the index of a point into the mesh->array given its u & v 
 * coordinates.
 */
#define MESHINDEX(u, v, mesh) \
  (((v)%(mesh)->nv)*(mesh)->nu + ((u)%(mesh)->nu))

/* 
 * MESHPOINT(u, v, mesh, plist)
 * Returns plist[MESHINDEX(u, v, mesh)]
 */
#define MESHPOINT(u, v, mesh, plist) ((plist)[MESHINDEX(u, v, mesh)])

extern Mesh *MeshComputeNormals(Mesh *m, int need);

#endif /* ! MESHPDEF */

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
