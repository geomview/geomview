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

/* $Header: /home/mbp/geomview-git/geomview-cvs/geomview/src/lib/gprim/npolylist/npldraw.c,v 1.7 2006/10/18 19:41:41 rotdrop Exp $ */

/*
 * Draw a PolyList using mg library.
 */

#include "npolylistP.h"
#include "polylistP.h"	/* Need plain PolyList, too, for mgpolylist() */
#include "appearance.h"
#include "mgP.h"	/* need mgP.h (instead of mg.h) for _mgc below */
#include "hpointn.h"
#include <stdlib.h>
#ifndef alloca
#include <alloca.h>
#endif

static void
draw_projected_polylist(mgNDctx *NDctx, NPolyList *pl)
{
  PolyList newpl;
  HPointN *h;
  NPoly *op;
  Poly *np;
  Vertex *nv;
  float *ov;
  ColorA *oc;
  Vertex **vps;
  int i, j, k, colored = 0;
  float *hdata;
  mgNDmapfunc mapHPtN = NDctx->mapHPtN;

  /* Copy the PolyList onto the stack. */
  newpl.n_polys = pl->n_polys;
  newpl.n_verts = pl->n_verts;
  newpl.flags = pl->flags;
  newpl.vl = (Vertex *)alloca(pl->n_verts * sizeof(Vertex));
  newpl.p = (Poly *)alloca(pl->n_polys * sizeof(Poly));

  for(i = 0, op = pl->p, np = newpl.p; i < pl->n_polys; i++, op++, np++) {
    np->n_vertices = op->n_vertices;
    np->v = vps = (Vertex **)alloca(np->n_vertices * sizeof(Vertex *));
    np->pcol = op->pcol;
    for(j = 0, k = op->vi0; j < np->n_vertices; j++, k++)
      np->v[j] = &newpl.vl[pl->vi[k]];
  }

  /* Transform vertices */
  h = HPtNCreate(pl->pdim, NULL);
  hdata = h->v;
  ov = pl->v;
  oc = pl->vcol;
  for(i = 0, ov = pl->v, nv = newpl.vl; i < pl->n_verts; i++, nv++) {
    h->v = ov;
    colored = mapHPtN(NDctx, h, &nv->pt, &nv->vcol);
    ov += pl->pdim;
  }

  if(colored) {
    newpl.flags = (newpl.flags &~ PL_HASPCOL) | PL_HASVCOL;
  } else if((oc = pl->vcol) != NULL) {
    for(i = pl->n_verts, nv = newpl.vl; --i >= 0; nv++)
      nv->vcol = *oc++;
  }
  if(pl->st != NULL) {
    float *st = pl->st;
    for(i = 0, nv = newpl.vl; i < pl->n_verts; i++, nv++) {
      nv->st[0] = *st++;
      nv->st[1] = *st++;
    }
  }
  newpl.flags &= ~(PL_HASVN|PL_HASPN);
  PolyListComputeNormals(&newpl);
  mgpolylist(newpl.n_polys, newpl.p, newpl.n_verts, newpl.vl, newpl.flags);
  h->v = hdata;
  HPtNDelete(h);
}

NPolyList *
NPolyListDraw( NPolyList *pl )
{
  static int warned = 0;
  mgNDctx *NDctx = NULL;

  if (pl == NULL)
    return NULL;

  mgctxget(MG_NDCTX, &NDctx);

  if(NDctx) {
    draw_projected_polylist(NDctx, pl);
    return pl;
  }

  if(!warned) {
    OOGLError(0,"Sorry, need to turn on N-D mode before nOFF objects become visible.");
    warned = 1;
  }
  return NULL;
}
