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

/*
 * Draw an Inst using the mg library.
 */
#include "instP.h"
#include "mgP.h"

static TmCoord (*coords2W(int system))[4]
{
    WnPosition vp;
    static Transform Tndc;

    switch(system) {

    default:  return _mgc->xstk->T;  /* Default is local coords: obj->world */

    case L_GLOBAL: return TM3_IDENTITY;
    case L_CAMERA: return _mgc->C2W;
    case L_SCREEN: return _mgc->S2W;
    case L_NDC:
		WnGet(_mgc->win, WN_VIEWPORT, &vp);
		TmScale(Tndc, .5*(vp.xmax-vp.xmin+1), .5*(vp.ymax-vp.ymin+1), 1.0);
		CtmTranslate(Tndc, 1.0, 1.0, 0.0);
		TmConcat(Tndc, _mgc->S2W, Tndc);
		return Tndc;

    }
}

static TmCoord (*coordsto(int from, int to))[4]
{
    WnPosition vp;
    static Transform Tmap;

    if(from == L_NONE) from = L_LOCAL;
    if(to == L_NONE) to = L_LOCAL;
    if(from == to)
	return TM3_IDENTITY;

    switch(to) {
    case L_GLOBAL: return coords2W(from);
    case L_LOCAL:
	if(!(_mgc->has & HAS_S2O)) mg_findS2O();
	switch(from) {
	case L_GLOBAL: return _mgc->xstk->Tinv;	/* W2O */
	case L_SCREEN: return _mgc->S2O;
	}
	break;
    case L_SCREEN:
	switch(from) {
	case L_GLOBAL: return _mgc->W2S;
	case L_LOCAL: if(!(_mgc->has & HAS_S2O)) mg_findS2O();
			return _mgc->O2S;
	case L_NDC:
		WnGet(_mgc->win, WN_VIEWPORT, &vp);
		TmScale(Tmap, .5*(vp.xmax-vp.xmin+1), .5*(vp.ymax-vp.ymin+1), 1.0);
		CtmTranslate(Tmap, 1.0, 1.0, 0.0);
		return Tmap;
	}
	break;
    case L_CAMERA:
	if(from == L_GLOBAL) return _mgc->W2C;
	break;
    case L_NDC:
	switch(from) {
	case L_GLOBAL: TmConcat(_mgc->W2S, coordsto(L_SCREEN, L_NDC), Tmap);
			return Tmap;
	case L_SCREEN:
		WnGet(_mgc->win, WN_VIEWPORT, &vp);
		TmTranslate(Tmap, -1.0, -1.0, 0.0);
		CtmScale(Tmap, 2.0/(vp.xmax-vp.xmin+1),
				2.0/(vp.ymax-vp.ymin+1), 1.0);
		return Tmap;
	}
	break;

    default:
	OOGLError(1, "InstDraw: internal error: unknown coord system 0x%x in coordsto(0x%x, 0x%x)", to, from, to);
	return TM3_IDENTITY;
    }

    /* It's safe to call both of these, since coords2W() can never return Tmap. */
    TmConcat(coords2W(from), coordsto(L_GLOBAL, to), Tmap);
    return Tmap;
}
    
Inst *
InstDraw( Inst *inst )
{
    GeomIter *it;
    Transform T, tT;

    it = GeomIterate((Geom *)inst, DEEP);
    while(NextTransform(it, T)) {
	if ( inst->geomflags & VERT_4D )	{
	    if (NextTransform(it, T)) 	{
		DestroyIter(it);
		return NULL;/* Error -- InstDraw on a multi-element 4inst */
	    }
	    mgset4to3(inst->axis, 0);
	}
	mgpushtransform();

	/* Compute origin *before* changing mg tfm */
	if(inst->origin != L_NONE) {
	    Point3 originpt, originwas, delta;
	    TmCoord (*l2o)[4], (*o2W)[4];
	    Transform To2W, Tl2o, Tl2W;
	    static HPoint3 zero = { 0, 0, 0, 1 };

	    /* We have location2W, origin2W.
	     * We want to translate in 'origin' coords such that
	     * (0,0,0) in location coords maps to originpt in origin coords.
	     */
	    o2W = coords2W(inst->origin);
	    l2o = coordsto(inst->location, inst->origin);
	    HPt3TransPt3(l2o, &zero, &originwas);
	    Pt3Sub(&inst->originpt, &originwas, &delta);
	    TmTranslate( tT, delta.x, delta.y, delta.z );
	    TmConcat( l2o, tT, Tl2o );
	    TmConcat( T, Tl2o, tT );
	    TmConcat( tT, o2W, T );
	    mgsettransform( T );

	} else if(inst->location > L_LOCAL) {

	    TmConcat( T, coords2W(inst->location), T );
	    mgsettransform( T );

	} else {

	    /* Ordinary case */
	    mgtransform( T );

	}
	GeomDraw( inst->geom );
	mgpoptransform();
    }
    return inst;
}
