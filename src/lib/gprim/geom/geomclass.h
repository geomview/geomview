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

#ifndef GEOMCLASSDEF
#define GEOMCLASSDEF

#undef	GEOM_SM

#include "geom.h"
#include "appearance.h"
#include "ooglutil.h"
#include "pick.h"

typedef char   *GeomNameFunc( void );
typedef GeomClass   *GeomMethodsFunc( Geom *object );

typedef Geom *GeomMessageFunc( Geom *, int ac, char *av[] );

typedef Geom *GeomCopyFunc( Geom *object );
typedef Geom *GeomCreateFunc( Geom *existing, GeomClass *Class, va_list *args );
typedef int   GeomGetFunc( Geom *g, int attribute, void *attrp );
typedef int   GeomDeleteFunc( Geom *object );
typedef Geom *GeomReplaceFunc( Geom *object, Geom *newchild );
typedef Geom *GeomIterateFunc( GeomIter *iter, int op );  /* What should the interface be? */
typedef Geom *GeomLoadFunc( char *name );
typedef Geom *GeomFLoadFunc( IOBFILE *file, char *name );
typedef Geom *GeomSaveFunc( Geom *object, char *name );
typedef Geom *GeomFSaveFunc( Geom *object, FILE *file, char *name );

typedef Geom *GeomPositionFunc( Geom *object, Transform T );
typedef Geom *GeomTransformFunc( Geom *object, Transform T, TransformN *TN );
typedef Geom *GeomTransformToFunc( Geom *object, Transform T, TransformN *TN );
typedef Geom *GeomRotateFunc( Geom *object, float angle, Point3 *axis  );
typedef Geom *GeomTranslateFunc( Geom *object, float x, float y, float z );
typedef Geom *GeomScaleFunc( Geom *object, float x, float y, float z );

typedef Geom *GeomEvertFunc( Geom *object );
typedef Geom *GeomBoundFunc( Geom *object, Transform T, TransformN *TN );
typedef Geom *GeomBoundSphereFunc( Geom *object, Transform T,
				   TransformN *TN, int * axes, int space );
typedef Geom *GeomEvalFunc(  Geom *object, float u, float v );
typedef Geom *GeomDiceFunc( Geom *object, int nu, int nv );
typedef Geom *GeomSubdivideFunc( /* Geom *object */ );

typedef Geom *GeomPickFunc( /* Geom *, Pick *, Appearance *, Transform */ );

typedef Geom *GeomFacingFunc( /* Geom *object, ... */ );
typedef Geom *GeomDrawFunc( /* Geom *object */ );

typedef	int GeomExportFunc( Geom *object, Pool *p );
typedef Geom *GeomImportFunc( Pool *p );
typedef	Geom *GeomUnexportFunc( Handle *h );
typedef Geom *GeomScanFunc( Geom *g, int (*func)(/*Geom*,Handle**,void *arg*/), void *arg );
typedef Geom *GeomAppendFunc( Geom *g, Handle *h, Geom *newitem );

struct GeomClass
{

	/* General Methods */
	GeomClass	*super;		/* superclass of this class */
	GeomNameFunc	*name;
	GeomMethodsFunc *methods;
	GeomMessageFunc *message;

	GeomGetFunc	*get;
	GeomCreateFunc	*create;
	GeomDeleteFunc	*Delete;
	GeomCopyFunc	*copy;
	GeomReplaceFunc *replace;
	GeomExtFunc	**extensions;	/* Extension methods live here */
	GeomFLoadFunc	*fload;
	int		n_extensions;	/* Size of extensions[] array */
	GeomFSaveFunc	*fsave;

	/* Geometric Methods */
	GeomPositionFunc *position;
	GeomTransformFunc *transform;
	GeomTransformToFunc *transformto;

	GeomEvertFunc  *evert;
	GeomBoundFunc  *bound;
	GeomEvalFunc   *eval;
	GeomDiceFunc   *dice;
	GeomSubdivideFunc *subdivide;

	/* Picking methods */
	GeomPickFunc	*pick;
	GeomBoundSphereFunc *boundsphere;
	GeomIterateFunc	*iterate;
	GeomAppendFunc	*append;	/* Append new item to hierarchy object */
	GeomScanFunc	*scan;

	/* Graphics Methods */
	GeomFacingFunc	*facing;
	GeomDrawFunc	*draw;

	/* Communications methods */
	GeomExportFunc	*export;
	GeomImportFunc	*import;
	GeomUnexportFunc *unexport;
};

extern GeomClass *GeomClassCreate();
extern GeomClass *GeomSubClassCreate();
extern GeomClass *GeomClassLookup(char *classname);
extern void	 *GeomClassIterate();
extern GeomClass *GeomNextClass( void *iter );
extern GeomClass *GeomFName2Class( char *fname );

extern Geom *GeomBoundSphereFromBBox(Geom *, Transform, TransformN *, int *axes,
				     int space);

/* internal routine used by file-reading routines */
extern char *GeomToken(IOBFILE *f);

/* This is the "common" geom stuff which starts every geom */
#define GEOMFIELDS					\
    REFERENCEFIELDS	/* magic, ref_count, handle */	\
    struct GeomClass	*Class;				\
    struct Appearance	*ap;				\
    Handle		*aphandle;			\
    int			geomflags;			\
    int pdim /* does this belong here? */

struct Geom {	/* common data structures for all Geom's */
    GEOMFIELDS;
} ;

typedef struct HGeom {	/* This tuple appears in hierarchy objects */
    Handle *h;
    Geom *g;
} HGeom;

	/* 32-bit magic numbers for OOGL data types */
#define	GeomMagic(key, ver)	OOGLMagic('g', ((key) << 8) | ((ver)&0xff))
#define GeomIsMagic(magic)	(((magic) & 0xffff0000) == GeomMagic(0,0))

#endif /*GEOMCLASSDEF*/
