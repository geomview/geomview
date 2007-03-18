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

#ifndef BEZIERPDEFS
#define BEZIERPDEFS

#include "geomclass.h"
#include "bezier.h"
#include "meshP.h"

struct Bezier {
  GEOMFIELDS;
  int    degree_u, degree_v, dimn;
  int    nu, nv; /* these must match mesh dimensions or remake mesh */
  float  *CtrlPnts;
  TxST   STCoords[4];
  Mesh   *mesh;
  Handle *meshhandle; /* to be able to define call-backs when remeshing */
  ColorA c[4];  /* if flag & BEZ_C */
};

/*
 * Hidden routines, not user-callable
 */
extern Bezier *BezierCreate( Bezier *exist, GeomClass *, va_list *attrlist );
extern void  BezierDelete( Bezier * );
extern Bezier *BezierCopy( Bezier * );

#endif /* ! BEZIERPDEFS */

/*
 * Local Variables: ***
 * mode: c ***
 * c-basic-offset: 2 ***
 * End: ***
 */
