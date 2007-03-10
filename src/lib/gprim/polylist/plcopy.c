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

/* $Header: /home/mbp/geomview-git/geomview-cvs/geomview/src/lib/gprim/polylist/plcopy.c,v 1.6 2007/03/10 12:57:54 rotdrop Exp $ */

/*
 * Geometry object routines
 *
 * Copy a PolyList.
 */


#include "polylistP.h"

PolyList *PolyListCopy(PolyList *polylist)
{
  PolyList *newpl;
  Poly *newp;
  Vertex *newvl;
  int i, j;
  Poly *op, *np;

  if(polylist == NULL) return NULL;

  newvl = OOGLNewNE(Vertex, polylist->n_verts, "PolyList verts");
  newp = OOGLNewNE(Poly, polylist->n_polys, "PolyList polygons");
  newpl = OOGLNewE(PolyList, "PolyList");
  *newpl = *polylist;
  newpl->p = newp;
  newpl->vl = newvl;
  memcpy(newvl, polylist->vl, polylist->n_verts * sizeof(Vertex));
  memcpy(newp, polylist->p, polylist->n_polys * sizeof(Poly));

  for(i = polylist->n_polys, op = polylist->p, np = newp;
      --i >= 0;
      op++, np++) {
    np->v = OOGLNewNE(Vertex *, op->n_vertices, "PolyList vert list");
    for(j = op->n_vertices; --j >= 0; )
      np->v[j] = &newvl[ op->v[j] - polylist->vl ];
  }

  newpl->plproj = NULL;

  return newpl;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
