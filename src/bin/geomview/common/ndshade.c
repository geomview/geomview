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

#include "mgP.h"
#include "hpointn.h"
#include "../common/drawer.h"

#include <stdlib.h>
#ifndef alloca
#include <alloca.h>
#endif

/*
 * Arguments:
 *  mginfo - pointer to an "ndstuff" structure (from drawer.h), containing:
 *     T - transformation from object to N-D camera space
 *     axes - array of 4 axis indices, giving the subspace seen by this view
 *     Tc - (dimension) x (n-color-axes) transform from object space to
 *		color axis space.  Use NULL if not projecting colors.
 *		Given vectors in camera space, could get Tc with:
 *		Tc = TmNConcat(T, {matrix of projection vectors}, NULL)
 *     ncm - number of colormaps
 *     cm - colormap array
 *  p - the N-D HPointN to be transformed, in object space
 *  np - resulting 3-D HPoint3 point, in camera space
 *  c - resulting color; only changed if ncm > 0
 *
 * Results: returns 1 if color was set, else 0.
 */

static int
map_ND_point(mgNDctx *mgNDctx, HPointN *p, HPoint3 *np, ColorA *c)
{
    NDstuff *nds = (NDstuff *)mgNDctx;    
#define iT    nds->T
#define iaxes nds->axes
#define iTc   nds->Tc
#define incm  nds->ncm
#define icm   nds->cm
#define ihc   nds->hc
    ColorA ci;
    float t;
    int i;

    HPtNTransformComponents(p, iT, 4, iaxes, (float *)np);
    if(np->w != 1) HPt3Dehomogenize(np, np);
    
    if(incm > 0 && !(_mgc->astk->ap.flag & APF_KEEPCOLOR) && c != NULL) {
	ci.r = ci.g = ci.b = ci.a = 0;

	HPtNTransform(iTc, p, ihc);
	for(i = 0; i < incm; i++) {
	    cent *ce = VVEC(icm[i].cents, cent);
	    float v = ihc->v[i];
	    if(!(v > ce->v)) {
		ci.r += ce->c.r;
		ci.g += ce->c.g;
		ci.b += ce->c.b;
		ci.a += ce->c.a;
	    } else {
		do ce++; while(v > ce->v);
		if((ce-1)->interp) {
		    t = (v - (ce-1)->v) / (ce->v - (ce-1)->v);
		} else {
		    t = 1;
		}
		ci.r += t*ce->c.r + (1-t)*(ce-1)->c.r;
		ci.g += t*ce->c.g + (1-t)*(ce-1)->c.g;
		ci.b += t*ce->c.b + (1-t)*(ce-1)->c.b;
		ci.a += t*ce->c.a + (1-t)*(ce-1)->c.a;
	    }
	}
	/* XXX Clamp colors to range 0..1 here?? */
	if(ci.r < 0) ci.r = 0; else if(ci.r > 1) ci.r = 1;
	if(ci.g < 0) ci.g = 0; else if(ci.g > 1) ci.g = 1;
	if(ci.b < 0) ci.b = 0; else if(ci.b > 1) ci.b = 1;
	if(ci.a < 0) ci.a = 0; else if(ci.a > 1) ci.a = 1;
	*c = ci;
	return 1;
    }
    return 0;

#undef iT
#undef iTc
#undef iaxes
#undef icm
#undef incm
#undef ihc
}

/* push/pop could be done much more efficiently, but ND is inefficient
 * anyway, so what.
 */
static void *saveCTXpushTN(mgNDctx *NDctx, TransformN *TN)
{
    NDstuff *nds = (NDstuff *)NDctx;
    TransformN **savedCTX = OOGLNewNE(TransformN *, 2, "saved NDstuff");

    savedCTX[0] = nds->T;
    savedCTX[1] = nds->Tc;
    
    /* everything else is scratch */
    nds->T = TmNConcat(TN, nds->T, NULL);
    if (nds->Tc) {
	nds->Tc = TmNConcat(TN, nds->Tc, NULL);
    }

    return savedCTX;
}

static void restoreCTX(mgNDctx *NDctx, void *vsavedCTX)
{
    NDstuff *nds = (NDstuff *)NDctx;
    TransformN **savedCTX = (TransformN **)vsavedCTX;

    TmNDelete(nds->T);
    TmNDelete(nds->Tc);
    nds->T = savedCTX[0];
    nds->Tc = savedCTX[0];

    OOGLFree(savedCTX);
}

mgNDctx NDctx_proto = {
    map_ND_point,
    saveCTXpushTN,
    restoreCTX,
};
