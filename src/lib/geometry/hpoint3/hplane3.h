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

/*
**	hplane3.c - procedural interface to 3D plane geometry
**
**	pat hanrahan
*/

/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef _GV_HPLANE3_H_
#define _GV_HPLANE3_H_

#if HAVE_CONFIG_H
# include "config.h"
#endif

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif

#include <math.h>
#include "tolerance.h"
#include "geomtypes.h"

static inline HPlane3 *HPl3Create(void);
static inline void HPl3Delete(HPlane3 *pl);
static inline void HPl3Print(HPlane3 *pl);
static inline void HPl3Copy(HPlane3 *pl1, HPlane3 *pl2);
static inline void HPl3From(HPlane3 *pl,
			    HPl3Coord a, HPl3Coord b, HPl3Coord c, HPl3Coord d);
static inline int HPl3From3HPt3s(HPlane3 *pl,
				 HPoint3 *pt1, HPoint3 *pt2, HPoint3 *pt3);
static inline int HPl3From2HLn3s(HPlane3 *pl, HLine3 *ln1, HLine3 *ln2);
static inline int HPl3IntersectHPl3(HPlane3 *pl1, HPlane3 *pl2, HLine3 *ln);
static inline void HPl3Pencil( HPl3Coord t1, HPlane3 *pl1,
			       HPl3Coord t2, HPlane3 *pl2, HPlane3 *pl3 );
static inline HPt3Coord HPl3DotHPt3(HPlane3 *pl, HPoint3 *pt);
static inline int HPl3Undefined(HPlane3 *pl);
static inline int HPl3Infinity(HPlane3 *pl);
static inline int HPl3Compare(HPlane3 *pl1, HPlane3 *pl2);
static inline int HPl3CoincidentHPt3(HPlane3 *pl, HPoint3 *pt);
static inline int HPl3CoincidentHLn3(HPlane3 *pl, HLine3 *ln);
static inline int HPl3CoincidentHPl3( HPlane3 *pl1, HPlane3 *pl2 );
static inline void HPl3Transform(Transform3 T, HPlane3 *pl1, HPlane3 *pl2);
static inline void HPl3TransformN(Transform3 T,
				  HPlane3 *pl1, HPlane3 *pl2, int n );
static inline void HPl3Dual(HPlane3 *pl, HPoint3 *pt);
static inline void HPl3Perp(HPlane3 *pl, HPoint3 *pt);

#include "hg4.h"
#include "hline3.h"
#include "transform3.h"

static const HPlane3 HPl3Ideal = { 0., 0., 0., 1. };

static inline HPlane3 *
HPl3Create(void)
{
    return (HPlane3 *) Hg4Create();
}

static inline void
HPl3Delete( HPlane3 *pl )
{
    Hg4Delete( (Hg4Tensor1Ptr)pl );
}

static inline void
HPl3Print( HPlane3 *pl )
{
    Hg4Print( (Hg4Tensor1Ptr)pl );
}

static inline void
HPl3Copy( HPlane3 *pl1, HPlane3 *pl2 )
{
    Hg4Copy( (Hg4Tensor1Ptr)pl1, (Hg4Tensor1Ptr)pl2 );
}

static inline void
HPl3From( HPlane3 *pl, HPl3Coord a, HPl3Coord b, HPl3Coord c, HPl3Coord d )
{
    Hg4From( (Hg4Tensor1Ptr)pl, a, b, c, d );
}

static inline int
HPl3From3HPt3s( HPlane3 *pl, HPoint3 *pt1, HPoint3 *pt2, HPoint3 *pt3 )
{
    return Hg4Intersect3( 
	(Hg4Tensor1Ptr)pt1, (Hg4Tensor1Ptr)pt2, (Hg4Tensor1Ptr)pt3,
	(Hg4Tensor1Ptr)pl, 1 );
}

static inline int
HPl3From2HLn3s( HPlane3 *pl, HLine3 *ln1, HLine3 *ln2 )
{
    HPoint3 pt;

    return HLn3IntersectHLn3( ln1, ln2, pl, &pt );
}

static inline int
HPl3IntersectHPl3( HPlane3 *pl1, HPlane3 *pl2, HLine3 *ln )
{
    return HLn3From2HPl3s( ln, pl1, pl2 );
}

static inline void
HPl3Pencil( HPl3Coord t1, HPlane3 *pl1, HPl3Coord t2, HPlane3 *pl2, HPlane3 *pl3 )
{
    Hg4Pencil( t1, (Hg4Tensor1Ptr)pl1, t2, (Hg4Tensor1Ptr)pl2, 
	       (Hg4Tensor1Ptr)pl3 );
}

static inline HPt3Coord
HPl3DotHPt3( HPlane3 *pl, HPoint3 *pt )
{
    return Hg4ContractPiQi( (Hg4Tensor1Ptr)pl, (Hg4Tensor1Ptr)pt );
}

static inline int
HPl3Undefined( HPlane3 *pl )
{
    return Hg4Undefined( (Hg4Tensor1Ptr)pl );
}

static inline int
HPl3Infinity( HPlane3 *pl )
{
    return Hg4Infinity( (Hg4Tensor1Ptr) pl, 1 );
}

static inline int
HPl3Compare( HPlane3 *pl1, HPlane3 *pl2 )
{
    return Hg4Compare( (Hg4Tensor1Ptr)pl1, (Hg4Tensor1Ptr)pl2 );
}

static inline int
HPl3CoincidentHPt3( HPlane3 *pl, HPoint3 *pt )
{
    return fzero(HPl3DotHPt3(pl,pt));
}

static inline int
HPl3CoincidentHLn3( HPlane3 *pl, HLine3 *ln )
{
    HPoint3 pt;

    return HLn3IntersectHPl3( ln, pl, &pt );
}

static inline int
HPl3CoincidentHPl3( HPlane3 *pl1, HPlane3 *pl2 )
{
    return Hg4Coincident( (Hg4Tensor1Ptr)pl1, (Hg4Tensor1Ptr)pl2 );
}

static inline void
HPl3Transform( Transform3 T, HPlane3 *pl1, HPlane3 *pl2 )
{
    Hg4Transform( T, (Hg4Tensor1Ptr)pl1, (Hg4Tensor1Ptr)pl2 );
}

static inline void
HPl3TransformN( Transform3 T, HPlane3 *pl1, HPlane3 *pl2, int n )
{
    while( n-- )
	Hg4Transform( T, (Hg4Tensor1Ptr)pl1++, (Hg4Tensor1Ptr)pl2++ );
}

static inline void
HPl3Dual( HPlane3 *pl, HPoint3 *pt )
{
    pt->x = pl->a;
    pt->y = pl->b;
    pt->z = pl->c;
    pt->w = pl->d;
}

static inline void
HPl3Perp( HPlane3 *pl, HPoint3 *pt )
{
/*
    HLine3 ln, lndual;
    HQuadric Q;

    HLn3From2HPl3s( &ln, pl, &HPl3Ideal );
    HLn3CorrelateHLn3( &ln, Q, &lndual );
    HLn3Perp( &lndual, pt ):
*/
    /* if HPl3Ideal == (0,0,0,1) */
    pt->x = pl->a;
    pt->y = pl->b;
    pt->z = pl->c;
    pt->w = 0;
}

#endif /* _GV_HPLANE3_H_ */
