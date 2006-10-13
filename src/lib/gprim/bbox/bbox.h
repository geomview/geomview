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


/* Authors: Charlie Gunn, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef	BBOXDEF
#define BBOXDEF

#include "geom.h"

#define BBOXMAGIC GeomMagic ('x', 1)

typedef struct BBox BBox;

extern GeomClass *BBoxMethods( void );
extern char	*BBoxName( void );
extern BBox	*BBoxBound( BBox *, Transform T, TransformN *TN);
extern BBox	*BBoxDraw( BBox * );
extern BBox	*BBoxTransform( BBox *, Transform T, TransformN *TN );
extern BBox	*BBoxMinMax( BBox *, HPoint3 *min, HPoint3 *max );
extern BBox	*BBoxMinMaxND( BBox *, HPointN **min, HPointN **max );
extern void     BBoxCenter(BBox *bbox, HPoint3 *center);
extern HPointN  *BBoxCenterND(BBox *bbox, HPointN *center);
extern BBox	*BBoxUnion( BBox *, BBox * );
extern BBox	*BBoxUnion3( BBox *b1, BBox *b2, BBox *result );

#ifdef GEOM_SM
extern int	 BBoxShare( Geom *obj, GeomSMRegion *sm );
extern BBox	*BBoxPrivate();
#endif

#endif
