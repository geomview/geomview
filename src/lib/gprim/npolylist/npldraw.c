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

/* $Header: /home/mbp/geomview-git/geomview-cvs/geomview/src/lib/gprim/npolylist/npldraw.c,v 1.17 2007/04/19 00:05:17 rotdrop Exp $ */

/*
 * Draw a PolyList using mg library.
 */

#include "npolylistP.h"
#include "polylistP.h"	/* Need plain PolyList, too, for mgpolylist() */
#include "appearance.h"
#include "mgP.h"	/* need mgP.h (instead of mg.h) for _mgc below */
#include "bsptreeP.h"
#include "hpointn.h"
#include <stdlib.h>
#ifndef alloca
#include <alloca.h>
#endif

static void
draw_projected_polylist(mgNDctx *NDctx, NPolyList *pl)
{
  PolyList npl;
  HPointN *h;
  Poly *np;
  Vertex *nv;
  float *ov;
  ColorA *oc;
  int i, j;
  bool colored = false, alpha = false;
  float *hdata;
  mgNDmapfunc mapHPtN = NDctx->mapHPtN;
  Appearance *ap = &_mgc->astk->ap;
  Material *mat = &_mgc->astk->mat;
  int normal_need;

  /* Copy the PolyList onto the stack. */
  memset(&npl, 0, sizeof(PolyList));
  GGeomInit((Geom *)(void *)&npl, PolyListMethods(), PLMAGIC, NULL);
  npl.n_polys   = pl->n_polys;
  npl.n_verts   = pl->n_verts;
  npl.geomflags = pl->geomflags;
  npl.vl        = pl->vl;
  npl.p         = pl->p;

  h = HPtNCreate(pl->pdim, NULL);
  if (ap->flag & APF_KEEPCOLOR) {
    colored = false;
  } else {
    HPoint3 dummyv;
    ColorA dummyc;
    /* Dummy transform to determine whether we have ND colors or not */
    colored = mapHPtN(NDctx, h, &dummyv, &dummyc) != 0;
  }

  /* Transform vertices */
  hdata = h->v;
  npl.geomflags &= ~VERT_4D;
  for(i = 0, ov = pl->v, nv = npl.vl; i < pl->n_verts; i++, nv++) {
    h->v = ov;
    if (colored) {
      mapHPtN(NDctx, h, &nv->pt, &nv->vcol);
      if (nv->vcol.a != 1.0) {
	alpha = true;
      }
    } else {
      mapHPtN(NDctx, h, &nv->pt, NULL);
    }
    ov += pl->pdim;
  }

  if (colored) {
    if (alpha) {
      npl.geomflags |= COLOR_ALPHA;
    } else {
      npl.geomflags &= ~COLOR_ALPHA;
    }
    npl.geomflags &= ~PL_HASPCOL;
    npl.geomflags |= PL_HASVCOL;
    pl->geomflags &= ~NPL_HASVLVCOL; /* mark as invalid */
  } else if ((pl->geomflags & PL_HASVCOL) && !(pl->geomflags & NPL_HASVLVCOL)) {
    /* copy per vertex colors into vertex list */
    for(i = 0, nv = npl.vl, oc = pl->vcol; i < pl->n_verts; ++i, ++nv, ++oc) {
      nv->vcol = *oc;
    }
    pl->geomflags |= NPL_HASVLVCOL; /* mark as valid */
  } else if (npl.geomflags & GEOM_COLOR) {
    colored = true;
  }

  /* The drawing routines might need either polygon or vertex normals,
   * so if either is missing and either might be needed, we force it
   * to be computed.
   */
  npl.geomflags &= ~(PL_HASVN|PL_HASPN|PL_HASPFL);
  normal_need = (ap->flag & APF_NORMALDRAW) ? PL_HASPN|PL_HASVN : 0;
  if (ap->flag & APF_FACEDRAW) {
    switch (ap->shading) {
    case APF_FLAT:
    case APF_VCFLAT: normal_need |= PL_HASPN; break;
    case APF_SMOOTH: normal_need |= PL_HASVN; break;
    default: break;
    }
    if (GeomHasAlpha((Geom *)(void *)&npl, ap)) {
      normal_need |= PL_HASPFL|PL_HASPN;
    }
  }
  if (normal_need) {
    PolyListComputeNormals(&npl, normal_need);
  }

  if((_mgc->astk->flags & MGASTK_SHADER) && !(npl.geomflags & GEOM_ALPHA)) {
    ColorA *c = !colored || (mat->override & MTF_DIFFUSE)
      ? (ColorA *)&mat->diffuse : NULL;
	
    switch (ap->shading) {
    case APF_SMOOTH:
      if (!(npl.geomflags & PL_HASVCOL)) {
	if (npl.geomflags & PL_HASPCOL) {
	  for (i = 0, np = npl.p; i < npl.n_polys; i++, np++) {
	    for (j = 0; j < np->n_vertices; j++) {
	      np->v[j]->vcol = np->pcol;
	    }
	  }
	}
      }
      for (i = 0, nv = npl.vl; i < npl.n_verts; i++, nv++) {
	(*_mgc->astk->shader)(1, &nv->pt, &nv->vn,
			      c ? c : &nv->vcol, &nv->vcol);
      }
      npl.geomflags |= PL_HASVCOL;
      mgpolylist(npl.n_polys, npl.p, npl.n_verts, npl.vl, npl.geomflags);
      break;
    case APF_FLAT:
      for (i = 0, np = npl.p; i < npl.n_polys; i++, np++) {
	(*_mgc->astk->shader)(1, &np->v[0]->pt, (Point3 *)&np->pn,
			      c ? c : &np->pcol, &np->pcol);
      }
      npl.geomflags |= PL_HASPCOL;
      mgpolylist(npl.n_polys, npl.p, npl.n_verts, npl.vl, npl.geomflags);
      break;
    case APF_VCFLAT:
      for (i = 0, np = npl.p; i < npl.n_polys; i++, np++) {
	HPoint3 V[np->n_vertices];
	ColorA C[np->n_vertices];
	for (j = 0; j < np->n_vertices; j++) {
	  V[j] = np->v[j]->pt;
	  C[j] = (npl.geomflags & PL_HASVCOL)
	    ? np->v[j]->vcol
	    : ((npl.geomflags & PL_HASPCOL) ? np->pcol : *c);

	  (*_mgc->astk->shader)(1, &V[j], (Point3 *)&np->pn, &C[j], &C[j]);
	}
	mgpolygon(np->n_vertices, V, 1, &np->pn, np->n_vertices, C);
      }
      break;
    }
  } else {
    /* ordinary shading */
    mgpolylist(npl.n_polys, npl.p, npl.n_verts, npl.vl, npl.geomflags);
  }
  

  /* Generate a BSP-tree if the object or parts of it might be
   * translucent.
   */
  if (NDctx->bsptree && (npl.geomflags & GEOM_ALPHA)) {
    GeomNodeDataMove((Geom *)pl, (Geom *)(void *)&npl);
    GeomBSPTree((Geom *)(void *)&npl, NDctx->bsptree, BSPTREE_ADDGEOM);
    GeomNodeDataMove((Geom *)(void *)&npl, (Geom *)pl);
  }

  h->v = hdata;
  HPtNDelete(h);
}

NPolyList *NPolyListDraw(NPolyList *pl)
{
  mgNDctx *NDctx = NULL;

  mgctxget(MG_NDCTX, &NDctx);

  if(NDctx) {
    draw_projected_polylist(NDctx, pl);
    return pl;
  }

  return NULL;
}

/* A dummy, just to make GeomBSPTree() not bail out. */
NPolyList *NPolyListBSPTree(NPolyList *pl, BSPTree *tree, int action)
{
  return pl;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
