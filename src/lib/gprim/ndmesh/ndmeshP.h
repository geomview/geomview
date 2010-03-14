/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 * Copyright (C) 2006-2007 Claus-Justus Heine
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

#ifndef NDMESHPDEF
#define NDMESHPDEF

#include "geomclass.h"
#include "hpointn.h"
#include "ndmesh.h"

struct NDMesh {
  GEOMFIELDS;
  int     seq;          /* cH: what is this??? */
  int     meshd;        /* mesh dimension */
  int     *mdim;        /* mesh size, indexed 0..meshd-1 */
  HPointN **p;          /* N-D vertices; dim[0] axis varies fastest */
  ColorA  *c;           /* vertex colors, indexed likewise */
  TxST    *u;           /* texture coordinates, should we need any */
};

static inline Ref *NDMeshRef(NDMesh *m)
{
  union castit {
    Ref    ref;
    NDMesh mesh;
  };

  return &((union castit *)m)->ref;
}

static inline Geom *NDMeshGeom(NDMesh *m)
{
  union castit {
    Geom geom;
    NDMesh mesh;
  };

  return &((union castit *)m)->geom;
}


/* NOTE that these meshes may have missing vertices; some of the p[]
 * and c[] entries may be NULL.
 */

/*
 * mesh indexing:
 *  given a mesh index vector (of length 'meshd') i[0] i[1] ... i[meshd-1]
 * the corresponding index into the p[] and c[] arrays is given by
 *  index = 0;  size = 1;
 *  for(k = 0; k < meshd; k++) { index += size*i[k];  size *= dim[k]; }
 */

#endif /* ! NDMESHPDEF */
