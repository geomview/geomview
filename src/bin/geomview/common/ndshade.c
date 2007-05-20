/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 * Copyright (C) 2006-20007 Claus-Justus Heine
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

#if 0
static void compute_mg_factor(NDstuff *nds);
static inline void fast_map_ND_point(NDstuff *nds, HPointN *from, HPoint3 *to);
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

#if 0
  /* cH: here is still room for optimization: provided that iT has
   * maximal rank (i.e. rank 4) it would be possible to factor out an
   * ordinary Transform and handle that one over to the MG layer. The
   * remaining N-Transform would act as identity on a 4x4 sub-space
   * which would reduce the number of floating point operations to
   * perform the projection. The MG layer uses an object-camera
   * transform anyway, so this would speed up things.
   *
   * Well. The code at the end of this files hacks this, but: the
   * resulting 4x4 factor is -- of course -- not orthogonal. Therefore
   * I stopped working at this optimization. The code for the
   * factorization is at the end of this file. cH.
   * 
   */
  fast_map_ND_point(nds, p, np);
#else
  HPtNTransProj(iT, p, np);
  HPt3Dehomogenize(np, np);
#endif
    
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
typedef struct savedCTX 
{
  TransformN *T;
  TransformN *Tc;
  TransformN *rest;
  Transform  MGFactor;
  int        perm[1];
} SavedCTX;

static void *saveCTX(mgNDctx *NDctx)
{
  NDstuff *nds = (NDstuff *)NDctx;
  SavedCTX *savedCTX =
    (SavedCTX *)OOGLNewNE(char,
			  sizeof(SavedCTX)+(nds->T->idim-1)*sizeof(int),
			  "SavedCTX");

  savedCTX->T = nds->T;
  nds->T = TmNCopy(nds->T, NULL);
  if (nds->Tc) {
    savedCTX->Tc = nds->Tc;
    nds->Tc = TmNCopy(nds->Tc, NULL);
  } else {
    savedCTX->Tc = NULL;
  }
#if 0
  savedCTX->rest = nds->rest;
  nds->rest = TmNCopy(nds->rest, NULL);
  TmCopy(nds->MGFactor, savedCTX->MGFactor);
  memcpy(savedCTX->perm, nds->perm, nds->T->idim * sizeof(int));

  mgpushtransform();
#endif
  
  return savedCTX;
}

static void pushTN(mgNDctx *NDctx, TransformN *TN)
{
  NDstuff *nds = (NDstuff *)NDctx;

  TmNConcat(TN, nds->T, nds->T);

  if (nds->Tc) {
    TmNConcat(TN, nds->Tc, nds->Tc);
  }

#if 0
  compute_mg_factor(nds);
  mgtransform(nds->MGFactor);
#endif
}

static void pushT(mgNDctx *NDctx, Transform T)
{
  NDstuff *nds = (NDstuff *)NDctx;

  TmNApplyT3TN(T, NULL, nds->T);

  if (nds->Tc) {
    TmNApplyT3TN(T, NULL, nds->Tc);
  }

#if 0
  compute_mg_factor(nds);
  mgtransform(nds->MGFactor);
#endif
}

static void restoreCTX(mgNDctx *NDctx, void *vsavedCTX)
{
  NDstuff *nds = (NDstuff *)NDctx;
  SavedCTX *savedCTX = (SavedCTX *)vsavedCTX;

#if 0
  mgpoptransform();
#endif

  TmNDelete(nds->T);
  nds->T = savedCTX->T;
  if (nds->Tc) {
    TmNDelete(nds->Tc);
  }
  nds->Tc = savedCTX->Tc;
#if 0
  TmNDelete(nds->rest);
  nds->rest = savedCTX->rest;
  TmCopy(savedCTX->MGFactor, nds->MGFactor);
  memcpy(nds->perm, savedCTX->perm, nds->T->idim * sizeof(int));
#endif

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

#if 0
  mgpushtransform();
#endif

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
  nds->T    = NULL;

#if 0
  nds->rest = NULL;
  nds->perm = OOGLNewNE(int, dim, "permutation for fast ND mapping");
#endif

  nds->W2C = TmNProject(W2C, dv->NDPerm, NULL);

  return nds;
}

void drawer_destroy_ndstuff(NDstuff *nds)
{
  TmNDelete(nds->Tc);
  TmNDelete(nds->W2c);

  TmNDelete(nds->T);
  TmNDelete(nds->W2C);

#if 0
  TmNDelete(nds->rest);
  OOGLFree(nds->perm);
#endif

  HPtNDelete(nds->hc);

  /* ... and finally do not forget to destroy nds. Gnah. */
  OOGLFree(nds);

#if 0
  mgpoptransform();
#endif
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

#if 0
  compute_mg_factor(nds);
  mgtransform(nds->MGFactor);
#endif
}

#if 0

/* The stuff below is incomplete and does not work in its current
 * shape. See the comment at the start of map_ND_point().
 */

/* Factor out a 4x4 matrix such that
 *
 *               /  I  \
 * (nds->T) = P  | --- | A
 *               \  B  /
 *
 * with a permutation matrix P, an (Nd x 4) matrix B and a 4x4 matrix
 * A. The goal is to hand over A to the MG-layer (and thus possibly to
 * the hardware). This should speed up the projection process
 * considerably, because we need 16 FLOPS (matrix multiplication) less
 * than before. A is added to the object->screen transform which is
 * applied anyway.
 */
static inline void swap_rows(int r1, int r2, int n, HPtNCoord *Ta, int *perm)
{
  int swapi, col;
  HPtNCoord swapf;

  if (r1 == r2) {
    return;
  }

  /* record permutation */
  swapi    = perm[r1];
  perm[r1] = perm[r2];
  perm[r2] = swapi;

  for (col = 0; col < 4; col++) {
    swapf          = Ta[r1*4 + col];
    Ta[r1*4 + col] = Ta[r2*4 + col];
    Ta[r2*4 + col] = swapf;
  }
}

static inline void
swap_cols(int rc1, int rc2, int n, HPtNCoord *Ta, Transform A)
{
  HPtNCoord swapf;
  int row, col;
  
  if (rc1 == rc2) {
    return;
  }

  /* swap columns in T */
  for (row = 0; row < n; row++) {
    swapf           = Ta[row*4 + rc1];
    Ta[row*4 + rc1] = Ta[row*4 + rc2];
    Ta[row*4 + rc2] = swapf;
  }

  /* swap rows in A */
  for (col = 0; col < 4; col++) {
    swapf       = A[rc1][col];
    A[rc1][col] = A[rc2][col];
    A[rc2][col] = swapf;
  }
}

#define DEBUG_MG_FACTOR 1

static void compute_mg_factor(NDstuff *nds)
{
  int idim = nds->T->idim;
  int i, row, col;
  TransformN   *T;
  TransformPtr A     = nds->MGFactor;
  int          perm[idim];
  HPtNCoord max, pivot;
  int maxrow, maxcol;

  T = nds->rest = TmNCopy(nds->T, nds->rest);

  for (i = 0; i < idim; i++) {
    perm[i] = i;
  }

  TmCopy(TM_IDENTITY, A);

  for (i = 0; i < 4; i++) {
    /* find pivot element */
    max = 0.0;
    maxrow = maxcol = i;
    for (row = i; row < idim; row++) {
      for (col = i; col < 4; col++) {
	if (fabs(T->a[row*4+col]) > max) {
	  max = fabs(T->a[row*4+col]);
	  maxrow = row;
	  maxcol = col;
	}
      }
    }
    /* exchange row i and maxrow */
    swap_rows(i, maxrow, idim, T->a, perm);

    /* exchange column i and maxcol in T and row i and maxcol in A */
    swap_cols(i, maxcol, idim, T->a, A);
    
    /* now perform the Gauss' elimination process; note that we have
     * to perform the operation on the columns of T as T operates from
     * the right.
     *
     * We perform the inverse operation on the rows of A.
     */

    pivot = T->a[i*4+i];

    /* Clear i-th row of T, record inverse operation in A */
    for (col = i+1; col < 4; col++) {
      HPtNCoord factor;
      
      factor = T->a[i*4+col] / pivot;

#if DEBUG_MG_FACTOR
      /* debugging: also perform the operation on the upper part of T */
      for (row = 0; row < i+1; row++) {
	T->a[row*4+col] -= T->a[row*4+i] * factor;
      }
#endif

      for (row = i+1; row < idim; row++) {
	T->a[row*4+col] -= T->a[row*4+i] * factor;
      }
      for (row = 0; row < 4; row++) {
	A[i][row] += A[col][row] * factor;
      }
    }
  }

  /* At this point T has the form 
   *
   *                    /  L  \
   * (nds->T) = P^{-1}  | --- | A'
   *                    \  B' /
   *
   * with a lower triangular matrix L, proceed now by converting L
   * to the 4x4 unity matrix.
   *
   * We need to apply the operations only on B' and A'
   */
  for (col = 4; --col >= 0;) {
    HPtNCoord factor;
    int col2;

    for (col2 = 0; col2 < col; col2++) {
      /* clear the col-th row */
      factor = T->a[col*4+col2] / T->a[col*4+col];
#if DEBUG_MG_FACTOR
      /* debugging: also perform the operation on the upper part of T */
      for (row = 0; row < 4; row++) {
	T->a[row*4+col2] -= T->a[row*4+col] * factor;
      }
#endif
      for (row = 4; row < idim; row++) {
	T->a[row*4+col2] -= T->a[row*4+col] * factor;
      }
      for (row = 0; row < 4; row++) {
	A[col][row] += A[col2][row] * factor;
      }
    }

    /* finally scale the diagonal such that we really have an
     * identity matrix
     */
    factor = T->a[col*4+col];
    for (row = 0; row < 4; row++) {
      A[col][row] *= factor;
    }
    factor = 1.0 / T->a[col*4+col];
#if DEBUG_MG_FACTOR
    /* debugging: also perform the operation on the upper part of T */
    for (row = 0; row < 4; row++) {
      T->a[row*4+col] *= factor;
    }
#endif
    for (row = 4; row < idim; row++) {
      T->a[row*4+col] *= factor;
    }
  }

  for (row = 0; row < idim; row++) {
    nds->perm[row] = perm[row];
  }

  for (col = 0; col < 4; col++) {
    HPt3Coord swap;
    swap = A[0][col];
    A[0][col] = A[1][col];
    A[1][col] = A[2][col];
    A[2][col] = A[3][col];
    A[3][col] = swap;
  }

  for (row = 4; row < idim; row++) {
    HPt3Coord swap;
      
    swap = T->a[row*4+0];
    T->a[row*4+0] = T->a[row*4+1];
    T->a[row*4+1] = T->a[row*4+2];
    T->a[row*4+2] = T->a[row*4+3];
    T->a[row*4+3] = swap;
  }

#if DEBUG_MG_FACTOR
  for (row = 0; row < 4; row++) {
    HPt3Coord swap;
      
    swap = T->a[row*4+0];
    T->a[row*4+0] = T->a[row*4+1];
    T->a[row*4+1] = T->a[row*4+2];
    T->a[row*4+2] = T->a[row*4+3];
    T->a[row*4+3] = swap;
  }
  fprintf(stderr, "Permuation: ");
  for (row = 0; row < idim; row++) {
    fprintf(stderr, "%d ", perm[row]);
  }
  fprintf(stderr, "\nRest:\n");
  TmNPrint(stderr, T);
  fprintf(stderr, "MG Factor:\n"); 
  TmPrint(stderr, A);
  fprintf(stderr, "Origimal matrix:\n");
  TmNPrint(stderr, nds->T);

  fprintf(stderr, "Product of factors:\n");
  {
    TransformN *prod, *prod2;
    int i;
    
    prod = TmNCreate(idim, 4, NULL);
    for (row = 0; row < idim; row++) {
      for (col = 0; col < 4; col++) {
	for (i = 0; i < 4; i++) {
	  prod->a[row*4+col] += T->a[row*4+i]*A[i][col];
	}
      }
    }

    prod2 = TmNCreate(idim, 4, NULL);
    for (row = 0; row < idim; row++) {
      for (col = 0; col < 4; col++) {
	prod2->a[perm[row]*4+col] = prod->a[row*4+col];
      }
    }
    /*TmNPrint(stderr, prod);*/
    TmNPrint(stderr, prod2);
    TmNDelete(prod);
    TmNDelete(prod2);
  }
#endif
}

/* Use the factorization computed above to map "from" to "to"
 * efficiently
 */
static inline void fast_map_ND_point(NDstuff *nds, HPointN *from, HPoint3 *to)
{
  HPtNCoord *v = from->v;
  int idim;
  const int odim = 4;
  int *perm = nds->perm;
  int i;

  idim = nds->T->idim > from->dim ? from->dim : nds->T->idim;

  /* The following 4 lines used to consume 16 multiplications */
  to->x = v[perm[1]];
  to->y = v[perm[2]];
  to->z = v[perm[3]];
  to->w = v[perm[0]];
  for (i = 4; i < idim; i++) {
    to->x += v[perm[i]] * nds->rest->a[i*odim+0];
    to->y += v[perm[i]] * nds->rest->a[i*odim+1];
    to->z += v[perm[i]] * nds->rest->a[i*odim+2];
    to->w += v[perm[i]] * nds->rest->a[i*odim+3];
  }

  /*HPt3Transform(nds->MGFactor, to, to);*/

#if 1
  {
    HPoint3 tp[2];
    static HPt3Coord max;
    HPtNTransProj(nds->T, from, tp);

    HPt3Transform(nds->MGFactor, to, tp+1);
    /**(tp+1) = *to;*/

    if (HPt3Distance(tp, tp+1) > max) {
      max = HPt3Distance(tp, tp+1);
      fprintf(stderr, "max dist: %e\n", max);
    }
  }
#endif

}
#endif

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
