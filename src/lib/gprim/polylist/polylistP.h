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

#ifndef	POLYLISTPDEFS
#define	POLYLISTPDEFS
/*
 * Private definitions for PolyList's
 */

#include "bboxP.h"
#include "polylist.h"

#define PLMAGIC GeomMagic ('p', 1)

typedef struct Vertex
{
	HPoint3	pt;
	ColorA	vcol;
	Point3	vn;
	float st[2];
}  Vertex;

typedef struct Poly
{
	int	n_vertices;
	Vertex	**v;
	ColorA  pcol;
	Point3	pn;
}  Poly;

struct PolyList
{
	GEOMFIELDS;
	int	n_polys;
	int	n_verts;
	Poly	*p;
	Vertex	*vl;
	int	flags;
#  define	  PL_HASVN	0x1	/* Per-vertex normals (vn) valid */
#  define	  PL_HASPN	0x2	/* Per-polygon normals (pn) valid */
#  define	  PL_HASVCOL	0x4	/* Per-vertex colors (vcol) valid */
#  define	  PL_HASPCOL	0x8	/* Per-polygon colors (pcol) valid */
#  define	  PL_EVNORM	0x10	/* Normals are everted */
#  define	  PL_HASST	0x20	/* Has s,t texture coords */
			/* For 4-D points, see geomflags & VERT_4D */
};


extern PolyList *PolyListCreate(PolyList *exist, GeomClass *Class, va_list *a_list);
extern PolyList *PolyListCopy( PolyList *poly );
extern PolyList *PolyListDelete( PolyList *poly );
extern PolyList *PolyListPick( PolyList *, Pick *, Appearance *, Transform );
extern GeomClass *PolyListMethods();
extern PolyList *PolyListComputeNormals( PolyList *polyList);

#endif/*POLYLISTPDEFS*/
