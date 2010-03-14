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
 * Mark Phillips, Claus-Justus Heine */

#ifndef _GV_HPOINTN_H_
#define _GV_HPOINTN_H_

#if HAVE_CONFIG_H
# include "config.h"
#endif

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

#include <stdlib.h>
#include <math.h>
#include <ooglutil.h>
#include "geomtypes.h"
#include "freelist.h"

extern DEF_FREELIST(HPointN);

static inline HPointN *HPtNCreate(int dim, const HPtNCoord *vec);
static inline void HPtNDelete(HPointN *pt);
static inline HPointN *HPtNCopy(const HPointN *pt1, HPointN *pt2);
static inline HPointN *Pt4ToHPtN(const HPoint3 *v4, HPointN *vN);
static inline HPointN *Pt3ToHPtN(Point3 *v3, HPointN *vN);
static inline HPointN *HPt3ToHPtN(const HPoint3 *v4, int *perm, HPointN *vN);
static inline HPoint3 *HPtNToHPt3(const HPointN *from, int *axes, HPoint3 *hp3);
static inline HPointN *HPtNPad(const HPointN *pt1, int dim2, HPointN *pt2);
static inline HPointN *HPtNAdd(const HPointN *pt1, const HPointN *pt2,
			       HPointN *sum);
static inline int HPtNSpace(const HPointN *pt);
static inline HPointN *HPtNSetSpace(HPointN *pt, int space);
static inline HPointN *HPtNComb(HPtNCoord s1, const HPointN *pt1,
				HPtNCoord s2, const HPointN *pt2,
				HPointN *sum);
static inline HPtNCoord HPtNDot(const HPointN *p1, const HPointN *p2);
static inline HPtNCoord HPtNDehomogenize(const HPointN *from, HPointN *to);
static inline HPointN *HPtNTransform3(Transform3 T, int *perm,
				      const HPointN *from, HPointN *to);
static inline HPointN *HPtNTransform(const TransformN *T,
				     const HPointN *from, HPointN *to);
static inline HPointN *HPtNTTransform(const TransformN *T,
				      const HPointN *from, HPointN *to);
static inline HPoint3 *HPtNTransProj(const TransformN *T,
				     const HPointN *from,
				     HPoint3 *result);
static inline HPoint3 *HPtNTransformComponents(const TransformN *T,
					       const HPointN *from,
					       int *perm,
					       HPoint3 *results);
static inline HPointN *HPt3NTransform(const TransformN *T,
				      const HPoint3 *from, HPointN *to);
static inline HPointN *Pt4NTransform(const TransformN *T,
				     const HPoint3 *from, HPointN *to);
static inline HPt3Coord HPtNNTransPt3(TransformN *TN, int *axes,
				      const HPointN *ptN, Point3 *result);
static inline HPt3Coord HPt3NTransPt3(TransformN *TN, int *axes,
				      const HPoint3 *hpt4, int v4d,
				      Point3 *result);
static inline void HPt3NTransHPt3(TransformN *TN, int *axes,
				  const HPoint3 *hpt4, int v4d,
				  HPoint3 *result);
static inline void HPtNMinMax(HPointN *min, HPointN *max, HPointN *other);

#include "transform3.h"

#ifndef max
# define _gv_hptn_max_
# define max(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
# define _gv_hptn_min_
# define min(a,b) ((a) < (b) ? (a) : (b))
#endif

static inline void HPtNFreeListPrune(void)
{
  FreeListNode *old;
  HPointN *oldpt;
  size_t size = 0;
  
  while (HPointNFreeList) {
    old = HPointNFreeList;
    HPointNFreeList = old->next;
    oldpt = (HPointN *)old;
    if (oldpt->size && oldpt->v) {
      OOGLFree(oldpt->v);
      size += oldpt->size * sizeof(HPtNCoord);
    }
    OOGLFree(old);
    size += sizeof(HPointN);
  }
  OOGLWarn("Freed %ld bytes.\n", size);
}

static inline HPointN *
HPtNCreate(int dim, const HPtNCoord *vec)
{
  HPointN *pt;

  FREELIST_NEW(HPointN, pt);

  if(dim <= 0) {
    dim = 1;
  }
  pt->dim = dim;
  pt->flags = 0; /* for now */
  if (pt->size < dim) {
    pt->v = OOGLRenewNE(HPtNCoord, pt->v, dim, "new HPointN data");
    pt->size = dim;
  }
  if(vec == NULL) {
    memset(pt->v+1, 0, (dim-1)*sizeof(HPtNCoord));
    pt->v[0] = 1.0;
  } else {
    memcpy(pt->v, vec, dim*sizeof(HPtNCoord));
  }
  return pt;
}

static inline void
HPtNDelete(HPointN *pt)
{
  if(pt) {
    /* if(pt->v) OOGLFree(pt->v); */
    FREELIST_FREE(HPointN, pt);
  }
}

static inline HPointN *
HPtNCopy(const HPointN *pt1, HPointN *pt2)
{
  if(pt2 == NULL) {
    pt2 = HPtNCreate(pt1->dim, pt1->v);
  } else {
    if(pt2->dim != pt1->dim) {
      pt2->v = OOGLRenewNE(HPtNCoord, pt2->v, pt1->dim, "renew HPointN");
      pt2->dim = pt1->dim;
    }
    memcpy(pt2->v, pt1->v, pt1->dim*sizeof(HPtNCoord));
  }
  return pt2;
}

/* Convert a HPoint3 into a HPointN while interpreting the HPoint3 as
 * a 4-point. This means that we do NOT perfom dehomogenization here.
 */
static inline HPointN *
Pt4ToHPtN(const HPoint3 *v4, HPointN *vN)
{
  int i;

  if (!vN) {
    vN = HPtNCreate(5, NULL);
  } else if (vN->dim < 5) {
    vN->v = OOGLRenewNE(HPtNCoord, vN->v, 5, "renew HPointN");
    vN->dim = 5;
  }
  vN->v[0] = 1.0;
  for (i = 0; i < 4; ++i) {
    vN->v[i+1] = ((HPt3Coord *)v4)[i];
  }
  for (++i; i < vN->dim; i++) {
    vN->v[i] = 0.0;
  }
  return vN;
}

/* Convert a Point3 into a HPointN */
static inline HPointN *
Pt3ToHPtN(Point3 *v3, HPointN *vN)
{
int i;

  if (!vN) {
    vN = HPtNCreate(4, NULL);
  } else if (vN->dim < 4) {
    vN->v = OOGLRenewNE(HPtNCoord, vN->v, 4, "renew HPointN");
    vN->dim = 5;
  }
  vN->v[0] = 1.0;
  for (i = 0; i < 3; ++i) {
    vN->v[i+1] = ((HPt3Coord *)v3)[i];
  }
  for (++i; i < vN->dim; i++) {
    vN->v[i] = 0.0;
  }

  return vN;
}

/* Convert a HPoint3 into a HPointN while interpreting the HPoint3 as
 * a 3-point. The homogeneous component of v4 is moved to vN->v[0].
 */
static inline HPointN *
HPt3ToHPtN(const HPoint3 *v4, int *perm, HPointN *vN)
{
  const int d3 = 4;
  int i;
  int perm_dim;

  if (!perm) {
    perm_dim = 4;
  } else {
    perm_dim = perm[0];
    for (i = 0; i < d3; i++) {
      perm_dim = max(perm[i], perm_dim);
    }
    ++perm_dim;
  }
  if (!vN) {
    vN = HPtNCreate(perm_dim, NULL);
  } else if (vN->dim < perm_dim) {
    vN->v = OOGLRenewNE(HPtNCoord, vN->v, perm_dim, "renew HPointN");
    vN->dim = perm_dim;      
  }
  if (!perm) {
    vN->v[0] = v4->w;
    vN->v[1] = v4->x;
    vN->v[2] = v4->y;
    vN->v[3] = v4->z;
    for (i = 4; i < vN->dim; i++) {
      vN->v[i] = 0.0;
    }
  } else {
    memset(vN->v, 0, vN->dim*sizeof(HPtNCoord));
    for (i = 0; i < d3; i++) {
      vN->v[perm[i]] = ((HPt3Coord *)v4)[i];
    }
  }
  return vN;
}

/* Transform a 4-point to a 3-point according to the mapping defined
 * in "axes".
 */
static inline HPoint3 *
HPtNToHPt3(const HPointN *from, int *axes, HPoint3 *hp3)
{
  HPt3Coord *to = (HPt3Coord *)hp3;
  int i, dim = from->dim;

  if (!axes) {
    hp3->w = from->v[0];
    hp3->x = from->v[1];
    hp3->y = from->v[2];
    hp3->z = from->v[3];
  } else {
    for (i = 0; i < 4; i++) {
      if (axes[i] > dim-1) {
	to[i] = 0.0;
      } else {
	to[i] = from->v[axes[i]];
      }
    }
  }
  return hp3;
}

/* Copy pt1 to pt2 and make sure that pt2 has dimension dim2.  If
 * pt1->dim < dim2, then pt1 is implicitly padded with zeros.
 */
static inline HPointN *
HPtNPad(const HPointN *pt1, int dim2, HPointN *pt2)
{
  int dim1 = pt1->dim;

  if( dim2 < 1 ) { /* Uhh? */
    return(NULL);
  }

  if(pt1 == NULL) {
    pt2 = HPtNCreate(dim2, NULL);
    return pt2;
  }

  if(pt1 != pt2) {
    if(pt2 == NULL) {
      pt2 = HPtNCreate(dim2,NULL);
    } else if (pt2->dim != dim2) {
      pt2->v = OOGLRenewNE(HPtNCoord, pt2->v, dim2, "renew HPointN");
      pt2->dim = dim2;
    }
    if (dim1 <= dim2) {
      memcpy(pt2->v, pt1->v, dim1* sizeof(HPtNCoord));
      memset(pt2->v+dim1, 0, (dim2-dim1)*sizeof(HPtNCoord));
    } else {
      memcpy(pt2->v, pt1->v, dim2*sizeof(HPtNCoord));
    }
  } else {
    /* now that the homogeneous component is at 0 this is a simple
     * padding operation.
     */
    if (pt2->dim != dim2) {
      pt2->v = OOGLRenewNE(HPtNCoord, pt2->v, dim2, "renew HPointN");
    }
    if (dim2 > pt2->dim) {
      memset(pt2->v+pt2->dim, 0, (dim2-pt2->dim)*sizeof(HPtNCoord));
    }
  }
  return pt2;
}

/* Add two HPointN's */
static inline HPointN *
HPtNAdd(const HPointN *pt1, const HPointN *pt2, HPointN *sum)
{
  int dim1 = pt1->dim, dim2 = pt2->dim;
  HPtNCoord c1 = pt1->v[0], c2 = pt2->v[0];
  int i;
	
  if (dim1 == dim2) {
    if (sum == NULL) {
      sum = HPtNCreate(dim1, NULL);
    } else if(sum->dim < dim1) {
      sum->v = OOGLRenewNE(HPtNCoord, sum->v, dim1, "renew HPointN");
      sum->dim = dim1;
    }
    sum->v[0] = c1*c2;
    for (i = 1; i < dim1; i++) {
      sum->v[i] = c2*pt1->v[i] + c1*pt2->v[i];
    }
  } else {
    /* make sure pt1 is the larger one */
    if (dim1 < dim2) {
      const HPointN *swap = pt1;

      pt1 = pt2; pt2 = swap; dim2 = dim1; dim1 = pt1->dim;
    }
    
    if (sum == NULL) {
      sum = HPtNCreate(dim1,NULL);
    } else if(sum->dim < dim1) {
      sum->v = OOGLRenewNE(HPtNCoord, sum->v, dim1, "renew HPointN");
      sum->dim = dim1;
    }
    sum->v[0] = c1*c2;
    for (i = 1; i < dim2; i++) {
      sum->v[i] = c2*pt1->v[i] + c1*pt2->v[i];
    }
    for (; i < dim1; i++) {
      sum->v[i] = c2*pt1->v[i];
    }
  }
  return sum;
}
	
/* Space */
static inline int 
HPtNSpace(const HPointN *pt)
{
  (void)pt;
  return TM_EUCLIDEAN;
}

static inline HPointN *
HPtNSetSpace(HPointN *pt, int space)
{
  if (space != TM_EUCLIDEAN) {
    OOGLError(1, "Non-Euclidean space not support in higher dimensions.\n");
    return NULL;
  }
  return pt;
}

/* Linear combination */
static inline HPointN *
HPtNComb(HPtNCoord s1, const HPointN *pt1,
	 HPtNCoord s2, const HPointN *pt2,
	 HPointN *sum)
{
  int dim1 = pt1->dim, dim2 = pt2->dim;
  HPtNCoord c1 = pt1->v[0], c2 = pt2->v[0];
  int i;
	
  if (dim1 == dim2) {
    if (sum == NULL) {
      sum = HPtNCreate(dim1, NULL);
    } else if(sum->dim < dim1) {
      sum->v = OOGLRenewNE(HPtNCoord, sum->v, dim1, "renew HPointN");
      sum->dim = dim1;
    }
    sum->v[0] = c1*c2;
    for (i = 1; i < dim1; i++) {
      sum->v[i] = c2*s1*pt1->v[i] + c1*s2*pt2->v[i];
    }
  } else {
    /* make sure pt1 is the larger one */
    if (dim1 < dim2) {
      const HPointN *swap = pt1;
      pt1 = pt2; pt2 = swap; dim2 = dim1; dim1 = pt1->dim;
    }
    
    if (sum == NULL) {
      sum = HPtNCreate(dim1,NULL);
    } else if(sum->dim < dim1) {
      sum->v = OOGLRenewNE(HPtNCoord, sum->v, dim1, "renew HPointN");
      sum->dim = dim1;
    }
    sum->v[0] = c1*c2;
    for (i = 1; i < dim2; i++) {
      sum->v[i] = c2*s1*pt1->v[i] + c1*s2*pt2->v[i];
    }
    for (; i < dim1; i++) {
      sum->v[i] = c2*s1*pt1->v[i];
    }
  }
  return sum;
}

/* Dot product of two vectors */
static inline HPtNCoord
HPtNDot(const HPointN *p1, const HPointN *p2)
{
  HPtNCoord result;
  int i;
  int dim = p1->dim;
  
  if (p2->dim < dim) {
    dim = p2->dim;
  }

  result = 0;
  for(i = 1; i< dim; i++)
    result += p1->v[i] * p2->v[i];

  return result / (p1->v[0] * p2->v[0]);
}

/* Dehomogenize */
static inline HPtNCoord
HPtNDehomogenize(const HPointN *from, HPointN *to)
{
  int dim = from->dim;
  HPtNCoord c = from->v[0], inv = 1.0 / c;
  int i;

  if (c == 1.0 || c == 0.0) {
    if (from != to) {
      HPtNCopy(from, to);
    }
    return (HPtNCoord)0.0;
  }

  if (to == NULL) {
    to = HPtNCreate(dim, NULL);
  } else if (to->dim != dim) {
    to->v = OOGLRenewNE(HPtNCoord, to->v, dim, "renew HPointN");
    to->dim = dim;
  }

  for( i=1; i < dim; i++)
    to->v[i] = from->v[i] * inv;
  to->v[0] = 1.0;

  return c;
}

/* Transform an HPointN according to a 3d transform acting only on the
 * sub-space defined by "axes". The standard axes should look like
 * {dx,dy,dz,0} because the homogeneous divisor of HPoint3's is
 * located at index 3.
 */
static inline HPointN *
HPtNTransform3(Transform3 T, int *perm, const HPointN *from, HPointN *to)
{
  const int d3 = 4;
  int i;
  HPoint3 from3;
  HPt3Coord *from3data = HPoint3Data(&from3);
  int perm_dim;

  if (!perm) {
    perm_dim = 4;
  } else {
    perm_dim = perm[0];
    for (i = 0; i < d3; i++) {
      perm_dim = max(perm[i], perm_dim);
    }
    ++perm_dim;
  }
  HPtNToHPt3(from, perm, &from3);

  HPt3Transform(T, &from3, &from3);
  if (from->dim < perm_dim) {
    to = HPtNPad(from, perm_dim, to);
  } else {
    to = HPtNCopy(from, to);
  }
  if (perm) {
    for (i = 0; i < 4; i++) {
      to->v[perm[i]] = from3data[i];
    }
  } else {
    to->v[0] = from3data[3];
    to->v[1] = from3data[0];
    to->v[2] = from3data[1];
    to->v[3] = from3data[2];
  }
  return to;
}

/* Apply a TransformN to an HPointN.
 *
 * If from->dim < T->idim, then from is implicitly padded with zeros,
 * if from->dim > T->idim, then T is interpreted as identity on the
 * sub-space defined by T->idim < idx < T->odim, the remaining input
 * components are mapped to zero.
 */
static inline HPointN *
HPtNTransform(const TransformN *T, const HPointN *from, HPointN *to)
{
  int idim, odim, dim = from->dim;
  int i, j;
  VARARRAY(vspace, HPtNCoord, dim);
  HPtNCoord *v;

  if (!T) {
    return HPtNCopy(from, to);
  }

  idim = T->idim;
  odim = T->odim;

  if (from == to) {
    v = vspace;
    for (i = 0; i < dim; i++) {
      v[i] = from->v[i];
    }
  } else {
    v = from->v;
  }

  if(to == NULL) {
    to = HPtNCreate(odim, NULL);
  } else if (to->dim != odim) {
    to->v = OOGLRenewNE(HPtNCoord, to->v, odim, "renew HPointN");
    to->dim = odim;
  }

  if (idim == dim) {
    /* the easy case */
    for (i = 0; i < odim; i++) {
      to->v[i] = 0;
      for (j = 0; j < idim; j++) {
	to->v[i] += v[j] * T->a[j*odim+i];
      }
    }
  } else if (idim > dim) {
    /* pad with zeroes, the homogeneous component sits at index zero
       and is automatically handled correctly. */
    for(i = 0; i < odim; i++) {
      to->v[i] = 0;
      for (j = 0; j < dim; j++) {
	to->v[i] += v[j] * T->a[j*odim+i];
      }
    }
  } else { /* obviously the case idim < dim */
    /* implicitly pad the matrix, i.e. T acts as unity on the missing
     * dimens+ions.
     */
    for (i = 0; i < odim; i++) {
      to->v[i] = 0;
      for (j = 0; j < idim; j++) {
	to->v[i] += v[j] * T->a[j*odim+i];
      }
      if (i >= idim && i < dim) {
	to->v[i] += v[i];
      }
    }
  }

  return to;
}

/* Apply the transpose of a TransformN to an HPointN.
 *
 * If from->dim < T->odim, then form is implicitly padded with zeros,
 * if from->dim > T->odim, then T is interpreted as identity on the
 * sub-space defined by T->odim < idx < T->idim, the remaining input
 * components are mapped to zero.
 */
static inline HPointN *
HPtNTTransform(const TransformN *T, const HPointN *from, HPointN *to)
{
  int idim, odim, dim = from->dim;
  int i, j;
  VARARRAY(vspace, HPtNCoord, dim);
  HPtNCoord *v;

  if (!T) {
    return HPtNCopy(from, to);
  }

  idim = T->idim;
  odim = T->odim;

  if (from == to) {
    v = vspace;
    for (i = 0; i < dim; i++) {
      v[i] = from->v[i];
    }
  } else {
    v = from->v;
  }

  if(to == NULL) {
    to = HPtNCreate(odim, NULL);
  } else if (to->dim != idim) {
    to->v = OOGLRenewNE(HPtNCoord, to->v, idim, "renew HPointN");
    to->dim = idim;
  }

  if (odim == dim) {
    /* the easy case */
    for (i = 0; i < idim; i++) {
      to->v[i] = 0;
      for (j = 0; j < odim; j++) {
	to->v[i] += v[j] * T->a[i*odim+j];
      }
    }
  } else if (odim > dim) {
    /* pad with zeroes, the homogeneous component sits at index zero
       and is automatically handled correctly. */
    for(i = 0; i < idim; i++) {
      to->v[i] = 0;
      for (j = 0; j < dim; j++) {
	to->v[i] += v[j] * T->a[i*odim+j];
      }
    }
  } else { /* obviously the case odim < dim */
    /* implicitly pad the matrix, i.e. T acts as unity on the missing
     * dimens+ions.
     */
    for (i = 0; i < idim; i++) {
      to->v[i] = 0;
      for (j = 0; j < odim; j++) {
	to->v[i] += v[j] * T->a[i*odim+j];
      }
      if (i >= odim && i < dim) {
	to->v[i] += v[i];
      }
    }
  }

  return to;
}

/* A short and efficient routine. This routine assumes that T is the
 * result of a call to TmNProject() and is therefor non-NULL. "from"
 * may have more or less dimensions than T->idim, it is padded with
 * zeroes or truncated as necessary. from must have dimension 1 at
 * least.
 *
 * T already contains all necessary permutations and the projection to
 * map "from" to result "properly".
 */
static inline HPoint3 *
HPtNTransProj(const TransformN *T,
	      const HPointN *from,
	      HPoint3 *result)
{
  HPtNCoord *v = from->v;
  int idim;
  const int odim = 4; /* must be 4 */
  int i;

  idim = T->idim > from->dim ? from->dim : T->idim;

  result->x = v[0] * T->a[0*odim+0];
  result->y = v[0] * T->a[0*odim+1];
  result->z = v[0] * T->a[0*odim+2];
  result->w = v[0] * T->a[0*odim+3];
  for (i = 1; i < idim; i++) {
    result->x += v[i] * T->a[i*odim+0];
    result->y += v[i] * T->a[i*odim+1];
    result->z += v[i] * T->a[i*odim+2];
    result->w += v[i] * T->a[i*odim+3];
  }
  return result;
}

/* Transform p by T and then project to the sub-space defined by
 * "perm".
 */
static inline HPoint3 *
HPtNTransformComponents(const TransformN *T,
			const HPointN *from,
			int *perm,
			HPoint3 *results)
{
  int idim = T->idim, odim = T->odim, dim = from->dim;
  int i, j, k;
  HPt3Coord *res = (HPt3Coord *)results;

  if (idim == dim) {
    /* the easy case */
    for (k = 0; k < 4; k++) {
      i = perm[k];
      if (i > odim)
	continue;
      res[k] = 0;
      for (j = 0; j < idim; j++) {
	res[k] += from->v[j] * T->a[j*odim+i];
      }
    }
  } else if (idim > dim) {
    /* pad with zeroes, the homogeneous component sits at index zero
       and is automatically handled correctly. */
    for (k = 0; k < 4; k++) {
      i = perm[k];
      if (i > odim)
	continue;
      res[k] = 0;
      for (j = 0; j < dim; j++) {
	res[k] += from->v[j] * T->a[j*odim+i];
      }
    }
  } else { /* obviously the case idim < dim */
    /* implicitly pad the matrix, i.e. T acts as unity on the missing
     * dimens+ions.
     */
    for (k = 0; k < 4; k++) {
      i = perm[k];
      if (i > odim)
	continue;
      res[k] = 0;
      for (j = 0; j < idim; j++) {
	res[k] += from->v[j] * T->a[j*odim+i];
      }
      if (i >= idim && i < dim) {
	res[k] += from->v[i];
      }
    }
  }

  return results;
}

static inline HPointN *
HPt3NTransform(const TransformN *T, const HPoint3 *from, HPointN *to)
{
  int idim, odim;
  int i, j;
  HPtNCoord *v;

  if (!T) {
    return HPt3ToHPtN(from, NULL, to);
  }

  v = (HPtNCoord *)from;

  idim = T->idim;
  odim = T->odim;

  if(to == NULL) {
    to = HPtNCreate(odim, NULL);
  } else if (to->dim != odim) {
    to->v = OOGLRenewNE(HPtNCoord, to->v, odim, "renew HPointN");
    to->dim = odim;
  }

  if (idim == 4) {
    /* the easy case */
    for (i = 0; i < odim; i++) {
      to->v[i] = 0;
      for (j = 0; j < idim; j++) {
	to->v[i] += v[(j+3)%4] * T->a[j*odim+i];
      }
    }
  } else if (idim > 4) {
    /* pad with zeroes, the homogeneous component sits at index zero
       and is automatically handled correctly. */
    for(i = 0; i < odim; i++) {
      to->v[i] = 0;
      for (j = 0; j < 4; j++) {
	to->v[i] += v[(j+3)%4] * T->a[j*odim+i];
      }
    }
  } else { /* obviously the case idim < dim */
    /* implicitly pad the matrix, i.e. T acts as unity on the missing
     * dimens+ions.
     */
    for (i = 0; i < odim; i++) {
      to->v[i] = 0;
      for (j = 0; j < idim; j++) {
	to->v[i] += v[(j+3) % 4] * T->a[j*odim+i];
      }
      if (i >= idim && i < 4) {
	to->v[i] += v[i];
      }
    }
  }

  return to;
}

static inline HPointN *
Pt4NTransform(const TransformN *T, const HPoint3 *from, HPointN *to)
{
  int idim, odim;
  int i, j;
  HPtNCoord *v;

  if (!T) {
    return Pt4ToHPtN(from, to);
  }

  v = (HPtNCoord *)from;

  idim = T->idim;
  odim = T->odim;

  if(to == NULL) {
    to = HPtNCreate(odim, NULL);
  } else if (to->dim != odim) {
    to->v = OOGLRenewNE(HPtNCoord, to->v, odim, "renew HPointN");
    to->dim = odim;
  }

  if (idim == 5) {
    /* the easy case */
    for (i = 0; i < odim; i++) {
      to->v[i] = T->a[i];
      for (j = 1; j < idim; j++) {
	to->v[i] += v[j-1] * T->a[j*odim+i];
      }
    }
  } else if (idim > 5) {
    /* pad with zeroes, the homogeneous component sits at index zero
       and is automatically handled correctly. */
    for(i = 0; i < odim; i++) {
      to->v[i] = T->a[i];
      for (j = 1; j < 5; j++) {
	to->v[i] += v[j-1] * T->a[j*odim+i];
      }
    }
  } else { /* obviously the case idim < dim */
    /* implicitly pad the matrix, i.e. T acts as unity on the missing
     * dimens+ions.
     */
    for (i = 0; i < odim; i++) {
      to->v[i] = T->a[i];
      for (j = 0; j < idim; j++) {
	to->v[i] += v[j-1] * T->a[j*odim+i];
      }
      if (i >= idim && i < 5) {
	to->v[i] += v[i-1];
      }
    }
  }

  return to;
}

static inline HPt3Coord
HPtNNTransPt3(TransformN *TN, int *axes, const HPointN *ptN, Point3 *result)
{
  HPoint3 tmp;

  HPtNTransformComponents(TN, ptN, axes, &tmp);

  result->x = tmp.x / tmp.w;
  result->y = tmp.y / tmp.w;
  result->z = tmp.z / tmp.w;
  
  return tmp.w;
}

static inline HPt3Coord
HPt3NTransPt3(TransformN *TN, int *axes,
	      const HPoint3 *hpt4, int v4d, Point3 *result)
{
  HPointN *tmp;
  HPt3Coord retval;

  /* axes[3] should be 0 */

  if (v4d) {
    tmp = Pt4NTransform(TN, hpt4, NULL);
  } else {
    tmp = HPt3NTransform(TN, hpt4, NULL);
  }
  result->x = tmp->v[axes[0]] / tmp->v[axes[3]];
  result->y = tmp->v[axes[1]] / tmp->v[axes[3]];
  result->z = tmp->v[axes[2]] / tmp->v[axes[3]];

  retval = tmp->v[axes[3]];
  
  HPtNDelete(tmp);
  
  return retval;
}

static inline void
HPt3NTransHPt3(TransformN *TN, int *axes,
	       const HPoint3 *hpt4, int v4d, HPoint3 *result)
{
  HPointN *tmp;

  if (v4d) {
    tmp = Pt4NTransform(TN, hpt4, NULL);
  } else {
    tmp = HPt3NTransform(TN, hpt4, NULL);
  }

  result->w = tmp->v[axes[3]];
  result->x = tmp->v[axes[0]];
  result->y = tmp->v[axes[1]];
  result->z = tmp->v[axes[2]];

  HPtNDelete(tmp);
}

/* Utility function for bounding box computations. We assume that min
 * and max are dehomogenized (third part of the #if clause below), and
 * that they are large enough (min->dim >= other->dim <= max->dim)
 */
static inline void HPtNMinMax(HPointN *min, HPointN *max, HPointN *other)
{
#if 0
  int i;
  
  for (i = 1; i < other->dim; i++) {
    if (min->v[i] > other->v[i]) {
      min->v[i] = other->v[i];
    } else if (max->v[i] < other->v[i]) {
      max->v[i] = other->v[i];
    }
  }
#elif 0
  int i;
  HPtNCoord c = min->v[0], C = max->v[0], oc = other->v[0];
  
  for (i = 1; i < other->dim; i++) {
    if (oc * min->v[i] > c * other->v[i]) {
      min->v[i] = other->v[i] * c / oc ;
    } else if (oc * max->v[i] < C * other->v[i]) {
      max->v[i] = other->v[i] * C / oc;
    }
  }
#else
  int i;
  HPtNCoord oc = other->v[0];
  
  for (i = 1; i < other->dim; i++) {
    if (oc * min->v[i] > other->v[i]) {
      min->v[i] = other->v[i] / oc ;
    } else if (oc * max->v[i] < other->v[i]) {
      max->v[i] = other->v[i] / oc;
    }
  }
#endif  
}

#ifdef _gv_hptn_max_
# undef max
#endif
#ifdef _gv_hptn_min_
# undef min
#endif

#endif /* _GV_HPOINTN_H_ */

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
