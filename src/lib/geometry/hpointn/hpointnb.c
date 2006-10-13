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

/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner, Mark Phillips */


#include <ooglutil.h>
#include "hpointn.h"
#include <stdlib.h>
#include <math.h>

#ifndef alloca
#include <alloca.h>
#endif


HPointN *
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
/*    pt2->space = pt1->space; */
  return(pt2);
}

/* Convert a HPoint3 into a HPointN while interpreting the HPoint3 as
 * a 4-point.
 */
HPointN *Pt4ToHPtN(HPoint3 *v4, HPointN *vN)
{
  int i;

  if (!vN) {
    vN = HPtNCreate(5, NULL);
  }
  for (i = 0; i < 4; ++i) {
    vN->v[i] = ((HPt3Coord *)v4)[i];
  }
  for (; i < vN->dim-1; i++) {
    vN->v[i] = 0.0;
  }
  vN->v[i] = 1.0;

  return vN;
}

HPointN *
HPtNPad(HPointN *pt1, short dim2, HPointN *pt2)
{
	short dim1 = pt1->dim;
	int i;

	if( dim2 < 1 ) {
		return(NULL);
	}

	if(pt1 == NULL) {
		pt2 = HPtNCreate(dim2,NULL);
		return(pt2);
	}


	if(pt1 != pt2) {
		if(pt2 == NULL)
			pt2 = HPtNCreate(dim2,NULL);
		else if (pt2->dim != dim2) {
	   	 	pt2->v = OOGLRenewNE(HPtNCoord, pt2->v, dim2, "renew HPointN");
			pt2->dim = dim2;
		}
		if (dim1 <= dim2) {
			memset(pt2->v, 0, dim2*sizeof(HPtNCoord));
			i = dim1;
		} else {
			i = dim2;
		}
		memcpy(pt2->v, pt1->v, (i-1)*sizeof(HPtNCoord));
		pt2->v[dim2-1] = pt1->v[dim1-1];
	} else {
		HPointN *newpt = HPtNCreate(dim2,NULL);
		if (dim1 <= dim2) {
			i = dim1;
		} else {
			i = dim2;
		}
		memcpy(newpt->v, pt1->v, (i-1)*sizeof(HPtNCoord));
		newpt->v[dim2-1] = pt1->v[dim1-1];
		HPtNCopy(newpt,pt2);
	}
	return(pt2);
}

HPointN *
HPtNAdd(const HPointN *pt1, const HPointN *pt2, HPointN *sum)
{
	short dim1 = pt1->dim, dim2 = pt2->dim;
	int i;
	
  if(dim1 == dim2) {
  	if(sum == NULL)
		sum = HPtNCreate(dim1,NULL);
	else if(sum->dim < dim1) {
	   	sum->v = OOGLRenewNE(HPtNCoord, sum->v, dim1, "renew HPointN");
		sum->dim = dim1;
	}
	for( i=0; i<dim1-1; i++)
		sum->v[i] = pt1->v[i] + pt2->v[i];
	sum->v[dim1-1] = pt1->v[dim1-1];
	sum->dim = dim1;
  } else if (dim1 > dim2) {
  	if(sum == NULL)
		sum = HPtNCreate(dim1,NULL);
	else if(sum->dim < dim1) {
	   	sum->v = OOGLRenewNE(HPtNCoord, sum->v, dim1, "renew HPointN");
		sum->dim = dim1;
	}
	for( i=0; i<dim2-1; i++)
		sum->v[i] = pt1->v[i] + pt2->v[i];
	for( i=dim2-1; i<dim1-1; i++)
		sum->v[i] = pt1->v[i];
	sum->v[dim1-1] = pt1->v[dim1-1];
	sum->dim = dim1;
  } else if (dim1 < dim2) {
  	if(sum == NULL)
		sum = HPtNCreate(dim2,NULL);
	else if(sum->dim < dim2) {
	   	sum->v = OOGLRenewNE(HPtNCoord, sum->v, dim2, "renew HPointN");
		sum->dim = dim2;
	}
	for( i=0; i<dim1-1; i++)
		sum->v[i] = pt1->v[i] + pt2->v[i];
	for( i=dim1-1; i<dim2-1; i++)
		sum->v[i] = pt2->v[i];
	sum->v[dim2-1] = pt1->v[dim1-1];
	sum->dim = dim2;
  }
  return(sum);

}
	
	/* Space */
int 
HPtNSpace( const HPointN *pt );
HPointN *
HPtNSetSpace( HPointN *pt, int space );

	/* Linear combination */
HPointN *
HPtNComb(HPtNCoord u, const HPointN *pu, HPtNCoord v, const HPointN *pv, HPointN *sum)
{
	short dimu = pu->dim, dimv = pv->dim;
	int i;

  if(dimu == dimv) {
  	if(sum == NULL)
		sum = HPtNCreate(dimu,NULL);
	else if(sum->dim < dimu) {
	   	sum->v = OOGLRenewNE(HPtNCoord, sum->v, dimu, "renew HPointN");
		sum->dim = dimu;
	}
	for( i=0; i<dimu-1; i++)
		sum->v[i] = u * pu->v[i] + v * pv->v[i];
	sum->v[dimu-1] = pu->v[dimu-1];
	sum->dim = dimu;
  } else if( dimu > dimv ) {
  	if(sum == NULL)
		sum = HPtNCreate(dimu,NULL);
	else if(sum->dim < dimu) {
	   	sum->v = OOGLRenewNE(HPtNCoord, sum->v, dimu, "renew HPointN");
		sum->dim = dimu;
	}
	for( i=0; i<dimv-1; i++)
		sum->v[i] = u * pu->v[i] + v * pv->v[i];
	for( i=dimv-1; i<dimu-1; i++)
		sum->v[i] = u * pu->v[i];
	sum->v[dimu-1] = pu->v[dimu-1];
	sum->dim = dimu;
  } else if( dimu < dimv ) {
  	if(sum == NULL)
		sum = HPtNCreate(dimv,NULL);
	else if(sum->dim < dimv) {
	   	sum->v = OOGLRenewNE(HPtNCoord, sum->v, dimv, "renew HPointN");
		sum->dim = dimv;
	}
	for( i=0; i<dimu-1; i++)
		sum->v[i] = u * pu->v[i] + v * pv->v[i];
	for( i=dimu-1; i<dimv-1; i++)
		sum->v[i] = v * pv->v[i];
	sum->v[dimv-1] = pu->v[dimu-1];
	sum->dim = dimv;
  }
  return(sum);
}

	/* Reduce to unit vector */
HPointN *
HPtNUnit(const HPointN *from, HPointN *to)
{
	HPtNCoord len = 0;
	short dim = from->dim;
	int i;

	for( i=0; i<dim-1; i++)
		len += from->v[i] * from->v[i];

	if( (len = sqrt( len )) != 0) {
  		if(to == NULL)
			to = HPtNCreate(dim,NULL);
		else if(to->dim != dim) {
		   	to->v = OOGLRenewNE(HPtNCoord, to->v, dim, "renew HPointN");
			to->dim = dim;
		}
		for( i=0; i<dim-1; i++)
			to->v[i] = from->v[i] /len;
		to->v[dim-1] = from->v[dim-1];
	} else
		HPtNCopy(from,to);

	return(to);
}

	/* Dehomogenize */
HPtNCoord
HPtNDehomogenize(const HPointN *from, HPointN *to)
{
	short dim = from->dim;
	HPtNCoord inv = 1.0 / from->v[dim-1];
	int i;

	
	if( from->v[dim-1] == 1.0 || from->v[dim-1] == 0.0) {
		if( from != to) HPtNCopy(from,to);
		return (HPtNCoord)0;
	}


  	if(to == NULL)
		to = HPtNCreate(dim,NULL);
	else if(to->dim != dim) {
	  	to->v = OOGLRenewNE(HPtNCoord, to->v, dim, "renew HPointN");
		to->dim = dim;
	}

	for( i=0; i<dim-1; i++)
		to->v[i] = from->v[i] * inv;
	to->v[dim-1] = 1.0;
	return(1./inv);
}

/* Transform an HPointN according to a 3d transform acting only on the
 * sub-space defined by "axes"
 */
HPointN * HPtNTransform3(Transform3 T, int *perm,
			 const HPointN *from, HPointN *to)
{
  int i, dim = from->dim;
  HPt3Coord from3[4];
  int permute[4];

  to = HPtNCopy(from, to);


  /* Map "-1" in perm[] array to dimension N-1 (homogeneous divisor) */
  for(i = 0; i < 4; i++) {
    permute[i] = (perm[i] >= 0 && perm[i] < dim) ? perm[i] : dim-1;
  }

  for (i = 0; i < 4; i++) {
    from3[i] = from->v[permute[i]];
  }
  
  HPt3Transform(T, (HPoint3 *)from3, (HPoint3 *)from3);

  for (i = 0; i < 4; i++) {
    to->v[permute[i]] = from3[i];
  }
  return to;
}

	/* Apply a TransformN to an HPointN */
HPointN *
HPtNTransform( const TransformN *T, const HPointN *from, HPointN *to )
{
	short idim = T->idim, odim = T->odim, dim = from->dim;
	int i, j;
	HPtNCoord *v = (HPtNCoord *)alloca(idim*sizeof(HPtNCoord));

	for( i=0; i<dim; i++)
		v[i] = from->v[i];
	for (; i<idim; i++) {
		v[i] = 0.0;
	}

  	if(to == NULL)
		to = HPtNCreate(odim,NULL);
	else if(to->dim != odim) {
	  	to->v = OOGLRenewNE(HPtNCoord, to->v, odim, "renew HPointN");
		to->dim = odim;
	}

  if (idim == dim) {
	for( i=0; i<odim; i++) {
		to->v[i] = 0;
		for( j=0; j<idim; j++)
			to->v[i] += v[j] * T->a[j*odim+i];
	}
  } else if (idim > dim) { /* padding with zeroes */
	for( i=0; i<odim; i++) {
		to->v[i] = 0;
		for( j=0; j<dim; j++)
			to->v[i] += v[j] * T->a[j*odim+i];
	}
  } else if (idim < dim) {
	for( i=0; i<odim; i++) {
		to->v[i] = 0;
		for( j=0; j<idim; j++)
			to->v[i] += v[j] * T->a[j*odim+i];
		if( i>idim-1)
			to->v[i] += v[i];
	}
  }

  return(to);
}

	/* Dot product of two vectors */
HPtNCoord 
HPtNDot( const HPointN *p1, const HPointN *p2)
{
	HPtNCoord result;
	int i;
	short dim = p1->dim;

	if (p2->dim < dim) {
		dim = p2->dim;
	}

	result = 0;
	for( i = 0; i< dim-1; i++)
		result += p1->v[i] * p2->v[i];

	return(result);
}

    /* Return index'th component of p . T */
    /* If index is out of range (e.g. -1), return N-1'th component
     * (i.e. homogeneous divisor).
     */
HPtNCoord *
HPtNTransformComponents( const HPointN *p, const TransformN *T, int ncomponents, int *indices, HPtNCoord *results )
{
	short idim = T->idim, odim = T->odim, dim = p->dim;
	int i, j, k;
	HPtNCoord *v = p->v;

	if( ncomponents < 0 || ncomponents > odim ) {
		return NULL;
	}

	if ( idim == dim ) {
 		for ( k = 0; k < ncomponents; k++ ) {
			i = indices[k];
			if( i < 0 || i > odim )
				i = odim-1;
			results[k] = 0;
			for( j=0; j<idim; j++)
				results[k] += v[j] * T->a[j*odim+i];
		}
	} else if ( idim < dim ) {
 		for ( k = 0; k < ncomponents; k++ ) {
			i = indices[k];
			if( i < 0 || i > odim )
				i = odim-1;
			results[k] = v[dim-1] * T->a[(idim-1)*odim + i];
			for( j=0; j<idim-1; j++)
				results[k] += v[j] * T->a[j*odim+i];
			if( i>idim-1)
				results[k] += v[i];
		}
	} else { /* idim > dim */
 		for ( k = 0; k < ncomponents; k++ ) {
			i = indices[k];
			if( i < 0 || i > odim )
				i = odim-1;
			results[k] = v[dim-1] * T->a[(idim-1)*odim + i];
			for( j=0; j<dim-1; j++)
				results[k] += v[j] * T->a[j*odim+i];
		}
	}

  return(results);
}

/* Transform a 4-point to a 3-point according to the mapping defined
 * in "axes"
 */
HPoint3 *
HPtNToHPt3(HPointN *from, HPoint3 *hp3, int *axes)
{
  HPt3Coord *to = (HPt3Coord *)hp3;
  int i, dim = from->dim;

  if (!axes) {
    hp3->x = from->v[0];
    hp3->y = from->v[1];
    hp3->z = from->v[2];
    hp3->w = from->v[dim-1];
  } else {
    for (i = 0; i < 3; i++) {
      if (axes[i] > dim-1) {
	to[i] = 0.0;
      } else if (axes[i] != -1) {
	to[i] = from->v[axes[i]];
      }
    }
    hp3->w = from->v[dim-1];
  }
  return hp3;
}

void HPtNMinMax(HPointN *min, HPointN *max, HPointN *other, int dim)
{
  int i;
  
  for (i = 0; i < dim; i++) {
    if (min->v[i] > other->v[i]) {
      min->v[i] = other->v[i];
    } else if (max->v[i] < other->v[i]) {
      max->v[i] = other->v[i];
    }
  }    
}
