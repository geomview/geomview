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


/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef TRANSFORMNDEF
#define TRANSFORMNDEF

#include "hpointn.h"	/* Defines HPointN and TransformN */
#include "transform.h"
#include <stdio.h>

/*
 * Space & model flags; used to specify what space we're in, and
 * a model for hyperbolic space.  The actual value may be a logical
 * OR of a space and a model value.
 */

	/* Construct a transform.  NULL a => identity */
/*skip*/
extern TransformN *TmNCreate( int idim, int odim, HPtNCoord *a);
	/* Destroy */
/*skip*/
extern void TmNDelete( TransformN *T );

	/* Get and set space */
/*skip*/
extern int TmNSpace( const TransformN *T );
/*skip*/
extern TransformN *TmNSetSpace( TransformN *T, int space );

	/* Invert */
extern TransformN *TmNInvert( const TransformN *T, TransformN *Tinv);

	/* Transpose */
extern TransformN *TmNTranspose( const TransformN *from, TransformN *to);

	/* Multiply transforms */
extern TransformN *TmNConcat( const TransformN *A, const TransformN *B, TransformN *result);

	/* Copy */
extern TransformN *TmNCopy( const TransformN *Tsrc, TransformN *Tdst);

	/* Set to identity */
extern TransformN *TmNIdentity( TransformN *T);

	/* Euclidean translations */
extern TransformN *TmNTranslate( TransformN *T, const HPointN *p);
extern TransformN *TmNTranslateOrigin( TransformN *T, const HPointN *pt);

	/* Translations by the space of 'pt' */
/*skip*/
extern TransformN *TmNSpaceTranslate( TransformN *T, HPointN *pt);
/*skip*/
extern TransformN *TmNSpaceTranslateOrigin( TransformN *T, HPointN *pt);

	/* Scale by the components of 'amount' */
extern TransformN *TmNScale( TransformN *T, const HPointN *amount);

	/* Construct a geodesic rotation taking vector 'from' to 'toward' */
extern TransformN *TmNRotate( TransformN *T, const HPointN *from, const HPointN *toward);

	/* Modify nxn matrix to accomodate usual matrix from Geomview */
/*skip*/
extern TransformN *TmNApplyDN( TransformN *T, int *permute, Transform3 delta);

	/* Add ones and zeros to a matrix to make it larger, with special treatment
	for the last row */
extern TransformN *TmNPad(TransformN *T1, short idim, short odim, TransformN *T2);

	/* Add ones and zeros to a matrix to make it larger  */
extern TransformN *TmNPadSimple(TransformN *T1, short idim, short odim, TransformN *T2);

	/* Add just zeros to a matrix to make it larger */
extern TransformN *TmNPadZero(TransformN *T1, short idim, short odim, TransformN *T2);

	/* Return dimensions of a TransformN.  Value is first dimension. */
	/* idim and/or odim may be NULL, in which case they're not returned */
extern int TmNGetSize(const TransformN *T, int *idim, int *odim);

	/* Print a TransformN */
extern void TmNPrint(FILE *f, const TransformN *T);

	/* Get a TransformN, given a file pointer */
extern TransformN *TmNRead(FILE *f);

	/* scale all elements of "upper left" of matrix by a scalar */
extern TransformN *CtmNScale( HPtNCoord s, TransformN *in, TransformN *out);

# endif
