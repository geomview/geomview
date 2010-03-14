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

#ifndef NPOLYLISTPDEFS
#define NPOLYLISTPDEFS
/*
 * Private definitions for PolyList's
 */

#include "geom.h"
#include "bboxP.h"
#include "polylistP.h"
#include "npolylist.h"

#define NPLMAGIC GeomMagic ('P', 1)

struct NPolyList
{
  GEOMFIELDS;
  int       n_polys;
  int       n_verts;
  int       *vi;        /* array of vertex indices (all faces) */
  int       nvi;        /* room allocated for vertex indices */
  int       *pv;        /* start index for each poly into vi */
  HPtNCoord *v;         /* v[n_verts][pdim] */
  ColorA    *vcol;      /* per vertex colors */
  Poly      *p;         /* list of polygons, including projected vertices
			 * when drawing the list.
			 */
  Vertex    *vl;        /* 3d vertex definitions, including texture
			 * coordinates and per vertex colors,
			 * projected vertices and normals when
			 * drawing.
			 */
};

static inline Ref *NPolyListRef(NPolyList *pl)
{
  union castit {
    Ref       ref;
    NPolyList pl;
  };

  return &((union castit *)pl)->ref;
}

static inline Geom *NPolyListGeom(NPolyList *pl)
{
  union castit {
    Geom      geom;
    NPolyList pl;
  };

  return &((union castit *)pl)->geom;
}

#define NPL_HASVLVCOL GEOMFLAG(0x8000) /* pl->vl[i].vcol is uptodate */

extern NPolyList *NPolyListCreate(NPolyList *exist, GeomClass *Class, va_list *a_list);
extern NPolyList *NPolyListCopy( NPolyList *poly );
extern NPolyList *NPolyListDelete( NPolyList *poly );
extern NPolyList *NPolyListTransform(NPolyList *np,
                                     Transform T, TransformN *TN);
extern Geom *NPolyListSphere(NPolyList *npl,
                             Transform T, TransformN *TN, int *axes, int space);
extern GeomClass *NPolyListMethods();

#endif/*NPOLYLISTPDEFS*/

/*
 * Local Variables: ***
 * c-basic-offset: 2 ***
 * End: *
 */

