/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 * Copyright (C) 2006 Claus-Justus Heine
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

/*
 * Draw an Inst using the mg library.
 */
#include "instP.h"
#include "mgP.h"
#include "bsptreeP.h"

static inline
TmCoord (*coords2W(int system, TransformPtr T))[4]
{
  WnPosition vp;
  static Transform Tndc;

  switch(system) {
  case L_GLOBAL: return TM3_IDENTITY;
  case L_CAMERA: return _mgc->C2W;
  case L_SCREEN: return _mgc->S2W;
  case L_NDC:
    WnGet(_mgc->win, WN_VIEWPORT, &vp);
    TmScale(Tndc, .5*(vp.xmax-vp.xmin+1), .5*(vp.ymax-vp.ymin+1), 1.0);
    CtmTranslate(Tndc, 1.0, 1.0, 0.0);
    TmConcat(Tndc, _mgc->S2W, Tndc);
    return Tndc;
  default:  return T;  /* Default is local coords: obj->world */
  }
}

static inline
TmCoord (*coordsto(int from, int to, TransformPtr T, TransformPtr Tinv))[4]
{
  WnPosition vp;
  static Transform Tmap;

  if(from == L_NONE) from = L_LOCAL;
  if(to == L_NONE) to = L_LOCAL;
  if(from == to)
    return TM3_IDENTITY;

  switch(to) {
  case L_GLOBAL: return coords2W(from, T);
  case L_LOCAL:
    if(!(_mgc->has & HAS_S2O)) mg_findS2O();
    switch(from) {
    case L_GLOBAL: return Tinv;	/* W2O */
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
    case L_GLOBAL: TmConcat(_mgc->W2S,
			    coordsto(L_SCREEN, L_NDC, T, Tinv),
			    Tmap);
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
  TmConcat(coords2W(from, T), coordsto(L_GLOBAL, to, T, Tinv), Tmap);
  return Tmap;
}
    
Inst *InstDraw(Inst *inst) 
{
  GeomIter *it, *txit = NULL;
  Transform T, tT, Tl2o, Ttx;
  mgNDctx *NDctx = NULL;
  void *saved_ctx;

  GeomMakePath(inst, 'I', path, pathlen);

  if (inst->geom == NULL) {
    return inst;
  }

  inst->geom->ppath = path;
  inst->geom->ppathlen = pathlen;

  inst->geomflags &= ~GEOM_ALPHA;

  mgctxget(MG_NDCTX, &NDctx);

  if (NDctx) {
    if (inst->location > L_LOCAL) {
      /* temporarily disable ND-drawing, makes sense only for
       * L_LOCAL, really. The universe is 3d in Geomview, in
       * some sense.
       */
      mgctxset(MG_NDCTX, NULL, MG_END);
    } else if (inst->origin != L_NONE) {
      static int was_here;
      if (!was_here)
	OOGLError(1,
		  "FIXME: don't know how to handle origin != L_LOCAL "
		  "with ND-drawing.\n");
      return NULL;
    } else {
      if (inst->NDaxis) {
	saved_ctx = NDctx->saveCTX(NDctx);
	NDctx->pushTN(NDctx, inst->NDaxis);
	GeomDraw(inst->geom);	
	NDctx->restoreCTX(NDctx, saved_ctx);
      } else {
	it = GeomIterate((Geom *)inst, DEEP);
	txit = GeomIterate((Geom *)inst->txtlist, DEEP);
	while (NextTransform(it, T)) {
	  if (!NextTransform(txit, Ttx)) {
	    txit = NULL;
	  } else {
	    mgpushtxtransform();
	    mgtxtransform(Ttx);
	  }
	  saved_ctx = NDctx->saveCTX(NDctx);
	  NDctx->pushT(NDctx, T);
	  GeomDraw(inst->geom);
	  NDctx->restoreCTX(NDctx, saved_ctx);
	  if (txit != NULL) {
	    mgpoptxtransform();
	  }
	}
      }
      if (inst->geom->geomflags & GEOM_ALPHA) {
	inst->geomflags |= GEOM_ALPHA;
      }
      return inst;
    }
  }

  it = GeomIterate((Geom *)inst, DEEP);
  txit = GeomIterate((Geom *)inst->txtlist, DEEP);
  while (NextTransform(it, T)) {

    mgpushtransform();

    if (!NextTransform(txit, Ttx)) {
      txit = NULL;
    } else {
      mgpushtxtransform();
      mgtxtransform(Ttx);
    }

    /* Compute origin *before* changing mg tfm */
    if (inst->origin != L_NONE) {	    
      Point3 originwas, delta;
      TmCoord (*l2o)[4], (*o2W)[4];
      static HPoint3 zero = { 0, 0, 0, 1 };

      /* We have location2W, origin2W. We want to translate
       * in 'origin' coords such that (0,0,0) in location
       * coords maps to originpt in origin coords.
       */
      o2W = coords2W(inst->origin, _mgc->xstk->T);
      l2o = coordsto(inst->location, inst->origin,
		     _mgc->xstk->T, _mgc->xstk->Tinv);
      HPt3TransPt3(l2o, &zero, &originwas);
      Pt3Sub(&inst->originpt, &originwas, &delta);
      TmTranslate(tT, delta.x, delta.y, delta.z);
      TmConcat(l2o, tT, Tl2o);
      TmConcat(T, Tl2o, tT);
      TmConcat(tT, o2W, T);
      mgsettransform(T);
    } else if (inst->location > L_LOCAL) {
      TmConcat(T, coords2W(inst->location, _mgc->xstk->T), T);
      mgsettransform(T);
    } else {
      mgtransform(T);
    }
    GeomDraw(inst->geom);
    mgpoptransform();
    if (txit != NULL) {
      mgpoptxtransform();
    }
  }

  if(NDctx) {
    /* restore the ND-context, also: if we have a BSP-tree, then we
     * need to add our object to the tree, because GeomBSPTreeDraw()
     * does not do so for ND drawing.
     */
    if (NDctx->bsptree != NULL && (inst->geom->geomflags & GEOM_ALPHA)) {
      GeomBSPTree((Geom *)inst, NDctx->bsptree, BSPTREE_ADDGEOM);
    }
    mgctxset(MG_NDCTX, NDctx, MG_END);
  }

  if (inst->geom->geomflags & GEOM_ALPHA) {
    inst->geomflags |= GEOM_ALPHA;
  }

  return inst;
}

Inst *InstBSPTree(Inst *inst, BSPTree *bsptree, int action)
{
  TransformPtr oldT, oldTxT;
  GeomIter *it, *txit;
  Transform T, tT, Tl2o, TxT;
  Transform oldTinv;

  if (inst->geom) {
    GeomMakePath(inst, 'I', path, pathlen);
    inst->geom->ppath = path;
    inst->geom->ppathlen = pathlen;
  }

  /* No need to loop over all transforms unless action == BSPTREE_ADDGEOM */
  switch (action) {
  case BSPTREE_CREATE:
    GeomBSPTree(inst->geom, bsptree, action);
    HandleRegister(&inst->geomhandle,
		   (Ref *)inst, bsptree, BSPTreeInvalidate);
    HandleRegister(&inst->tlisthandle,
		   (Ref *)inst, bsptree, BSPTreeInvalidate);
    HandleRegister(&inst->axishandle,
		   (Ref *)inst, bsptree, BSPTreeInvalidate);
    HandleRegister(&inst->NDaxishandle,
		   (Ref *)inst, bsptree, BSPTreeInvalidate);
    return inst;

  case BSPTREE_DELETE:
    /* unregister any pending callback */
    HandleUnregisterJust(&inst->geomhandle,
			 (Ref *)inst, bsptree, BSPTreeInvalidate);
    HandleUnregisterJust(&inst->tlisthandle,
			 (Ref *)inst, bsptree, BSPTreeInvalidate);
    HandleUnregisterJust(&inst->axishandle,
			 (Ref *)inst, bsptree, BSPTreeInvalidate);
    HandleUnregisterJust(&inst->NDaxishandle,
			 (Ref *)inst, bsptree, BSPTreeInvalidate);
    GeomBSPTree(inst->geom, bsptree, action);
    return inst;

  case BSPTREE_ADDGEOM:
    if (inst->NDaxis) {
      /* No need to add to the BSPTree here, will be handled by the
       * various draw_projected_BLAH() stuff.
       */
      return inst;
    }

    if ((inst->origin != L_NONE || inst->location > L_LOCAL)) {
      BSPTreeSet(bsptree, BSPTREE_ONESHOT, true, BSPTREE_END);
      if (bsptree->Tidinv == NULL) {
	if (bsptree->Tid != TM_IDENTITY) {
	  bsptree->Tidinv = obstack_alloc(&bsptree->obst, sizeof(Transform));
	  TmInvert(bsptree->Tid, bsptree->Tidinv);
	} else {
	  bsptree->Tidinv = TM_IDENTITY;
	}
      }
    }

    oldT = BSPTreePushTransform(bsptree, TM_IDENTITY);
    oldTxT = BSPTreePushTxTransform(bsptree, TM_IDENTITY);

    if (inst->origin != L_NONE) {
      TmInvert(oldT, oldTinv);
    }

    it = GeomIterate((Geom *)inst, DEEP);
    txit = GeomIterate((Geom *)inst->txtlist, DEEP);
    while (NextTransform(it, T)) {

      /* Compute origin *before* changing mg tfm */
      if (inst->origin != L_NONE) {	    
	Point3 originwas, delta;
	TmCoord (*l2o)[4], (*o2W)[4];
	static HPoint3 zero = { 0, 0, 0, 1 };

	/* We have location2W, origin2W. We want to translate
	 * in 'origin' coords such that (0,0,0) in location
	 * coords maps to originpt in origin coords.
	 */
	o2W = coords2W(inst->origin, oldT);
	l2o = coordsto(inst->location, inst->origin, oldT, oldTinv);
	HPt3TransPt3(l2o, &zero, &originwas);
	Pt3Sub(&inst->originpt, &originwas, &delta);
	TmTranslate(tT, delta.x, delta.y, delta.z);
	TmConcat(l2o, tT, Tl2o);
	TmConcat(T, Tl2o, tT);
	TmConcat(tT, o2W, T);
	/* finally concat with tree->Tid^{-1} to get the correct
	 * absolute positioning.
	 */
	if (inst->location > L_LOCAL) {
	  if (bsptree->Tid != TM_IDENTITY) {
	    TmConcat(T, bsptree->Tidinv, T);
	  }
	}
      } else if (inst->location > L_LOCAL) {
	TmConcat(T, coords2W(inst->location, oldT), T);
	if (bsptree->Tid != TM_IDENTITY) {
	  TmConcat(T, bsptree->Tidinv, T);
	}
      } else {
	TmConcat(T, oldT, T);
      }
      BSPTreeSetTransform(bsptree, T);
      if (!NextTransform(txit, TxT)) {
	txit = NULL;
      } else {
	TmConcat(TxT, oldTxT, TxT);
	BSPTreeSetTxTransform(bsptree, TxT);
      }
      GeomBSPTree(inst->geom, bsptree, action);
    }

    BSPTreePopTxTransform(bsptree, oldT);
    BSPTreePopTransform(bsptree, oldT);

    return inst;
  default:
    return NULL;
  }
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
