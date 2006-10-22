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

#ifndef TLISTDEFS
#define TLISTDEFS
/*
 * Public definitions for a Tlist
 */

#include "tlist.h"
#include "bboxP.h"

#define TLISTMAGIC	 GeomMagic ('t', 1)

typedef struct Tlist Tlist;

extern GeomClass *TlistClass;	/* Our class pointer */

extern GeomClass *TlistMethods( void );

extern Geom  *TlistFLoad( FILE *, char *fname );
extern Tlist *TlistFSave( Tlist *, FILE *, char *fname );
extern Tlist *TlistTransform( Tlist *, Transform, TransformN * );
extern Tlist *TlistPosition( Tlist *, Transform );
extern Geom *TlistReplace( Tlist *tlist, Geom *newtlist );
extern int    TlistGet( Tlist *tlist, int attr, void *attrp );
extern Geom *TlistImport( Pool * );
extern int    TlistExport( Tlist *, Pool * );


#endif /*TLISTDEFS*/
