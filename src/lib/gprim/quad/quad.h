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

#ifndef QUADDEFS
#define QUADDEFS

#include "bbox.h"
#include "bsptree.h"

#define QUADMAGIC GeomMagic ('q', 1)

typedef struct Quad Quad;

extern GeomClass *QuadMethods( void );
extern char *QuadName( void );
extern Quad *QuadLoad( char * );
extern Quad *QuadFLoad( IOBFILE *, char *fname );
extern Quad *QuadSave( Quad *, char * );
extern Quad *QuadFSave( Quad *, FILE *, char *fname );
extern BBox *QuadBound(Quad *q, Transform T, TransformN *TN);
extern Geom *QuadSphere(Quad *q,
			Transform T, TransformN *TN, int *axes,
			int space);
extern Quad *QuadEvert( Quad * );
extern Quad *QuadDraw( Quad * );
extern Quad *QuadBSPTree(Quad *quad, BSPTree *bsptree, int action);
extern Quad *QuadTransform( Quad *, Transform );
extern Quad *QuadTransformTo( Quad *, Transform );

#endif

