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

#ifndef INSTDEFS
#define INSTDEFS

/*
 * Private definitions for Inst's.
 */

#include "bbox.h"
#include "bsptree.h"

enum InstLocation {		/* Coordinate system where inst lives */
	L_NONE = 0,
	L_LOCAL = 1,		/* local coordinates in hierarchy (default) */
	L_GLOBAL = 2,		/* Global (just above Geomview world) */
	L_CAMERA = 3,		/* In 3-D camera space (cam=0,0,0, look=-Z) */
	L_NDC = 4,		/* NDC space: -1 <= X,Y,Z <= 1 */
	L_SCREEN = 5		/* screen space: X,Y in 0..screensize, Z in -1..1 */
};

typedef struct Inst Inst;

extern GeomClass *InstClass;		/* Public read-only; valid only
					 * after first call to InstMethods()
					 */

extern GeomClass *InstMethods( void );

extern Geom *InstImport( Pool *p );
extern int   InstExport( Inst *, Pool * );
extern int   InstGet( Inst *, int attr, void *attrp );
extern Inst *InstLoad( char * );
extern Inst *InstFLoad( FILE *, char *name );
extern Inst *InstSave( Inst *, char * );
extern Inst *InstFSave( Inst *, FILE *, char * );
extern BBox *InstBound( Inst *, Transform T, TransformN *TN);
extern Geom *InstBoundSphere( Inst *,
			      Transform T, TransformN *TN, int *axes, int );
extern Inst *InstPosition( Inst *, Transform T );
extern Inst *InstTransformTo( Inst *, Transform T, TransformN *TN );
extern Inst *InstTransform( Inst *, Transform T, TransformN *TN );
extern Inst *InstEvert( Inst * );
extern Inst *InstDraw( Inst * );
extern Geom *InstReplace( Inst *, Geom * );
extern Inst *InstBSPTree(Inst *inst, BSPTree *tree, int action);

#endif /*INSTDEFS*/
