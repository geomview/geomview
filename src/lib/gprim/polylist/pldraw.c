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

/* $Header: /home/mbp/geomview-git/geomview-cvs/geomview/src/lib/gprim/polylist/pldraw.c,v 1.20 2008/05/19 04:46:23 rotdrop Exp $ */

/*
 * Draw a PolyList using mg library.
 */

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
  PolyList *npl;
  HPointN *h;
  Poly *np;
  Vertex *ov, *nv;
  int i, j;
  bool colored = false, alpha = false;
  mgNDmapfunc mapHPtN = NDctx->mapHPtN;
  Appearance *ap = &_mgc->astk->ap;
  Material *mat = &_mgc->astk->mat;
  int normal_need;

  if (pl->plproj == NULL) {
    pl->plproj = (PolyList *)GeomCopy((Geom *)pl);
    ApDelete(pl->plproj->ap);
  }

  npl = pl->plproj;

  h = HPtNCreate(5, NULL);
  if (ap->flag & APF_KEEPCOLOR) {
    colored = false;
  } else {
    HPoint3 dummyv;
    ColorA dummyc;
    /* Dummy transform to determine whether we have ND colors or not */
    colored = mapHPtN(NDctx, h, &dummyv, &dummyc) != 0;
  }

  /* Transform vertices */
  npl->geomflags &= ~VERT_4D;
  for(i = 0, ov = pl->vl, nv = npl->vl; i < pl->n_verts; i++, ov++, nv++) {
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
	alpha = true;
      }
    } else {
      nv->vcol = ov->vcol;
      mapHPtN(NDctx, h, &nv->pt, NULL);
    }
  }

  if (colored) {
    if (alpha) {
      npl->geomflags |= COLOR_ALPHA;
    } else {
      npl->geomflags &= ~COLOR_ALPHA;
    }
    npl->geomflags &= ~PL_HASPCOL;
    npl->geomflags |= PL_HASVCOL;    
  } else if (npl->geomflags & GEOM_COLOR) {
    colored = true;
  }

  /* The drawing routines might need either polygon or vertex normals,
   * so if either is missing and either might be needed, we force it
   * to be computed.
   */
  npl->geomflags &= ~(PL_HASVN|PL_HASPN|PL_HASPFL);
  normal_need = (ap->flag & APF_NORMALDRAW) ? PL_HASPN|PL_HASVN : 0;
  if (ap->flag & APF_FACEDRAW) {
    switch (ap->shading) {
    case APF_FLAT:
    case APF_VCFLAT: normal_need |= PL_HASPN; break;
    case APF_SMOOTH: normal_need |= PL_HASVN; break;
    default: break;
    }
    if (GeomHasAlpha((Geom *)npl, ap)) {
      normal_need |= PL_HASPFL|PL_HASPN;
    }
  }
  if (normal_need) {
    PolyListComputeNormals(npl, normal_need);
  }

  if ((_mgc->astk->flags & MGASTK_SHADER) && !(npl->geomflags & GEOM_ALPHA)) {
    ColorA *c = !colored || (mat->override & MTF_DIFFUSE)
      ? (ColorA *)&mat->diffuse : NULL;
	
    switch (ap->shading) {
    case APF_SMOOTH:
      for (i = 0, nv = npl->vl; i < npl->n_verts; i++, nv++) {
	(*_mgc->astk->shader)(1, &nv->pt, &nv->vn,
			      c ? c : &nv->vcol, &nv->vcol);
      }
      npl->geomflags |= PL_HASVCOL;
      mgpolylist(npl->n_polys, npl->p, npl->n_verts, npl->vl, npl->geomflags);
      break;
    case APF_FLAT:
      for (i = 0, np = npl->p; i < npl->n_polys; i++, np++) {
	(*_mgc->astk->shader)(1, &np->v[0]->pt, &np->pn,
			      c ? c : &np->pcol, &np->pcol);
      }
      npl->geomflags |= PL_HASPCOL;
      mgpolylist(npl->n_polys, npl->p, npl->n_verts, npl->vl, npl->geomflags);
      break;
    case APF_VCFLAT:
      for (i = 0, np = npl->p; i < npl->n_polys; i++, np++) {
	VARARRAY(V, HPoint3, np->n_vertices);
	VARARRAY(C, ColorA, np->n_vertices);
	for (j = 0; j < np->n_vertices; j++) {
	  V[j] = np->v[j]->pt;
	  C[j] = (npl->geomflags & PL_HASVCOL)
	    ? np->v[j]->vcol
	    : ((npl->geomflags & PL_HASPCOL) || c == NULL ? np->pcol : *c);

	  (*_mgc->astk->shader)(1, &V[j], (Point3 *)&np->pn, &C[j], &C[j]);
	}
	mgpolygon(np->n_vertices, V, 1, &np->pn, np->n_vertices, C);
      }
      break;
    default:
      break;
    }
  } else {
    /* ordinary shading */
    mgpolylist(npl->n_polys, npl->p, npl->n_verts, npl->vl, npl->geomflags);
  }

  /* Generate a BSP-tree if the object or parts of it might be
   * translucent.
   */
  if (NDctx->bsptree && (npl->geomflags & GEOM_ALPHA)) {
    GeomNodeDataMove((Geom *)pl, (Geom *)npl);
    GeomBSPTree((Geom *)npl, NDctx->bsptree, BSPTREE_ADDGEOM);
    GeomNodeDataMove((Geom *)npl, (Geom *)pl);
  }

  HPtNDelete(h);
}

PolyList *PolyListDraw(PolyList *pl)
{
  mgNDctx *NDctx = NULL;
  Appearance *ap = &_mgc->astk->ap;

  mgctxget(MG_NDCTX, &NDctx);

  if (NDctx) {
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
    int need = PL_HASPFL;

    if (ap->flag & APF_NORMALDRAW) {
      need |= PL_HASPN|PL_HASVN;
    } else if (ap->flag & APF_FACEDRAW) {
      switch (ap->shading) {
      case APF_FLAT:
      case APF_VCFLAT: need |= PL_HASPN; break;
      case APF_SMOOTH: need |= PL_HASVN; break;
      default: break;
      }
    }
    if (need) {
      PolyListComputeNormals(pl, need);
    }
  }
    
  if (_mgc->space & TM_CONFORMAL_BALL) {
    cmodel_clear(_mgc->space);
    cm_read_polylist(pl);
    cmodel_draw(pl->geomflags);
  } else if (!(pl->geomflags & GEOM_ALPHA) &&
	     _mgc->astk->flags & MGASTK_SHADER) {
    /*
     * Software shading
     */
    Poly *p;
    ColorA *c0 = (ColorA *)&_mgc->astk->mat.diffuse;
    ColorA pc, *nc = NULL, *savedc = NULL;
    int i, j;
    int flags = pl->geomflags;

    /* Smooth or facetted? */
    switch (ap->shading) {
    case APF_SMOOTH: {
      Vertex *v;

      if (flags & PL_HASVCOL) {
	savedc = OOGLNewNE(ColorA, pl->n_verts, "saved colours");
      } else if(flags & PL_HASPCOL) {
	/* Rats.  We need vertex colors, but only face colors are
	 * supplied.  Associate a face with each vertex.
	 */
	p = pl->p;
	for(i = 0; i < pl->n_polys; i++, p++)
	  for(j = p->n_vertices; --j >= 0; )
	    p->v[j]->vcol = p->pcol;
	c0 = &pc;
      }
      for (i = 0, nc = savedc, v = pl->vl; i < pl->n_verts; i++, v++) {
	if (savedc) {
	  /* If we had per-vertex colors, save & use them */
	  *nc = v->vcol;
	  (*_mgc->astk->shader)(1, &v->pt, &v->vn, nc, &v->vcol);
	  nc++;
	} else {
	  /* If no per-vertex colors, use material default */
	  if (flags & PL_HASPCOL) {
	    pc = v->vcol;
	  }
	  (*_mgc->astk->shader)(1, &v->pt, &v->vn, c0, &v->vcol);
	}
      }
      mgpolylist(pl->n_polys, pl->p, pl->n_verts, pl->vl, flags|PL_HASVCOL);

      /* Restore colors if trashed */
      if(savedc) {
	for(i = 0, v = pl->vl, nc = savedc; i < pl->n_verts; i++, v++) {
	  v->vcol = *nc++;
	}
	OOGLFree(savedc);
      }
      break;
    }
    case APF_FLAT:
      if (flags & PL_HASPCOL) {
	nc = savedc = OOGLNewNE(ColorA, pl->n_polys, "saved colours");
      }
      for (i = 0, p = pl->p; i < pl->n_polys; i++, p++) {
	if (savedc) {
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
      if (savedc) {
	for (i = 0, p = pl->p, nc = savedc; i < pl->n_polys; i++, p++) {
	  p->pcol = *nc++;
	}
	OOGLFree(savedc);
      }
      break;
    case APF_VCFLAT:
      for (i = 0, p = pl->p; i < pl->n_polys; i++, p++) {
	VARARRAY(V, HPoint3, p->n_vertices);
	VARARRAY(C, ColorA, p->n_vertices);
	for (j = 0; j < p->n_vertices; j++) {
	  V[j] = p->v[j]->pt;
	  C[j] = (flags & PL_HASVCOL)
	    ? p->v[j]->vcol
	    : ((flags & PL_HASPCOL) ? p->pcol : *c0);

	  (*_mgc->astk->shader)(1, &V[j], (Point3 *)&p->pn, &C[j], &C[j]);
	}
	mgpolygon(p->n_vertices, V, 1, &p->pn, p->n_vertices, C);
      }
      break;
    default:
      break;
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
  if (!never_translucent((Geom *)pl) && action == BSPTREE_ADDGEOM) {
    BSPTreeAddObject(tree, (Geom *)pl);
  }

  return pl;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
