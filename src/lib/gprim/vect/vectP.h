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

#ifndef VECTPDEF
#define VECTPDEF

#include "geomclass.h"
#include "vect.h"

/*
 * A Vect is a collection of polylines.
 * It's represented by:
 *  p,  an array of vertices (for all the polylines in sequence),
 *  c,  an array of r/g/b/alpha colors (not necessarily one per vertex),
 *  vnvert, an array of sizes (vertex counts) of each polyline,
 *  vncolor, an array of numbers of colors (c[] entry counts) of each polyline,
 *  nvec, giving the number of polylines (entries in vnvert and vncolor),
 *  nvert, giving the total number of vertices (entries in p),
 *  ncolor, the total number of colors (entries in c).
 *
 * Here's the association:
 *	Let vN = sum from i = 0 to N-1 of vnvert[i],
 *	and cN = sum from i = 0 to N-1 of vncolor[i],
 *    then polyline N comprises the vnvert[N] vertices
 *	from p[vN] through p[vN + vnvert[N] - 1]
 *    and is drawn using the vncolor[N] different colors
 *	from c[cN] through c[cN + vncolor[N] - 1].
 *
 * This encoding implies colors and vertices may not be reused from line to
 * line (but the previous color persists for lines with 0 colors, see below).
 *
 * Closed polylines:
 *	A polyline is drawn closed (the last element connected to the first)
 *	if its vnvert[N] is negative.  It's then considered to have
 *	abs(vnvert[N]) vertices in the p[] array.
 *
 * A polyline with 1 vertex is a point.  0 vertices are illegal.
 *
 * Coloring:
 *	It's intended that polylines will be specified with either
 *	0, 1, or abs(vnvert[N]) colors.  The effects in each case are:
 *	 0: the polyline is drawn with the same color as the previous polyline.
 *	 1: the polyline is drawn with the specified single color.
 *	 abs(vnvert[N]): each vertex is colored with the specified color.
 *	    intermediate points on the line are interpolated.
 */

struct Vect {
	GEOMFIELDS;
	int	nvec, nvert, ncolor;
	short	*vnvert;	/* vnvert[nvec] (# p[]'s per vec) */
	short	*vncolor;	/* vncolor[nvec] (# c[]'s per vec) */
	    /* NOTE: vnvert and vncolor lie in a *single* malloc'ed area */
	    /* To free, OOGLFree(vnvert). */
	HPoint3	*p;		/* p[nvert] */
	ColorA	*c;		/* c[ncolor] */
};

extern Vect *VectCreate( Vect *, GeomClass *, va_list *a_list);
extern void  VectDelete( Vect * );
extern Vect *VectCopy( Vect * );
extern Vect *VectPick( Vect *, Pick *, Appearance *,
		       Transform, TransformN *, int *axes );

#endif /*VECTPDEF*/
