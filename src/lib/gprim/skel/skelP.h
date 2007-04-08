/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips
 * Copyright (C) 2007 Claus-Justus Heine
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

#ifndef SKELPDEF
#define SKELPDEF

#include "geomclass.h"
#include "skel.h"

typedef struct Skline {
  int nv;			/* Number of vertices for this polyline */
  int v0;			/* offset into vi[] of all vertices on line */
  int nc;			/* Number of colors for this polyline */
  int c0;			/* index into c[] of first color */
} Skline;

struct Skel {
  GEOMFIELDS;
  int nvert, nlines;
  float *p;		/* Array[nvert][pdim] of point coords */
  Skline *l;		/* nl[nlines] */
  int nvi;		/* Total number of vertex indices */
  int *vi;		/* All vertex index lists */
  int nc;		/* Total number of colors */
  ColorA *c;		/* Array of per-polyline colors */
  ColorA *vc;           /* Array of per-vertex colors */
};

extern Skel *SkelCreate( Skel *, GeomClass *, va_list *a_list);
extern void  SkelDelete( Skel * );
extern Skel *SkelCopy( Skel * );
extern Skel *SkelPick( Skel *, Pick *, Appearance *,
		       Transform, TransformN *TN, int *axes );
extern Skel *SkelFLoad( IOBFILE *f, char *fname );
extern Skel *SkelFSave( Skel *, FILE * );
extern BBox *SkelBound(Skel *s, Transform T, TransformN *TN);
extern Skel *SkelDraw( Skel * );
extern Skel *SkelTransform( Skel *s, Transform T, TransformN *TN );
extern Geom *SkelBoundSphere(Skel *skel, Transform T, TransformN *TN, int *axes,
			     int space);

#endif /*SKELPDEF*/
