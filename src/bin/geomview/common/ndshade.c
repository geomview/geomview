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
#define iTc   nds->Tc
#define incm  nds->ncm
#define icm   nds->cm
#define ihc   nds->hc
  ColorA ci;
  float t;
  int i;

  /* cH: here is still room for optimization: provided that iT has
   * maximal rank (i.e. rank 4) it would be possible to factor out an
   * ordinary Transform and handle that one over to the MG layer. The
   * remaining N-Transform would act as identity on a 4x4 sub-space
   * which would reduce the number of floating point operations to
   * perform the projection. The MG layer uses an object-camera
   * transform anyway, so this would speed up things.
   */
  HPtNTransProj(iT, p, np);
  HPt3Dehomogenize(np, np);
    
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
static void *saveCTX(mgNDctx *NDctx)
{
  NDstuff *nds = (NDstuff *)NDctx;
  TransformN **savedCTX = OOGLNewNE(TransformN *, 2, "saved NDstuff");

  savedCTX[0] = nds->T;
  nds->T = TmNCopy(nds->T, NULL);
  if (nds->Tc) {
    savedCTX[1] = nds->Tc;
    nds->Tc = TmNCopy(nds->Tc, NULL);
  } else {
    savedCTX[1] = NULL;
  }
    
  return savedCTX;
}

static void pushTN(mgNDctx *NDctx, TransformN *TN)
{
  NDstuff *nds = (NDstuff *)NDctx;

  TmNConcat(TN, nds->T, nds->T);
  if (nds->Tc) {
    TmNConcat(TN, nds->Tc, nds->Tc);
  }
}

static void pushT(mgNDctx *NDctx, Transform T)
{
  NDstuff *nds = (NDstuff *)NDctx;

  TmNApplyT3TN(T, NULL, nds->T);
  if (nds->Tc) {
    TmNApplyT3TN(T, NULL, nds->Tc);
  }
}

static void restoreCTX(mgNDctx *NDctx, void *vsavedCTX)
{
  NDstuff *nds = (NDstuff *)NDctx;
  TransformN **savedCTX = (TransformN **)vsavedCTX;

  TmNDelete(nds->T);
  nds->T = savedCTX[0];
  if (nds->Tc) {
    TmNDelete(nds->Tc);
  }
  nds->Tc = savedCTX[1];

  OOGLFree(savedCTX);
}

static mgNDctx NDctx_proto = {
  map_ND_point,
  saveCTX,
  pushTN,
  pushT,
  restoreCTX,
  NULL,
};

NDstuff *drawer_init_ndstuff(DView *dv, TransformN *W2C, TransformN *W2U)
{
  NDstuff *nds = OOGLNewE(NDstuff, "new NDstuff");
  int dim = W2C->idim;

  nds->mgNDctx = NDctx_proto;

  /* Initialize the color transform */
  nds->ncm = dv->nNDcmap;
  nds->cm  = dv->NDcmap;
  nds->W2c = NULL;
  nds->Tc  = NULL;
  nds->hc  = NULL;

  if(dv->nNDcmap > 0) {
    HPointN *caxis = NULL;
    int i, j;

    /* Build array of N-D-to-color projection vectors: it becomes a
     * matrix, multiplied by N-D row vector on the left, yielding an
     * array of dv->nNDcmap projections used for coloring.  So it has
     * (dimension) rows, (dv->nNDcmap) columns.
     */
    nds->W2c = TmNCreate(dim, dv->nNDcmap, NULL);
    nds->hc = HPtNCreate(dim, NULL);

    for(i = 0; i < dv->nNDcmap; i++) {
      cmap *cm = &dv->NDcmap[i];
      int cdim = cm->axis->dim;
      int mindim = (dim < cdim) ? dim : cdim;
      HPointN *our_caxis = cm->axis;
      TransformN *TxC;
      
      if(cm->coords != UNIVERSE) {
	TxC = drawer_get_ND_transform(cm->coords, UNIVERSE);
	if(TxC) {
	    our_caxis = caxis = HPtNTransform(TxC, cm->axis, caxis);
	    TmNDelete(TxC);
	}
      }
      for(j = 0; j < mindim; j++)
	nds->W2c->a[j*dv->nNDcmap + i] = our_caxis->v[j];
    }
    TmNConcat(W2U, nds->W2c, nds->W2c);
    HPtNDelete(caxis);
  }
  
  /* initialize the W2C geometry transform. Note that nds->W2C is a
   * projection matrix, i.e. it includes the mapping defined in
   * dv->NDperm. This spares a lot of unnecessary floating point
   * operations.
   */
  nds->T   = NULL;
  nds->W2C = TmNProject(W2C, dv->NDPerm, NULL);
  
  return nds;
}

void drawer_destroy_ndstuff(NDstuff *nds)
{
  TmNDelete(nds->Tc);
  TmNDelete(nds->W2c);

  TmNDelete(nds->T);
  TmNDelete(nds->W2C);

  HPtNDelete(nds->hc);

  /* ... and finally do not forget to destroy nds. Gnah. */
  OOGLFree(nds);
}

/* install a new object transformation, this is simply a matter of
 * matrix multiplication; the resulting O2C transfrom will contain the
 * proper sub-space projection.
 */
void drawer_transform_ndstuff(NDstuff *nds, TransformN *T)
{
  nds->T = TmNConcat(T, nds->W2C, nds->T);
  if (nds->W2c) {
    nds->Tc = TmNConcat(T, nds->W2c, nds->Tc);
  }
}

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
