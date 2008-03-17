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

/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner,
 * Mark Phillips, Olaf Holt, Claus-Justus Heine
 */
#ifndef _GV_TRANSFORMN_H_
#define _GV_TRANSFORMN_H_

#if HAVE_CONFIG_H
# include "config.h"
#endif

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

#include <math.h>
#include <ooglutil.h>
#include "geomtypes.h"
#include "freelist.h"

#define	TMNMAGIC	OOGLMagic('T', 1)

extern DEF_FREELIST(TransformN);

/* Refer to the (i,j)'th entry in a TransformN object;
 * beware that (i,j) are not checked!
 */
#define  TmNentry(T, i, j)  ((T)->a[(i)*(T)->odim + (j)])

extern TransformN *TmNPolarDecomp(const TransformN *A, TransformN *Q);

static inline TransformN *TmNCreate(int idim, int odim, HPtNCoord *a);
static inline void TmNDelete(TransformN *T);
static inline int TmNSpace(const TransformN *T);
static inline TransformN *TmNSetSpace(TransformN *T, int space);
static inline TransformN *TmNInvert(const TransformN *T, TransformN *Tinv);
static inline TransformN *TmNTranspose(const TransformN *from, TransformN *to);
static inline TransformN *TmNConcat(const TransformN *A, const TransformN *B,
				    TransformN *result);
static inline TransformN *TmNCopy(const TransformN *Tsrc, TransformN *Tdst);
static inline TransformN *TmNIdentity(TransformN *T);
static inline TransformN *TmNTranslateOrigin(TransformN *T, const HPointN *pt);
static inline TransformN *TmNPad(const TransformN *Tin, int idim, int odim,
				 TransformN *Tout);
static inline TransformN *TmNPadZero(const TransformN *Tin, int idim, int odim,
				     TransformN *Tout);
static inline TransformN *TmNSpaceTranslateOrigin( TransformN *T, HPointN *pt );
static inline TransformN *TmNScale(TransformN *T, const HPointN *amount);
static inline TransformN *TmNRotate(TransformN *T,
				    const HPointN *from, const HPointN *toward);
static inline TransformN *TmNApplyDN(TransformN *mat, int *perm,
				     Transform3 delta);
static inline TransformN *TmNApplyT3TN(Transform3 T3, int *perm,
				       TransformN *mat);
static inline TransformN *TmNMap(const TransformN *from, int *axes,
				 TransformN *to);
static inline TransformN *TmNProject(const TransformN *from, int *perm,
				     TransformN *to);
static inline int TmNGetSize(const TransformN *T, int *idim, int *odim);
static inline void TmNPrint(FILE *f, const TransformN *T);
static inline TransformN *TmNRead(IOBFILE *f, int binary);

#include "hpointn.h"
#include "hpoint3.h"

#ifndef max
# define _gv_tmn_max_
# define max(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
# define _gv_tmn_min_
# define min(a,b) ((a) < (b) ? (a) : (b))
#endif

/* BIG FAT NOTE: internally the homogeneous component of a vector is
 * located at 0, consequently the translation part of a transform is
 * stored in the 0-th row of a matrix. This just makes concatenation
 * etc. much easier when the dimensions of objects do not match.
 *
 * Prolongation/truncation is defined as follows: if an input vectors
 * dim does not match a TransformN's idim, then the input vector is
 * implicitly padded with zero's (if idim > dim) or the matrix is
 * interpreted as identity on the "missing" dimensions up to odim (if
 * dim > idim). The part of the vector with dim > odim is mapped to 0
 * (during padding)
 */

/* Construct a transform.  NULL a => identity */
static inline TransformN *
TmNCreate(int idim, int odim, HPtNCoord *a)
{
  TransformN *T;

  FREELIST_NEW(TransformN, T);

  RefInit((Ref *)T, TMNMAGIC);
  if (idim <= 0) idim = 1;
  if (odim <= 0) odim = 1;
  T->idim = idim;	 T->odim = odim;
  T->a = OOGLNewNE(HPtNCoord, idim*odim, "new TransformN data");
  if (a == NULL)
    memset(T->a, 0, idim*odim*sizeof(HPtNCoord));
  else
    memcpy(T->a, a, idim*odim*sizeof(HPtNCoord));
  return T;
}

/* Destroy */
static inline void 
TmNDelete(TransformN *T)
{
  if (T && RefDecr((Ref*)T) == 0) {
    if (T->a) {
      OOGLFree(T->a);
    }
    FREELIST_FREE(TransformN, T);
  }
}

/* Get and set space */
static inline int 
TmNSpace(const TransformN *T)
{
  (void)T;
  return TM_EUCLIDEAN;
}

static inline TransformN *
TmNSetSpace(TransformN *T, int space)
{
  if (space != TM_EUCLIDEAN) {
    OOGLError(1, "Non-Euclidean space not support in higher dimensions.\n");
    return NULL;
  }
  return T;
}

/* Invert */
static inline TransformN * 
TmNInvert(const TransformN *T, TransformN *Tinv)
{
  int i, j, k;
  HPtNCoord x;
  HPtNCoord f;
  int dim = T->idim; 
  TransformN *t = TmNCreate(dim, dim, T->a);

  if (T->odim != dim) {
    OOGLError(1, "Matrix for inversion is not square");
    return 0;
  }
  if (Tinv == NULL) {
    Tinv = TmNCreate(dim, dim, NULL);
  } else if (Tinv->idim != dim || Tinv->odim != dim) {
    Tinv->a = OOGLRenewNE(HPtNCoord, Tinv->a, dim*dim, "renew TransformN");
    Tinv->idim = dim; Tinv->odim = dim;
  }

  TmNIdentity(Tinv);

  /* Components of unrolled inner loops: */
#define	SUB(v, k)  v[j*dim+k] -= f*v[i*dim+k]
#define	SWAP(v, k) x = v[i*dim+k], v[i*dim+k] = v[largest*dim+k], v[largest*dim+k] = x

  for (i = 0; i < dim; i++) {
    int largest = i;
    HPtNCoord largesq = t->a[i*dim+i]*t->a[i*dim+i];
    for (j = i+1; j < dim; j++)
      if ((x = t->a[j*dim+i]*t->a[j*dim+i]) > largesq)
	largest = j,  largesq = x;

    /* swap t->a[i][] with t->a[largest][] */
    for(k = 0; k < dim; k++) {
      SWAP(t->a, k);
      SWAP(Tinv->a, k);
    }

    for (j = i+1; j < dim; j++) {
      f = t->a[j*dim+i] / t->a[i*dim+i];
      /* subtract f*t->a[i][] from t->a[j][] */
      for(k = 0; k < dim; k++) {
	SUB(t->a, k);
	SUB(Tinv->a, k);
      }
    }
  }
  for (i = 0; i < dim; i++) {
    f = t->a[i*dim+i];
    for (k = 0; k < dim; k++) {
      t->a[i*dim+k] /= f;
      Tinv->a[i*dim+k] /= f;
    }
  }
  for (i = dim-1; i >= 0; i--)
    for (j = i-1; j >= 0; j--) {
      f = t->a[j*dim+i];
      for(k = 0; k < dim; k++) {
	SUB(t->a, k);
	SUB(Tinv->a, k);
      }
    }
#undef SUB
#undef SWAP
		
  TmNDelete(t); /* those Geometry Center peoples forgot that ... (cH) */
		
  return Tinv;
}

/* Transpose */
static inline TransformN *
TmNTranspose(const TransformN *from, TransformN *to)
{
  int i, j;
  int idim = from->idim, odim = from->odim;
  HPtNCoord t;

  if (from != to) {
    if (to == NULL)
      to = TmNCreate(odim, idim, NULL);
    else if (to->idim != odim || to->odim != idim) {
      to->a = OOGLRenewNE(HPtNCoord, to->a, idim*odim, "renew TransformN");
      to-> idim = odim; to->odim = idim;
    }
    for (i = 0; i < idim; i++)
      for( j=0; j<odim; j++)
	to->a[j*idim+i] = from->a[i*odim+j];
  } else {

#define SWITCH(a, b)		 t = a; a = b; b = t;						
    if (idim == odim) {
      for (i = 0; i < idim; i++)
	for (j = 0; j < i; j++) {
	  SWITCH(to->a[i*odim+j], to->a[j*odim+i]);
	}
    } else {
      int remainder, dividend;
      to->idim = odim; to->odim = idim;
      for(i=0; i < idim; i++)
	for(j=0; j < odim; j++) {
	  remainder = (i*idim +j)%odim;
	  dividend = (i*idim + j -remainder)/odim;
	  SWITCH(to->a[i*odim+j], to->a[dividend*odim+remainder]);
	}
    }
#undef SWITCH
  }

  return to;
}

/* Multiply transforms
 *
 * We pad as necessary, filling diagonals down with 1's as
 * appropriate. We do not use PadZero(), because the concatenation
 * would not commute with the matrix operation; i.e. (v A) B should be
 * the same as v (A B).
 */
static inline TransformN *
TmNConcat(const TransformN *A, const TransformN *B, TransformN *result)
{
  int i, j, k;
  int dim1, dim2, dim3;

  if (A == NULL && B == NULL) {
    return NULL;
  } else if (A == NULL) {
    return TmNCopy(B, result);
  } else if (B == NULL) {
    return TmNCopy(A, result);
  }

  dim1 = A->idim; 
  dim2 = A->odim;
  dim3 = B->odim;

#define MAKEPRODUCT(T, A, B)					\
  T->idim = dim1; T->odim = dim3;				\
  for( i=0; i<dim1; i++)					\
    for( j=0; j<dim3; j++) {					\
      T->a[i*dim3+j] = 0;					\
      for( k=0; k<dim2; k++ )					\
	T->a[i*dim3+j] += A->a[i*dim2+k] * B->a[k*dim3+j];	\
    }

  if (B->idim == dim2) {
    /* the easy case, padding not necessary */
    
    if ( A == result || B == result ) {
      TransformN *T = TmNCreate( dim1, dim3, NULL);

      MAKEPRODUCT(T, A, B);
      TmNCopy(T, result);
      TmNDelete(T);
    } else {
      if (result == NULL)
	result = TmNCreate(dim1, dim3, NULL);
      else if (result->idim != dim1 || result->odim != dim3) {
	result->a =
	  OOGLRenewNE(HPtNCoord, result->a, dim1*dim3, "renew TransformN");
	result->idim = dim1; result->odim = dim3;
      }
      MAKEPRODUCT(result, A, B);
    }
  } else if ( B->idim > dim2) {
    TransformN *Atmp = TmNCreate(dim1, dim2, NULL);
    
    dim2 = B->idim;
    TmNPad(A, dim1, dim2, Atmp);
    if ( B == result ) {
      TransformN *T = TmNCreate( dim1, dim3, NULL);

      MAKEPRODUCT(T, Atmp, B);
      TmNCopy(T, result);
      TmNDelete(T);
    } else {
      if (result == NULL)
	result = TmNCreate(dim1, dim3, NULL);
      else if (result->idim != dim1 || result->odim != dim3) {
	result->a =
	  OOGLRenewNE(HPtNCoord, result->a, dim1*dim3, "renew TransformN");
	result->idim = dim1; result->odim = dim3;
      }
      MAKEPRODUCT(result, Atmp, B);
    }
    TmNDelete(Atmp);
  } else {  /* B->idim < dim2 */
    TransformN *Btmp = TmNCreate(dim2, dim3, NULL);

    TmNPad(B, dim2, dim3, Btmp);
    if ( A == result ) {
      TransformN *T = TmNCreate( dim1, dim3, NULL);

      MAKEPRODUCT(T, A, Btmp);
      TmNCopy(T, result);
      TmNDelete(T);
    } else {
      if (result == NULL)
	result = TmNCreate(dim1, dim3, NULL);
      else if (result->idim != dim1 || result->odim != dim3) {
	result->a =
	  OOGLRenewNE(HPtNCoord, result->a, dim1*dim3, "renew TransformN");
	result->idim = dim1; result->odim = dim3;
      }
      MAKEPRODUCT(result, A, Btmp);
    }
    TmNDelete(Btmp);
  }
    
#undef MAKEPRODUCT
    
  return result;
}
  
/* Copy */
static inline TransformN *
TmNCopy(const TransformN *Tsrc, TransformN *Tdst)
{
  if (Tsrc != Tdst) {
    if (Tdst == NULL) {
      Tdst = TmNCreate(Tsrc->idim, Tsrc->odim, Tsrc->a);
    } else {
      if (Tdst->idim != Tsrc->idim || Tdst->odim != Tsrc->odim) {
	Tdst->a = OOGLRenewNE(HPtNCoord, Tdst->a, Tsrc->idim*Tsrc->odim,
			      "renew TransformN");
	Tdst->idim = Tsrc->idim; Tdst->odim = Tsrc->odim;
      }
      memcpy(Tdst->a, Tsrc->a, Tsrc->idim*Tsrc->odim*sizeof(HPtNCoord));
    }
  }
  return Tdst;
}

/* Set to identity */
static inline TransformN * 
TmNIdentity(TransformN *T)
{
  if (T == NULL) {
    T = TmNCreate(1, 1, NULL);
    T->a[0] = 1;
  } else {
    int i;
    int idim = T->idim, odim = T->odim;
    memset(T->a, 0, idim*odim*sizeof(HPtNCoord));
    if (idim == odim || idim < odim) {
      for(i = 0; i < idim; i++)
	T->a[i*odim+i] = 1;
    } else {  /*  idim > odim */
      for(i = 0; i < odim; i++)
	T->a[i*odim+i] = 1;
    }
  }
  return T;
}

/* Euclidean translations
 *
 * Be careful: the homogeneous component is now located at index 0.
 */
static inline TransformN * 
TmNTranslateOrigin(TransformN *T, const HPointN *pt)
{
  int i;
  HPtNCoord c = pt->v[0];
  int dim = pt->dim;

  /* first make sure T has the appropriate size etc. */
  if (T == NULL) {
    T = TmNCreate(dim, dim, NULL);
    TmNIdentity(T);
  } else {
    TmNIdentity(T);
    if (dim > T->odim) {
      TmNPad(T, T->idim, dim, T);
    }
  }

  /* its so easy now! */
  for(i = 1; i < dim; i++) {
    T->a[i] = pt->v[i] / c;
  }

  return T;
}

/* Pad the transform, with ones down the main diagonal, not assuming a
 * homogeneous coordinate that needs special attention. This function
 * explicitly performs the padding which is used implicitly by,
 * e.g. HPtNTransform() if the dimensions do not match.
 *
 * Padding is really simple now because the homogeneous component is
 * (internally) located at index 0 for HPointN and TransformN.
 * Therefor TmNPadSimple() has simply be renamed to TmNPad() because
 * it does the right thing automatically now.
 */
static inline TransformN *
TmNPad(const TransformN *Tin, int idim, int odim, TransformN *Tout)
{
  if (Tin == NULL) {
    if (Tout == NULL) {
      Tout = TmNCreate(idim, odim, NULL);
    } else {
      Tout->a = OOGLRenewNE(HPtNCoord, Tout->a, idim*odim, "renew TransformN");
      Tout->idim = idim; Tout->odim = odim;
    }
    TmNIdentity(Tout);
  } else if ( odim <= 0 || idim <= 0 ) {
    ;  /* do nothing */
  } else {
    int oldidim = Tin->idim, oldodim = Tin->odim;
    int i, j;
	
    if (Tin != Tout) {
      if (Tout == NULL) {
	Tout = TmNCreate(idim, odim, NULL);
      } else if (Tout->idim != idim || Tout->odim != odim) {
	Tout->a =
	  OOGLRenewNE(HPtNCoord, Tout->a, idim*odim, "renew TransformN");
	Tout->idim = idim; Tout->odim = odim;
      }

      if (oldidim < idim && oldodim < odim) {
	/* copy over first oldidim rows */
	for (i = 0; i < oldidim; i++) {
	  /* copy start of row */
	  for(j = 0; j < oldodim; j++) 
	    Tout->a[i*odim+j] = Tin->a[i*oldodim+j];
	  /* fill remaining part */
	  for(; j < odim; j++)
	    if (i == j)
	      Tout->a[i*odim+j] = 1.0;
	    else
	      Tout->a[i*odim+j] = 0.0;
	}
	/* fill remaining rows */
	for (; i < idim; i++) {
	  for(j = 0; j < odim; j++)
	    if (i==j)
	      Tout->a[i*odim+j] = 1;
	    else
	      Tout->a[i*odim+j] = 0;
	}
      } else if (oldidim < idim && oldodim >= odim) {	
	/* copy over the first oldidim rows, excess colomns are discarded */
	for (i = 0; i < oldidim; i++) {
	  for(j = 0; j < odim; j++) 
	    Tout->a[i*odim+j] = Tin->a[i*oldodim+j];
	}
	/* copy over the remaining rows, discarding excess columns */
	for (; i<idim; i++) {
	  for(j = 0; j < odim; j++)
	    if (i == j)
	      Tout->a[i*odim+j] = 1.0;
	    else
	      Tout->a[i*odim+j] = 0.0;
	}
      } else if (oldidim >= idim && oldodim < odim) {
	/* copy over the first idim rows, excess rows are discarded */
	for (i=0; i < idim; i++) {
	  for(j = 0; j < oldodim; j++) 
	    Tout->a[i*odim+j] = Tin->a[i*oldodim+j];
	  for(; j < odim; j++)
	    if (i == j)
	      Tout->a[i*odim+j] = 1;
	    else
	      Tout->a[i*odim+j] = 0;
	}
      } else if (oldidim >= idim && oldodim >= odim) {
	/* copy operation, discard excess columns and rows */
	for (i = 0; i < idim; i++)
	  for(j = 0; j < odim; j++) 
	    Tout->a[i*odim+j] = Tin->a[i*oldodim+j];
      }
    } else {
      TransformN *Tnew = TmNCreate(idim, odim, NULL);
      if ( oldidim < idim && oldodim < odim) {
	for ( i=0; i<oldidim; i++) {
	  for( j=0; j<oldodim; j++) 
	    Tnew->a[i*odim+j] = Tin->a[i*oldodim+j];
	  for( j=oldodim; j<odim; j++)
	    if (i==j)
	      Tnew->a[i*odim+j] = 1;
	}
	for ( i=oldidim; i<idim; i++) {
	  for( j=0; j<odim; j++)
	    if (i==j)
	      Tnew->a[i*odim+j] = 1;
	}
      } else if ( oldidim < idim && oldodim >= odim) {	
	for ( i=0; i<oldidim; i++) {
	  for( j=0; j<odim; j++) 
	    Tnew->a[i*odim+j] = Tin->a[i*oldodim+j];
	}
	for ( i=oldidim; i<idim; i++) {
	  for( j=0; j<odim; j++)
	    if (i==j)
	      Tnew->a[i*odim+j] = 1;
	}
      } else if ( oldidim >= idim && oldodim < odim) {	
	for ( i=0; i<idim; i++) {
	  for( j=0; j<oldodim; j++) 
	    Tnew->a[i*odim+j] = Tin->a[i*oldodim+j];
	  for( j=oldodim; j<odim; j++)
	    if (i==j)
	      Tnew->a[i*odim+j] = 1;
	}
      } else if ( oldidim >= idim && oldodim >= odim) {	
	for ( i=0; i<idim; i++)
	  for( j=0; j<odim; j++) 
	    Tnew->a[i*odim+j] = Tin->a[i*oldodim+j];
      }
      TmNCopy(Tnew, Tout);
      TmNDelete(Tnew);
    }
  }
  return Tout;
}
	
/* Pad with zeroes */
static inline TransformN *
TmNPadZero(const TransformN *Tin, int idim, int odim, TransformN *Tout)
{
  if (Tin == NULL) {
    if (Tout == NULL) {
      Tout = TmNCreate(idim, odim, NULL);
    } else {
      Tout->a = OOGLRenewNE(HPtNCoord, Tout->a, idim*odim, "renew TransformN");
      memset(Tout->a, 0, idim*odim*sizeof(HPtNCoord));
      Tout->idim = idim; Tout->odim = odim;
    }
  } else if ( odim <= 0 || idim <= 0 ) {
    ;  /* do nothing */
  } else {


    int oldidim = Tin->idim, oldodim = Tin->odim;
    int i, j;
	
    if (Tin != Tout) {
      if (Tout == NULL) {
	Tout = TmNCreate(idim, odim, NULL);
      } else if (Tout->idim != idim || Tout->odim != odim) {
	Tout->a = OOGLRenewNE(HPtNCoord, Tout->a, idim*odim, "renew TransformN");
	Tout->idim = idim; Tout->odim = odim;
      }

      memset(Tout->a, 0, idim*odim*sizeof(HPtNCoord) );

      if ( oldidim < idim && oldodim < odim) {
	for ( i=0; i<oldidim; i++) {
	  for( j=0; j<oldodim; j++) 
	    Tout->a[i*odim+j] = Tin->a[i*oldodim+j];
	}
      } else if ( oldidim < idim && oldodim >= odim) {	
	for ( i=0; i<oldidim; i++) {
	  for( j=0; j<odim; j++) 
	    Tout->a[i*odim+j] = Tin->a[i*oldodim+j];
	}
      } else if ( oldidim >= idim && oldodim < odim) {	
	for ( i=0; i<idim; i++) {
	  for( j=0; j<oldodim; j++) 
	    Tout->a[i*odim+j] = Tin->a[i*oldodim+j];
	}
      } else if ( oldidim >= idim && oldodim >= odim) {	
	for ( i=0; i<idim; i++)
	  for( j=0; j<odim; j++) 
	    Tout->a[i*odim+j] = Tin->a[i*oldodim+j];
      }
    } else {
      TransformN *Tnew = TmNCreate(idim, odim, NULL);
      if ( oldidim < idim && oldodim < odim) {
	for ( i=0; i<oldidim; i++) {
	  for( j=0; j<oldodim; j++) 
	    Tnew->a[i*odim+j] = Tin->a[i*oldodim+j];
	}
      } else if ( oldidim < idim && oldodim >= odim) {	
	for ( i=0; i<oldidim; i++) {
	  for( j=0; j<odim; j++) 
	    Tnew->a[i*odim+j] = Tin->a[i*oldodim+j];
	}
      } else if ( oldidim >= idim && oldodim < odim) {	
	for ( i=0; i<idim; i++) {
	  for( j=0; j<oldodim; j++) 
	    Tnew->a[i*odim+j] = Tin->a[i*oldodim+j];
	}
      } else if ( oldidim >= idim && oldodim >= odim) {	
	for ( i=0; i<idim; i++)
	  for( j=0; j<odim; j++) 
	    Tnew->a[i*odim+j] = Tin->a[i*oldodim+j];
      }
      TmNCopy(Tnew, Tout);
      TmNDelete(Tnew);
    }
  }
  return Tout;
}
	
/* Translations by the space of 'pt' */
static inline TransformN *
TmNSpaceTranslateOrigin( TransformN *T, HPointN *pt )
{
  /* this has not yet been implemented
   *  for now, just call the euclidean one */
  return TmNTranslateOrigin(T, pt);
}

/* Scale by the components of 'amount'
 *
 * I.e. generate a matrix which acts as such a scaling.  Therefore, if
 * T != NULL, we resize T such that idim >= dim and odim >= dim.
 */
static inline TransformN *
TmNScale(TransformN *T, const HPointN *amount)
{
  int i;
  int dim = amount->dim;

  if (T == NULL) {
    T = TmNCreate(dim, dim, NULL);
  }

  TmNIdentity(T);

  if (dim != T->idim || dim != T->odim) {
#if 0 /* this seems overly complicated ??? */
    int idim = T->idim, odim = T->odim;
    switch( (dim > idim) + 2*(dim>odim) + 4*(idim>odim) ) {
    case 0:  /* odim > idim > dim */
      idim = odim;
      break;
    case 1:  /* odim > dim > idim */
      idim = odim;
      break;
    case 2:  /* impossible */
      OOGLError(1, "incorrect switch");
      break;
    case 3:  /* dim > odim > idim */
      odim = dim; idim = dim;
      break;
    case 4:  /* idim > odim > dim */
      odim = idim;
      break;
    case 5:  /* impossible */
      OOGLError(1, "incorrect switch");
      break;
    case 6:  /* idim > dim > odim */
      odim = idim;
      break;
    case 7:  /* dim > idim > odim */
      odim = dim; idim = dim;
      break;
    }
#else
    /* T shall act as a scaling, so we just have to make sure that
     * input and output dimension are >= dim.
     */
    int idim = max(T->idim, dim), odim = max(T->odim, dim);
#endif
    TmNPad(T, idim, odim, T);
  }

  for(i = 0; i < dim; i++) {
    T->a[i*T->odim+i] = amount->v[i];
  }

  return T;
}

/* Construct a geodesic rotation taking vector 'from' to 'toward' */
static inline TransformN *
TmNRotate(TransformN *T, const HPointN *from, const HPointN *toward)
{
  int i, j, k;
  int dim = from->dim;
  HPtNCoord len, proj, cosA, sinA;
  HPtNCoord *planebasis1, *planebasis2; /* not homogeneous coords!*/
  HPtNCoord planecoord1, planecoord2;

  planebasis1 = OOGLNewNE(HPtNCoord, 2*dim, "TmNRotate data");
  planebasis2 = planebasis1 + dim;

#define NORMALIZE(v, vout)			\
  len = 0;					\
  for (k = 1; k < dim; k++)			\
    len += v[k] * v[k];				\
  if (len == 0) {				\
    len = 1;					\
  } else					\
    len = sqrt(len);				\
  for(k = 1; k < dim; k++)			\
    vout[k] = v[k] / len;

#define DOTPRODUCT(v1, v2, result)		\
  result = 0;					\
  for (i = 1; i < dim; i++)			\
    result += v1[i] * v2[i];
		

  if (dim != toward->dim) {
    if (toward->dim < dim) {
      HPointN *towardtmp = HPtNCreate(dim, NULL);
      HPtNPad((HPointN *)toward, dim, towardtmp);
      if (T == NULL)
	T = TmNCreate(dim, dim, NULL);
      else if (T->idim != dim || T->odim != dim) {
	T->a = OOGLRenewNE(HPtNCoord, T->a, dim*dim, "renew TransformN");
	T->idim = dim;  T->odim = dim;
      }
      memset(T->a+1, 0, (dim*dim-1)*sizeof(HPtNCoord));
      T->a[0] = 1.0;
      NORMALIZE(from->v, planebasis1);
      DOTPRODUCT(towardtmp->v, planebasis1, proj);
      for (i = 1; i < dim; i++)
	planebasis2[i] = towardtmp->v[i] - proj * planebasis1[i];
      NORMALIZE(planebasis2, planebasis2);
      DOTPRODUCT(towardtmp->v, towardtmp->v, len);
      if ((len = sqrt(len)) == 0) {
	OOGLError(1, "zero vector unexpected");
	return NULL;
      }
      cosA = proj / len;
      sinA = sqrt(1-cosA*cosA);
      for(i = 1; i < dim; i++) {  /* each basis vector */
	planecoord1 = (cosA - 1) * planebasis1[i] + sinA * planebasis2[i];
	planecoord2 = -sinA * planebasis1[i] + (cosA - 1) * planebasis2[i];
	for(j = 1; j < dim; j++) {
	  if (i == j) {
	    T->a[j*dim+i] =
	      1 + 
	      planecoord1*planebasis1[j] + planecoord2*planebasis2[j];
	  } else {
	    T->a[j*dim+i] = planecoord1*planebasis1[j] + 
	      planecoord2*planebasis2[j];
	  }
	}
      }
    } else {
      HPointN *fromtmp = HPtNCreate(dim, NULL);
      dim = toward->dim;
      HPtNPad((HPointN *)from, dim, fromtmp);
      if (T == NULL)
	T = TmNCreate(dim, dim, NULL);
      else if (T->idim != dim || T->odim != dim) {
	T->a = OOGLRenewNE(HPtNCoord, T->a, dim*dim, "renew TransformN");
	T->idim = dim;  T->odim = dim;
      }
      memset(T->a+1, 0, (dim*dim-1)*sizeof(HPtNCoord));
      T->a[0] = 1.0;
      NORMALIZE(fromtmp->v, planebasis1);
      DOTPRODUCT(toward->v, planebasis1, proj);
      for (i = 1; i < dim; i++)
	planebasis2[i] = toward->v[i] - proj * planebasis1[i];
      NORMALIZE(planebasis2, planebasis2);
      DOTPRODUCT(toward->v, toward->v, len);
      if ((len = sqrt(len)) == 0) {
	OOGLError(1, "zero vector unexpected");
	return NULL;
      }
      cosA = proj / len;
      sinA = sqrt(1-cosA*cosA);
      for (i = 1; i < dim; i++) {  /* each basis vector */
	planecoord1 = (cosA - 1) * planebasis1[i] + sinA * planebasis2[i];
	planecoord2 = -sinA * planebasis1[i] + (cosA - 1) * planebasis2[i];
	for (j = 1; j < dim; j++) {
	  if (i == j) {
	    T->a[j*dim+i] = 1 + 
	      planecoord1*planebasis1[j] + planecoord2*planebasis2[j];
	  } else {
	    T->a[j*dim+i] = planecoord1*planebasis1[j] + 
	      planecoord2*planebasis2[j];
	  }
	}
      }
    }
  } else {
    if (T == NULL)
      T = TmNCreate(dim, dim, NULL);
    else if (T->idim != dim || T->odim != dim) {
      T->a = OOGLRenewNE(HPtNCoord, T->a, dim*dim, "renew TransformN");
      T->idim = dim;  T->odim = dim;
    }
    memset(T->a+1, 0, (dim*dim-1)*sizeof(HPtNCoord));
    T->a[0] = 1;

    /* form orthonormal basis for plane */

    NORMALIZE(from->v, planebasis1);
    DOTPRODUCT(toward->v, planebasis1, proj);
    for (i = 1; i < dim; i++)
      planebasis2[i] = toward->v[i] - proj * planebasis1[i];
    NORMALIZE(planebasis2, planebasis2);
	

    /* now that we have the basis vectors for the plane,
       calculate the rotation matrix */
    DOTPRODUCT(toward->v, toward->v, len);
    if ((len = sqrt(len)) == 0) {
      OOGLError(1, "zero vector unexpected");
      return NULL;
    }
    cosA = proj / len;
    sinA = sqrt(1-cosA*cosA);
	
    for(i = 1; i < dim; i++) {  /* each basis vector */
      planecoord1 = (cosA - 1) * planebasis1[i] + sinA * planebasis2[i];
      planecoord2 = -sinA * planebasis1[i] + (cosA - 1) * planebasis2[i];
      for(j = 1; j < dim; j++)
	if (i == j) T->a[j*dim+i] = 1 + 
	  planecoord1*planebasis1[j] + planecoord2*planebasis2[j];
	else T->a[j*dim+i] = planecoord1*planebasis1[j] + 
	  planecoord2*planebasis2[j];
    }
  }
#undef DOTPRODUCT
#undef NORMALIZE

  OOGLFree(planebasis1);
  return T;
}

/* special operation for computing n-d counterpart for movement in a
 * subspace
 *
 * permute tells which dimensions are projected
 *
 * delta is the usual 4x4 matrix used in Geomview
 *
 * NOTE: this functions computes mat * delta, i.e. delta is applied
 * from the right
 */
static inline TransformN *
TmNApplyDN(TransformN *mat, int *perm, Transform3 delta)
{
  HPtNCoord sum;
  int idim, odim;
  const int d3 = 4;  /* this is the dimension of the delta matrix */
  HPtNCoord *tmp;
  int i, j, k;
  int perm_dim;
  static const int dflt_perm[] = { 1, 2, 3, 0 };

  if (!perm) {
    perm = (int *)dflt_perm;
  }

  perm_dim = perm[0];
  for (i = 0; i < d3; i++) {
    perm_dim = max(perm[i], perm_dim);
  }
  ++perm_dim;
  idim = mat->idim;
  if (mat->odim < perm_dim) {
    odim = max(mat->odim, perm_dim);
    TmNPad(mat, idim, odim, mat); /* Note: the input matrix is changed here */
  } else {
    odim = mat->odim;
  }

  /* Make a "copy" of mat, copying only the COLUMNS mentioned in perm.
   * As delta operates from the right, we need only those columns, but
   * all rows of mat.
   */
  tmp = OOGLNewNE(HPtNCoord, idim*d3, "TmNApplyDN data");

  for (i = 0; i < idim; i++) {
    for (j= 0; j < d3; j++) {
      tmp[i*d3+j] = mat->a[i*odim+perm[j]];
    }
  }

  /* apply delta to the appropriate columns and copy the result back
   * to mat
   */
  for(i = 0; i < idim; i++) {
    for (j= 0; j < d3; j++) {
      sum = 0;
      for(k = 0; k < d3; k++) {
	sum += tmp[i*d3+k] * delta[k][j];
      }
      mat->a[i*odim + perm[j]] = sum;
    }
  }

  OOGLFree(tmp);
  return mat;
}

/* Apply a 3d transformation from the left (in contrast to
 * TmNApplyDN() which applies it from the right.
 */
static inline TransformN *
TmNApplyT3TN(Transform3 T3, int *perm, TransformN *mat)
{
  HPtNCoord sum;
  int idim, odim, d3 = 4;  /* this is the dimension of the delta matrix */
  HPtNCoord *tmp;
  int i, j, k;
  int perm_dim;
  static const int dflt_perm[] = { 1, 2, 3, 0 };

  if (!perm) {
    perm = (int *)dflt_perm;
  }

  /* As TransfromN's are virtually infinite (i.e. act as identity if
   * the input dimensions do not match), we only have to make sure
   * that "mat" has enough space such that perm can be applied.
   */
  perm_dim = perm[0];
  for (i = 0; i < d3; i++) {
    perm_dim = max(perm[i], perm_dim);
  }
  ++perm_dim;

  if (!mat) {
    idim = odim = perm_dim;
    mat = TmNCreate(idim, odim, NULL);
    TmNIdentity(mat);
  } else {
    odim = mat->odim;
    if (mat->idim < perm_dim) {
      idim = max(mat->idim, perm_dim);
      TmNPad(mat, idim, odim, mat); /* Note: the input matrix is changed here */
    } else {
      idim = mat->idim;
    }
  }

  /* Make a "copy" of mat, copying only the ROWS mentioned in perm.
   * As delta operates from the right, we need only those rows, but
   * all columns of mat.
   */
  tmp = OOGLNewNE(HPtNCoord, odim*d3, "TmNApplyDN data");

  for(i = 0; i < d3; i++) {
    for(j= 0; j < odim; j++) {
      tmp[i*odim+j] = mat->a[perm[i]*odim+j];
    }
  }

  for (i = 0; i < d3; i++) {
    for (j = 0; j < odim; j++) {
      sum = 0;
      for(k = 0; k < d3; k++) {	
	sum += T3[i][k] * tmp[k*odim+j];
      }
      mat->a[perm[i]*odim + j] = sum;
    }
  }

  OOGLFree(tmp);

  return mat;
}

/* concatenate (from the left) with a permutation matrix which maps x,
 * y, z to the 3d sub-space described by "perm". This means that we
 * simply have to interchange the rows of from.
 *
 * This is meant to map a 3d object to another 3d sub-space,
 * e.g. a camera geometry. This means that we really want to use 
 *
 * dflt_perm^{-1} perm
 *
 * as permutation.
 */
static inline TransformN *
TmNMap(const TransformN *from, int *axes, TransformN *to)
{
  const int d3 = 4;
  int odim;
  int row1, row2, col;
  int perm_dim, i;
  static const int dflt_perm[] = { 1, 2, 3, 0 };
  static const int inv_perm[] = { 3, 0, 1, 2 };
  int perm[4];

  if (!axes) {
    axes = (int *)dflt_perm;
  }

  perm_dim = axes[0];
  for (i = 1; i < d3; i++) {
    perm_dim = max(axes[i], perm_dim);
  }
  ++perm_dim;
  for (i = 0; i < d3; i++) {
    perm[i] = axes[inv_perm[i]];
  }

  if (from == NULL) {
    odim = perm_dim;
    to = TmNPadZero(to, d3, odim, to);
    for (i = 0; i < d3; i++) {
      to->a[odim*i+perm[i]] = 1.0;
    }
    return to;
  } 

  if (to != from) {
    odim = from->odim;
    to = TmNPadZero(to, d3, odim, to);  

    for (row1 = 0; row1 < d3; row1++) {
      row2 = perm[row1];
      for (col = 0; col < odim; col++) {
	to->a[row1*odim+col] = from->a[row2*odim+col];
      }
    }
    return to;
  }

  /* from == to, need to swap rows in place */
  if (to->odim < perm_dim) {
    TmNPad(to, from->idim, perm_dim, to);
  }
  odim = to->odim;

  for (row1 = 0; row1 < d3; row1++) {
    row2 = perm[row1];
    for (col = 0; col < odim; col++) {
      HPtNCoord swap = to->a[row1*odim+col];
      to->a[row1*odim+col] = to->a[row2*odim+col];
      to->a[row2*odim+col] = swap;
    }
  }
  
  /* simply discard all excess rows */
  to->a = OOGLRenewNE(HPtNCoord, to->a, d3*odim, "renew TransformN");
  to->idim = d3;

  return to;
}

/* Concatenate (from the right) from with the permutation and
 * projection defined by perm. This function creates an N-transform
 * which has the effect of HPtNTransformComponents(), but doing the
 * projection and permutation by a single matrix multiplication spares
 * a lot of floating point operations, especially in higher
 * diemensions.
 * 
 * To will be an idim x 4 matrix. If "perm" contains dimensions not
 * present in "from", then we can simply ignore those.
 */
static inline TransformN *
TmNProject(const TransformN *from, int *perm, TransformN *to)
{
  static const int dflt_perm[] = { 1, 2, 3, 0 };
  const int d3 = 4; /* grin */
  int perm_dim, i, j, idim, odim;

  if (!perm) {
    perm = (int *)dflt_perm;
  }

  perm_dim = perm[0];
  for (i = 0; i < d3; i++) {
    perm_dim = max(perm[i], perm_dim);
  }
  ++perm_dim;

  if (from == NULL) {
    idim = perm_dim;
    odim = 0;
  } else {
    idim = from->idim;
    odim = from->odim;
  }
  if (to == NULL) {
    to = TmNCreate(max(perm_dim, idim), d3, NULL);
  } else {
    to->idim = max(perm_dim, idim);
    to->odim = d3;
    to->a = OOGLRenewNE(HPtNCoord, to->a, to->idim*d3, "renew TransformN");
    memset(to->a, 0, perm_dim*d3*sizeof(HPtNCoord));
  }
  for (j = 0; j < d3; j++) {
    if (perm[j] < odim) {
      for (i = 0; i < idim; i++) {
	to->a[i*d3+j] = from->a[i*odim+perm[j]];
      }
    } else {
      to->a[perm[j]*d3+j] = 1.0;
    }
  }

  return to;
}

/* Return dimensions of a TransformN.  Value is first dimension. */
/* idim and/or odim may be NULL, in which case they're not returned */
static inline int 
TmNGetSize(const TransformN *T, int *idim, int *odim)
{
  if ( T == NULL)
    return 0;

  if (idim) *idim = T->idim;
  if (odim) *odim = T->odim;
  return T->idim;
}

static inline void
TmNPrint(FILE *f, const TransformN *T) 
{
  int i, j;
  int idim = T->idim, odim = T->odim;

  if (f == NULL) {
    return;
  }
  fprintf(f, "ntransform { %d %d\n", idim, odim);
  for(i = 0; i < idim; i++) {
    for(j = 0; j < odim; j++) {
      fprintf(f, "%10.7f ", T->a[i*odim+j]);
    }
    fprintf(f, "\n");
  }
  fprintf(f, "}\n");
}

static inline TransformN *
TmNRead(IOBFILE *f, int binary)
{
  HPtNCoord *a;
  int got, n, brack = 0;
  int dim[2];

  iobfexpecttoken(f, "ntransform");
    
  if (iobfnextc(f, 0) == '{')
    brack = iobfgetc(f);

  if (iobfgetni(f, 2, dim, binary) < 2 || dim[0] <= 0 || dim[1] <= 0) {
    OOGLSyntax(f, "Expected dimensions of N-D transform");
    return NULL;
  }
  n = dim[0]*dim[1];
  a = OOGLNewNE(HPtNCoord, n, "new TransformN data");

  got = iobfgetnf(f, n, a, binary);
  if (got < n) {
      OOGLSyntax(f, "N-D transform: error reading %d'th of %d values.", got, n);
    OOGLFree(a);
    return NULL;
  }

  if (brack)
    iobfexpecttoken(f, "}");

  return TmNCreate(dim[0], dim[1], a);
}

#ifdef _gv_tmn_max_
# undef max
# undef _gv_tmn_max_
#endif
#ifdef _gv_tmn_min_
# undef min
# undef _gv_tmn_min_
#endif

#endif /* _GV_TRANSFORMN_H_ */

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
