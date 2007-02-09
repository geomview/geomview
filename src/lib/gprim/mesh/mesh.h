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

#ifndef MESHDEF
#define MESHDEF

#include "bbox.h"
#include "meshflag.h"
#include "bsptree.h"

#ifndef NULL
#define NULL 0
#endif

#ifndef	FALSE
#define	FALSE 0
#endif

#define MESHMAGIC	GeomMagic ('m', 1)

typedef struct Mesh Mesh;


extern GeomClass *MeshMethods( void );
extern char	*MeshName( void );
extern Mesh	*MeshFLoad( IOBFILE *, char * );
extern Mesh	*MeshLoad( char * );
extern Mesh	*MeshSave( Mesh *, char * );
extern Mesh	*MeshFSave( Mesh *, FILE * );
extern BBox	*MeshBound( Mesh *, Transform T, TransformN *TN);
extern Geom	*MeshBoundSphere( Mesh *,
				  Transform T, TransformN *TN, int *axes, int );
extern Mesh	*MeshDice( Mesh *, int (*proc)() );
extern Mesh	*MeshEvert( Mesh * );
extern Mesh	*MeshDraw( Mesh * );
extern Mesh     *MeshBSPTree(Mesh *mesh, BSPTree *bsptree, int action);
extern Mesh	*MeshPick( Mesh *, Pick *, Appearance *,
			   Transform, TransformN *, int *axes );
extern Mesh     *MeshTransform( Mesh *, Transform, TransformN * );

#endif /*MESHDEF*/
