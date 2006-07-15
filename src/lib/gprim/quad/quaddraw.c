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

#include "quadP.h"
#include "mgP.h"
#include "cmodel.h" 
#include "hpointn.h"
#include <stdlib.h>
#ifndef alloca
#include <alloca.h>
#endif

static void
draw_projected_quad(mgmapfunc NDmap, void *NDinfo, Quad *qquad)
{
    Quad q = *qquad;
    HPointN *h = HPtNCreate(5, NULL);
    HPoint3 *op, *np;
    ColorA *nc;
    int npts = 4 * qquad->maxquad;
    int i, colored = 0;
    q.p = (QuadP *)alloca(npts*sizeof(HPoint3));
    q.n = NULL;
    q.c = (QuadC *)alloca(npts*sizeof(ColorA));
    nc = q.c[0];
    np = q.p[0];
    op = qquad->p[0];
    for(i = 0; i < npts; i++, op++, np++, nc++) {
        /* Set the point's first four components from our 4-D vertex */
        *(HPoint3 *)h->v = *op;
        colored = (*NDmap)(NDinfo, h, np, nc);
    }
    q.flag &= ~QUAD_4D;
    if(colored) q.flag |= QUAD_C;
    QuadComputeNormals(&q);
    mgquads( q.maxquad, q.p[0], q.n[0], colored ? q.c[0] : qquad->c[0] );
    OOGLFree(q.n);
    HPtNDelete(h);
}

Quad *
QuadDraw(Quad *q)
{

    if (q == NULL)
      return NULL;

    if(_mgc->NDinfo) {
	Transform T;
	float focallen;
	mgpushtransform();
	CamGet(_mgc->cam, CAM_FOCUS, &focallen);
	TmTranslate(T, 0., 0., -focallen);
	TmConcat(T, _mgc->C2W, T);
	mgsettransform(T);
	draw_projected_quad(_mgc->NDmap, _mgc->NDinfo, q);
	mgpoptransform();
    } else if (_mgc->space & TM_CONFORMAL_BALL) {
      cmodel_clear(_mgc->space);
      cm_read_quad(q);
      cmodel_draw(PL_HASVN|PL_HASPN|PL_HASVCOL);
    } else {

      if ((((Quad *)q)->flag & VERT_N) == 0) {
    	Appearance *ap = mggetappearance();

	if(ap->valid & APF_NORMSCALE ||
		(ap->flag & APF_FACEDRAW && ap->shading != APF_CONSTANT)) {
	    QuadComputeNormals(q);
	    q->flag |= VERT_N;
	}
      }
      if(_mgc->astk->useshader) {
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
	mgquads( q->maxquad, q->p[0], q->n[0], c );
      } else {
	/*
	 * Ordinary shading
	 */
	mgquads( q->maxquad, q->p[0], q->n[0], q->c[0] );
      }
    }

    return q;
}
