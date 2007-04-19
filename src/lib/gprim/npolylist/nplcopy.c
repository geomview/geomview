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

/* $Header: /home/mbp/geomview-git/geomview-cvs/geomview/src/lib/gprim/npolylist/nplcopy.c,v 1.7 2007/04/19 00:05:17 rotdrop Exp $ */

/*
 * Geometry object routines
 *
 * Copy an NPolyList.
 */


#include "npolylistP.h"

NPolyList *
NPolyListCopy(NPolyList *pl)
{
  
  NPolyList *newpl;
  Poly *newp;
  Vertex *newvl;
  HPtNCoord *newv;
  int *newvi;
  int *newpv;
  ColorA *newvcol = NULL;
  Vertex **newvp;
  int i, k;

  if (pl == NULL) {
    return NULL;
  }
	
  newv = OOGLNewNE(HPtNCoord, pl->pdim*pl->n_verts, "NPolyList verts");
  newvl = OOGLNewNE(Vertex, pl->n_verts, "NPolyList verts description");
  newp = OOGLNewNE(Poly, pl->n_polys, "NPolyList polygons");
  newvi = OOGLNewNE(int, pl->nvi, "NPolyList vert indices");
  newpv = OOGLNewNE(int, pl->n_polys, "NPolyList polygon vertices");
  if (pl->vcol) {
    newvcol = OOGLNewNE(ColorA, pl->n_verts, "NPolyList vertex colors");
  }
  newpl = OOGLNewE(NPolyList, "NPolyList");
  *newpl = *pl;

  newpl->vi   = newvi;
  newpl->pv   = newpv;
  newpl->v    = newv;
  newpl->vcol = pl->vcol ? newvcol : NULL;
  newpl->p    = newp;
  newpl->vl   = newvl;

  memcpy(newvi, pl->vi, pl->nvi * sizeof(int));
  memcpy(newpv, pl->pv, pl->n_polys * sizeof(int));
  memcpy(newv, pl->v, pl->n_verts * pl->pdim * sizeof(HPtNCoord));
  if (pl->vcol) {
    memcpy(newvcol, pl->vcol, pl->n_verts * sizeof(ColorA));
  }
  memcpy(newp, pl->p, pl->n_polys * sizeof(Poly));
  memcpy(newvl, pl->vl, pl->n_verts * sizeof(Vertex));
  
  newvp = OOGLNewNE(Vertex *, pl->nvi, "NPolyList 3d connectivity");
  for (i = 0; i < newpl->n_polys; i++) {
    Poly *p = &newpl->p[i];
    
    p->v   = newvp;
    newvp += p->n_vertices;
    
    for (k = 0; k < p->n_vertices; k++) {
      p->v[k] = &newpl->vl[newpl->vi[newpl->pv[i]+k]];
    }
  }

  return newpl;
}

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
