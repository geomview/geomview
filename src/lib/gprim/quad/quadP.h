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

#ifndef QUADPDEFS
#define QUADPDEFS

#include "geomclass.h"
#include "quad.h"
#include "bsptree.h"

#define QUAD_N      VERT_N
#define QUAD_C      VERT_C
#define QUAD_BINARY GEOMFLAG(0x01)

typedef HPoint3 QuadP[4];
typedef Point3 QuadN[4];
typedef ColorA QuadC[4];

struct Quad {
  GEOMFIELDS;
  int     maxquad;
  QuadP   *p;
  QuadN   *n;
  QuadC   *c;
};

Quad *QuadPick(Quad *, Pick *, Appearance *,
	       Transform, TransformN *, int *axes);
Quad *QuadComputeNormals(Quad *q);

static inline Ref *QuadRef(Quad *q)
{
  union castit {
    Ref  ref;
    Quad quad;
  };

  return &((union castit *)q)->ref;
}

static inline Geom *QuadGeom(Quad *q)
{
  union castit {
    Geom geom;
    Quad quad;
  };

  return &((union castit *)q)->geom;
}

#endif /* ! QUADPDEFS */

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: ***
 */
