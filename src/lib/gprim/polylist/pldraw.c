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

/* $Header: /home/mbp/geomview-git/geomview-cvs/geomview/src/lib/gprim/polylist/pldraw.c,v 1.12 2007/02/09 17:15:35 rotdrop Exp $ */

/*
 * Draw a PolyList using mg library.
 */

#ifndef alloca
#include <alloca.h>
#endif
#include <stdlib.h>
#include "polylistP.h"
#include "appearance.h"
#include "mgP.h"	/* need mgP.h (instead of mg.h) for _mgc below */
#include "cmodel.h"
#include "hpointn.h"
#include "bsptreeP.h"

static void
draw_projected_polylist(mgNDctx *NDctx, PolyList *pl)
{
  PolyList newpl = *pl;
  HPointN *h;
  Poly *op, *np;
  Vertex *ov, *nv;
  Vertex **vps;
  int i, j, colored = 0, alpha = 0;
  mgNDmapfunc mapHPtN = NDctx->mapHPtN;
  Appearance *ap = &_mgc->astk->ap;
  Material *mat = &_mgc->astk->mat;
  int normal_need;
    
  /* Copy the PolyList onto the stack. TODO: do NOT do this. */
  newpl.vl = (Vertex *)alloca(pl->n_verts * sizeof(Vertex));
  newpl.p  = (Poly *)alloca(pl->n_polys * sizeof(Poly));
  newpl.ap = NULL;

  for(i = 0, op = pl->p, np = newpl.p; i < pl->n_polys; i++, op++, np++) {
    *np = *op;
    vps = (Vertex **)alloca(np->n_vertices * sizeof(Vertex *));
    np->v = vps;
    for(j = 0; j < np->n_vertices; j++)
      np->v[j] = newpl.vl + (op->v[j] - pl->vl);
  }

  h = HPtNCreate(5, NULL);
  if (ap->flag & APF_KEEPCOLOR) {
    colored = 0;
  } else {
    HPoint3 dummyv;
    ColorA dummyc;
    /* Dummy transform to determine whether we have ND colors or not */
    colored = mapHPtN(NDctx, h, &dummyv, &dummyc);
  }

  /* Transform vertices */
  newpl.geomflags &= ~VERT_4D;
  for(i = 0, ov = pl->vl, nv = newpl.vl; i < pl->n_verts; i++, ov++, nv++) {
    if (pl->geomflags & VERT_4D) {
      /* Set the point's first four components from our 4-D vertex */
      Pt4ToHPtN(&ov->pt, h);
    } else {
      /* Set the point's first THREE components from our 4-D vertex */
      HPt3ToHPtN(&ov->pt, NULL, h);
    }
    if (colored) {
      mapHPtN(NDctx, h, &nv->pt, &nv->vcol);
      if (nv->vcol.a < 1.0) {
	alpha = 1;
      }
    } else {
      nv->vcol = ov->vcol;
      mapHPtN(NDctx, h, &nv->pt, NULL);
    }
  }

  if (colored) {
    if (alpha) {
      newpl.geomflags |= COLOR_ALPHA;
    } else {
      newpl.geomflags &= ~COLOR_ALPHA;
    }
    newpl.geomflags &= ~PL_HASPCOL;
    newpl.geomflags |= PL_HASVCOL;    
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
    if (ap->flag & APF_TRANSP) {
      if ((mat->override & MTF_ALPHA) && (mat->valid & MTF_ALPHA)) {
	if (mat->diffuse.a != 1.0) {
	  newpl.geomflags |= COLOR_ALPHA;
	} else {
	  newpl.geomflags &= COLOR_ALPHA;
	}
      }
      if (newpl.geomflags & COLOR_ALPHA) {
	normal_need |= PL_HASPFL;
      }
    }
  }
  PolyListComputeNormals(&newpl, normal_need);

  if(_mgc->astk->flags & MGASTK_SHADER) {
    ColorA *c = !colored && (mat->override & MTF_DIFFUSE)
      ? (ColorA *)&mat->diffuse : NULL;
	
    if(IS_SMOOTH(ap->shading)) {
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
  if (newpl.bsptree &&
      (ap->flag & APF_FACEDRAW) &&
      (ap->flag & APF_TRANSP) &&
      (newpl.geomflags & COLOR_ALPHA)) {
    void *old_tagged_app = BSPTreePushAppearance((Geom *)pl, ap);
    GeomBSPTree((Geom *)(void *)&newpl, newpl.bsptree, BSPTREE_ADDGEOM);
    BSPTreePopAppearance((Geom *)pl, old_tagged_app);
  }

  HPtNDelete(h);
}

PolyList *PolyListDraw(PolyList *pl)
{
  mgNDctx *NDctx = NULL;

  if (pl->bsptree != NULL) {
    BSPTreeSetAppearance((Geom *)pl);
  }
  
  mgctxget(MG_NDCTX, &NDctx);

  if(NDctx) {
    draw_projected_polylist(NDctx, pl);
    return pl;
  }

  /* The drawing routines might need either polygon or vertex normals,
   * so if either is missing and either might be needed, we force it
   * to be computed.
   */
  if ((pl->geomflags & (PL_HASVN|PL_HASPN|PL_HASPFL))
      !=
      (PL_HASVN|PL_HASPN|PL_HASPFL)) {
    Appearance *ap = &_mgc->astk->ap;
    int need = PL_HASPFL;

    if (ap->flag & APF_NORMALDRAW) {
      need |= PL_HASPN|PL_HASVN;
    } else if (ap->flag & APF_FACEDRAW) {
      if (ap->shading == APF_FLAT) {
	need |= PL_HASPN;
      }
      if (ap->shading == APF_SMOOTH) {
	need |= PL_HASVN;
      }
    }
    PolyListComputeNormals(pl, need);
  }
    
  if (_mgc->space & TM_CONFORMAL_BALL) {
    cmodel_clear(_mgc->space);
    cm_read_polylist(pl);
    cmodel_draw(pl->geomflags);
  } else if(_mgc->astk->flags & MGASTK_SHADER) {
    /*
     * Software shading
     */
    ColorA *c0 = (ColorA *)&_mgc->astk->mat.diffuse;
    ColorA pc, *nc = NULL, *savedc = NULL;
    int i, j;
    int flags = pl->geomflags;

    if(_mgc->astk->mat.override & MTF_DIFFUSE)
      flags &= ~(PL_HASVCOL | PL_HASPCOL);

    /* Smooth or facetted? */
    if(_mgc->astk->ap.shading == APF_SMOOTH) {
      Vertex *v;
      if(flags & PL_HASVCOL) {
	savedc = (ColorA *)alloca(pl->n_verts * sizeof(ColorA));
      } else if(flags & PL_HASPCOL) {
	/* Rats.  We need vertex colors, but only face colors are
	 * supplied.  Associate a face with each vertex.
	 */
	Poly *p = pl->p;
	for(i = 0; i < pl->n_polys; i++, p++)
	  for(j = p->n_vertices; --j >= 0; )
	    p->v[j]->vcol = p->pcol;
	c0 = &pc;
      }
      for(i = 0, nc = savedc, v = pl->vl; i < pl->n_verts; i++, v++) {
	if(savedc) {
	  /* If we had per-vertex colors, save & use them */
	  *nc = v->vcol;
	  (*_mgc->astk->shader)(1, &v->pt, &v->vn, nc, &v->vcol);
	  nc++;
	} else {
	  /* If no per-vertex colors, use material default */
	  if(flags & PL_HASPCOL) pc = v->vcol;
	  (*_mgc->astk->shader)(1, &v->pt, &v->vn, c0, &v->vcol);
	}
      }
      mgpolylist(pl->n_polys, pl->p, pl->n_verts, pl->vl, flags|PL_HASVCOL);

      /* Restore colors if trashed */
      if(savedc) {
	for(i = 0, v = pl->vl, nc = savedc; i < pl->n_verts; i++, v++)
	  v->vcol = *nc++;
      }
    } else /* facetted */ {
      Poly *p;

      if(flags & PL_HASPCOL)
	nc = savedc = (ColorA *)alloca(pl->n_polys * sizeof(ColorA));
      for(i = 0, p = pl->p; i < pl->n_polys; i++, p++) {
	if(savedc) {
	  *nc = p->pcol;
	  (*_mgc->astk->shader)(1, &p->v[0]->pt, (Point3 *)&p->pn,
				nc, &p->pcol);
	  nc++;
	} else {
	  (*_mgc->astk->shader)(1, &p->v[0]->pt, (Point3 *)&p->pn,
				flags & PL_HASVCOL ? &p->v[0]->vcol : c0,
				&p->pcol);
	}
      }
      mgpolylist(pl->n_polys, pl->p, pl->n_verts, pl->vl, flags|PL_HASPCOL);

      /* Restore colors if trashed */
      if(savedc) {
	for(i = 0, p = pl->p, nc = savedc; i < pl->n_polys; i++, p++)
	  p->pcol = *nc++;
      }
    }
  } else {
    /*
     * Ordinary shading
     */
    mgpolylist(pl->n_polys, pl->p, pl->n_verts, pl->vl, pl->geomflags);
  }

  return pl;
}

PolyList *PolyListBSPTree(PolyList *pl, BSPTree *tree, int action)
{

  if (pl->bsptree != NULL && action == BSPTREE_ADDGEOM) {
    BSPTreeAddObject(tree, (Geom *)pl);
  }

  return pl;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
