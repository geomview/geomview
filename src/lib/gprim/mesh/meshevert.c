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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#include "meshP.h"

Mesh *MeshEvert(Mesh *m)
{
  int i;
  Point3 *n;

  if (m)
    {
      if (m->geomflags & MESH_EVERT)
	m->geomflags &= ~MESH_EVERT;
      else
	m->geomflags |= MESH_EVERT;
      if (m->geomflags & MESH_N) {
	for (i = 0, n = m->n; i < m->nu*m->nv; i++,n++){
	  n->x = -n->x;
	  n->y = -n->y;
	  n->z = -n->z;
	}
      }
      if (m->geomflags & MESH_NQ) {
	for (i = 0, n = m->nq; i < m->nu*m->nv; i++,n++){
	  n->x = -n->x;
	  n->y = -n->y;
	  n->z = -n->z;
	}
      }
      MeshComputeNormals(m, MESH_N|MESH_NQ);
    }
  return m;
}
