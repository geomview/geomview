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

#ifndef BEZIERDEFS
#define BEZIERDEFS

#include "mesh.h"
#include "list.h"
#include "bsptree.h"

#define BEZIERMAGIC	GeomMagic ('b', 1)

#define BEZ_ST 		VERT_ST
#define	BEZ_C		VERT_C
#define BEZ_4D		VERT_4D
#define BEZ_BINARY 	GEOMFLAG(0x01)
#define BEZ_REMESH 	GEOMFLAG(0x02) /* need to recompute mesh */

#define MAX_BEZ_DEGREE	12
#define MAX_BEZ_DIMN	4
#define BEZ_DEFAULT_MESH_SIZE	10

typedef struct Bezier Bezier;

GeomClass	*BezierMethods(), *BezierListMethods();
Mesh 		*BezierReDice( Bezier * );
Bezier		*BezierDice( Bezier *, int nu, int nv );
char           	*BezierListName();
List 		*BezierListLoad();
List		*BezierListFLoad();
List 		*BezierListSave();
List		*BezierListFSave();
char           	*BezierName();
BBox           	*BezierBound();
Geom 		*BezierBoundSphere( Bezier *,
				    Transform T, TransformN *TN, int *axes,
				    int );
Bezier      	*BezierEvert();
Bezier      	*BezierDraw( Bezier *bezier );
Bezier      	*BezierBSPTree(Bezier *bezier, BSPTree *tree, int action);
Geom      	*BezierPick( Bezier *, Pick *, Appearance *, Transform, TransformN *, int *axes );
Bezier      	*BezierTransform( Bezier *, Transform, TransformN * );

int		 BezierExport();
int		 BezierUnexport();
Bezier		*BezierImport();

#endif /* ! BEZIERDEFS */
