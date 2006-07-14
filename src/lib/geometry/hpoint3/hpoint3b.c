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

#if HAVE_CONFIG_H
# include "config.h"
#endif

#if 0
static char copyright[] = "Copyright (C) 1992-1998 The Geometry Center\n\
Copyright (C) 1998-2000 Stuart Levy, Tamara Munzner, Mark Phillips";
#endif


/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner, Mark Phillips */

#
/*
**	hpoint3.c - procedural interface to 3D point geometry
**
**	pat hanrahan
*/

#include <math.h>
#include "hg4.h"
#include "hpoint3.h"
#include "hline3.h"
#include "tolerance.h"

HPoint3 *
HPt3Create()
{
    return (HPoint3 *) Hg4Create();
}

void
HPt3Delete( HPoint3 *pt )
{
    Hg4Delete( (Hg4Tensor1 *) pt );
}

void
HPt3Print( HPoint3 *pt )
{
    Hg4Print( (Hg4Tensor1 *) pt );
}

void
HPt3Add( HPoint3 *pt1, HPoint3 *pt2, HPoint3 *pt3 )
{
    Hg4Add( (Hg4Tensor1 *) pt1, (Hg4Tensor1 *) pt2 , (Hg4Tensor1 *) pt3);
}


void
HPt3From( HPoint3 *pt, HPt3Coord x, HPt3Coord y, HPt3Coord z, HPt3Coord w )
{
    Hg4From( (Hg4Tensor1 *)pt, x, y, z, w );
}

int
HPt3From3HPl3s( HPoint3 *pt, HPlane3 *pl1, HPlane3 *pl2, HPlane3 *pl3 )
{
    return Hg4Intersect3( 
	(Hg4Tensor1 *)pl1, (Hg4Tensor1 *)pl2, (Hg4Tensor1 *)pl3,
	(Hg4Tensor1 *)pt, 1 );
}

int
HPt3From2HLn3s( HPoint3 *pt, HLine3 *ln1, HLine3 *ln2 )
{
    HPlane3 pl;

    return HLn3IntersectHLn3( ln1, ln2, &pl, pt );
}

int
HPt3IntersectHPt3( HPoint3 *pt1, HPoint3 *pt2, HLine3 *ln )
{
    return HLn3From2HPt3s( ln, pt1, pt2 );
}

void
HPt3Pencil( HPt3Coord t1, HPoint3 *pt1, HPt3Coord t2, HPoint3 *pt2, HPoint3 *pt3 )
{
    Hg4Pencil( t1, (Hg4Tensor1 *)pt1, t2, (Hg4Tensor2 *)pt2, (Hg4Tensor1 *)pt3);
}

float
HPt3DotHPl3( HPoint3 *pt, HPlane3 *pl )
{
    return Hg4ContractPiQi( (Hg4Tensor1 *)pt, (Hg4Tensor1 *)pl );
}

int
HPt3Compare( HPoint3 *pt1, HPoint3 *pt2 )
{
    return Hg4Compare( (Hg4Tensor1 *)pt1, (Hg4Tensor1 *)pt2 );
}

int
HPt3Undefined( HPoint3 *pt )
{
    return Hg4Undefined( (Hg4Tensor1 *)pt );
}

int
HPt3Infinity( HPoint3 *pt )
{
    return Hg4Infinity( (Hg4Tensor1 *)pt, 0 );
}

int
HPt3CoincidentHPl3( HPoint3 *pt, HPlane3 *pl )
{
    return fzero(HPt3DotHPl3(pt,pl));
}

int
HPt3CoincidentHLn3( HPoint3 *pt, HLine3 *ln )
{
    HPlane3 pl;

    return HLn3IntersectHPt3( ln, pt, &pl );
}

int
HPt3CoincidentHPt3( HPoint3 *pt1, HPoint3 *pt2 )
{
    return Hg4Coincident( (Hg4Tensor1 *)pt1, (Hg4Tensor1 *)pt2 );
}


/* Fishy procedure */
void
HPt3Dual( HPoint3 *pt, HPlane3 *pl )
{
    pl->a = pt->x;
    pl->b = pt->y;
    pl->c = pt->z;
    pl->d = pt->w;
}

void
HPt3LinSum (HPt3Coord scale1, HPoint3 *in1, HPt3Coord scale2, HPoint3 *in2, HPoint3 *out)
{
    if ((in1->w == 0) || (in2->w == 0)) {
	out->w = 0;
	out->x = scale1 * in1->x + scale2 * in2->x;
	out->y = scale1 * in1->y + scale2 * in2->y;
	out->z = scale1 * in1->z + scale2 * in2->z;
	return;
    }
    out->w = 1;
    out->x = scale1 * (in1->x/in1->w) + scale2 * (in2->x/in2->w);
    out->y = scale1 * (in1->y/in1->w) + scale2 * (in2->y/in2->w);
    out->z = scale1 * (in1->z/in1->w) + scale2 * (in2->z/in2->w);
}

void
HPt3SizeOne ( HPoint3 *pt, HPoint3 *out )
{
    HPt3Coord size;

    size = sqrt (pt->x * pt->x + pt->y * pt->y + pt->z * pt->z);
    if (size == 0) return;
    out->x = pt->x / size;
    out->y = pt->y / size;
    out->z = pt->z / size;
    out->w = 1.;
}
