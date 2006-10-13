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

#ifndef LISTDEF
#define LISTDEF

#include "bbox.h"

#define LISTMAGIC	GeomMagic ('l', 1)

typedef struct List List;

extern Geom *ListRemove( Geom *list, Geom *car_of_member );
extern Geom *ListAppend( Geom *list, Geom *new_member_car );

extern GeomClass *ListClass;
extern GeomClass *ListMethods( void );
extern char *ListName( void );
extern List *ListLoad( char *fname );
extern List *ListFLoad( FILE *f, char *fname );
extern BBox *ListBound( List *, Transform, TransformN *TN);
extern Geom *ListBoundSphere( List *,
			      Transform, TransformN *TN, int *axes, int );
extern List *ListTransform( List *, Transform, TransformN* );
extern List *ListRotate( List *, float, Point * );
extern List *ListTranslate( List *, float, float, float );
extern List *ListScale( List *, float, float, float );
extern List *ListEvert( List * );
extern Geom *ListReplace( List *, Geom * );
extern List *ListDraw( List * );
extern List *ListSave( List *, char * );
extern List *ListFSave( List *, FILE *file, char *fname );

extern int  ListGet( List *, int attr, void *attrp );
extern List *ListImport( Pool * );

#endif /*LISTDEF*/
