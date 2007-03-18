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

#ifndef GEOMDEF
#define GEOMDEF


#include "handle.h"
#include "3d.h"
#include "create.h"


#ifndef __cplusplus
typedef struct Geom      Geom;		/* Complete OOGL object */
typedef struct GeomClass GeomClass;	/* Virtual func tbl for Geom */
typedef struct GeomIter  GeomIter;	/* opaque iteration handle */
#else
struct Geom;
struct GeomClass;
struct GeomIter;
#endif

struct BSPTree;

extern char    *GeomName( Geom *obj );
extern GeomClass *GeomMethods( Geom * );
extern Geom    *GeomMessage( Geom *obj, int ac, char *av[] );
extern Geom    *GeomCopy( Geom *obj );
extern Geom    *GeomCreate( char *classname, ... );
extern Geom    *GeomCCreate( Geom *existing_value, GeomClass *Class, ... );
extern void	GeomDelete( Geom *obj );
extern Geom    *GeomLoad( char *name );
extern Geom    *GeomFLoad( IOBFILE *inf, char *fname ); /* name for errs & type-guessing, may be NULL */
extern int	GeomFLoadEmbedded( Geom **obj, Handle **handlep, IOBFILE *inf, char *fname );
extern int	GeomStreamIn( Pool *, Handle **handlep, Geom **geomp );
extern int	GeomStreamOut( Pool *, Handle *h, Geom *g );
extern Geom    *GeomSave( Geom *obj, char *name );
extern Geom    *GeomFSave( Geom *obj, FILE *outf, char *fname );
extern Geom    *GeomFSaveEmbedded( Geom *obj, Handle *handle, FILE *inf, char *fname );
extern void	GeomReplace( Geom *parent, Geom *newchild );
extern Geom    *GeomEvert( Geom *obj );
extern Geom    *GeomBound( Geom *obj, Transform T, TransformN *TN);
extern Geom    *GeomBoundSphere( Geom *obj,
				 Transform T, TransformN *, int *axes,
				 int space );
extern Geom    *GeomEval( Geom *obj );
extern Geom    *GeomDice( Geom *obj, int nu, int nv );
extern Geom    *GeomSubdivide( Geom *obj );
extern Geom    *GeomPosition( Geom *obj, Transform T );
extern Geom    *GeomTransform( Geom *obj, Transform T, TransformN *TN );
extern Geom    *GeomTransformTo( Geom *obj, Transform T, TransformN *TN );

extern int	GeomSet( Geom *g, /* int attr, */ ... /* , CR_END */ );
extern int	GeomGet( Geom *g, int attr, void *attrp );

extern void	GeomHandleScan( Geom *g, int (*func)(), void *arg );

extern Geom    *GeomDraw( Geom *obj );	/* Using current mg context */
extern struct BSPTree *
GeomBSPTree(Geom *geom, struct BSPTree *tree, int action);
extern Geom    *GeomBSPTreeDraw(Geom *geom); /* only for transparency */

	/* Extensions.  These are roughly objective-C style.
 	 * Extension-functions are named with ASCII strings; for efficiency,
	 * the library interface uses integer "selectors".  Typical usage:
	 * int sel = GeomNewMethod( "new_method_name", my_default_func );
	 *   to define a new method, and/or
	 * GeomDefineMethod( sel, PolyListMethods(), my_func_for_polylists );
	 * GeomDefineMethod( sel, InstMethods(), my_func_for_insts );
	 * ...
	 *   to define a new method & its behavior on various object types. Or:
	 * int sel = GeomMethodSel( "method_name" );
	 *   returns the selector of an already-defined extension.
	 *   (GeomMethodSel() returns 0 for undefined extensions).
	 * To use the new method:
	 *   result = GeomCall( sel, geom, ... );
	 */

typedef  void *GeomExtFunc( int sel, Geom *geom, va_list *args );

extern int   GeomMethodSel( char *name );	/* name -> selector, or 0 */
extern char *GeomMethodName( int sel );		/* selector -> name or NULL */
extern int   GeomNewMethod( char *name, GeomExtFunc *defaultfunc );/* -> selector */
GeomExtFunc *GeomSpecifyMethod( int sel, GeomClass *Class, GeomExtFunc *func );

extern void *GeomCall( int sel, Geom *geom, ... );
extern void *GeomCallV( int sel, Geom *geom, va_list *args );

    /* Notes:
     * GeomMethodSel() returns the selector for a given extension; 0 = unknown.
     * GeomMethodName() returns the string naming an extension; NULL = unknown.
     * GeomNewMethod() returns the newly assigned selector.
     * GeomSpecifyMethod() returns the old method function, or NULL if none.
     * GeomCall() invokes a method function; if none was specified for that
     *		object class, the method's default function is called.
     * GeomCallV() takes a va_list rather than GeomCall's explicit param list.
     */

	/* Iteration */
			/* Iterate() flag bits: */
#define SHALLOW	1	/* Shallow traversal */
#define	DEEP	0	/* vs. Fully recursive */
#define	TRANSFORMED 2	/* Fully flatten all Geom's */
#define	INSTANCED 0	/* vs. Return Inst -> Geom's (saves copying data) */
#define	RETAIN	4	/* Retain returned Geom's indefinitely */
#define	DESTROY	0	/* vs. Destroy returned Geom's at next iteration step */

extern GeomIter *_GeomIterate( Geom *root, int flags );
extern Geom	*NextGeom( GeomIter * );
extern int	 _NextTransform( GeomIter *, Transform T );
extern void	 DestroyIter( GeomIter * );

static inline GeomIter *GeomIterate(Geom *root, int flags)
{
  if (!root) {
    return NULL;
  } else {
    return _GeomIterate(root, flags);
  }
}

static inline int NextTransform(GeomIter *iter, Transform T)
{
  if (iter == NULL) {
    return 0;
  }
  return _NextTransform(iter, T);
}
	/* Import/export */
extern int	 GeomUpdate( Geom *obj, int doImport );	/* Recursive bind */

/* to support compatibility with the old way of doing it */
#define GeomScale(ggg, sx, sy, sz)	{	\
	Transform t;				\
	Tm3Scale(t, sz, sy, sz);		\
	GeomTransform(ggg, t); 	}

#define GeomTranslate(ggg, tx, ty, tz)	{	\
	Transform t;				\
	Tm3Translate(t, tz, ty, tz);		\
	GeomTransform(ggg, t); 	}

#define GeomRotate(ggg, aa, axis)	{	\
	Transform t;				\
	Tm3Rotate(t, aa, axis);			\
	GeomTransform(ggg, t); 	}

extern Geom *GeomCCreate(Geom *g, GeomClass *c, ...);
extern int  GeomDecorate(/* Geom *g, int *copyp, int feature, va_list *ap */);
extern void GeomAcceptToken();
extern void GGeomInit(/*Geom *g, GeomClass *Class, int magic, Appearance *ap*/);

extern void GeomKnownClassInit();
extern void GeomAddTranslator(char *prefix, char *cmd);



#endif /*GEOMDEF*/

/* 
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
