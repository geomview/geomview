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

#if defined(HAVE_CONFIG_H) && !defined(CONFIG_H_INCLUDED)
#include "config.h"
#endif

static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

 /*
  * Geometry Routines
  * 
  * Geometry Supercomputer Project
  * 
  * ROUTINE DESCRIPTION:
  *	Draw a Vect (collection of vectors).
  * 
  */

#include "mgP.h"
#include "vectP.h"
#include "hpointn.h"
#include "transformn.h"
#include "cmodel.h"

#include <stdlib.h>
#ifndef alloca
#include <alloca.h>
#endif

static void
draw_projected_vect(mgmapfunc NDmap, void *NDinfo, Vect *v, int flags, int penultimate, int hascolor)
{
    HPointN *h = HPtNCreate(5, NULL);
    HPoint3 *p, *op, *np, *newp;
    ColorA *lastcolor, *c, *newc;
    int i, nc, colored = 0;

    newp = (HPoint3 *)alloca(v->nvert*sizeof(HPoint3));
    newc = (ColorA *)alloca(v->nvert*sizeof(ColorA));

    for(i = 0, op = v->p, np = newp; i < v->nvert; i++, op++, np++) {
	*(HPoint3 *)h->v = *op;
	colored = (*NDmap)(NDinfo, h, np, &newc[i]);
	hascolor = colored;
    }

    for(i = 0, p = newp, c = colored ? newc : v->c; i < v->nvec; i++) {
	register int nv;

	nv = vcount(v->vnvert[i]);
	if(colored) nc = nv;
	else if (hascolor) nc = v->vncolor[i];

	flags |= vwrapped(v->vnvert[i]);

	if(nc==0)
	    if(lastcolor)
		mgpolyline(nv,p,1,lastcolor, flags);
	    else
		mgpolyline(nv,p,nc,c,flags);
	else
	    mgpolyline(nv,p,nc, lastcolor=c, flags);

	p += nv;
	if (hascolor) c += nc;
	flags = (i < penultimate) ? 6 : 2;	/* 2: not first batch member */
    }
    HPtNDelete(h);
}

Vect *
VectDraw(v)
     register Vect *v;
{
	register HPoint3 *p;
	register ColorA *c;
	ColorA edgecolor;
	register int n, hascolor, nc;
	int flags, penultimate, softshadelines = 0;
	ColorA *lastcolor=NULL;
	register Appearance *ap = mggetappearance();

	/* Don't draw if vect-drawing is off. */
	if (v == NULL || (ap->flag & APF_VECTDRAW) == 0)
	    return NULL;
	
	/* draw in conformal model if necessary */
	if (_mgc->space & TM_CONFORMAL_BALL) {
            cmodel_clear(_mgc->space);
            cm_read_vect(v);
            cmodel_draw(0);
	    return v;
	}
 
	p = v->p;
	c = v->c;
	hascolor = (v->ncolor > 0) &&
		!(ap->mat && (ap->mat->override & MTF_EDGECOLOR));

	if (!hascolor && ap->mat) {
	  *(Color *)&edgecolor = ap->mat->edgecolor;
	  edgecolor.a = 1;
	  c = &edgecolor;
	  nc = 1;
	}

	flags = v->nvec > 1 ? 4 : 0; 	/* 4: not last mbr of batch of lines */
	penultimate = v->nvec - 2;

	if(_mgc->NDinfo) {
	    Transform T;
	    float focallen;
	    mgpushtransform();
	    CamGet(_mgc->cam, CAM_FOCUS, &focallen);
	    TmTranslate(T, 0., 0., -focallen);
	    TmConcat(T, _mgc->C2W, T);
	    mgsettransform(T);
	    draw_projected_vect(_mgc->NDmap, _mgc->NDinfo, v, flags, penultimate, hascolor);
	    mgpoptransform();
	    return v;
	}

	if(_mgc->astk->ap.flag & APF_SHADELINES && _mgc->astk->useshader) {
	    ColorA *cs = (ColorA *)alloca(v->nvert * sizeof(ColorA));
	    HPoint3 *tp = p;
	    ColorA *tc = c - hascolor;
	    ColorA *tcs = cs;
	    if(!(_mgc->has & HAS_CPOS))
		mg_findcam();
	    for(n = 0; n < v->nvec; n++) {
		int i, nv = vcount(v->vnvert[n]);
		nc = hascolor ? v->vncolor[n] : 0;
		if(nc > 0) tc++;
		for(i = 0; i < nv; i++, tp++, tcs++) {
		    (*_mgc->astk->shader)(1, tp, &_mgc->camZ, tc, tcs);
		    if(nc > 1) { tc++; nc--; }
		}
	    }
	    tcs = cs;
	    nc = v->nvert;
	    hascolor = 0;

	    for(n = 0; n < v->nvec; n++) {
		int nv = vcount(v->vnvert[n]);
		flags |= vwrapped(v->vnvert[n]);
		mgpolyline(nv, p, nv, tcs, flags);	/* One color per vert */
		tcs += nv;
		p += nv;
		flags = (n < penultimate) ? 6 : 2;
	    }
	    return v;
	}


	for(n = 0; n < v->nvec; n++) {
	    register int nv;

	    nv = vcount(v->vnvert[n]);
	    if (hascolor) nc = v->vncolor[n];


	    flags |= vwrapped(v->vnvert[n]);

	    if(nc == 0)
		if(lastcolor)
		    mgpolyline(nv, p, 1, lastcolor, flags);
		else
		    mgpolyline(nv, p, nc, c, flags);
	    else
		mgpolyline(nv,p,nc,lastcolor=c, flags);

	    p += nv;
	    if (hascolor) c += nc;
	    flags = (n < penultimate) ? 6 : 2;	/* 2: not first batch member */
	}
	return v;
}

