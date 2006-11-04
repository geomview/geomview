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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips,
 * Celeste Fowler */

#include "instP.h"
#include "ooglutil.h"
#include "pickP.h"

static TmCoord (*coords2NDC(int system, Pick *p, Transform Tl2n))[4]
{
  switch(system) {
  case L_CAMERA: return p->Tc2n;
  case L_NDC:	 return TM3_IDENTITY;
  case L_SCREEN: return p->Ts2n;
  case L_GLOBAL: return p->Tw2n;
  default:	 return Tl2n;
  }
}

static TmCoord (*coordsbtwn(int from, int to, Pick *p, Transform Tl2n))[4]
{
    static Transform Tmap;

    if(from == L_NONE) from = L_LOCAL;
    if(to == L_NONE) to = L_LOCAL;
    if(from == to)
	return TM3_IDENTITY;

    if(to == L_NDC)
	return coords2NDC(from, p, Tl2n);

    TmInvert( coords2NDC(to, p, Tl2n), Tmap );
    TmConcat( coords2NDC(from, p, Tl2n), Tmap, Tmap );
    return Tmap;
}

Geom *
InstPick(Inst *inst, Pick *p, Appearance *ap,
	 Transform T, TransformN *TN, int *axes)
{
  int elem = 0, pathInd;
  Transform tT;
  GeomIter *it;
  Geom *v = NULL;

  if(inst == NULL || inst->geom == NULL)
    return NULL;

  if (TN && inst->origin != L_NONE) {
    static int was_here;

    if (!was_here)
      OOGLError(1, "FIXME: \"origin\" unsupported int ND-context.\n");
    
    return NULL;
  }

  if (inst->NDaxis) {
    TransformN *tT;

    if (TN) {
      pathInd = VVCOUNT(p->gcur);
      vvneeds(&p->gcur, pathInd + 1);
      *VVINDEX(p->gcur, int, pathInd) = elem;
      tT = TmNConcat(inst->NDaxis, TN, NULL);
      if (GeomPick(inst->geom, p, ap, NULL, tT, axes))
	v = (Geom *)inst;
      TmNDelete(tT);
    }

    return v;
  }

  if (inst->origin > L_LOCAL || inst->location > L_LOCAL) {
    TmTranslate( tT, p->x0, p->y0, 0. );
    TmConcat( T, tT, T );
    if (inst->origin > L_LOCAL) {
	Point3 originwas, delta;
	TmCoord (*l2o)[4], (*o2N)[4];
	Transform Tl2o;
	static HPoint3 zero = { 0, 0, 0, 1 };

	/* We have location2W, origin2W.
	 * We want to translate in 'origin' coords such that
	 * (0,0,0) in location coords maps to originpt in origin coords.
	 */
	o2N = coords2NDC(inst->origin, p, T);
	l2o = coordsbtwn(inst->location, inst->origin, p, T);
	HPt3TransPt3(l2o, &zero, &originwas);
	Pt3Sub(&inst->originpt, &originwas, &delta);
	TmTranslate( tT, delta.x, delta.y, delta.z );
	TmConcat( l2o, tT, Tl2o );
	TmConcat( T, Tl2o, tT );
	TmConcat( tT, o2N, T );

    } else {
	TmCopy( coords2NDC(inst->location, p, T), T );
    }
    TmTranslate( tT, -p->x0, -p->y0, 0. );
    TmConcat( T, tT, T );
  }

  if (!TN || inst->location > L_LOCAL) {
    pathInd = VVCOUNT(p->gcur);
    vvneeds(&p->gcur, pathInd + 1);
    VVCOUNT(p->gcur)++;
    it = GeomIterate((Geom *)inst, DEEP);
    while (NextTransform(it, tT)) {
      *VVINDEX(p->gcur, int, pathInd) = elem;
      TmConcat(tT, T, tT);
      if (GeomPick(inst->geom, p, ap, tT, NULL, NULL)) 
	v = (Geom *)inst;
      elem++;
    }
    VVCOUNT(p->gcur)--;

    return v;
  } else {
    TransformN *tTN = NULL;

    pathInd = VVCOUNT(p->gcur);
    vvneeds(&p->gcur, pathInd + 1);
    VVCOUNT(p->gcur)++;
    it = GeomIterate((Geom *)inst, DEEP);
    while (NextTransform(it, tT)) {
      *VVINDEX(p->gcur, int, pathInd) = elem;
      tTN = TmNCopy(TN, tTN);
      TmNApplyT3TN(tT, NULL, tTN);
      if (GeomPick(inst->geom, p, ap, NULL, tTN, axes))
	v = (Geom *)inst;
      elem++;
    }
    VVCOUNT(p->gcur)--;

    if (tTN) {
      TmNDelete(tTN);
    }

    return v;
  }

  return v;
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
