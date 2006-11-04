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


/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef HPOINTNDEF
#define HPOINTNDEF

#include "reference.h"
#include "hpoint3.h"

typedef float HPtNCoord;
typedef struct HPtN {
	int dim;	/* Dimension, including homogeneous divisor */
	int flags;	/* Space tag */
	HPtNCoord *v;	/* Array of coordinates; v[dim-1] is the homogenous divisor */
} HPointN;


/* N-dimensional transformation matrix.
 * Transforms row vectors (multiplied on the left) of dimension 'idim',
 * yielding row vectors of dimension 'odim'.
 */
typedef struct TmN {
	REFERENCEFIELDS
	int idim, odim;
	int flags;
	HPtNCoord *a;	/* Array of idim rows, odim columns */
} TransformN;

#define	TMNMAGIC	OOGLMagic('T', 1)

/* Refer to the (i,j)'th entry in a TransformN object;
 * beware that (i,j) are not checked!
 */
#define  TmNentry(T, i,j)  ((T)->a[(i)*(T)->odim + (j)])


/* Construct point */
/*skip*/
extern HPointN *HPtNCreate(int dim, const HPtNCoord *vec);
/* Destroy point */
/*skip*/
extern void HPtNDelete(HPointN *pt);
/* Copy point */
extern HPointN *HPtNCopy(const HPointN *pt1, HPointN *pt2);
/* Sum of points */
extern HPointN *HPtNAdd(const HPointN *pt1, const HPointN *pt2, HPointN *sum);
/* Space */
/*skip*/
extern int HPtNSpace( const HPointN *pt );
/*skip*/
extern HPointN *HPtNSetSpace( HPointN *pt, int space );

/* Linear combination */
extern HPointN *HPtNComb(HPtNCoord u, const HPointN *pu, HPtNCoord v, const HPointN *pv, HPointN *sum);

/* Reduce to unit vector */
extern HPointN *HPtNUnit(const HPointN *from, HPointN *to);

/* Dehomogenize, returning old 'w' component */
extern HPtNCoord HPtNDehomogenize(const HPointN *from, HPointN *to);

/* Apply a TransformN to an HPointN */
extern HPointN *
HPtNTransform( const TransformN *T, const HPointN *from, HPointN *to );

/* Apply the transpose of a TransformN to an HPointN */
extern HPointN *
HPtNTTransform(const TransformN *T, const HPointN *from, HPointN *to);

extern HPointN * HPtNTransform3(Transform3 T, int *axes,
				const HPointN *from, HPointN *to);

/* Add zeros to a vector to make it a given dimension */
extern HPointN *HPtNPad(const HPointN *pt1, int dim2, HPointN *pt2);

/* Dot product of two vectors */
extern HPtNCoord HPtNDot( const HPointN *p1, const HPointN *p2);

/* Return the projection of (p . T) on the axis-aligned subspace
 * given by indices[0 .. ncomponents-1].
 * If index is out of range (e.g. -1), return N-1'th component
 * (i.e. homogeneous divisor).  So a typical indices[] might read
 *   [0 1 2 -1]
 */
extern HPoint3 *HPtNTransformComponents(const TransformN *T,
					const HPointN *from,
					int *perm,
					HPoint3 *results);
extern HPointN *
HPt3NTransform(const TransformN *T, const HPoint3 *from, HPointN *to);
extern HPointN *
Pt4NTransform(const TransformN *T, const HPoint3 *from, HPointN *to);

extern HPointN *Pt4ToHPtN(const HPoint3 *v4, HPointN *vN);
extern HPointN *Pt3ToHPtN(Point3 *v3, HPointN *vN);
extern HPointN *HPt3ToHPtN(const HPoint3 *v4, int *perm, HPointN *vN);
extern HPoint3 *HPtNToHPt3(const HPointN *from, int *axes, HPoint3 *hp3);
extern HPt3Coord HPt3NTransPt3(TransformN *TN, int *axes,
			       const HPoint3 *hpt4, int v4d, Point3 *result);
extern HPt3Coord HPtNNTransPt3(TransformN *TN, int *axes,
			       const HPointN *ptN, Point3 *result);
extern void HPt3NTransHPt3(TransformN *TN, int *axes,
			   const HPoint3 *hpt4, int v4d, HPoint3 *result);
extern void HPtNMinMax(HPointN *min, HPointN *max, HPointN *other);

#endif
