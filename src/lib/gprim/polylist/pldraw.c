/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Geometry Technologies, Inc.
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

#if defined(HAVE_CONFIG_H) && !defined(CONFIG_H_INCLUDED)
#include "config.h"
#endif

static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Geometry Technologies, Inc.";


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

/* $Header: /home/mbp/geomview-git/geomview-cvs/geomview/src/lib/gprim/polylist/pldraw.c,v 1.1 2000/08/15 16:33:19 mphillips Exp $ */

/*
 * Draw a PolyList using mg library.
 */

#include "polylistP.h"
#include "appearance.h"
#include "mgP.h"	/* need mgP.h (instead of mg.h) for _mgc below */
#include "cmodel.h"
#include "hpointn.h"
#include <stdlib.h>
#ifndef alloca
#include <alloca.h>
#endif

static void
draw_projected_polylist(mgmapfunc NDmap, void *NDinfo, PolyList *pl)
{
    PolyList newpl = *pl;
    HPointN *h;
    Poly *op, *np, *polys;
    Vertex *ov, *nv, *verts;
    Vertex **vps;
    int i, j, colored = 0;
    float *hdata;

    /* Copy the PolyList onto the stack. */
    newpl.vl = (Vertex *)alloca(pl->n_verts * sizeof(Vertex));
    newpl.p = (Poly *)alloca(pl->n_polys * sizeof(Poly));

    for(i = 0, op = pl->p, np = newpl.p; i < pl->n_polys; i++, op++, np++) {
	*np = *op;
	vps = (Vertex **)alloca(np->n_vertices * sizeof(Vertex *));
	np->v = vps;
	for(j = 0; j < np->n_vertices; j++)
	    np->v[j] = newpl.vl + (op->v[j] - pl->vl);
    }

    /* Transform vertices */
    h = HPtNCreate(5, NULL);
    hdata = h->v;
    for(i = 0, ov = pl->vl, nv = newpl.vl; i < pl->n_verts; i++, ov++, nv++) {
	*(HPoint3 *)h->v = ov->pt;
	nv->vcol = ov->vcol;
	colored = (*NDmap)(NDinfo, h, &nv->pt, &nv->vcol);
    }

    newpl.flags &= ~(PL_HASVN|PL_HASPN);
    PolyListComputeNormals(&newpl);
    if(colored)
	newpl.flags = (newpl.flags &~ PL_HASPCOL) | PL_HASVCOL;

    if(_mgc->astk->useshader) {
        ColorA *c = !colored && (_mgc->astk->mat.override & MTF_DIFFUSE)
		? (ColorA *)&_mgc->astk->mat.diffuse : NULL;
	
	if(IS_SMOOTH(_mgc->astk->ap.shading)) {
	    for(i = 0, nv = newpl.vl; i < newpl.n_verts; i++, nv++) {
		(*_mgc->astk->shader)(1, &nv->pt, &nv->vn,
			c ? c : &nv->vcol, &nv->vcol);
	    }
	    newpl.flags |= PL_HASVCOL;
	} else {
	    for(i = 0, np = newpl.p; i < newpl.n_polys; i++, np++) {
		(*_mgc->astk->shader)(1, &np->v[0]->pt, &np->pn,
			c ? c : &np->pcol, &np->pcol);
	    }
	    newpl.flags |= PL_HASPCOL;
	}
    }

    mgpolylist(newpl.n_polys, newpl.p, newpl.n_verts, newpl.vl, newpl.flags);
    h->v = hdata;
    HPtNDelete(h);
}

PolyList *
PolyListDraw( register PolyList *pl )
{
    extern int conformal_sphere;
    if (pl == NULL)
      return NULL;
    

    if(_mgc->NDinfo) {
	Transform T;
	float focallen;
	mgpushtransform();
	CamGet(_mgc->cam, CAM_FOCUS, &focallen);
	TmTranslate(T, 0., 0., -focallen);
	TmConcat(T, _mgc->C2W, T);
	mgsettransform(T);

	draw_projected_polylist(_mgc->NDmap, _mgc->NDinfo, pl);

	mgpoptransform();
	return pl;
    }

    /*
     * The drawing routines might need either polygon or vertex normals,
     * so if either is missing and either might be needed,
     * we force it to be computed.
     */
    if((pl->flags & (PL_HASVN|PL_HASPN)) != (PL_HASVN|PL_HASPN)) {
	register Appearance *ap = mggetappearance();
	if((ap->shading != APF_CONSTANT && ap->flag & APF_FACEDRAW)
			|| ap->flag & APF_NORMALDRAW)
	    PolyListComputeNormals(pl);
    }

    if (_mgc->space & TM_CONFORMAL_BALL) {
	cmodel_clear(_mgc->space);
	cm_read_polylist(pl);
	cmodel_draw(pl->flags);
    } else if(_mgc->astk->useshader) {

	/*
	 * Software shading
	 */
	ColorA *c0 = (ColorA *)&_mgc->astk->mat.diffuse;
	ColorA pc, *nc, *savedc = NULL;
	int i, j;
	int flags = pl->flags;

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
		    (*_mgc->astk->shader)(1, &v->pt, &v->vn,
			nc, &v->vcol);
		    nc++;
		} else {
			/* If no per-vertex colors, use material default */
		    if(flags & PL_HASPCOL) pc = v->vcol;
		    (*_mgc->astk->shader)(1, &v->pt, &v->vn, c0, &v->vcol);
		}
	    }
	    mgpolylist(pl->n_polys, pl->p, pl->n_verts, pl->vl,
		flags|PL_HASVCOL);

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
		    (*_mgc->astk->shader)(1, &p->v[0]->pt, &p->pn,
				nc, &p->pcol);
		    nc++;
		} else {
		    (*_mgc->astk->shader)(1, &p->v[0]->pt, &p->pn,
				flags & PL_HASVCOL ? &p->v[0]->vcol : c0,
				&p->pcol);
		}
	    }
	    mgpolylist(pl->n_polys, pl->p, pl->n_verts, pl->vl,
		flags|PL_HASPCOL);

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
	mgpolylist(pl->n_polys, pl->p, pl->n_verts, pl->vl, pl->flags);
    }
    return pl;

}
