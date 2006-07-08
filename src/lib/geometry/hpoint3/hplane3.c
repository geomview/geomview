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

#if defined(HAVE_CONFIG_H) && !defined(CONFIG_H_INCLUDED)
#include "config.h"
#endif

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif


/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner, Mark Phillips */

#
/*
**	hplane3.c - procedural interface to 3D plane geometry
**
**	pat hanrahan
*/

#include <math.h>
#include "hg4.h"
#include "hpoint3.h"
#include "hplane3.h"
#include "hline3.h"
#include "transform3.h"
#include "tolerance.h"

HPlane3 HPl3Ideal = { 0., 0., 0., 1. };

HPlane3 *
HPl3Create()
{
    return (HPlane3 *) Hg4Create();
}

void
HPl3Delete( HPlane3 *pl )
{
    Hg4Delete( (Hg4Tensor1 *)pl );
}

void
HPl3Print( HPlane3 *pl )
{
    Hg4Print( (Hg4Tensor1 *)pl );
}

void
HPl3Copy( HPlane3 *pl1, HPlane3 *pl2 )
{
    Hg4Copy( (Hg4Tensor1 *)pl1, (Hg4Tensor1 *)pl2 );
}

void
HPl3From( HPlane3 *pl, HPl3Coord a, HPl3Coord b, HPl3Coord c, HPl3Coord d )
{
    Hg4From( (Hg4Tensor1 *)pl, a, b, c, d );
}

int
HPl3From3HPt3s( HPlane3 *pl, HPoint3 *pt1, HPoint3 *pt2, HPoint3 *pt3 )
{
    return Hg4Intersect3( 
	(Hg4Tensor1 *)pt1, (Hg4Tensor1 *)pt2, (Hg4Tensor1 *)pt3,
	(Hg4Tensor1 *)pl, 1 );
}

int
HPl3From2HLn3s( HPlane3 *pl, HLine3 *ln1, HLine3 *ln2 )
{
    HPoint3 pt;

    return HLn3IntersectHLn3( ln1, ln2, pl, &pt );
}

int
HPl3IntersectHPl3( HPlane3 *pl1, HPlane3 *pl2, HLine3 *ln )
{
    return HLn3From2HPl3s( ln, pl1, pl2 );
}

void
HPl3Pencil( HPl3Coord t1, HPlane3 *pl1, HPl3Coord t2, HPlane3 *pl2, HPlane3 *pl3 )
{
    Hg4Pencil( t1, (Hg4Tensor1 *)pl1, t2, (Hg4Tensor2 *)pl2, 
	(Hg4Tensor1 *)pl3 );
}

float
HPl3DotHPt3( HPlane3 *pl, HPoint3 *pt )
{
    return Hg4ContractPiQi( (Hg4Tensor1 *)pl, (Hg4Tensor1 *)pt );
}

int
HPl3Undefined( HPlane3 *pl )
{
    return Hg4Undefined( (Hg4Tensor1 *)pl );
}

int
HPl3Infinity( HPlane3 *pl )
{
    return Hg4Infinity( (Hg4Tensor1 *) pl, 1 );
}

int
HPl3Compare( HPlane3 *pl1, HPlane3 *pl2 )
{
    return Hg4Compare( (Hg4Tensor1 *)pl1, (Hg4Tensor1 *)pl2 );
}

int
HPl3CoincidentHPt3( HPlane3 *pl, HPoint3 *pt )
{
    return fzero(HPl3DotHPt3(pl,pt));
}

int
HPl3CoincidentHLn3( HPlane3 *pl, HLine3 *ln )
{
    HPoint3 pt;

    return HLn3IntersectHPl3( ln, pl, &pt );
}

int
HPl3CoincidentHPl3( HPlane3 *pl1, HPlane3 *pl2 )
{
    return Hg4Coincident( (Hg4Tensor1 *)pl1, (Hg4Tensor1 *)pl2 );
}

void
HPl3Transform( Transform3 T, HPlane3 *pl1, HPlane3 *pl2 )
{
    Hg4Transform( T, (Hg4Tensor1 *)pl1, (Hg4Tensor1 *)pl2 );
}

void
HPl3TransformN( Transform3 T, HPlane3 *pl1, HPlane3 *pl2, int n )
{
    while( n-- )
	Hg4Transform( T, (Hg4Tensor1 *)pl1++, (Hg4Tensor1 *)pl2++ );
}

void
HPl3Dual( HPlane3 *pl, HPoint3 *pt )
{
    pt->x = pl->a;
    pt->y = pl->b;
    pt->z = pl->c;
    pt->w = pl->d;
}

void
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
