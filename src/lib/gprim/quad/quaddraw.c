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

#ifndef alloca
#include <alloca.h>
#endif
#include <stdlib.h>

#include "quadP.h"
#include "mgP.h"
#include "cmodel.h" 
#include "hpointn.h"
#include "bsptreeP.h"

static void
draw_projected_quad(mgNDctx *NDctx, Quad *qquad)
{
  Quad q = *qquad;
  HPointN *h;
  HPoint3 *op, *np;
  ColorA *nc;
  int npts = 4 * qquad->maxquad;
  int i, colored = 0, alpha = 0;
  mgNDmapfunc mapHPtN = NDctx->mapHPtN;
  Appearance *ap = &_mgc->astk->ap;
  Material *mat = &_mgc->astk->mat;

  q.p = (QuadP *)alloca(npts*sizeof(HPoint3));
  q.n = NULL;
  q.c = (QuadC *)alloca(npts*sizeof(ColorA));
  q.bsptree = NULL;
  nc = q.c[0];
  np = q.p[0];
  op = qquad->p[0];

  h = HPtNCreate(5, NULL);
  if (ap->flag & APF_KEEPCOLOR) {
    colored = 0;
  } else {
    HPoint3 dummyv;
    ColorA dummyc;
    /* Dummy transform to determine whether we have ND colors or not */
    colored = mapHPtN(NDctx, h, &dummyv, &dummyc);
  }

  q.geomflags &= ~VERT_4D;
  for(i = 0; i < npts; i++, op++, np++, nc++) {
    if (qquad->geomflags & VERT_4D) {
      /* Set the point's first THREE components from our 4-D vertex */
      Pt4ToHPtN(op, h);
    } else {
      /* Set the point's first four components from our 4-D vertex */
      HPt3ToHPtN(op, NULL, h);	    
    }
    if (colored) {
      mapHPtN(NDctx, h, np, nc);
      if (nc->a < 1.0) {
	alpha = 1;
      }
    } else {
      mapHPtN(NDctx, h, np, NULL);
    }
  }

  if(colored) {
    q.flag &= ~QUAD_ALPHA;
    q.flag |=  alpha ? QUAD_C|QUAD_ALPHA : QUAD_C;
  }
  
  if (ap->flag & APF_FACEDRAW) {
    if (ap->shading != APF_CONSTANT) {
      QuadComputeNormals(&q);
    }
    if (ap->flag & APF_TRANSP) {
      if ((mat->override & MTF_ALPHA) && (mat->valid & MTF_ALPHA)) {
	if (mat->diffuse.a != 1.0) {
	  q.flag |=  QUAD_ALPHA;
	} else {
	  q.flag &= ~QUAD_ALPHA;
	}
      }
    }
    if ((ap->flag & APF_TRANSP) && (q.flag & QUAD_ALPHA)) {
      BSPTreeCreate((Geom *)(void *)&q);
      BSPTreeAddObject(q.bsptree, (Geom *)(void *)&q);
      BSPTreeFinalize(q.bsptree);
    }
  }
  mgquads(q.maxquad, q.p[0], q.n[0], colored ? q.c[0] : qquad->c[0], q.flag);

  if (q.bsptree) {
    mgbsptree(q.bsptree);
    BSPTreeFree((Geom *)(void *)&q);
  }

  OOGLFree(q.n);
  HPtNDelete(h);
}

Quad *
QuadDraw(Quad *q)
{
  mgNDctx *NDctx = NULL;

  if (q == NULL)
    return NULL;

  mgctxget(MG_NDCTX, &NDctx);

  if(NDctx) {
    draw_projected_quad(NDctx, q);
    return q;
  }
  
  if ((((Quad *)q)->flag & VERT_N) == 0) {
    Appearance *ap = mggetappearance();
    
    if(ap->valid & APF_NORMSCALE ||
       (ap->flag & APF_FACEDRAW && ap->shading != APF_CONSTANT)) {
      QuadComputeNormals(q);
      q->flag |= VERT_N;
    }
  }

  if (q->bsptree == NULL) {
    /* This means we are a top-level drawing node (will never happen ...) */
    BSPTreeCreate((Geom *)q);
  }
  if (q->bsptree->tree == NULL) {
    /* This means we are an inferior drawing node and may add our
     * polygons to the tree, do that.
     */
    BSPTreeAddObject(q->bsptree, (Geom *)q);
  }

  if (_mgc->space & TM_CONFORMAL_BALL) {
    cmodel_clear(_mgc->space);
    cm_read_quad(q);
    cmodel_draw(PL_HASVN|PL_HASPN|PL_HASVCOL);
  } else if(_mgc->astk->useshader) {
    /*
     * Special software shading
     */
    int i, step, lim = q->maxquad * 4;
    HPoint3 *v = q->p[0];
    Point3 *n = q->n[0];
    int cquad = q->c && !(_mgc->astk->mat.override & MTF_DIFFUSE);
    ColorA *oc = cquad ? q->c[0] : (ColorA *)&_mgc->astk->mat.diffuse;
    ColorA *c = (ColorA *)alloca(lim * sizeof(ColorA));
    ColorA *tc = c;

    step = (_mgc->astk->ap.shading == APF_SMOOTH) ? 1 : 4;
    for(i = 0; i < lim; i += step) {
      (*_mgc->astk->shader)(1, v, n, oc, tc);
      if(cquad) oc += step;
      if(step == 4) {
	tc[1] = *tc;
	tc[2] = *tc;
	tc[3] = *tc;
	v += 4;  n += 4;  tc += 4;
      } else {
	v++; n++; tc++;
      }
    }
    mgquads(q->maxquad, q->p[0], q->n[0], c, q->flag);
  } else {
    /*
     * Ordinary shading
     */
    mgquads(q->maxquad, q->p[0], q->n[0], q->c[0], q->flag);
  }

  /* If we have a private BSP-tree, then draw it now. Software shading
   * & transparency will not work, to be fixed.
   */
  if (q->bsptree->geom == (Geom *)q) {
    if (q->bsptree->tree == NULL) {
      BSPTreeFinalize(q->bsptree);
    }
    mgbsptree(q->bsptree);
  }

  return q;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
