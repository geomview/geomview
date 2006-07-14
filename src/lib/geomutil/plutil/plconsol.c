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
#include <stdlib.h>
#include "geomclass.h"
#include "geom.h"
#include "polylist.h"
#include "polylistP.h"
#include "hpoint3.h"
#include "plutil.h"

static char msg[] = "plconsol.c";

/* This is the precision, declared globally so that it will
 * be accessible in the compare routine which must be called 
 * from qsort */
static float precision;

float Pt4Distance(HPoint3 *a, HPoint3 *b) {
  float dx, dy, dz, dw;
  dx = a->x - b->x;
  dy = a->y - b->y;
  dz = a->z - b->z;
  dw = a->w - b->w;
  return (sqrt(dx*dx + dy*dy + dz*dz + dw*dw));
}


/*
 * Note:  This deals only with the location of the points, not with 
 * their colors or normals.  
 */
int VertexCmp(const void *a, const void *b) 
{
  if (Pt4Distance(&((Vertex *)a)->pt, &((Vertex *)b)->pt) > precision) {
    return memcmp(a, b, sizeof(Vertex));
  }
  return 0;
}

Geom *PLConsol(Geom *g, float prec) {
  Vertex **table;
  PolyList *o = (PolyList *)g, *o2;
  int i, j;

  if (g == NULL) return NULL;

  /* Make sure we have the right thing. */
  if (strcmp(GeomName(g), "polylist")) {
    OOGLError(0, "Object not of polylist type.");
    return NULL;
  }

  precision = prec;

  /* Create a copy of the data structure. */
  o2 = (PolyList *)GeomCopy((Geom *)o);
  
  /* Sort the new array. */
  precision = 0.0;
  qsort(o2->vl, o2->n_verts, sizeof(Vertex), VertexCmp);
  precision = prec;

  /* Consolidate the new array to eliminate identical vertices. */
  for (i = j = 0; i < o2->n_verts; i++) 
    if (VertexCmp(&o2->vl[i], &o2->vl[j]))
      o2->vl[++j] = o2->vl[i]; 
  o2->n_verts = j+1;

  /* Create a lookup table of the new addresses of the vertices. */
  table = (Vertex **)OOG_NewE(o->n_verts * sizeof(Vertex *), msg);
  for (i = 0; i < o->n_verts; i++) 
    table[i] = bsearch(&(o->vl[i]), o2->vl, o2->n_verts, sizeof(Vertex),
		       VertexCmp);

  /* Match the polygons in the structure with the new vertices.  */
  for (i = 0; i < o2->n_polys; i++)
    for (j = 0; j < o2->p[i].n_vertices; j++)
      o2->p[i].v[j] = table[o2->p[i].v[j] - o2->vl];

  /* Trim off the excess memory */
  o2->vl = OOGLRenewNE(Vertex, o2->vl, o2->n_verts, msg);

  return((Geom *)o2);

}









