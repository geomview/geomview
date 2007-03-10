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

/* $Header: /home/mbp/geomview-git/geomview-cvs/geomview/src/lib/gprim/npolylist/npldraw.c,v 1.13 2007/03/10 13:12:07 rotdrop Exp $ */

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
  PolyList newpl;
  HPointN *h;
  Poly *np;
  Vertex *nv;
  float *ov;
  ColorA *oc;
  int i, colored = 0, alpha = 0;
  float *hdata;
  mgNDmapfunc mapHPtN = NDctx->mapHPtN;
  Appearance *ap = &_mgc->astk->ap;
  Material *mat = &_mgc->astk->mat;
  int normal_need;

  /* Copy the PolyList onto the stack. */
  memset(&newpl, 0, sizeof(PolyList));
  GGeomInit((Geom *)&newpl, PolyListMethods(), PLMAGIC, NULL);
  newpl.n_polys   = pl->n_polys;
  newpl.n_verts   = pl->n_verts;
  newpl.geomflags = pl->geomflags;
  newpl.vl        = pl->vl;
  newpl.p         = pl->p;

  h = HPtNCreate(pl->pdim, NULL);
  if (ap->flag & APF_KEEPCOLOR) {
    colored = 0;
  } else {
    HPoint3 dummyv;
    ColorA dummyc;
    /* Dummy transform to determine whether we have ND colors or not */
    colored = mapHPtN(NDctx, h, &dummyv, &dummyc);
  }

  /* Transform vertices */
  hdata = h->v;
  newpl.geomflags &= ~VERT_4D;
  for(i = 0, ov = pl->v, nv = newpl.vl; i < pl->n_verts; i++, nv++) {
    h->v = ov;
    if (colored) {
      mapHPtN(NDctx, h, &nv->pt, &nv->vcol);
      if (nv->vcol.a != 1.0) {
	alpha = 1;
      }
    } else {
      mapHPtN(NDctx, h, &nv->pt, NULL);
    }
    ov += pl->pdim;
  }

  if(colored) {
    if (alpha) {
      newpl.geomflags |= COLOR_ALPHA;
    } else {
      newpl.geomflags &= ~COLOR_ALPHA;
    }
    newpl.geomflags &= ~PL_HASPCOL;
    newpl.geomflags |= PL_HASVCOL;
    pl->geomflags &= ~NPL_HASVLVCOL; /* mark as invalid */
  } else if ((pl->geomflags & PL_HASVCOL) && !(pl->geomflags & NPL_HASVLVCOL)) {
    /* copy per vertex colors into vertex list */
    for(i = 0, nv = newpl.vl, oc = pl->vcol; i < pl->n_verts; ++i, ++nv, ++oc) {
      nv->vcol = *oc;
    }
    pl->geomflags |= NPL_HASVLVCOL; /* mark as valid */
  }

  /* The drawing routines might need either polygon or vertex normals,
   * so if either is missing and either might be needed, we force it
   * to be computed.
   */
  newpl.geomflags &= ~(PL_HASVN|PL_HASPN|PL_HASPFL);
  normal_need = (ap->flag & APF_NORMALDRAW) ? PL_HASPN|PL_HASVN : 0;
  if (ap->flag & APF_FACEDRAW) {
    if (ap->shading == APF_FLAT) {
      normal_need |= PL_HASPN;
    }
    if (ap->shading == APF_SMOOTH) {
      normal_need |= PL_HASVN;
    }
    if (GeomHasAlpha((Geom *)&newpl, ap)) {
      normal_need |= PL_HASPFL|PL_HASPN;
    }
  }
  PolyListComputeNormals(&newpl, normal_need);

  if(_mgc->astk->flags & MGASTK_SHADER) {
    ColorA *c = !colored && (mat->override & MTF_DIFFUSE)
      ? (ColorA *)&mat->diffuse : NULL;
	
    if(IS_SMOOTH(_mgc->astk->ap.shading)) {
      for(i = 0, nv = newpl.vl; i < newpl.n_verts; i++, nv++) {
	(*_mgc->astk->shader)(1, &nv->pt, &nv->vn,
			      c ? c : &nv->vcol, &nv->vcol);
      }
      newpl.geomflags |= PL_HASVCOL;
    } else {
      for(i = 0, np = newpl.p; i < newpl.n_polys; i++, np++) {
	(*_mgc->astk->shader)(1, &np->v[0]->pt, (Point3 *)&np->pn,
			      c ? c : &np->pcol, &np->pcol);
      }
      newpl.geomflags |= PL_HASPCOL;
    }
  }

  mgpolylist(newpl.n_polys, newpl.p, newpl.n_verts, newpl.vl, newpl.geomflags);

  /* Generate a BSP-tree if the object or parts of it might be
   * translucent.
   */
  if (NDctx->bsptree && (newpl.geomflags & GEOM_ALPHA)) {
    void *old_tagged_app = BSPTreePushAppearance(NDctx->bsptree, (Geom *)pl);
    GeomBSPTree((Geom *)(void *)&newpl, NDctx->bsptree, BSPTREE_ADDGEOM);
    BSPTreePopAppearance(NDctx->bsptree, old_tagged_app);
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
