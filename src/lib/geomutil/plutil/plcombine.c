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
 * plcombine.c
 * author:  Celeste Fowler
 * date:  June 12, 1992
 */
#include <stdlib.h>
#include "transformn.h"
#include "polylistP.h"
#include "hpoint3.h"
#include "point3.h"
#include "plutil.h"

static char msg[] = "plcombine.c";

/*
 * PolyList combiner.
 * A minor sideshow.  If a and b differ in some fundamental way in terms
 * of per vertex / per face colors / normals, the colors / normals will
 * be eliminated in the result.  If one polylist is three-d and the other
 * is four-d, the result will be three-d
 */
Geom *PLCombine(Geom *a1, Geom *b1) 
{
  PolyList *a = (PolyList *)a1, *b = (PolyList *)b1;
  int i, j, k;
  HPoint3 *point4;
  ColorA *color, *polycolor;
  Point3 *normal;
  Point3 *polynormal;
  int *vert, nvertices, *nvert, *polyflags;
  Geom *new;
  int flags, fourd = 0;

  if (a == NULL) return(GeomCopy((Geom *)b));
  if (b == NULL) return(GeomCopy((Geom *)a));

  if (strcmp(GeomName(a1), "polylist") || strcmp(GeomName(b1), "polylist"))
    OOGLError(0, "Arguements to PLCombine() not of polylist type.");

  nvertices = a->n_verts + b->n_verts;

  /* Copy points, normals, colors. */
  point4 = (HPoint3 *)OOG_NewE(nvertices * sizeof(HPoint3), msg);
  color = (ColorA *)OOG_NewE(nvertices * sizeof(ColorA), msg);
  normal = (Point3 *)OOG_NewE(nvertices * sizeof(Point3), msg);

  for (i = 0; i < a->n_verts; i++) {
    point4[i] = a->vl[i].pt;
    color[i] = a->vl[i].vcol;
    normal[i] = a->vl[i].vn;
  }
  for (i = 0; i < b->n_verts; i++) {
    point4[a->n_verts + i] = b->vl[i].pt;
    color[a->n_verts + i] = b->vl[i].vcol;
    normal[a->n_verts + i] = b->vl[i].vn;
  }

  /* Create polygons */
  nvert = (int *)OOG_NewE((a->n_polys + b->n_polys) * sizeof(int), msg);
  polycolor = 
    (ColorA *)OOG_NewE((a->n_polys + b->n_polys) * sizeof(ColorA), msg);
  polynormal =
    (Point3 *)OOG_NewE((a->n_polys + b->n_polys) * sizeof(Point3), msg);
  polyflags =
    (int *)OOG_NewE((a->n_polys + b->n_polys) * sizeof(int), msg);
  for (i = j = 0; i < a->n_polys; i++) j += a->p[i].n_vertices;
  for (i = 0; i < b->n_polys; i++) j += b->p[i].n_vertices;
  vert = (int *)OOG_NewE(j * sizeof(int), msg);
  for (i = k = 0; i < a->n_polys; i++) {
    nvert[i] = a->p[i].n_vertices;
    polycolor[i] = a->p[i].pcol;
    polynormal[i] = a->p[i].pn;
    polyflags[i] = a->p[i].flags;
    for (j = 0; j < a->p[i].n_vertices; j++) 
      vert[k++] = a->p[i].v[j] - a->vl;
  }
  for (i = 0; i < b->n_polys; i++) {
    nvert[a->n_polys + i] = b->p[i].n_vertices;
    polycolor[a->n_polys + i] = b->p[i].pcol;
    polynormal[a->n_polys + i] = b->p[i].pn;
    polyflags[a->n_polys + i] = b->p[i].flags;
    for (j = 0; j < b->p[i].n_vertices; j++)
      vert[k++] = a->n_verts + b->p[i].v[j] - b->vl;
  }

  flags = a->geomflags;
  fourd = (a->geomflags & VERT_4D) ? 1 : 0;
  
  if ((a->geomflags & PL_HASVCOL) && !(b->geomflags & PL_HASVCOL)) {
    if (b->geomflags & PL_HASPCOL) {
      for (i = 0; i < a->n_polys; i++) 
	polycolor[i] = a->p[i].v[0]->vcol;
      flags ^= PL_HASVCOL;
      flags |= PL_HASPCOL;
    }
    else for (i = 0; i < b->n_verts; i++) {
      color[a->n_verts + i].r = color[a->n_verts + i].g =
	color[a->n_verts + i].b = 0.15;
      color[a->n_verts + i].a = 1.0;
    } 
  }
  if ((a->geomflags & PL_HASPCOL) && !(b->geomflags & PL_HASPCOL)) {
    if (b->geomflags & PL_HASVCOL) {
      for (i = 0; i < b->n_polys; i++) 
	polycolor[a->n_polys + i] = b->p[i].v[0]->vcol;
    }
    else for (i = 0; i < b->n_polys; i++) {
      polycolor[a->n_polys + i].r = polycolor[a->n_polys + i].g = 
	polycolor[a->n_polys + i].b = 0.15;
      polycolor[a->n_polys + i].a = 1.0;
    }
  }
  if (!(flags & PL_HASVCOL) && !(flags & PL_HASPCOL)) {
    if (b->geomflags & PL_HASPCOL) {
      for (i = 0; i < a->n_polys; i++) {
	polycolor[i].r = polycolor[i].g = polycolor[i].b = 0.15;
	polycolor[i].a = 1.0;
      }
      flags |= PL_HASPCOL;
    }
    if (b->geomflags & PL_HASVCOL) {
      for (i = 0; i < a->n_verts; i++) {
	color[i].r = color[i].g = color[i].b = 0.15;
	color[i].a = 1.0;
      }
      flags |= PL_HASVCOL;
    }
  }

  if ((a->geomflags & PL_HASVN)  && !(b->geomflags & PL_HASVN))
    flags ^= PL_HASVN;
  if ((a->geomflags & PL_HASPN)  && !(b->geomflags & PL_HASPN))
    flags ^= PL_HASPN;
  if ((a->geomflags & PL_HASPFL) && !(b->geomflags & PL_HASPFL))
    flags ^= PL_HASPFL;
  if ((a->geomflags & VERT_4D) || (b->geomflags & VERT_4D))
    fourd = 1;

  new = GeomCreate("polylist", 
		   CR_NPOLY, a->n_polys + b->n_polys,
		   CR_POINT4, point4,
		   CR_NORMAL, normal,
		   CR_COLOR, color,
		   CR_NVERT, nvert,
		   CR_VERT, vert,
		   CR_POLYNORMAL, polynormal,
		   CR_POLYFLAGS, polyflags,
		   CR_POLYCOLOR, polycolor,
		   CR_FLAG, flags,
		   CR_4D, fourd,
		   CR_END);

  OOGLFree(point4);
  OOGLFree(color);
  OOGLFree(normal);
  OOGLFree(nvert);
  OOGLFree(polycolor);
  OOGLFree(polynormal);
  OOGLFree(polyflags);
  OOGLFree(vert);

  return(new);

}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
