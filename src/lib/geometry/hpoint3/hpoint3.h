/* Copyright (C) 1992-1998 The Geometry Center
 * Copyright (C) 1998-2000 Geometry Technologies, Inc.
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


/* Authors: Charlie Gunn, Pat Hanrahan, Stuart Levy, Tamara Munzner, Mark Phillips */

#ifndef HPOINT3DEF
#define HPOINT3DEF


#include "point3.h"

typedef float HPt3Coord;
typedef struct { HPt3Coord x, y, z, w; } HPoint3;

/* These really belong in transform3/transform3.h, but some of our
 * functions take Transform3 arguments, and this makes things much easier.
 */
typedef float Tm3Coord;
typedef Tm3Coord Transform3[4][4];

/* Likewise, these belong in hplane3.h and hline3.h, but we need to
 * centralize them.
 */
typedef float HPl3Coord;
typedef struct { HPl3Coord a, b, c, d; } HPlane3;

typedef float HLn3Coord;
typedef struct { 
    float L[4][4];
    int type;
} HLine3;

extern HPoint3 *HPt3Create();
extern void HPt3Delete( HPoint3 *pt );

extern void HPt3Print( HPoint3 *pt );
extern void HPt3Copy( HPoint3 *pt1, HPoint3 *pt2 );
extern void HPt3Add( HPoint3 *pt1, HPoint3 *pt2, HPoint3 *pt3 );
extern void HPt3From( HPoint3 *pt, HPt3Coord x, HPt3Coord y, HPt3Coord z, HPt3Coord w );
extern int HPt3From3HPl3s( HPoint3 *pt, HPlane3 *pl1, HPlane3 *pl2, HPlane3 *pl3 );
extern int HPt3From2HLn3s( HPoint3 *pt, HLine3 *ln1, HLine3 *ln2 );

extern int HPt3IntersectHPt3( HPoint3 *pt1, HPoint3 *pt2, HLine3 *ln );
extern void HPt3Pencil( HPt3Coord t1,HPoint3 *pt1,HPt3Coord t2,HPoint3 *pt2, HPoint3 *pt );

extern float HPt3DotHPl3( HPoint3 *pt, HPlane3 *pl );

extern void HPt3Normalize( HPoint3 *pt1, HPoint3 *pt2 );

extern int HPt3Undefined( HPoint3 *pt );
extern int HPt3Infinity( HPoint3 *pt );
extern int HPt3Compare( HPoint3 *pt1, HPoint3  *pt2 );

extern int HPt3CoincidentHPt3( HPoint3 *pt1, HPoint3  *pt2 );
extern int HPt3CoincidentHLn3( HPoint3 *pt, HLine3 *ln );
extern int HPt3CoincidentHPl3( HPoint3 *pt, HPlane3 *pl );

extern void HPt3Transform( Transform3 T, HPoint3 *pt1, HPoint3 *pt2 );
extern void HPt3TransformN( Transform3 T, HPoint3 *pt1, HPoint3 *pt2, int n );
extern HPt3Coord HPt3TransPt3( Transform3 T, HPoint3 *from, Point3 *into );
extern void Pt3ToPt4( Point3 *src, HPoint3 *dst, int npoints );
extern void HPt3ToPt3( HPoint3 *src, Point3 *dst );
extern void HPt3Dehomogenize( HPoint3 *src, HPoint3 *dst );

extern void HPt3Dual( HPoint3 *pt, HPlane3 *pl );

extern void HPt3LinSum( HPt3Coord scale1,HPoint3 *in1,HPt3Coord scale2, HPoint3 *in2, HPoint3  *out );
extern void HPt3SizeOne ( HPoint3 *pt, HPoint3 *out );

extern HPt3Coord HPt3R40Dot(HPoint3 *a, HPoint3 *b);
extern HPt3Coord HPt3R31Dot(HPoint3 *a, HPoint3 *b);
extern HPt3Coord HPt3R30Dot(HPoint3 *a, HPoint3 *b);
extern HPt3Coord HPt3SpaceDot(HPoint3 *a, HPoint3 *b, int space);
extern void HPt3R40Normalize(HPoint3 *a);
extern void HPt3R31Normalize(HPoint3 *a);
extern void HPt3R30Normalize(HPoint3 *a);
extern void HPt3SpaceNormalize(HPoint3 *a, int space);
extern HPt3Coord HPt3HypDistance(HPoint3 *a, HPoint3 *b);
extern HPt3Coord HPt3Distance( HPoint3 *a, HPoint3 *b );
extern HPt3Coord HPt3SphDistance(HPoint3 *a, HPoint3 *b);
extern HPt3Coord HPt3SpaceDistance(HPoint3 *a, HPoint3 *b, int space);
extern void HPt3Sub(HPoint3 *a, HPoint3 *b, HPoint3 *aminusb);
extern void HPt3Scale(HPt3Coord s, HPoint3 *a, HPoint3 *sa);
extern HPt3Coord HPt3HypAngle(HPoint3 *base, HPoint3 *v1, HPoint3 *v2);
extern void HPt3SpaceGramSchmidt(HPoint3 *base, HPoint3 *v, int space);
extern void HPt3SubPt3(HPoint3 *p1, HPoint3 *p2, Point3 *v);
extern void HPt3SphGramSchmidt(HPoint3 *base, HPoint3 *v);
extern void HPt3HypGramSchmidt(HPoint3 *base, HPoint3 *v);
#endif
