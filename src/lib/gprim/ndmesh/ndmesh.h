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

#ifndef NDMESHDEF
#define NDMESHDEF

#include "bbox.h"
#include "meshflag.h"

#ifndef NULL
#define NULL 0
#endif

#ifndef	FALSE
#define	FALSE 0
#endif

#define NDMESHMAGIC	GeomMagic ('N', 1)

typedef struct NDMesh NDMesh;


extern GeomClass *NDMeshMethods( void );
extern char	*NDMeshName( void );
extern NDMesh	*NDMeshFLoad( IOBFILE *, char * );

extern NDMesh	*NDMeshLoad( char * );
extern NDMesh	*NDMeshSave( NDMesh *, char * );
extern NDMesh	*NDMeshFSave( NDMesh *, FILE * );
extern BBox	*NDMeshBound(NDMesh *, Transform T, TransformN *TN);
extern Geom     *NDMeshBoundSphere(NDMesh *mesh,
				   Transform T, TransformN *TN, int *axes,
				   int space);
extern NDMesh	*NDMeshDice( NDMesh *, int (*proc)() );
extern NDMesh	*NDMeshDraw( NDMesh * );
extern NDMesh	*NDMeshPick( NDMesh *, Pick *, Appearance *,
			     Transform, TransformN *TN, int *axes );
extern NDMesh *NDMeshTransform(NDMesh *m, Transform T, TransformN *TN);

#endif /*MESHDEF*/
