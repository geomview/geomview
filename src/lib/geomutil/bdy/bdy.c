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

/*
 * bdy.c
 * author:  Celeste Fowler
 * date:  June 12, 1992
 */

#include <stdlib.h>
#include "geom.h"
#include "polylistP.h"
#include "ooglutil.h"
#include "plutil.h"
#include "bdy.h"

#ifndef min
# define min(a, b) ((a < b) ? a : b)
#endif
#ifndef max
# define max(a, b) ((a > b) ? a : b)
#endif

static char msg[] = "bdy.c";

/* Precision.  The global declaration is a hack to get the value out 
 * of Bdy and into EdgeCmp where it is needed (EdgeCmp must be called by 
 * qsort. */
static float precision;

int EdgeCmp(HPoint3 **a, HPoint3 **b)
{
  int d;
  float dist00, dist01, dist11, dist10;

  while (1) {
    dist00 = HPt3Distance(a[0], b[0]);
    dist01 = HPt3Distance(a[0], b[1]);
    dist11 = HPt3Distance(a[1], b[1]);
    dist10 = HPt3Distance(a[1], b[0]);

    if (dist00 > precision && dist01 > precision) break;
    if (dist11 > precision && dist10 > precision) break;
    if (dist00 < precision && dist11 > precision) break;
    if (dist01 < precision && dist10 > precision) break;
    if (dist11 < precision && dist00 > precision) break;
    if (dist10 < precision && dist01 > precision) break;
    return 0;
  }

  d = memcmp(a[0], b[0], sizeof(HPoint3));
  if(d == 0)
    d = memcmp(a[1], b[1], sizeof(HPoint3));
  return d;
}

Geom *Bdy(Geom *g, float prec) {
  int i, j, k;

  PolyList *p;
  int n_edges;
  HPoint3 **edges;

  Geom *vect;
  short *vcounts, *ccounts;
  Point3 *verts;
  ColorA color;

  g = (Geom *)AnyToPL(g, TM_IDENTITY);
  if (g == NULL) return NULL;
 
  precision = prec;

  /* Get a consolidated version of the polylist. */
  p = (PolyList *)PLConsol(g, 0.0); 
  GeomDelete(g); 
 
  /* Count the number of edges in the polygon */
  for (i = n_edges = 0; i < p->n_polys; i++) n_edges += p->p[i].n_vertices;

  /* Put the edges in an array and sort it */
  edges = OOGLNewNE(HPoint3 *, 2 * n_edges, msg);
  for (i = k = 0; i < p->n_polys; i++) {
    for (j = 0; j < p->p[i].n_vertices; j++) {
      edges[k * 2] = (HPoint3 *)
	min(p->p[i].v[j], p->p[i].v[(j + 1) % p->p[i].n_vertices]);
      edges[k * 2 + 1] = (HPoint3 *) 
	max(p->p[i].v[j], p->p[i].v[(j + 1) % p->p[i].n_vertices]);
      k++;
    }
    if (p->p[i].n_vertices == 2) k--;
  }
  n_edges = k;
  precision = 0.0;
  qsort(edges, n_edges, 2 * sizeof(HPoint3 *), (int (*)())EdgeCmp);
  precision = prec;

  /* Eliminate everything mentioned more than once */
  for (i = j = k = 0; i < n_edges; i++) 
    if (EdgeCmp(&edges[i*2], &edges[k*2])) {
      if (i == k + 1) {
	edges[j*2] = edges[k*2];
	edges[j*2 + 1] = edges[k*2 + 1];
	j++;
      }
      k = i;
    }
  if (i == k + 1) {
    edges[j*2] = edges[k*2];
    edges[j*2 + 1] = edges[k*2 + 1];
    j++;
  }
  if (!j) {
    OOGLFree(edges);
    GeomDelete((Geom *)p);
    return NULL;
  }
 
  /* Call the vect create routine */
  vcounts = OOGLNewNE(short, j, msg);
  ccounts = OOGLNewNE(short, j, msg);
  verts = OOGLNewNE(Point3, j*2, msg);
  for (i = 0; i < j; i++) {
    vcounts[i] = 2;
    ccounts[i] = 0;
    Pt3Copy((const Point3 *)edges[i*2], &verts[i*2]);
    Pt3Copy((const Point3 *)edges[i*2 + 1], &verts[i*2 + 1]);
  }
  ccounts[0] = 1;
  color.r = 0.0;
  color.g = 0.0;
  color.b = 0.0;
  color.a = 1.0;

  vect = GeomCreate("vect",
		    CR_NVECT, j,
		    CR_VECTC, vcounts,
		    CR_NVERT, j*2,
		    CR_POINT, verts,

		    CR_NCOLR, 1,
		    CR_COLRC, ccounts,
		    CR_COLOR, &color,
		    CR_END);

  OOGLFree(ccounts);
  OOGLFree(vcounts);
  OOGLFree(edges);

  GeomDelete((Geom *)p);

  return vect;

}

