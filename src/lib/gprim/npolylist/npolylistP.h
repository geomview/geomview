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

#ifndef	NPOLYLISTPDEFS
#define	NPOLYLISTPDEFS
/*
 * Private definitions for PolyList's
 */

#include "bboxP.h"
#include "npolylist.h"

#define NPLMAGIC GeomMagic ('P', 1)

typedef struct NPoly
{
	int	n_vertices;
	int	vi0;		/* Offset of first vertex index in vi[] array */
	ColorA  pcol;		/* polygon color */
}  NPoly;

struct NPolyList
{
	GEOMFIELDS
	int	n_polys;
	int	n_verts;
	int	pdim;		/* Space dimension, *including* homogeneous component */
	NPoly	*p;		/* array of polygon definitions */
	int	*vi;		/* array of vertex indices (all faces) */
	int	nvi;		/* room allocated for vertex indices */
	float	*v;		/* v[n_verts][pdim] */
	ColorA	*vcol;		/* vcol[n_verts] */
	float	*st;		/* st[n_verts][2] texture coords */
	int	flags;
	int	seq;		/* for 4D->3D tforms */
#  define	  PL_HASVCOL	0x4	/* Per-vertex colors (vcol) valid */
#  define	  PL_HASPCOL	0x8	/* Per-polygon colors (pcol) valid */
#  define	  PL_HASST	0x20	/* texture coords */
			/* For 4-D points, see geomflags & VERT_4D */
};


extern NPolyList *NPolyListCreate(NPolyList *exist, GeomClass *Class, va_list *a_list);
extern NPolyList *NPolyListCopy( NPolyList *poly );
extern NPolyList *NPolyListDelete( NPolyList *poly );
extern GeomClass *NPolyListMethods();

#endif/*NPOLYLISTPDEFS*/
