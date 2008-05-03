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

#ifndef TLISTPDEFS
#define TLISTPDEFS

#include "tlist.h"
#include "bboxP.h"

#define TLISTMAGIC	 GeomMagic ('t', 1)

/*
 * A Transformation List is a collection of 4x4 transformation matrices.
 * May take as argument another Tlist (or List of TList's);
 * if so, we act as the outer product: all our elements multiplied by
 *  all those in the given tlist.
 */

struct Tlist {
    GEOMFIELDS;
    int   	nelements;
    int         nallocated;
    Transform 	*elements;
    Geom 	*tlist;		/* More transformations: take product */
    Handle	*tlisthandle;
};

extern char *TlistName( void );
extern Tlist *TlistCreate( Tlist *existing, GeomClass *Class, va_list *args );
extern Tlist *TlistCopy( Tlist *t);
extern void TlistDelete( Tlist * );
extern Geom *TlistImport( Pool *p );
extern int  TlistExport( Tlist *tl, Pool *p );

#endif /*TLISTPDEFS*/
